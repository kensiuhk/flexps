#include "examples/rapid_reassignment/rapid_reassignment.hpp"

#include "glog/logging.h"

namespace flexps {

// for debug
void RapidReassignment::SendMsg() {
  std::thread send_thread([this]() {
    LocalChannel* lc = local_channels_[1];
    SArrayBinStream bin;
    bin << lc->GetId() + 1000;
    uint32_t to = (lc->GetId() + 1) % num_global_threads_;
    lc->PushTo(to, bin);
  });
  send_thread.join();
}
//

void RapidReassignment::FindTask(uint32_t from) {
  CHECK_LT(from, num_global_threads_);
  from = id_map_[from];
  //  std::thread send_thread([this, from]() {
  LocalChannel* lc = local_channels_[from % num_local_threads_];
  SArrayBinStream bin;
  flag = kFindTask;
  bin << flag;
  bin << lc->GetId() + 1000;
  for (int i = 0; i < num_nodes_; ++i) {
    // uint32_t to = (from + num_local_threads_ * i) % num_global_threads_;
    // to = id_map_[to];
    uint32_t to = (from + (i + 1) * 1000) % (num_nodes_ * 1000);
    // lc->SendTo(from, to, bin);
    if (to == from)
      continue;
    // Send(from, to, bin, Flag::kFindTask);
    Send(from, to, bin);
    LOG(INFO) << "Send FindTask from: " << from << " to: " << to;
  }
  //  });
  //  send_thread.join();
  // LOG(INFO) << "Send by AskForTask";
}

void RapidReassignment::SendStatus(uint32_t from, uint32_t to) {
  //  std::thread send_thread([this, from, to]() {
  LocalChannel* lc = local_channels_[from % num_local_threads_];
  SArrayBinStream bin;
  flag = kSendStatus;
  bin << flag;
  bin << lc->GetId() + 1000;
  // lc->SendTo(from, to, bin);
  // Send(from, to, bin, Flag::kSendStatus);
  Send(from, to, bin);
  //  });
  //  send_thread.join();
  LOG(INFO) << "Send SendStatus from: " << from << " to: " << to;
  // LOG(INFO) << "Send by SendStatus";
}

void RapidReassignment::SendHelp(uint32_t from, uint32_t to) {
  //  std::thread send_thread([this, from, to]() {
  LocalChannel* lc = local_channels_[from % num_local_threads_];
  SArrayBinStream bin;
  flag = kSendHelp;
  bin << flag;
  bin << lc->GetId() + 1000;

  // Send(from, to, bin, Flag::kSendHelp);
  Send(from, to, bin);
  //  });
  //  send_thread.join();
  LOG(INFO) << "Send SendHelp from: " << from << " to: " << to;
  // LOG(INFO) << "Send by SendHelp";
}

// void RapidReassignment::CalSlowestWorker(){

//}

// void RapidReassignment::Send(uint32_t id_o, uint32_t id_d, const SArrayBinStream& bin, Flag flag) {
// void RapidReassignment::Send(uint32_t tid_o, uint32_t tid_d, const SArrayBinStream& bin, Flag flag) {
void RapidReassignment::Send(uint32_t tid_o, uint32_t tid_d, const SArrayBinStream& bin) {
  /*CHECK_LT(id_o, num_global_threads_);
  CHECK_LT(id_d, num_global_threads_);
  uint32_t tid_o = id_map_[id_o];
  uint32_t tid_d = id_map_[id_d];*/
  Message msg = bin.ToMsg();
  msg.meta.sender = tid_o;
  msg.meta.recver = tid_d;
  msg.meta.model_id = -1;
  msg.meta.flag = Flag::kOther;
  // msg.meta.flag = flag;
  // msg.meta.model_id = -1;

  // mailbox_->Send(msg);
  sender_->GetMessageQueue()->Push(msg);
}

void RapidReassignment::GetLoop(LocalChannel* lc) {
  LOG(INFO) << "get loop start now";
  auto queue_ = lc->GetQueue();
  while (true) {
    if (finished)
      break;
    while (queue_->Size()) {
      // while (lc->GetQueue.Size()) {
      std::vector<SArrayBinStream> rets;
      Message msg;
      queue_->WaitAndPop(&msg);
      SArrayBinStream bin;
      bin.FromMsg(msg);

      // for debug
      // LOG(INFO) << "msg flag: " << FlagName[static_cast<int>(msg.meta.flag)];
      // int res;
      // float res;
      uint32_t res;
      bin >> res;
      LOG(INFO) << "msg sent to: " << msg.meta.recver << " from " << msg.meta.sender << ", id: " << lc->GetId()
                << ", res: " << res;
      // bin >> res;
      LOG(INFO) << "msg sent to: " << msg.meta.recver << " from " << msg.meta.sender << ", id: " << lc->GetId()
                << ", res: " << res;
      //
      flag = (Flag_rr) res;
      switch (flag) {
      case kFindTask:
        LOG(INFO) << "Recved FindTask by: " << msg.meta.recver << " from: " << msg.meta.sender;
        SendStatus(msg.meta.recver, msg.meta.sender);
        break;
      case kSendStatus:
        LOG(INFO) << "Recved SendStatus by: " << msg.meta.recver << " from: " << msg.meta.sender;
        break;
      case kSendHelp:
        LOG(INFO) << "Recved SendHelp by: " << msg.meta.recver << " from: " << msg.meta.sender;
        break;
      }
      /*
            switch (msg.meta.flag) {
            case Flag::kFindTask:
              LOG(INFO) << "Recved FindTask by: " << msg.meta.recver << " from: " << msg.meta.sender;
              SendStatus(msg.meta.recver, msg.meta.sender);
              break;
            case Flag::kSendStatus:
              LOG(INFO) << "Recved SendStatus by: " << msg.meta.recver << " from: " << msg.meta.sender;
              break;
            case Flag::kSendHelp:
              LOG(INFO) << "Recved SendHelp by: " << msg.meta.recver << " from: " << msg.meta.sender;
              break;
            }*/
    }
  }
}

void RapidReassignment::StartGetLoop() {
  sender_.reset(new Sender_rr(mailbox_));
  sender_->Start();
  for (int i = 0; i < local_channels_.size(); ++i) {
    get_threads.push_back(std::thread([this, i] { GetLoop(local_channels_[i]); }));
  }
}

void RapidReassignment::StopGetLoop() {
  sender_->Stop();
  for (auto& thread : get_threads) {
    finished = true;
    thread.join();
  }
}

/*void RapidReassignment::StartGetLoop() {
  for (int i = 0; i < local_channels_.size(); ++i) {
    LocalChannel* lc = local_channels_[i];
    LOG(INFO) << "Start get loop in rr: " << lc->GetId();
    lc->StartGetLoop();
  }
}

void RapidReassignment::StopGetLoop() {
  for (int i = 0; i < local_channels_.size(); ++i) {
    LocalChannel* lc = local_channels_[i];
    LOG(INFO) << "Stop get loop in rr: " << lc->GetId();
    lc->StopGetLoop();
  }
}*/

void RapidReassignment::StopEverything() {
  // StopGetLoop();
  // finished = true;
  // id_mapper_->ReleaseChannelThreads();
}

}  // namespace flexps
