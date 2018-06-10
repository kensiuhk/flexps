#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "examples/rapid_reassignment/job_meta_rr.hpp"

namespace flexps {

class StatusBox {
 public:
  StatusBox(uint32_t size) : size_(size){};

  uint32_t GetSize() { return size_; };
  std::vector<JobMeta> GetJobMetas() { return job_metas; };

  void AddStatus(JobMeta job_meta) { job_metas.push_back(job_meta); };

  bool IsFull() {
    if (size_ == job_metas.size()) {
      // LOG(INFO) << " size: " << size_ << "job size: " << job_metas.size();
      return true;
    }
    return false;
  };

  void Clear() { job_metas.clear(); }

 private:
  uint32_t size_ = 0;
  std::vector<JobMeta> job_metas;
};
}  // namespace flexps
