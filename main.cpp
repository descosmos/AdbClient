#include <iostream>

#include "HostCommand.h"
#include "HostSerialCommand.h"
#include "android/stringprintf.h"
#include "htime.h"
#include "libhv_evpp/TcpClient.h"
#include "utils.h"

#define TEST_RECONNECT 0
#define TEST_TLS 0

using namespace hv;
 
int execute_host_command() {
    int remote_port = 5037;
    const char* remote_host = "127.0.0.1";

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
    hostCommand.get_devices(devices_list);
    for (const auto& device : devices_list) {
        ADB_LOGI("devices_list: %s %s %s %s %s %d\n", device.serial.c_str(), device.state.c_str(), device.product.c_str(), device.model.c_str(),
                 device.device.c_str(), device.transport_id);
    }

    devices_list.clear();
    hostCommand.get_devices_with_path(devices_list);
    for (const auto& device : devices_list) {
        ADB_LOGI("devices_list: %s %s %s %s %s %d\n", device.serial.c_str(), device.state.c_str(), device.product.c_str(), device.model.c_str(),
                 device.device.c_str(), device.transport_id);
    }

    hostCommand.connect("10.11.234.57", "1314");

    hostCommand.disconnect("10.11.234.57", "1314");

    // hostCommand.track_devices();

    // hostCommand.kill();

    hostCommand.m_tcp_client.stop();
    hostCommand.m_tcp_client.closesocket();

    return 0;
}

int execute_serial_command() {
    int remote_port = 5037;
    const char* remote_host = "127.0.0.1";
    std::string_view serial = "18fd5384";

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
    hostSerialCommand.list_forward(serial, forward_list);
    for (const auto& forward : forward_list) {
        ADB_LOGI("%s \n", forward.c_str());
    }

    hostSerialCommand.kill_forward(serial, "tcp:1345");
    forward_list.clear();
    hostSerialCommand.list_forward(serial, forward_list);
    for (const auto& forward : forward_list) {
        ADB_LOGI("%s \n", forward.c_str());
    }

    hostSerialCommand.kill_forward_all(serial);
    forward_list.clear();
    hostSerialCommand.list_forward(serial, forward_list);
    for (const auto& forward : forward_list) {
        ADB_LOGI("%s \n", forward.c_str());
    }

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

int main(int argc, char* argv[]) {

    execute_host_command();

    execute_serial_command();

    return 0;
}