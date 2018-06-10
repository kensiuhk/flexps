#include "examples/rapid_reassignment/job_manager_rr.hpp"

#include "glog/logging.h"

namespace flexps {

JobMeta JobManager::GetJobMeta() { return job_meta; }
uint32_t JobManager::GetDoReassgn() { return doReassgn; }
void JobManager::SetWait(bool isWait) { wait = isWait; }
void JobManager::SetDoReassgn(bool isDoReassgn) { 
doReassgn = isDoReassgn; 
wait = false;
}

void JobManager::UpdateIteration(uint32_t iter_new) { job_meta.iter = iter_new; }

void JobManager::UpdateStartPt(uint32_t start_pt_new) {
  std::mutex mtx;
  mtx.lock();
  job_meta.start_pt = start_pt_new;
  job_meta.current_pt = start_pt_new;
  mtx.unlock();
  // LOG(INFO) << "tid: " << my_id_ << " new start pt: " << start_pt_new;
}

void JobManager::UpdateBatchSize(uint32_t batch_size_new) {
  std::mutex mtx;
  mtx.lock();
  job_meta.batch_size = batch_size_new;
  mtx.unlock();
  // LOG(INFO) << "tid: " << my_id_ << " batch size: " << batch_size_new;
}

bool JobManager::IsFinished() {
  if (job_meta.current_pt == job_meta.start_pt + job_meta.batch_size) {
    return true;
  } else
    return false;
}

void JobManager::Check() {
  job_meta.current_pt++;

  if (IsFinished()) {
    if (doReassgn) {
      for (int i = 0; i < my_group_members_.size(); ++i) {
        Message msg = FindTask(my_id_, my_group_members_[i]);
        std::mutex mtx;
        mtx.lock();
        send_queue_->Push(msg);
        mtx.unlock();
      }
      // LOG(INFO) << "current point: " << job_meta.current_pt << " batch size: " << job_meta.batch_size;

      wait = true;
      while (wait) {
      }
    }
  }
}


void JobManager::Terminate() {
//wait = true;
//      while (wait) {
//    }
if (doReassgn) {
for (int i = 0; i < my_group_members_.size(); ++i) {
//Message msg = TerminateReassgn(my_id_, my_id_);
    Message msg = TerminateReassgn(my_id_, my_group_members_[i]);
         std::mutex mtx;
         mtx.lock();
         send_queue_->Push(msg);
         mtx.unlock();
}
}
}

void JobManager::Stop() {}

}  // namespace flexps
