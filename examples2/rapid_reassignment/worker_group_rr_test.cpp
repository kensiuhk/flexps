#include "glog/logging.h"
#include "gtest/gtest.h"

#include "examples/rapid_reassignment/worker_group_rr.hpp"

#include <iostream>
#include <vector>

namespace flexps {
namespace {

class TestWorkerGroup_rr : public testing::Test {
 public:
  TestWorkerGroup_rr() {}
  ~TestWorkerGroup_rr() {}

 protected:
  void SetUp() {}
  void TearDown() {}
};

TEST_F(TestWorkerGroup_rr, GetMember) {
  std::vector<std::vector<uint32_t>> group_members = GetMember_rr(2, 5, 3, 5, 0);
  EXPECT_EQ(group_members.size(), 3);
  EXPECT_EQ(group_members[1].size(), 5 - 1);
  EXPECT_EQ(group_members[1][3], 4051);
}

}  // namespace
}  // namespace flexps
