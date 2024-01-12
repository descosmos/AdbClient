#include <gtest/gtest.h>
#include <string>
#include <stdio.h>
#include "utils.h"
#include "AdbCommand.h"
#include "htime.h"

using namespace hv;

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 + 6, add_t(7, 6));
}

TEST(ConnectTest, BasicAssertions) {
  AdbCommand adbcmd;
  int connfd = adbcmd.create_socket(DEFAULT_ADB_PORT, "127.0.0.1");
  ASSERT_GE(connfd, 0);

  adbcmd.close_socket();
}