#pragma once

#include "base/message.hpp"
#include "comm/channel.hpp"
#include "comm/local_channel.hpp"
#include "comm/mailbox.hpp"
#include "glog/logging.h"

#include "examples/rapid_reassignment/sender_rr.hpp"

namespace flexps {

class Action_rr {
  enum Flag_rr { kFindTask, kSendStatus, kSendHelp };

 public:
  Action_rr(Mailbox* mailbox) {
    // Action_rr(std::unique_ptr<Sender_rr> sender): sender_(sender){
    sender_.reset(new Sender_rr(mailbox));
    sender_->Start();
  };

  // void FindTask(uint32_t from);
  void SendStatus(uint32_t from, uint32_t to);
  void SendHelp(uint32_t from, uint32_t to);

  void Stop();

 private:
  void Send(uint32_t tid_o, uint32_t tid_d, const SArrayBinStream& bin);
  std::unique_ptr<Sender_rr> sender_;
  Flag_rr flag;
};

}  // namespace flexps
