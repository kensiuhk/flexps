#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

namespace flexps {

namespace {
std::vector<std::vector<uint32_t>> GetMember_rr(uint32_t node_id, uint32_t num_nodes, uint32_t num_local_threads,
                                                uint32_t num_members, uint32_t mode) {
  //  srand(time(NULL));
  // int id = rand() % num_members;
  // LOG(INFO) << "Random number: " << rand();
  // LOG(INFO) << "Random number: " << id;
  std::vector<std::vector<uint32_t>> group_members;
  for (int i = 0; i < num_local_threads; ++i) {
    std::vector<uint32_t> members;
    for (int j = 0; j < num_nodes; ++j) {
      if (node_id == j)
        continue;
      members.push_back(j * 1000 + 50 + i);
    }
    group_members.push_back(members);
  }

LOG(INFO) <<"gp memebers:" << group_members[0].size();
LOG(INFO) <<"gp mode:" << mode;
if (mode == 1){
  //for (int i = 0; i < group_members.size(); ++i){
  for (int i = 0; i < num_local_threads; ++i){
    uint32_t helpee = 50 + i + 1;
    //if (i == group_members.size() - 1)
    if (i == num_local_threads - 1)
      helpee = 50;
    group_members[i].push_back(helpee);
}
}

if (mode == 2){
  for (int i = 0; i < num_local_threads; ++i){
    uint32_t helpee = 50 + i + 1;
    if (i == num_local_threads - 1)
      helpee = 50;
    uint32_t helpee2 = 50  + i - 1;
    if ( i == 0)
      helpee2 = 50 + num_local_threads - 1;
    helpee2 = (num_nodes - 1)*1000 + helpee2;
    group_members[i].push_back(helpee);
    group_members[i].push_back(helpee2);
}
}
LOG(INFO) <<"gp memebers:";
LOG(INFO) <<"gp memebers:" << group_members[0].size();
  return group_members;
  

 

}
}  // namespace

}  // namespace flexps
