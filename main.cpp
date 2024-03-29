#include <stdio.h>

#include <iostream>

#include "adb_protocol.h"
#include "android/stringprintf.h"
#include "host_command.h"
#include "host_serial_command.h"
#include "htime.h"
#include "libhv_evpp/TcpClient.h"
#include "local_command.h"
#include "utils.h"

#define TEST_RECONNECT 0
#define TEST_TLS 0

#ifndef TRACK_LOOP
// #define TRACK_LOOP  // enable if you want to track devices consecutively.
#endif  // TRACK_LOOP

#ifndef SHELL_LOOP
// #define SHELL_LOOP  // enable if you want to execute consecutive commands such as "adb shell logcat"
#endif  // SHELL_LOOP

using namespace hv;

int remote_port = 5037;
const char* remote_host = "127.0.0.1";
std::string_view serial = "d17cdac6";

int execute_host_command() {
    int status = 0;
    HostCommand hostCommand;
    int connfd = hostCommand.m_tcp_client.createsocket(remote_port, remote_host);
    if (connfd < 0) {
        return -20;
    }
    ADB_LOGI("client connect to port %d, connfd=%d ...\n", remote_port, connfd);

#if TEST_RECONNECT
    // reconnect: 1,2,4,8,10,10,10...
    reconn_setting_t reconn;
    reconn_setting_init(&reconn);
    reconn.min_delay = 1000;
    reconn.max_delay = 10000;
    reconn.delay_policy = 2;
    hostCommand.m_tcp_client.setReconnect(&reconn);
#endif

#if TEST_TLS
    cli.withTLS();
#endif

    int version;
    hostCommand.get_version(version);
    ADB_LOGI("version: %d\n", version);

    std::vector<DeviceInfo> devices_list;
    std::string devices_list_str = "";
    hostCommand.get_devices(devices_list_str);
    ADB_LOGI("devices_list_str: %s\n", devices_list_str.c_str());
    get_device_info_from_buf(devices_list, devices_list_str);

    for (const auto& device : devices_list) {
        ADB_LOGI("devices_list: %s %s %s %s %s %I64d\n", device.serial.c_str(), device.state.c_str(),
                 device.product.c_str(), device.model.c_str(), device.device.c_str(), device.transport_id);
    }

    devices_list.clear();
    devices_list_str.clear();
    hostCommand.get_devices_with_path(devices_list_str);
    get_device_info_from_buf(devices_list, devices_list_str);
    for (const auto& device : devices_list) {
        ADB_LOGI("devices_list: %s %s %s %s %s %I64d\n", device.serial.c_str(), device.state.c_str(),
                 device.product.c_str(), device.model.c_str(), device.device.c_str(), device.transport_id);
    }

    // hostCommand.connect("10.11.252.57", "1314");

    // hostCommand.disconnect("10.11.252.57", "1314");
    // ADB_LOGE("Error: %s\n", hostCommand.error_message().c_str());

    status = hostCommand.track_devices();
#ifdef TRACK_LOOP
    while (true) {
        hostCommand.waits();
        std::string device_status = hostCommand.get_tracked_devices();
        ADB_LOGI("device_status: %s\n", device_status.c_str());
        hostCommand.resume();
    }
#else
    hostCommand.waits();
    std::string device_status = hostCommand.get_tracked_devices();
    ADB_LOGI("device_status: %s\n", device_status.c_str());
    ADB_LOGI("status: %d\n", status);
#endif

    // hostCommand.kill();

    hostCommand.m_tcp_client.stop();
    hostCommand.m_tcp_client.closesocket();

    return 0;
}

int execute_serial_command() {
    HostSerialCommand hostSerialCommand;
    int connfd = hostSerialCommand.m_tcp_client.createsocket(remote_port, remote_host);
    if (connfd < 0) {
        return -20;
        ADB_LOGI("createsocket failed.\n");
    }
    ADB_LOGI("client connect to port %d, connfd=%d ...\n", remote_port, connfd);

    hostSerialCommand.forward(serial, "tcp:1346", "tcp:1346");
    hostSerialCommand.forward(serial, "tcp:1345", "tcp:1345");

    std::vector<std::string> forward_list;
    std::string buf;
    hostSerialCommand.list_forward(serial, buf);
    get_lines_from_buf(forward_list, buf);
    for (const auto& forward : forward_list) {
        ADB_LOGI("%s \n", forward.c_str());
    }
    forward_list.clear();
    buf.clear();

    hostSerialCommand.kill_forward(serial, "tcp:1345");
    hostSerialCommand.list_forward(serial, buf);
    get_lines_from_buf(forward_list, buf);
    for (const auto& forward : forward_list) {
        ADB_LOGI("%s \n", forward.c_str());
    }
    forward_list.clear();
    buf.clear();

    hostSerialCommand.kill_forward_all(serial);
    forward_list.clear();
    hostSerialCommand.list_forward(serial, buf);
    get_lines_from_buf(forward_list, buf);
    for (const auto& forward : forward_list) {
        ADB_LOGI("%s \n", forward.c_str());
    }
    forward_list.clear();
    buf.clear();

    std::string device_path;
    hostSerialCommand.get_device_path(serial, device_path);
    ADB_LOGI("device_path: %s\n", device_path.c_str());

    std::string serial_no;
    hostSerialCommand.get_serial_no(serial, serial_no);
    ADB_LOGI("serial_no: %s\n", serial_no.c_str());

    std::string state;
    hostSerialCommand.get_state(serial, state);
    ADB_LOGI("state: %s\n", state.c_str());

    hostSerialCommand.m_tcp_client.stop();
    hostSerialCommand.m_tcp_client.closesocket();

    return 0;
}

int execute_local_command() {
    LocalCommand localCommand;
    int connfd = localCommand.m_tcp_client.createsocket(remote_port, remote_host);
    if (connfd < 0) {
        return -20;
        ADB_LOGI("createsocket failed.\n");
    }

    ADB_LOGI("client connect to port %d, connfd=%d ...\n", remote_port, connfd);

    // localCommand.transport(serial);

    std::string buf;
    std::vector<std::string> lines;
#ifdef SHELL_LOOP
    localCommand.shell(serial, "logcat", true);
    while (true) {
        localCommand.waits();
        std::string consecutive_data = localCommand.get_shell_data();
        ADB_LOGI("consecutive_data: %s\n", consecutive_data.c_str());
        localCommand.resume();
    }
#else
    localCommand.shell(serial, "getprop ro.build.version.release");
    // localCommand.shell(serial, "ls /sys/class/thermal/", buf);
    buf = localCommand.get_shell_data();
    get_lines_from_buf(lines, buf);
    for (const auto& line : lines) {
        ADB_LOGI("%s \n", line.c_str());
    }

    // localCommand.logcat(serial);
    // buf = localCommand.get_shell_data();
    // for (const auto& line : lines) {
    //     ADB_LOGI("%s \n", line.c_str());
    // }

#endif  // SHELL_LOOP

    lines.clear();
    buf.clear();
    localCommand.list_packages(serial, buf);
    get_lines_from_buf(lines, buf);
    for (const auto& line : lines) {
        ADB_LOGI("%s \n", line.c_str());
    }

    lines.clear();
    buf.clear();
    localCommand.get_properties(serial, buf);
    get_lines_from_buf(lines, buf);
    for (const auto& line : lines) {
        ADB_LOGI("%s \n", line.c_str());
    }

    std::string data;
    localCommand.screencap(serial, data);
    ADB_LOGI("data.size: %zd\n", data.size());
    FILE* file = fopen("screencap.png", "wb+");
    if (file != nullptr) {
        fwrite(data.c_str(), sizeof(char), data.size(), file);
        fclose(file);
        ADB_LOGI("Binary data has been written to file.\n");
    } else {
        ADB_LOGI("Failed to open file for writing.\n");
    }

    // localCommand.tcpip(serial, 5678);
    // localCommand.usb(serial);

    localCommand.root(serial);
    localCommand.sync(serial);

    lines.clear();
    buf.clear();

    localCommand.reverse(serial, "tcp:1247", "tcp:1247");
    localCommand.reverse(serial, "tcp:1248", "tcp:1248");
    localCommand.reverse(serial, "tcp:1249", "tcp:1249", true);

    localCommand.list_reverse(serial, buf);
    get_lines_from_buf(lines, buf);
    ADB_LOGI("lines.size: %zd\n", lines.size());
    for (const auto& forward : lines) {
        ADB_LOGI("%s \n", forward.c_str());
    }
    lines.clear();
    buf.clear();

    localCommand.kill_reverse(serial, "tcp:1248");
    localCommand.list_reverse(serial, buf);
    get_lines_from_buf(lines, buf);
    ADB_LOGI("lines.size: %zd\n", lines.size());
    for (const auto& forward : lines) {
        ADB_LOGI("%s \n", forward.c_str());
    }
    lines.clear();
    buf.clear();

    localCommand.kill_reverse_all(serial);
    localCommand.list_reverse(serial, buf);
    get_lines_from_buf(lines, buf);
    ADB_LOGI("lines.size: %zd\n", lines.size());
    for (const auto& forward : lines) {
        ADB_LOGI("%s \n", forward.c_str());
    }
    lines.clear();
    buf.clear();

    localCommand.m_tcp_client.stop();
    localCommand.m_tcp_client.closesocket();

    return 0;
}

int main(int argc, char* argv[]) {
    // execute_host_command();
    execute_serial_command();
    // execute_local_command();
    // ADB_LOGI("ID_OKAY: %x\n", ID_OKAY);

    return 0;
}