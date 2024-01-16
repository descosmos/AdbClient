#include <gtest/gtest.h>
#include <stdio.h>

#include <string>

#include "HostCommand.h"
#include "htime.h"
#include "utils.h"

using namespace hv;

typedef std::vector<std::string> vec_str;

TEST(UtilsTest, StringSplitAssertions) {
    std::string str1 = "Setence 1\nSetence 2";
    vec_str res1 = string_split(str1, '\n');
    ASSERT_EQ(res1.size(), 2);
    ASSERT_EQ(res1[0], std::string("Setence 1"));
    ASSERT_EQ(res1[1], std::string("Setence 2"));

    std::string str2 = "Setence 1\nSetence 2\n";
    vec_str res2 = string_split(str2, '\n');
    ASSERT_EQ(res2.size(), 3);
    ASSERT_EQ(res2[0], std::string("Setence 1"));
    ASSERT_EQ(res2[1], std::string("Setence 2"));
    ASSERT_EQ(res2[2], std::string(""));

    std::string str3 = "Setence 1\tSetence 2";
    vec_str res3 = string_split(str3, '\t');
    ASSERT_EQ(res3.size(), 2);
    ASSERT_EQ(res3[0], std::string("Setence 1"));
    ASSERT_EQ(res3[1], std::string("Setence 2"));
}

TEST(UtilsTest, StringUniqueAssertions) {
    std::string str1 = "Setence1       Setence2";
    std::string res1 = unique_character(str1);
    ASSERT_EQ(res1, std::string("Setence1 Setence2"));
}

TEST(ConnectTest, BasicAssertions) {
    HostCommand adbcmd;
    int connfd = adbcmd.m_tcp_client.createsocket(DEFAULT_ADB_PORT, "127.0.0.1");
    ASSERT_GE(connfd, 0);

    adbcmd.m_tcp_client.closesocket();
}