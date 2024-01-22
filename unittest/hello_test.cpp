#include <gtest/gtest.h>
#include <stdio.h>

#include <string>

#include "HostCommand.h"
#include "HostSerialCommand.h"
#include "htime.h"
#include "utils.h"

#ifndef WIFI_TEST
#define WIFI_TEST // test in wifi mode
#endif  // WIFI_TEST

#ifndef HOST_KILL_TEST
// #define HOST_KILL_TEST  // After "adb kill" executed, the subsequent test cannot execute correctly, for which I have
// to close it temporarily.
#endif  // HOST_KILL_TEST

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
    std::vector<DeviceInfo> devices_list;
    status = hostCommand.get_devices(devices_list);
    ASSERT_NE(status, -1);
    ASSERT_NE(devices_list.size(), 2);

    // adb devices -l
    std::vector<DeviceInfo> devices_list_l;
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
#endif  // WIFI_TEST

#ifdef HOST_KILL_TEST
    // adb kill
    status = hostCommand.kill();
    ASSERT_NE(status, -1);
#endif  // HOST_KILL_TEST
}

TEST(HostSerialCommandTest, BasicAssertions) {
    int remote_port = 5037;
    const char* remote_host = "127.0.0.1";
    std::string_view serial = "18fd5384";

    HostSerialCommand hostSerialCommand;
    int connfd = hostSerialCommand.m_tcp_client.createsocket(remote_port, remote_host);
    ASSERT_GE(connfd, 0);

    // remove all forward
    ASSERT_NE(hostSerialCommand.kill_forward_all(serial), -1);

    // adb forward local:port remote:port -s [SERIAL]
    hostSerialCommand.forward(serial, "tcp:1346", "tcp:1346");
    hostSerialCommand.forward(serial, "tcp:1345", "tcp:1345");
    std::vector<std::string> forward_list;
    hostSerialCommand.list_forward(serial, forward_list);
    ASSERT_EQ(forward_list.size(), 2);

    // adb forward --remove LOCAL -s [SERIAL]
    hostSerialCommand.kill_forward(serial, "tcp:1345");
    forward_list.clear();
    hostSerialCommand.list_forward(serial, forward_list);
    ASSERT_EQ(forward_list.size(), 1);

    // adb forward --remove-all -s [SERIAL]
    hostSerialCommand.kill_forward_all(serial);
    forward_list.clear();
    hostSerialCommand.list_forward(serial, forward_list);
    ASSERT_TRUE(forward_list.empty());

    // adb get-devpath -s [SERIAL]
    std::string device_path;
    hostSerialCommand.get_device_path(serial, device_path);
    ASSERT_FALSE(device_path.empty());

    // adb get-serialno -s [SERIAL]
    std::string serial_no;
    hostSerialCommand.get_serial_no(serial, serial_no);
    ASSERT_FALSE(serial_no.empty());

    // adb get-state -s [SERIAL]
    std::string state;
    hostSerialCommand.get_state(serial, state);
    ASSERT_FALSE(state.empty());

    hostSerialCommand.m_tcp_client.stop();
    hostSerialCommand.m_tcp_client.closesocket();
}