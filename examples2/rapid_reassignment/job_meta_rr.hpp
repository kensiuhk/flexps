#pragma once

namespace flexps {

struct JobMeta {
  // uint32_t my_id_;
  uint32_t iter = 0;
  uint32_t start_pt = 0;
  uint32_t batch_size = 0;
  uint32_t current_pt = 0;

  uint32_t thread_id = 0;
//  uint32_t next_start_pt = 0;
//  uint32_t next_batch_size = 0;
};

}  // namespace flexps
