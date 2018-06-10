#pragma once

#include "comm/mailbox.hpp"
#include "driver/simple_id_mapper.hpp"

#include "base/message.hpp"
#include "comm/channel.hpp"
#include "comm/local_channel.hpp"

#include "examples/rapid_reassignment/job_manager_rr.hpp"
#include "examples/rapid_reassignment/message_rr.hpp"
#include "examples/rapid_reassignment/sender_rr.hpp"
#include "examples/rapid_reassignment/status_box_rr.hpp"
#include "examples/rapid_reassignment/status_checker_rr.hpp"
#include "examples/rapid_reassignment/steal_job_rr.hpp"
#include "examples/rapid_reassignment/worker_group_rr.hpp"

namespace flexps {

// enum class Flag : char { kExit, kBarrier, kResetWorkerInModel, kClock, kAdd, kAddChunk, kGet, kGetChunk, kOther };
// enum class rrFlag : Flag { kAsk, kStatus, kHelp, kAck, kOther};

class RapidReassignment {
  //  enum Flag_rr { kFindTask, kSendStatus, kSendHelp };

 public:
  // RapidReassignment(Mailbox* mailbox, SimpleIdMapper* id_mapper, int32_t num_nodes, uint32_t num_local_threads)
  //  : mailbox_(mailbox), id_mapper_(id_mapper), num_nodes_(num_nodes), num_local_threads_(num_local_threads){};
  RapidReassignment(Mailbox* mailbox, SimpleIdMapper* id_mapper, int32_t num_nodes, uint32_t num_local_threads,
                    uint32_t num_global_threads, uint32_t node_id)
      : mailbox_(mailbox),
        id_mapper_(id_mapper),
        num_nodes_(num_nodes),
        num_local_threads_(num_local_threads),
        num_global_threads_(num_global_threads),
        node_id_(node_id){};
  /*RapidReassignment(std::vector<LocalChannel*> local_channels, Mailbox* mailbox,
                    std::unordered_map<uint32_t, uint32_t> id_map, int32_t num_nodes, uint32_t num_local_threads)
      : local_channels_(local_channels),
        mailbox_(mailbox),
        id_map_(id_map),
        num_nodes_(num_nodes),
        num_local_threads_(num_local_threads) {
    LOG(INFO) << "Start Rapid Reassignment";
    num_global_threads_ = num_nodes_ * num_local_threads_;
  }*/

  void SendMsg();
  //  void FindTask(uint32_t from);
  //  void SendStatus(uint32_t from, uint32_t to);
  //  void SendHelp(uint32_t from, uint32_t to);
  //  void Send(uint32_t tid_o, uint32_t tid_d, const SArrayBinStream& bin);
  JobManager* GetJobManager(uint32_t worker_id);

  void StartEverything();
  void GetLoop(LocalChannel* lc);
  void SetStealThreshold(float threshold);
  void SetGroupMode(uint32_t mode);
  //  void StartGetLoop();
  //  void StopGetLoop();
  void StopEverything();
  void PrintInfo();
  void PrintInfoJob(uint32_t worker_id);

  void DistData(std::vector<float> data);


 private:
  Mailbox* const mailbox_;
  SimpleIdMapper* const id_mapper_;
  std::unordered_map<uint32_t, uint32_t> id_map_;
  std::unique_ptr<Channel> channel_;
  std::unique_ptr<Sender_rr> sender_;
  std::vector<LocalChannel*> local_channels_;
  std::vector<std::thread> get_threads;
  //  Flag_rr flag;
  Message_rr m;
  uint32_t num_nodes_;
  uint32_t num_local_threads_;
  uint32_t num_global_threads_;
  uint32_t node_id_;
  //  bool stop_get_loop = false;
  bool finished = false;
  std::vector<std::vector<uint32_t>> group_member_;
  std::vector<JobManager*> job_managers;
  ThreadsafeQueue<Message>* send_queue_;
  std::vector<StatusBox*> status_box_;
  float steal_threshold_ = 0;
  uint32_t group_mode_ = 0;
  std::vector<std::vector<float>> val_values;

  void SendMsg(Message msg);
};

}  // namespace flexps
