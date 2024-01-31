#include <gtest/gtest.h>
#include <stdio.h>

#include <string>

#include "host_command.h"
#include "host_serial_command.h"
#include "local_command.h"
#include "htime.h"
#include "utils.h"

#ifndef HOST_TRACK_DEVICE_TEST
// #define HOST_TRACK_DEVICE_TEST // adb track-devices
#endif // HOST_TRACK_DEVICE_TEST

#ifndef WIFI_TEST
// #define WIFI_TEST  // test in wifi mode
#endif  // WIFI_TEST

#ifndef HOST_KILL_TEST
// #define HOST_KILL_TEST  // After "adb kill" executed, the subsequent test cannot execute correctly, for which I have
// to close it temporarily.
#endif  // HOST_KILL_TEST

#ifndef LOCAL_USB_TEST
// #define LOCAL_USB_TEST  // adb -s usb
#endif // LOCAL_USB_TEST

#ifndef LOCAL_TCPIP_TEST
// #define LOCAL_TCPIP_TEST  // adb -s tcpip
#endif // LOCAL_TCPIP_TEST

#ifndef LOCAL_LOGCAT_TEST
// #define LOCAL_LOGCAT_TEST   // adb shell logcat
#endif // LOCAL_LOGCAT_TEST

using namespace hv;

typedef std::vector<std::string> vec_str;

int remote_port = 5037;
const char* remote_host = "127.0.0.1";
std::string_view serial = "d17cdac6";

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
    unique_spaces(str1);
    ASSERT_EQ(str1, std::string("Setence1 Setence2"));
}

TEST(HostCommandTest, BasicAssertions) {
    int status;
    HostCommand hostCommand;
    int connfd = hostCommand.m_tcp_client.createsocket(remote_port, remote_host);
    ASSERT_GE(connfd, 0);

    // adb version
    int version;
    status = hostCommand.get_version(version);
    ASSERT_NE(status, -1);
    ASSERT_GE(version, 0);
    
    // adb devices
    std::vector<DeviceInfo> devices_list;
    std::string devices_list_str;
    status = hostCommand.get_devices(devices_list_str);
    get_device_info_from_buf(devices_list, devices_list_str);
    ASSERT_NE(status, -1);
    ASSERT_FALSE(devices_list.empty());

    // adb devices -l
    std::vector<DeviceInfo> devices_list_l;
    std::string devices_list_l_str;
    status = hostCommand.get_devices_with_path(devices_list_l_str);
    get_device_info_from_buf(devices_list_l, devices_list_l_str);
    ASSERT_NE(status, -1);
    ASSERT_FALSE(devices_list_l.empty());

#ifdef HOST_TRACK_DEVICE_TEST
    // adb track-devices
    std::string device_status;
    status = hostCommand.track_devices();
    hostCommand.waits();
    device_status = hostCommand.get_tracked_devices();
    ASSERT_NE(status, -1);
    ASSERT_FALSE(device_status.empty());
#endif // HOST_TRACK_DEVICE_TEST

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
    HostSerialCommand hostSerialCommand;
    int connfd = hostSerialCommand.m_tcp_client.createsocket(remote_port, remote_host);
    ASSERT_GE(connfd, 0);

    std::vector<std::string> forward_list;
    std::string buf;
    
    // remove all forward
    ASSERT_NE(hostSerialCommand.kill_forward_all(serial), -1);

    // adb forward local:port remote:port -s [SERIAL]
    hostSerialCommand.forward(serial, "tcp:1346", "tcp:1346");
    hostSerialCommand.forward(serial, "tcp:1345", "tcp:1345", true);
    hostSerialCommand.list_forward(serial, buf);
    get_lines_from_buf(forward_list, buf);
    ASSERT_EQ(forward_list.size(), 2);
    forward_list.clear();
    buf.clear();

    // adb forward --remove LOCAL -s [SERIAL]
    hostSerialCommand.kill_forward(serial, "tcp:1345");
    hostSerialCommand.list_forward(serial, buf);
    get_lines_from_buf(forward_list, buf);
    ASSERT_EQ(forward_list.size(), 1);
    forward_list.clear();
    buf.clear();

    // adb forward --remove-all -s [SERIAL]
    hostSerialCommand.kill_forward_all(serial);
    hostSerialCommand.list_forward(serial, buf);
    get_lines_from_buf(forward_list, buf);
    ASSERT_TRUE(forward_list.empty());
    forward_list.clear();
    buf.clear();

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

TEST(LocalCommandTest, BasicAssertions) {
    int status;
    LocalCommand localCommand;
    int connfd = localCommand.m_tcp_client.createsocket(remote_port, remote_host);
    ASSERT_GE(connfd, 0);
    std::vector<std::string> res;
    std::string buf;

    // adb -s [SERIAL] shell getprop ro.build.version.release
    status = localCommand.shell(serial, "getprop ro.build.version.release");
    buf = localCommand.get_shell_data();
    get_lines_from_buf(res, buf);
    ASSERT_EQ(res.size(), 1);
    res.clear();
    buf.clear();

    // adb -s [SERIAL] shell getprop
    status = localCommand.get_properties(serial, buf);
    get_lines_from_buf(res, buf);
    ASSERT_FALSE(res.empty());
    res.clear();
    buf.clear();

    // adb -s [SERIAL] shell pm list packages
    status = localCommand.list_packages(serial, buf);
    get_lines_from_buf(res, buf);
    ASSERT_FALSE(res.empty());
    res.clear();
    buf.clear();

    // adb -s [SERIAL] shell /system/bin/screencap -p
    std::string data;
    status = localCommand.screencap(serial, data);
    ASSERT_FALSE(data.empty());

    // adb -s [SERIAL] root
    status = localCommand.root(serial);
    ASSERT_EQ(status, 0);

    // adb -s [SERIAL] sync
    localCommand.sync(serial);
    ASSERT_EQ(status, 0);

    std::vector<std::string> forward_list;
    
    // remove all forward
    ASSERT_NE(localCommand.kill_reverse_all(serial), -1);

    // adb -s [SERIAL] reverse local:port remote:port 
    localCommand.reverse(serial, "tcp:1247", "tcp:1247");
    localCommand.reverse(serial, "tcp:1248", "tcp:1248");
    localCommand.reverse(serial, "tcp:1249", "tcp:1249", true);
    localCommand.list_reverse(serial, buf);
    get_lines_from_buf(forward_list, buf);
    ASSERT_EQ(forward_list.size(), 3);
    forward_list.clear();
    buf.clear();

    // adb -s [SERIAL] reverse --remove LOCAL 
    localCommand.kill_reverse(serial, "tcp:1248");
    localCommand.list_reverse(serial, buf);
    get_lines_from_buf(forward_list, buf);
    ASSERT_EQ(forward_list.size(), 2);
    forward_list.clear();
    buf.clear();

    // adb -s [SERIAL] reverse --remove-all 
    localCommand.kill_reverse_all(serial);
    localCommand.list_reverse(serial, buf);
    get_lines_from_buf(forward_list, buf);
    ASSERT_TRUE(forward_list.empty());
    forward_list.clear();
    buf.clear();

#ifdef LOCAL_TCPIP_TEST
    // adb -s [SERIAL] tcpip PORT
    status = localCommand.tcpip(serial, 5678);
    ASSERT_EQ(status, 0);
#endif // LOCAL_TCPIP_TEST

#ifdef LOCAL_USB_TEST
    // adb -s [SERIAL] usb
    status = localCommand.usb(serial);
    ASSERT_EQ(status, 0);
#endif // LOCAL_USB_TEST

#ifdef LOCAL_LOGCAT_TEST
    localCommand.logcat(serial);
    buf = localCommand.get_shell_data();
    ASSERT_EQ(status, 0);
    ASSERT_FALSE(buf.empty());
#endif // LOCAL_LOGCAT_TEST

    localCommand.m_tcp_client.stop();
    localCommand.m_tcp_client.closesocket();
}