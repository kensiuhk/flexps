// #include "driver/engine.hpp"

#include "gflags/gflags.h"
#include "glog/logging.h"

#include "driver/engine.hpp"
#include "driver/node_parser.hpp"
#include "worker/kv_client_table.hpp"

#include <math.h>
#include <algorithm>
#include <cstdlib>
#include <numeric>

#include <stdio.h>
#include <fstream>
#include <string>

DEFINE_int32(my_id, -1, "The process id of this program");
DEFINE_string(config_file, "", "The config file path");

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

  // read training data
  LOG(INFO) << "AHAHAHA|";
  std::ifstream input("/data/opt/tmp/yuzhen/datasets/a9.txt");
  //std::ifstream input("/data/opt/tmp/kensiu/dataset/webspam");
  // std::string line;
  // for(std::string line; getline(input,line);){
  // LOG(INFO)<<line;}

  struct feature {
    int fea;
    float val;
  };

  struct idata {
    int y;
    std::vector<feature> x;
  };
  std::vector<idata> data_store;
  std::vector<std::string> line_a;
  for (std::string line; getline(input, line);) {
    line_a.push_back(line);
    char* pos;
    std::unique_ptr<char> chunk_ptr(new char[line.size() + 1]);
    strncpy(chunk_ptr.get(), line.c_str(), line.size());
    chunk_ptr.get()[line.size()] = '\0';
    char* tok = strtok_r(chunk_ptr.get(), " \t:", &pos);
    int i = -1;
    idata mdata;
    feature feat;
    while (tok != NULL) {
      if (i == 0) {
        feat.fea = std::atoi(tok) - 1;
        i = 1;
      } else if (i == 1) {
        feat.val = std::atof(tok);
        mdata.x.push_back(feat);
        i = 0;
      } else {
        mdata.y = std::atof(tok);
        i = 0;
      }
      // Next key/value pair
      tok = strtok_r(NULL, " \t:", &pos);
    }
    data_store.push_back(mdata);
  }
  LOG(INFO) << "num feas: " << data_store[0].x.size();
  LOG(INFO) << "num feas: " << data_store[10].x.size();
  LOG(INFO) << "num feas: " << data_store[55].x.size();
  LOG(INFO) << "num feas: " << data_store[3000].x.size();
  LOG(INFO) << "num feas: " << data_store[6000].x.size();
  LOG(INFO) << "num feas: " << data_store[10000].x.size();
  // LOG(INFO)<<"nth data";
  // LOG(INFO)<<line_a[30020];
  // LOG(INFO)<<data_m[30020].x[6].idx;
  // LOG(INFO)<<data_m.at(30020).x.at(6).val;
  // LOG(INFO)<<data_m.size();
  // LOG(INFO)<<"hahahaa";
  //

  // int num_iters = 1000;
  // int num_features = 123;
  // int num_params = num_features + 1;
  uint32_t num_workers = 10;
  float alpha = 0.25;
  //

  // 3. Construct tasks
  MLTask task;
  std::vector<WorkerAlloc> worker_alloc;
  for (auto& node : nodes) {
    worker_alloc.push_back({node.id, num_workers});  // each node has 10 workers
  }
  task.SetWorkerAlloc(worker_alloc);
  task.SetTables({kTableId});  // Use table 0
  task.SetLambda([kTableId, kMaxKey, data_store, num_workers](const Info& info) {
    int num_iters = 4000;
    int num_features = 123;
    //int num_features = 14;
    int num_params = num_features + 1;
    float alpha = 0.25;
    LOG(INFO) << "Hi";
    LOG(INFO) << info.DebugString();
    auto table = info.CreateKVClientTable<float>(kTableId);
    std::vector<Key> keys(num_params, 1);
    std::iota(keys.begin(), keys.end(), 0);
    // std::vector<float> vals(keys.size(), 0.5);
    std::vector<float> params;
    int first = data_store.size() / num_workers * info.worker_id;
    int last = data_store.size() / num_workers * (info.worker_id + 1) - 1;
    LOG(INFO) << first;
    LOG(INFO) << last;
    std::vector<idata> training_data(&data_store[first], &data_store[last]);
    // std::vector
    for (int iter = 0; iter < num_iters; ++iter) {
      // table.Get(keys, &params);
      table->Get(keys, &params);
      // A full batch gradient descent
      // param += alpha * (y[i] - h(i)) * x
      std::vector<float> step_sum(num_params, 0);
      // calculate accumulated gradient
      for (auto& data : training_data) {
        auto& x = data.x;
        auto y = data.y;
        if (y < 0)
          y = 0;
        float pred_y = 0.0;
        for (auto field : x) {
          pred_y += params[field.fea] * field.val;
        }
        pred_y += params[num_params - 1];  // intercept
        pred_y = 1. / (1. + exp(-1 * pred_y));

        for (auto field : x) {
          step_sum[field.fea] += alpha * field.val * (y - pred_y);
        }
        step_sum[num_params - 1] += alpha * (y - pred_y);  // intercept
      }

      int count = 0;
      float c_count = 0;  // correct count
      for (auto& data : training_data) {
        count = count + 1;
        auto x = data.x;
        auto y = data.y;
        if (y < 0)
          y = 0;
        float pred_y = 0.0;

        for (auto field : x) {
          pred_y += params[field.fea] * field.val;
        }
        pred_y += params[num_params - 1];
        pred_y = 1. / (1. + exp(-pred_y));
        pred_y = (pred_y > 0.5) ? 1 : 0;
        if (int(pred_y) == int(y)) {
          c_count += 1;
        }
      }
      if (iter % 500 == 0)
        LOG(INFO) << iter << ":accuracy is " << c_count / count << " count is :" << count << " c_count is:" << c_count;
      // update params
      for (int j = 0; j < num_params; j++) {
        step_sum[j] /= float(count);
      }
      // table.Add(keys, step_sum);
      table->Add(keys, step_sum);

      // table.Clock();
      table->Clock();
      CHECK_EQ(params.size(), keys.size());
      // LOG(INFO) << ret[0];
      // LOG(INFO) << keys.size();
      // LOG(INFO) << params[0];
      // LOG(INFO) << "Iter: " << i << " finished on Node " << info.worker_id;
    }
  });

  // 4. Run tasks
  engine.Run(task);

  // 5. Stop engine
  engine.StopEverything();
}

}  // namespace flexps

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  flexps::Run();
}
