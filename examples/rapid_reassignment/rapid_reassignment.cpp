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

void RapidReassignment::PrintInfo() {
  LOG(INFO) << "nodes: " << num_nodes_ << ",local: " << num_local_threads_ << ",global: " << num_global_threads_
            << ",node id: " << node_id_;
  for (int i = 0; i < local_channels_.size(); ++i) {
    LOG(INFO) << "Local ID: " << local_channels_[i]->GetId() << " " << id_map_[local_channels_[i]->GetId()];
  }

  for (int i = 0; i < group_member_.size(); ++i)
    for (int j = 0; j < group_member_[i].size(); ++j)
      LOG(INFO) << "node_id: " << node_id_ << " " << i << " mem: " << group_member_[i][j];
}

void RapidReassignment::PrintInfoJob(uint32_t worker_id) {
  LOG(INFO) << "woker: " << worker_id
            << " startpt:" << job_managers[worker_id % num_local_threads_]->GetJobMeta().start_pt
            << " batch size:" << job_managers[worker_id % num_local_threads_]->GetJobMeta().batch_size;

  for (int i = 0; i < status_box_.size(); ++i) {
    LOG(INFO) << "status box: " << i << "size: " << status_box_[i]->GetSize();
  }
}
//

void RapidReassignment::DistData(std::vector<float> data){
  std::mutex mtx;
  mtx.lock();
  Message msg;
  //std::vector<DataObj> data;
  //std::vector<std::pair<std::vector<std::pair<int, float>>, float>> data;
  msg = SendData(50, 1050, data);
  send_queue_->Push(msg);

  mtx.unlock();
}

void RapidReassignment::SetStealThreshold(float threshold){
   steal_threshold_ = threshold;
}

void RapidReassignment::SetGroupMode(uint32_t mode){
  group_mode_ = mode;
}

void RapidReassignment::GetLoop(LocalChannel* lc) {
  LOG(INFO) << "get loop start now";
  auto queue_ = lc->GetQueue();
  while (true) {
    if (finished)
      break;
    while (queue_->Size()) {
      std::vector<SArrayBinStream> rets;
      Message msg;
      queue_->WaitAndPop(&msg);
      SArrayBinStream bin;
      bin.FromMsg(msg);

      uint32_t res;
      bin >> res;

//      m.flag = (Flag_rr) res;
      //switch (m.flag) {
      switch (res) {
      //case kSendData:{
      case 7:{
        LOG(INFO) << "Recved SendData by: " << msg.meta.recver << " from: " << msg.meta.sender
                     << ", id: " << lc->GetId();
       std::vector<float> ha;
       val_values.push_back(ha);
       bin >> ha;
       LOG(INFO) << val_values.size();
       LOG(INFO) << ha.size();
       //if (val_values.size() == 67829)
         //LOG(INFO) << val_values[67827][0];
         //LOG(INFO) << val_values[67827].size();
       //LOG(INFO) << ha[0][5];
       break;
      }
      //case kFindTask: {
      case 1: {
        /*if ((msg.meta.recver - 50) % 1000 == 0)
          LOG(INFO) << "Recved FindTask by: " << msg.meta.recver << " from: " << msg.meta.sender
                    << ", id: " << lc->GetId();*/
 //       LOG(INFO) << "Find task";
        int local_id = (msg.meta.recver - 50) % 1000;
        if (job_managers[local_id]->GetDoReassgn() == 0)
          break;
        SendMsg(SendStatus(msg.meta.recver, msg.meta.sender, job_managers[local_id]->GetJobMeta()));
        // SendMsg(SendStatus(msg.meta.recver, msg.meta.sender));
        break;
      }
      //case kSendStatus: {
      case 2: {
        int local_id = (msg.meta.recver - 50) % 1000;
//        if (job_managers[local_id]->GetDoReassgn() == 0)
//           break;
        /*if ((msg.meta.recver - 50) % 1000 == 0)
          LOG(INFO) << "Recved SendStatus by: " << msg.meta.recver << " from: " << msg.meta.sender;*/
//        LOG(INFO) << "Send Status";
        //LOG(INFO) << "Size of bin: " << bin.Size();
 //       if (bin.Size() < 16){
//SendMsg(TerminateReassgn(msg.meta.recver, msg.meta.sender));
// break;}
        JobMeta job_meta;
        bin >> job_meta.iter;
        //LOG(INFO) << "Send Status2";
        bin >> job_meta.start_pt;
        //LOG(INFO) << "Send Status3";
        bin >> job_meta.batch_size;
        //LOG(INFO) << "Send Status4";
        bin >> job_meta.current_pt;
        //LOG(INFO) << "Send Status5";
        job_meta.thread_id = msg.meta.sender;
        status_box_[local_id]->AddStatus(job_meta);

        if (status_box_[local_id]->IsFull()) {
          //if (IsInSameIteration(job_managers[local_id]->GetJobMeta(), status_box_[local_id]->GetJobMetas())) {
            auto slowest_worker = CalSlowestWorker(status_box_[local_id]->GetJobMetas());
          if (IsInSameIteration(job_managers[local_id]->GetJobMeta(), slowest_worker)){
            StolenJob stolen_job;
            auto toSteal = StealJob(slowest_worker, 0.5, stolen_job, steal_threshold_);
            if (toSteal) {
              //LOG(INFO) << "Stolen job: " << stolen_job.helper_batch_size_new << " " << stolen_job.helper_start_pt_new
                        //<< " " << stolen_job.helpee_batch_size_new;
              // job_managers[local_id]->UpdateStartPt(stolen_job.helper_start_pt_new);
              // job_managers[local_id]->UpdateBatchSize(stolen_job.helper_batch_size_new);
              SendMsg(SendHelp(msg.meta.recver, slowest_worker.thread_id, stolen_job));
            } else
              job_managers[local_id]->SetWait(false);
            // SendMsg(SendHelp(msg.meta.recver, slowest_worker.thread_id, stolen_job.helpee_batch_size_new));
          } else
            job_managers[local_id]->SetWait(false);
          // job_managers[local_id]->SetWait(false);
          status_box_[local_id]->Clear();
        }

        break;
      }
      //case kSendHelp: {
      case 3: {
//        LOG(INFO) << "Send Help";
        if (bin.Size() < 4) break;
        int local_id = (msg.meta.recver - 50) % 1000;
        if (job_managers[local_id]->GetDoReassgn() == 0)
           break;
        //int local_id = (msg.meta.recver - 50) % 1000;
        /*if ((msg.meta.recver - 50) % 1000 == 0)
          LOG(INFO) << "Recved SendHelp by: " << msg.meta.recver << " from: " << msg.meta.sender;*/
        // uint32_t batch_size_new;
        // bin >> batch_size_new;
        StolenJob stolen_job;
        bin >> stolen_job;
        JobMeta job_meta = job_managers[local_id]->GetJobMeta();
        if (job_meta.current_pt < job_meta.start_pt + stolen_job.helpee_batch_size_new &&
            job_meta.start_pt + job_meta.batch_size ==
                stolen_job.helper_start_pt_new + stolen_job.helper_batch_size_new) {
          job_managers[local_id]->UpdateBatchSize(stolen_job.helpee_batch_size_new);
          SendMsg(ConfirmHelp(msg.meta.recver, msg.meta.sender, stolen_job));
        } else {
          //LOG(INFO) << job_meta.current_pt<<" "<<job_meta.start_pt + stolen_job.helpee_batch_size_new<<" "<<job_meta.start_pt + job_meta.batch_size<<" "<<stolen_job.helper_start_pt_new + stolen_job.helper_batch_size_new;
          SendMsg(CancelHelp(msg.meta.recver, msg.meta.sender));
        }
        break;
      }
      //case kConfirmHelp: {
      case 4: {
         
//        LOG(INFO) << "Confirm Help receved";
        if (bin.Size() < 4) break;
        int local_id = (msg.meta.recver - 50) % 1000;
     //int local_id = (msg.meta.recver - 50) % 1000;
         if (job_managers[local_id]->GetDoReassgn() == 0)
            break;

        StolenJob stolen_job;
        bin >> stolen_job;
        job_managers[local_id]->UpdateStartPt(stolen_job.helper_start_pt_new);
        job_managers[local_id]->UpdateBatchSize(stolen_job.helper_batch_size_new);
        job_managers[local_id]->SetWait(false);
        break;
      }
      //case kCancelHelp: {
      case 5: {
//        LOG(INFO) << "Cancel Help receved";
        int local_id = (msg.meta.recver - 50) % 1000;
//        job_managers[local_id]->SetWait(false);
        //for (auto gp_member : group_member_[local_id])
          //SendMsg(FindTask(msg.meta.recver, gp_member));
        for (int i = 0; i < group_member_[local_id].size(); ++i) 
          SendMsg(FindTask(msg.meta.recver, group_member_[local_id][i]));
        break;
      }
      //case kTerminateReassgn: {
      case 6: {
        //for (auto job_manager : job_managers)
           //job_manager->SetDoReassgn(0); 
        LOG(INFO) << "Terminate Reassgn receved";
        int local_id = (msg.meta.recver - 50) % 1000;
        job_managers[local_id]->SetDoReassgn(0); 
        break;
      }
      }
    }
  }
}

void RapidReassignment::SendMsg(Message msg) {
  std::mutex mtx;
  mtx.lock();
  send_queue_->Push(msg);
  mtx.unlock();
}

JobManager* RapidReassignment::GetJobManager(uint32_t worker_id) {
  return (job_managers[worker_id % num_local_threads_]);
}

void RapidReassignment::StartEverything() {
  // setup local channel & channel for comm
  auto ret = id_mapper_->GetChannelThreads(num_local_threads_, num_global_threads_);
  id_map_ = ret.second;
  channel_.reset(new Channel(num_local_threads_, num_global_threads_, ret.first, ret.second, mailbox_));
  local_channels_ = channel_->GetLocalChannels();

  // start sender
  sender_.reset(new Sender_rr(mailbox_));
  sender_->Start();
  send_queue_ = sender_->GetMessageQueue();

  // start get loop
  for (int i = 0; i < local_channels_.size(); ++i) {
    get_threads.push_back(std::thread([this, i] { GetLoop(local_channels_[i]); }));
  }

  // assign members
  //uint32_t num_members = num_nodes_;
  uint32_t num_members = num_nodes_ + group_mode_;
  group_member_ = GetMember_rr(node_id_, num_nodes_, num_local_threads_, num_members, group_mode_);

  // setup job managers for own job management
  for (int i = 0; i < num_local_threads_; ++i) {
    uint32_t thread_id = node_id_ * 1000 + 50 + i;
    job_managers.push_back(new JobManager(send_queue_, thread_id, group_member_[i]));
  }

  // setup status box for collection of others'status
  for (int i = 0; i < num_local_threads_; ++i) {
  //for (int i = 0; i < group_member_[0].size() + 1; ++i) {
    status_box_.push_back(new StatusBox(num_members - 1));
  }

  LOG(INFO) << "number of status box: " << status_box_[0]->GetSize();
  LOG(INFO) << "Started everything, node: " << node_id_ << "num global: " << num_global_threads_;
}

void RapidReassignment::StopEverything() {
  sender_->Stop();
  for (auto& thread : get_threads) {
    finished = true;
    thread.join();
  }
}

}  // namespace flexps
