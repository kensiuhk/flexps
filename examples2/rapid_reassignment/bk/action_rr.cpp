#include "glog/logging.h"

#include "examples/rapid_reassignment/action_rr.hpp"

namespace flexps {

/*void Action_rr::FindTask(uint32_t from) {
  LOG(INFO) << "FindTask from, node:  " << node_id_;
  CHECK_LT(from, num_global_threads_);
  uint32_t local_id = from % num_nodes_;
  from = id_map_[from];
  LocalChannel* lc = local_channels_[from % num_local_threads_];
  SArrayBinStream bin;
  flag = kFindTask;
  bin << flag;
  bin << lc->GetId() + 1000;
  for (int i = 0; i < group_member_[local_id].size(); ++i) {
    uint32_t to = group_member_[local_id][i];
    Send(from, to, bin);
    LOG(INFO) << "Send FindTask from: " << from << " to: " << to;
  }
}*/

void Action_rr::SendStatus(uint32_t from, uint32_t to) {
  SArrayBinStream bin;
  flag = kSendStatus;
  bin << flag;
  bin << 1000;
  Send(from, to, bin);
  LOG(INFO) << "Send SendStatus from: " << from << " to: " << to;
}

void Action_rr::SendHelp(uint32_t from, uint32_t to) {
  SArrayBinStream bin;
  flag = kSendHelp;
  bin << flag;
  bin << 1000;

  Send(from, to, bin);
  LOG(INFO) << "Send SendHelp from: " << from << " to: " << to;
}

void Action_rr::Send(uint32_t tid_o, uint32_t tid_d, const SArrayBinStream& bin) {
  Message msg = bin.ToMsg();
  msg.meta.sender = tid_o;
  msg.meta.recver = tid_d;
  msg.meta.model_id = -1;
  msg.meta.flag = Flag::kOther;
  sender_->GetMessageQueue()->Push(msg);
}

void Action_rr::Stop() { sender_->Stop(); }

}  // namespace flexps
