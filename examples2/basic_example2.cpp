// #include "driver/engine.hpp"

#include "gflags/gflags.h"
#include "glog/logging.h"

#include "driver/engine.hpp"
#include "worker/kv_client_table.hpp"

#include <algorithm>
#include <numeric>
#include "comm/channel.hpp"
#include "examples/rapid_reassignment/rapid_reassignment.hpp"

DEFINE_int32(my_id, -1, "The process id of this program");
DEFINE_string(config_file, "", "The config file path");
DEFINE_int32(rapid_reassgn_mode, 0, "0:OFF 1:ON");

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

  // 1. Start engine
  Engine engine(my_node, nodes);
  engine.StartEverything();

  // 1.1 Rapid Reassignment setup
  // Retrieve id_mapper and mailbox
  auto* id_mapper = engine.GetIdMapper();
  auto* mailbox = engine.GetMailbox();

  // Create Channel
  const uint32_t num_local_threads = 10;
  const uint32_t num_global_threads = num_local_threads * nodes.size();

  RapidReassignment rr(mailbox, id_mapper, nodes.size(), num_local_threads, num_global_threads, my_node.id);
  rr.SetGroupMode(1);
  rr.StartEverything();
  //rr.SetGroupMode(1);
  //

  // 2. Create tables
  const int kTableId = 0;
  const int kMaxKey = 1000;
  const int kStaleness = 1;
  std::vector<third_party::Range> range;
  for (int i = 0; i < nodes.size() - 1; ++i) {
    range.push_back({kMaxKey / nodes.size() * i, kMaxKey / nodes.size() * (i + 1)});
  }
  range.push_back({kMaxKey / nodes.size() * (nodes.size() - 1), kMaxKey});
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
  task.SetLambda([kTableId, kMaxKey, &rr](const Info& info) {
    LOG(INFO) << "Hi";
    LOG(INFO) << info.DebugString();
    auto table = info.CreateKVClientTable<float>(kTableId);
    std::vector<Key> keys(kMaxKey);
    std::iota(keys.begin(), keys.end(), 0);
    std::vector<float> vals(keys.size(), 0.5);
    std::vector<float> ret;
    srand(time(0));

    // Rapid Reassignment
    auto job_manager = rr.GetJobManager(info.worker_id);
    job_manager->SetDoReassgn(FLAGS_rapid_reassgn_mode);
    // rr.PrintInfoJob(info.worker_id);
    //
    auto start_time = std::chrono::steady_clock::now();
    int cnter = 0;
    for (int i = 0; i < 2; ++i) {
      job_manager->UpdateIteration(i);
      //LOG(INFO) << "wkr: " << info.worker_id << "iter: " << i;
      job_manager->UpdateStartPt(info.worker_id + 600000);
      job_manager->UpdateBatchSize(1000);
      // rr.PrintInfoJob(info.worker_id);
      table->Get(keys, &ret);
      while (!job_manager->IsFinished()) {

        float loading = 100 / 100;
        float delay = 100 / 100;
        std::this_thread::sleep_for(std::chrono::milliseconds(int(loading)));

        if (info.worker_id > 2) {
          std::this_thread::sleep_for(std::chrono::milliseconds(int(delay)));
          // LOG(INFO) << "worker_id: " << info.worker_id << " added delay";
        }

        job_manager->Check();
        cnter++;
      }
      table->Add(keys, vals);
      table->Clock();
      CHECK_EQ(ret.size(), keys.size());
    }

    auto end_time = std::chrono::steady_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    LOG(INFO) << "total time: " << total_time << " ms on worker: " << info.worker_id << " count: " << cnter;
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
