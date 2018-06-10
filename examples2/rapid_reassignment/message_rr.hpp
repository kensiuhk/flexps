#pragma once

#include "base/message.hpp"
#include "base/sarray_binstream.hpp"
#include "examples/rapid_reassignment/job_meta_rr.hpp"
#include "examples/rapid_reassignment/steal_job_rr.hpp"
/*
#define kFindTask 1
#define kSendStatus 2 
#define kSendHelp 3
#define kConfirmHelp 4
#define kCancelHelp 5
#define kTerminateReassgn 6
#define kSendData 7 
*/
namespace flexps {

//enum Flag_rr { kFindTask, kSendStatus, kSendHelp, kConfirmHelp, kCancelHelp, kTerminateReassgn, kSendData };

struct Message_rr {
  // enum Flag_rr { kFindTask, kSendStatus, kSendHelp };
  //Flag_rr flag;

  Message messageBuilder(uint32_t tid_o, uint32_t tid_d, const SArrayBinStream& bin) {
    Message msg = bin.ToMsg();
    msg.meta.sender = tid_o;
    msg.meta.recver = tid_d;
    msg.meta.model_id = -1;
    msg.meta.flag = Flag::kOther;
    return msg;
  }
};

//using DataObj = std::pair<std::vector<std::pair<int, float>>, float>;
// std::pair<std::vector<std::pair<int, float>>, float> DataObj;

namespace {

//using DataObj = std::pair<std::vector<std::pair<int, float>>, float>;
//std::pair<std::vector<std::pair<int, float>>, float> DataObj;
Message FindTask(uint32_t from, uint32_t to) {
  Message_rr m;
  SArrayBinStream bin;
//  m.flag = kFindTask;
//  bin << m.flag;
  uint32_t kFindTask = 1;
  bin << kFindTask;
  //bin << 1000;
  // Send(from, to, bin);
//  if ((from - 54) % 1000 == 0)
//    LOG(INFO) << "Send FindTask from: " << from << " to: " << to;

  return m.messageBuilder(from, to, bin);
}

//Message SendStatus(uint32_t from, uint32_t to, JobMeta job_meta) {
Message SendStatus(uint32_t from, uint32_t to, JobMeta job_meta) {
  Message_rr m;
  Message mm;
  SArrayBinStream bin;
  //m.flag = kSendStatus;
  //bin << m.flag;
 uint32_t kSendStatus = 2;
 bin << kSendStatus;
  bin << job_meta.iter;
  bin << job_meta.start_pt;
  bin << job_meta.batch_size;
  bin << job_meta.current_pt;
if (bin.Size() < 16){
LOG(INFO) << "msg <16";
LOG(INFO) <<job_meta.iter;
LOG(INFO) <<job_meta.start_pt;
LOG(INFO) <<job_meta.batch_size;
LOG(INFO) <<job_meta.current_pt;
}

/*  bin << job_meta->iter;
  bin << job_meta->start_pt;
  bin << job_meta->batch_size;
  bin << job_meta->current_pt;*/
  // Send(from, to, bin);
//  if ((from - 54) % 1000 == 0)
//    LOG(INFO) << "Send SendStatus from: " << from << " to: " << to;
  return m.messageBuilder(from, to, bin);
}

Message SendHelp(uint32_t from, uint32_t to, StolenJob stolen_job) {
  Message_rr m;
  SArrayBinStream bin;
  //m.flag = kSendHelp;
  //bin << m.flag;
  uint32_t kSendHelp = 3;
  bin << kSendHelp;
  //  bin << batch_size;
  bin << stolen_job;

  //  Send(from, to, bin);
//  if ((from - 54) % 1000 == 0)
//    LOG(INFO) << "Send SendHelp from: " << from << " to: " << to;
  return m.messageBuilder(from, to, bin);
}

Message ConfirmHelp(uint32_t from, uint32_t to, StolenJob stolen_job) {
  Message_rr m;
  SArrayBinStream bin;
//  m.flag = kConfirmHelp;
//  bin << m.flag;
  uint32_t kConfirmHelp = 4;
  bin << kConfirmHelp;
  //  bin << batch_size;
  bin << stolen_job;

  //  Send(from, to, bin);
//  if ((from - 54) % 1000 == 0)
//    LOG(INFO) << "Send ConfirmHelp from: " << from << " to: " << to;
  return m.messageBuilder(from, to, bin);
}

Message CancelHelp(uint32_t from, uint32_t to) {
  Message_rr m;
  SArrayBinStream bin;
//  m.flag = kCancelHelp;
//  bin << m.flag;
  uint32_t kCancelHelp = 5;
  bin << kCancelHelp;
  //  bin << batch_size;

  //  Send(from, to, bin);
//  if ((from - 54) % 1000 == 0)
//    LOG(INFO) << "Send CancelHelp from: " << from << " to: " << to;
  return m.messageBuilder(from, to, bin);
}

Message TerminateReassgn(uint32_t from, uint32_t to) {
Message_rr m;
    SArrayBinStream bin;
//    m.flag = kTerminateReassgn;
//    bin << m.flag;
  uint32_t kTerminateReassgn = 6;
  bin << kTerminateReassgn;
    return m.messageBuilder(from, to, bin);
  }

Message SendData(uint32_t from, uint32_t to, std::vector<float> data) {
Message_rr m;
SArrayBinStream bin;
//m.flag = kSendData;
//bin << m.flag;
uint32_t kSendData = 7;
bin << kSendData;
bin << data;
return m.messageBuilder(from, to, bin);
}

}  // namespace
}  // namespace flexps
