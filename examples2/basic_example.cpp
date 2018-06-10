// #include "driver/engine.hpp"

#include "gflags/gflags.h"
#include "glog/logging.h"

#include "driver/engine.hpp"
#include "worker/kv_client_table.hpp"

#include <algorithm>
#include <numeric>
#include "comm/channel.hpp"
#include "examples/rapid_reassignment/rapid_reassignment.hpp"

#include <gperftools/profiler.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <thread>

#include "base/serialization.hpp"
#include "boost/utility/string_ref.hpp"
#include "io/hdfs_manager.hpp"
#include "lib/libsvm_parser.cpp"

DEFINE_int32(my_id, -1, "The process id of this program");
DEFINE_string(config_file, "", "The config file path");
DEFINE_int32(rapid_reassgn_mode, 0, "0:OFF 1:ON");
DEFINE_int32(rapid_reassgn_group_mode, 0, "0, 1");

DEFINE_string(hdfs_namenode, "", "The hdfs namenode hostname");
DEFINE_string(input, "", "The hdfs input url");
DEFINE_int32(hdfs_namenode_port, -1, "The hdfs namenode port");

DEFINE_uint64(num_dims, 1000, "number of dimensions");
DEFINE_int32(num_iters, 10, "number of iters");

DEFINE_string(kModelType, "", "ASP/SSP/BSP");
DEFINE_string(kStorageType, "", "Map/Vector");
DEFINE_int32(kStaleness, 0, "stalness");
DEFINE_uint32(num_workers_per_node, 1, "num_workers_per_node");
DEFINE_int32(num_servers_per_node, 1, "num_servers_per_node");
DEFINE_int32(batch_size, 100, "batch size of each epoch");
DEFINE_double(alpha, 0.1, "learning rate");
DEFINE_int32(report_interval, 100, "report interval");
DEFINE_int32(learning_rate_decay, 10, "learning rate decay");
DEFINE_string(trainer, "svm", "objective trainer");
DEFINE_double(lambda, 0.1, "lambda");
DEFINE_int32(load_injection, 0, "load injection");
DEFINE_double(threshold, 0.9, "reassignment threshold");

namespace flexps {

void Run() {
  CHECK_NE(FLAGS_my_id, -1);
  CHECK(!FLAGS_config_file.empty());
  VLOG(1) << FLAGS_my_id << " " << FLAGS_config_file;

  // 0. Parse config_file
  std::vector<Node> nodes = ParseFile(FLAGS_config_file);
  CHECK(CheckValidNodeIds(nodes));
  CHECK(CheckUniquePort(nodes));
  CHECK(CheckConsecutiveIds(nodes));
  Node my_node = GetNodeById(nodes, FLAGS_my_id);
  LOG(INFO) << my_node.DebugString();

  // 0.5. Load data
  HDFSManager::Config config;
  config.input = FLAGS_input;
  config.worker_host = my_node.hostname;
  config.worker_port = my_node.port;
  // config.master_port = 19715;
  config.master_port = 19717;
  config.master_host = nodes[0].hostname;
  config.hdfs_namenode = FLAGS_hdfs_namenode;
  config.hdfs_namenode_port = FLAGS_hdfs_namenode_port;
  config.num_local_load_thread = FLAGS_num_workers_per_node;

  // DataObj = <feature<key, val>, label>
  using DataObj = std::pair<std::vector<std::pair<int, float>>, float>;
/*struct feature {
     int fea;
     float val;
   };

   struct DataObj {
     int y;
     std::vector<feature> x;
   };*/

  zmq::context_t* zmq_context = new zmq::context_t(1);
  HDFSManager hdfs_manager(my_node, nodes, config, zmq_context);
  LOG(INFO) << "manager set up";
  hdfs_manager.Start();
  LOG(INFO) << "manager start";

  std::vector<DataObj> data;
  std::mutex mylock;
  hdfs_manager.Run([my_node, &data, &mylock](HDFSManager::InputFormat* input_format, int local_tid) {
    int count = 0;
    DataObj this_obj;
    while (input_format->HasRecord()) {
      auto record = input_format->GetNextRecord();
      if (record.empty())
        return;
      this_obj = libsvm_parser(record);

      mylock.lock();
      data.push_back(std::move(this_obj));
      mylock.unlock();
      count++;
    }
    LOG(INFO) << count << " lines in (node, thread):(" << my_node.id << "," << local_tid << ")";
  });
  LOG(INFO) << "hdfs before stop!";
  hdfs_manager.Stop();
  LOG(INFO) << "Finished loading data!";
  LOG(INFO) << "num of features: " << data[0].first.size();

  // 1. Start engine
  Engine engine(my_node, nodes);
  engine.StartEverything(FLAGS_num_servers_per_node);

  // 1.1 Rapid Reassignment setup
  // Retrieve id_mapper and mailbox
  auto* id_mapper = engine.GetIdMapper();
  auto* mailbox = engine.GetMailbox();

  // Create Channel
  const uint32_t num_local_threads = FLAGS_num_workers_per_node;
  const uint32_t num_global_threads = num_local_threads * nodes.size();

  RapidReassignment rr(mailbox, id_mapper, nodes.size(), num_local_threads, num_global_threads, my_node.id);
  rr.SetGroupMode(FLAGS_rapid_reassgn_group_mode);
  //rr.SetGroupMode(1);
  rr.StartEverything();
//  rr.SetStealThreshold( 0.9*FLAGS_batch_size ); 
  rr.SetStealThreshold( FLAGS_threshold*FLAGS_batch_size ); 
  //

  // 2. Create tables
  const int kTableId = 0;
  // const int kMaxKey = 1000;
  int num_total_servers = nodes.size() * FLAGS_num_servers_per_node;
  uint64_t num_features = FLAGS_num_dims + 1;
  const int kStaleness = FLAGS_kStaleness;
  std::vector<third_party::Range> range;
  /*  for (int i = 0; i < nodes.size() - 1; ++i) {
      range.push_back({kMaxKey / nodes.size() * i, kMaxKey / nodes.size() * (i + 1)});
    }
    range.push_back({kMaxKey / nodes.size() * (nodes.size() - 1), kMaxKey});*/
  for (int i = 0; i < num_total_servers - 1; ++i) {
    range.push_back({num_features / num_total_servers * i, num_features / num_total_servers * (i + 1)});
  }
  range.push_back({num_features / num_total_servers * (num_total_servers - 1), num_features});
  engine.CreateTable<float>(kTableId, range, ModelType::SSP, StorageType::Map, kStaleness);
  engine.Barrier();

  // 3. Construct tasks
  MLTask task;
  std::vector<WorkerAlloc> worker_alloc;
  for (auto& node : nodes) {
    // worker_alloc.push_back({node.id, 5});  // each node has 10 workers
    worker_alloc.push_back({node.id, num_local_threads});  // each node has 10 workers
  }
  task.SetWorkerAlloc(worker_alloc);
  task.SetTables({kTableId});  // Use table 0
  // task.SetLambda([kTableId, kMaxKey, local_channels, num_global_threads, &rr](const Info& info) {
  task.SetLambda([kTableId, &rr, &data](const Info& info) {
    int num_iters = FLAGS_num_iters;
    int num_features = FLAGS_num_dims;
    int num_params = num_features + 1;
    float alpha = FLAGS_alpha;

    LOG(INFO) << "Hi";
    LOG(INFO) << "Size of data: " << data.size();
    LOG(INFO) << info.DebugString();
    auto table = info.CreateKVClientTable<float>(kTableId);
    // std::vector<Key> keys(kMaxKey);
    // std::iota(keys.begin(), keys.end(), 0);
    // std::vector<float> vals(keys.size(), 0.5);
    // std::vector<float> ret;
    std::vector<Key> keys(num_params);
    std::iota(keys.begin(), keys.end(), 0);
    std::vector<float> params(keys.size(), 0);

    srand(time(0));

    // Rapid Reassignment
    auto job_manager = rr.GetJobManager(info.worker_id);
    job_manager->SetDoReassgn(FLAGS_rapid_reassgn_mode);
    // rr.PrintInfoJob(info.worker_id);
    //
    auto start_time = std::chrono::steady_clock::now();
    int cnter = 0;
    for (int iter = 0; iter < FLAGS_num_iters; ++iter) {
      job_manager->UpdateIteration(iter);
      int startpt = rand() % (data.size() - 2*FLAGS_batch_size - 1);
      //if (data.size() - startpt < FLAGS_batch_size)
        //startpt = startpt - FLAGS_batch_size;
      //LOG(INFO) << " start pt: " << startpt;
      job_manager->UpdateStartPt(startpt);
      job_manager->UpdateBatchSize(FLAGS_batch_size);

      //auto start_time2 = std::chrono::steady_clock::now();
      auto start_timeG = std::chrono::steady_clock::now();
      auto start_time2 = std::chrono::steady_clock::now();
      std::vector<float> step_sum(num_params, 0);
      table->Get(keys, &params);
      auto end_timeG = std::chrono::steady_clock::now();
      auto total_timeG = std::chrono::duration_cast<std::chrono::milliseconds>(end_timeG - start_timeG).count();
//      LOG(INFO) << iter  <<"Get total time: " << total_timeG << " ms on worker: " << info.worker_id << " count: " << cnter;
      int s = startpt;
//      auto start_time2 = std::chrono::steady_clock::now();
      while (!job_manager->IsFinished()) {
        //float loading = FLAGS_load_injection / 100;
        //float delay = 1000 / 100;
        float delay = FLAGS_load_injection;
        //std::this_thread::sleep_for(std::chrono::milliseconds(int(loading)));

        // main task
        // calculate accumulated gradient
        auto& x = data[s].first;
        auto y = data[s].second;
        if (y < 0)
          y = 0;
        float pred_y = 0.0;
        for (auto field : x) {
          pred_y += params[field.first] * field.second;
        }
        pred_y += params[num_params - 1];  // intercept
        pred_y = 1. / (1. + exp(-1 * pred_y));

        for (auto field : x) {
          step_sum[field.first] += alpha * field.second * (y - pred_y);
        }
        step_sum[num_params - 1] += alpha * (y - pred_y);  // intercept
        s++;
        //
/*         int n = info.worker_id +1;
                if (s % n == 0 ){
		   std::this_thread::sleep_for(std::chrono::milliseconds(FLAGS_load_injection));
        }
*/
/*        if(info.worker_id == 0
           ||info.worker_id ==10
           ||info.worker_id ==20
           ||info.worker_id ==30
           ||info.worker_id ==40)
          std::this_thread::sleep_for(std::chrono::milliseconds(FLAGS_load_injection));
if(info.worker_id == 4
            ||info.worker_id ==14
            ||info.worker_id ==24
            ||info.worker_id ==34
            ||info.worker_id ==44)
           std::this_thread::sleep_for(std::chrono::milliseconds(FLAGS_load_injection));
*/                //if (info.worker_id > 2) {
/*                if (info.thread_id > 99 && info.thread_id < 200){
                  //std::this_thread::sleep_for(std::chrono::milliseconds(int(delay)));
                  std::this_thread::sleep_for(std::chrono::milliseconds(FLAGS_load_injection));
                  // LOG(INFO) << "worker_id: " << info.worker_id << " added delay";
                }
 */       
        job_manager->Check();
        cnter++;
      }
      auto end_time2 = std::chrono::steady_clock::now();
     auto total_time2 = std::chrono::duration_cast<std::chrono::milliseconds>(end_time2 - start_time2).count();
  //   LOG(INFO) << iter  <<"total time: " << total_time2 << " ms on worker: " << info.worker_id << " count: " << cnter;

      // update params
      for (int j = 0; j < num_params; j++) {
        step_sum[j] /= float(FLAGS_batch_size);
      }
      auto start_timeA = std::chrono::steady_clock::now();
      table->Add(keys, step_sum);
      table->Clock();
      auto end_timeA = std::chrono::steady_clock::now();
      auto total_timeA = std::chrono::duration_cast<std::chrono::milliseconds>(end_timeA - start_timeA).count();
      //LOG(INFO) << iter  <<" Add total time: " << total_timeA << " ms on worker: " << info.worker_id << " count: " << cnter;

      CHECK_EQ(params.size(), keys.size());
      /*if (iter % FLAGS_report_interval == 0) {
        if (info.worker_id == 0) {
          for (int k = 0; k < 10; ++k){
            LOG(INFO) << params[k];}
          int count = 0;
          float c_count = 0;  // correct count
          std::vector<DataObj> training_data(&data[0], &data[20000]);
          for (auto& data_ : training_data) {
            count = count + 1;
            auto x = data_.first;
            auto y = data_.second;
            if (y < 0)
              y = 0;
            float pred_y = 0.0;

            for (auto field : x) {
              pred_y += params[field.first] * field.second;
            }
            pred_y += params[num_params - 1];
            pred_y = 1. / (1. + exp(-pred_y));
            pred_y = (pred_y > 0.5) ? 1 : 0;
            if (int(pred_y) == int(y)) {
              c_count += 1;
            }
          }
          LOG(INFO) << iter << ":accuracy is " << c_count / count << " count is :" << count
                    << " c_count is:" << c_count;
        }
      }*/
    }
    //job_manager->Terminate();
    auto end_time = std::chrono::steady_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    LOG(INFO) << "total time: " << total_time << " ms on worker: " << info.worker_id << " count: " << cnter;
//    while(true){};
  });

  // 4. Run tasks
  engine.Run(task);

  // 4.1 Stop Rapid Reassignment
  // rr.GetLoop(local_channels[1]);
  // rr.StopGetLoop();
  rr.StopEverything();
  id_mapper->ReleaseChannelThreads();
  //

  // 5. Stop engine
  engine.StopEverything();
}

}  // namespace flexps

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  flexps::Run();
}
