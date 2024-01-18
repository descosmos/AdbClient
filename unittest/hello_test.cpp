#include <gtest/gtest.h>
#include <stdio.h>

#include <string>

#include "HostCommand.h"
#include "htime.h"
#include "utils.h"

#ifndef WIFI_TEST
// #define WIFI_TEST // test in wifi mode

#endif // WIFI_TEST

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
    unique_character(str1);
    ASSERT_EQ(str1, std::string("Setence1 Setence2"));
}

TEST(HostCommandTest, BasicAssertions) {
    int remote_port = 5037;
    const char* remote_host = "127.0.0.1";

    HostCommand hostCommand;
    int connfd = hostCommand.m_tcp_client.createsocket(remote_port, remote_host);
    ASSERT_GE(connfd, 0);

    // adb version
    int version;
    int status = hostCommand.get_version(version);
    ASSERT_NE(status, -1);
    ASSERT_GE(version, 0);

    // adb devices
    std::vector<HostCommand::DevicesInfo> devices_list;
    status = hostCommand.get_devices(devices_list);
    ASSERT_NE(status, -1);
    ASSERT_NE(devices_list.size(), 2);

    // adb devices -l
    std::vector<HostCommand::DevicesInfo> devices_list_l;
    status = hostCommand.get_devices_with_path(devices_list_l);
    ASSERT_NE(status, -1);
    ASSERT_NE(devices_list.size(), 6);

#ifdef WIFI_TEST
    // adb connect
    status = hostCommand.connect("10.11.234.57", "1314");
    ASSERT_NE(status, -1);

    // adb disconnect
    status = hostCommand.disconnect("10.11.234.57", "1314");
    ASSERT_NE(status, -1);
#endif

    // adb kill
    // status = hostCommand.kill();
    // ASSERT_NE(status, -1);
}