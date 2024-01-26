#include <stdio.h>

#include <iostream>

#include "host_command.h"
#include "host_serial_command.h"
#include "local_command.h"
#include "android/stringprintf.h"
#include "htime.h"
#include "libhv_evpp/TcpClient.h"
#include "utils.h"

#define TEST_RECONNECT 0
#define TEST_TLS 0

using namespace hv;

int remote_port = 5037;
const char* remote_host = "127.0.0.1";
std::string_view serial = "d17cdac6";

int execute_host_command() {
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
    std::string devices_list_str;
    hostCommand.get_devices(devices_list_str);
    get_device_info_from_buf(devices_list, devices_list_str);

    for (const auto& device : devices_list) {
        ADB_LOGI("devices_list: %s %s %s %s %s %d\n", device.serial.c_str(), device.state.c_str(),
                 device.product.c_str(), device.model.c_str(), device.device.c_str(), device.transport_id);
    }

    devices_list.clear();
    devices_list_str.clear();
    hostCommand.get_devices_with_path(devices_list_str);
    get_device_info_from_buf(devices_list, devices_list_str);
    for (const auto& device : devices_list) {
        ADB_LOGI("devices_list: %s %s %s %s %s %d\n", device.serial.c_str(), device.state.c_str(),
                 device.product.c_str(), device.model.c_str(), device.device.c_str(), device.transport_id);
    }

    // hostCommand.connect("10.11.234.57", "1314");

    // hostCommand.disconnect("10.11.234.57", "1314");

    // hostCommand.track_devices();

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
    localCommand.shell(serial, "getprop ro.build.version.release", buf);
    get_lines_from_buf(lines, buf);
    // localCommand.shell(serial, "ls /sys/class/thermal/", lines);
    for (const auto& line : lines) {
        ADB_LOGI("%s \n", line.c_str());
    }

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
    ADB_LOGI("data.size: %d\n", data.size());
    FILE* file = fopen("screencap.png", "wb+");  // 以二进制写入模式打开文件
    if (file != nullptr) {
        fwrite(data.c_str(), sizeof(char), data.size(), file);  // 写入二进制数据

        fclose(file);  // 关闭文件
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
    ADB_LOGI("lines.size: %d\n", lines.size());
    for (const auto& forward : lines) {
        ADB_LOGI("%s \n", forward.c_str());
    }
    lines.clear();
    buf.clear();

    localCommand.kill_reverse(serial, "tcp:1248");
    localCommand.list_reverse(serial, buf);
    get_lines_from_buf(lines, buf);
    ADB_LOGI("lines.size: %d\n", lines.size());
    for (const auto& forward : lines) {
        ADB_LOGI("%s \n", forward.c_str());
    }
    lines.clear();
    buf.clear();

    localCommand.kill_reverse_all(serial);
    localCommand.list_reverse(serial, buf);
    get_lines_from_buf(lines, buf);
    ADB_LOGI("lines.size: %d\n", lines.size());
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

    return 0;
}