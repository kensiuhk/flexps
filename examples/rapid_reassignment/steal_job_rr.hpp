#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "examples/rapid_reassignment/job_meta_rr.hpp"

namespace flexps {

struct StolenJob {
  uint32_t helper_batch_size_new;
  uint32_t helper_start_pt_new;
  uint32_t helpee_batch_size_new;
};

namespace {

// std::vector<uint32_t> StealJob (JobMeta job_meta, float percent) {
bool StealJob(JobMeta job_meta, float percent, StolenJob& stolen_job, float steal_threshold) {
  // StolenJob stolen_job;
  //LOG(INFO) << "steal job threshold: " << 0.9 * job_meta.batch_size;
  //if (job_meta.batch_size == 0)
  if(job_meta.current_pt == job_meta.start_pt + job_meta.batch_size)
    return false;
  //if ((job_meta.current_pt - job_meta.start_pt) > 0.9 * job_meta.batch_size)
  if ((job_meta.current_pt - job_meta.start_pt) > steal_threshold)
    return false;

  stolen_job.helper_batch_size_new = percent * (job_meta.batch_size - (job_meta.current_pt - job_meta.start_pt));
  stolen_job.helper_start_pt_new = job_meta.start_pt + job_meta.batch_size - stolen_job.helper_batch_size_new;

  stolen_job.helpee_batch_size_new = job_meta.batch_size - stolen_job.helper_batch_size_new;

  // std::vector<uint32_t> reassgn;
  // reassgn.push_back(helper_batch_size_new);
  // reassgn.push_back(helper_start_pt_new);
  // reassgn.push_back(helpee_batch_size);

  return true;
}

}  // namespace
}  // namespace flexps
