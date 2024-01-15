#include <algorithm>
#include "HostCommand.h"
#include "android/stringprintf.h"

#define STRING_CONCAT(a, b)  a ## b

HostCommand::HostCommand() {}

HostCommand::~HostCommand() {}

void HostCommand::set_client_on_connection_callback(std::function<void(const TSocketChannelPtr&)> callback) {
    m_tcp_client.onConnection = callback;
}

void HostCommand::set_client_on_message_callback(std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) {
    m_tcp_client.onMessage = callback;
}

void HostCommand::set_client_on_write_complete_callback(std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) {
    m_tcp_client.onWriteComplete = callback;
}

int HostCommand::execute_cmd(std::string_view cmd) {    
    return 0;
}

int HostCommand::get_version(std::string& ARGS_OUT res) {
    return -1;
}

int HostCommand::get_devices(std::vector<DevicesInfo>& ARGS_OUT devices_list) {
    std::string_view cmd = STRING_CONCAT("host", ":devices");

    auto connection_callback = [&](const hv::SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            auto str = android::base::StringPrintf("%04x", cmd.length()).append(cmd);
            channel->write(str);
        } else {
            printf("disconnected to %s! connfd=%d\n", peeraddr.c_str(), channel->fd());
        }
    };
    set_client_on_connection_callback(connection_callback);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        if (buf->size() > 4) {
            if (strstr((char*)buf->data(), "OKAY") != NULL) {
                std::vector<std::string> devices_list_tmp = string_split(std::string((char*)buf->data() + 8), '\n');
                printf("devices_list_tmp.size: %d\n", devices_list_tmp.size());
                std::for_each(devices_list_tmp.begin(), devices_list_tmp.end(), [&](const std::string& it){
                    printf("sss it: %s\n", it.c_str());
                });
                // printf("< %.*s\n", (int)buf->size(), (char*)buf->data());
                // printf("< %.*s\n", (int)buf->size(), (char*)buf->data() + 4);
                // fflush(stdout);
            } 

        } else {
            if (strcmp((char*)buf->data(), "OKAY") == 0) {
                std::vector<std::string> devices_list_tmp = string_split(std::string((char*)buf->data()), '\n');
                printf("devices_list_tmp.size: %d\n", devices_list_tmp.size());
                std::for_each(devices_list_tmp.begin(), devices_list_tmp.end(), [&](const std::string& it){

                    printf("sss it: %s\n", it.c_str());
                });
                // printf("< %.*s\n", (int)buf->size(), (char*)buf->data());
                fflush(stdout);
            }
        }

    };
    set_client_on_message_callback(message_callback);

    // auto write_complete_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {};
    // set_client_on_write_complete_callback(write_complete_callback);

    m_tcp_client.start();

    if (execute_cmd(cmd) == -1) {
        printf("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }
    
    return -1;
}

