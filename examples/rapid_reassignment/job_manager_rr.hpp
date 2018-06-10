#pragma once

#include <cstdint>
#include <mutex>
#include "comm/mailbox.hpp"
#include "examples/rapid_reassignment/job_meta_rr.hpp"
#include "examples/rapid_reassignment/message_rr.hpp"

namespace flexps {

class JobManager {
 public:
  JobManager(ThreadsafeQueue<Message>* send_queue, uint32_t my_id, std::vector<uint32_t> my_group_members)
      : send_queue_(send_queue), my_id_(my_id), my_group_members_(my_group_members){};

  void UpdateIteration(uint32_t iter_new);
  void UpdateStartPt(uint32_t start_pt_new);
  void UpdateBatchSize(uint32_t batch_size_new);
  bool IsFinished();
  void Check();

  //JobMeta GetJobMeta();
  JobMeta GetJobMeta();
  uint32_t GetDoReassgn();
  void SetWait(bool isWait);
  void SetDoReassgn(bool isDoReassgn);
  void Terminate();

  void Stop();

 private:
  //  std::shared_ptr<Action_rr> action_;
  ThreadsafeQueue<Message>* send_queue_;
  std::vector<uint32_t> my_group_members_;
  JobMeta job_meta;
  uint32_t my_id_;
  bool wait;
  bool doReassgn = false;
};

}  // namespace flexps
