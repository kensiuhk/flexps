#pragma once

#include "comm/mailbox.hpp"
#include "driver/simple_id_mapper.hpp"

#include "base/message.hpp"
#include "comm/channel.hpp"
#include "comm/local_channel.hpp"

#include "examples/rapid_reassignment/sender_rr.hpp"

namespace flexps {

// enum class Flag : char { kExit, kBarrier, kResetWorkerInModel, kClock, kAdd, kAddChunk, kGet, kGetChunk, kOther };
// enum class rrFlag : Flag { kAsk, kStatus, kHelp, kAck, kOther};

class RapidReassignment {
  enum Flag_rr { kFindTask, kSendStatus, kSendHelp };

 public:
  // RapidReassignment(SimpleIdMapper* id_mapper, Mailbox* mailbox):id_mapper_(id_mapper),mailbox_(mailbox){LOG(INFO) <<
  // "wahahaha";}
  RapidReassignment(std::vector<LocalChannel*> local_channels, Mailbox* mailbox,
                    std::unordered_map<uint32_t, uint32_t> id_map, int32_t num_nodes, uint32_t num_local_threads)
      : local_channels_(local_channels),
        mailbox_(mailbox),
        id_map_(id_map),
        num_nodes_(num_nodes),
        num_local_threads_(num_local_threads) {
    LOG(INFO) << "Start Rapid Reassignment";
    // sender_ = Sender_rr(&mailbox);
    num_global_threads_ = num_nodes_ * num_local_threads_;
  }

  void StartEverything();
  void SendMsg();
  void FindTask(uint32_t from);
  void SendStatus(uint32_t from, uint32_t to);
  void SendHelp(uint32_t from, uint32_t to);
  // void Send(uint32_t id_o, uint32_t id_d, const SArrayBinStream& bin, Flag flag);
  // void Send(uint32_t tid_o, uint32_t tid_d, const SArrayBinStream& bin, Flag flag);
  void Send(uint32_t tid_o, uint32_t tid_d, const SArrayBinStream& bin);
  void GetLoop(LocalChannel* lc);
  void StartGetLoop();
  void StopGetLoop();
  void StopEverything();

 private:
  // SimpleIdMapper* id_mapper_;
  // Mailbox* mailbox_;
  Mailbox* const mailbox_;
  std::unordered_map<uint32_t, uint32_t> id_map_;
  // Sender_rr* sender_;
  std::unique_ptr<Sender_rr> sender_;
  // Channel channels_;
  std::vector<LocalChannel*> local_channels_;
  // std::vector<std::unique_ptr<LocalChannel>> local_channels_;
  std::vector<std::thread> get_threads;
  // std::thread get_threads;
  Flag_rr flag;
  uint32_t num_nodes_;
  uint32_t num_local_threads_;
  uint32_t num_global_threads_;
  bool stop_get_loop = false;
  bool finished = false;
};

}  // namespace flexps
