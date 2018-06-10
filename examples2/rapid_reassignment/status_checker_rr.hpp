#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "examples/rapid_reassignment/job_meta_rr.hpp"

namespace flexps {

namespace {

JobMeta CalSlowestWorker(std::vector<JobMeta> job_metas) {
  int j = 0;
  for (int i = 0; i < job_metas.size(); ++i) {
    // LOG(INFO) << job_metas[i].thread_id << " " << (float) job_metas[i].current_pt << "  " << job_metas[i].batch_size;
//    if ((float) (job_metas[j].current_pt - job_metas[j].start_pt) / job_metas[j].batch_size >
//        (float) (job_metas[i].current_pt - job_metas[i].start_pt) / job_metas[i].batch_size) {
    if ((float) (job_metas[j].batch_size - (job_metas[j].current_pt - job_metas[j].start_pt))< 
        (float) (job_metas[i].batch_size - (job_metas[i].current_pt - job_metas[i].start_pt))) {
      j = i;
    }
  }
  //LOG(INFO) << "slowest worker: " << job_metas[j].thread_id << j;
  return job_metas[j];
};

/*bool IsInSameIteration(JobMeta job_meta, std::vector<JobMeta> job_metas) {
  for (int i = 0; i < job_metas.size(); ++i) {
    if (job_meta.iter < job_metas[i].iter)
      return false;
  }
  return true;
}*/
bool IsInSameIteration(JobMeta job_meta, JobMeta job_meta_s) {
    if (job_meta.iter < job_meta_s.iter)
      return false;
  return true;
}
}  // namespace
}  // namespace flexps
