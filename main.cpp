/*
 * TcpClient_test.cpp
 *
 * @build   make evpp
 * @server  bin/TcpServer_test 1234
 * @client  bin/TcpClient_test 1234
 *
 */

#include <iostream>

#include "utils.h"
#include "android/stringprintf.h"
#include "libhv_evpp/TcpClient.h"
#include "htime.h"
#include "HostCommand.h"

#define TEST_RECONNECT  0
#define TEST_TLS        0

using namespace hv;

int main(int argc, char* argv[]) {
    int remote_port = 5037;
    const char* remote_host = "127.0.0.1";

    HostCommand hostCommand;
    int connfd = hostCommand.m_tcp_client.createsocket(remote_port, remote_host);
    if (connfd < 0) {
        return -20;
    }
    printf("client connect to port %d, connfd=%d ...\n", remote_port, connfd);

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

    std::vector<HostCommand::DevicesInfo> devices_list(2);
    hostCommand.get_devices(devices_list);

    return 0;
}