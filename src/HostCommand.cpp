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
            // ADB_LOGI("disconnected to %s! connfd=%d\n", peeraddr.c_str(), channel->fd());
        }
    };
    set_client_on_connection_callback(connection_callback);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());

        if (buf->size() > 4) {
            if (strstr((char*)buf->data(), "OKAY") != NULL) {
                std::vector<std::string> devices_list_tmp = string_split(std::string((char*)buf->data() + 8), '\n');
                ADB_LOGI("devices_list_tmp.size: %d\n", devices_list_tmp.size());
                std::for_each(devices_list_tmp.begin(), devices_list_tmp.end(), [&](const std::string& it) {
                    ADB_LOGI("sss it: %s\n", it.c_str());
                });
            } else {
                std::vector<std::string> devices_list_tmp = string_split(std::string((char*)buf->data() + 4), '\n');
                devices_list_tmp.pop_back();    // pop null 
                ADB_LOGI("devices_list_tmp.size: %d\n", devices_list_tmp.size());
                std::for_each(devices_list_tmp.begin(), devices_list_tmp.end(), [&](const std::string& it) {
                    ADB_LOGI("sss it: %s\n", it.c_str());
                });

                std::vector<std::string> devices_info = string_split(devices_list_tmp[0], '\t');
                ADB_LOGI("devices_info.size: %d\n", devices_info.size());
                std::for_each(devices_info.begin(), devices_info.end(), [&](const std::string& its) {
                    ADB_LOGI("sss it: %s\n", its.c_str());
                });
            } 
        } else if (buf->size() == 4) {
            if (strcmp((char*)buf->data(), "OKAY") == 0) {
                // channel->write("host:devices");
            }
        } else {
            // TODO: buf-size < 4
        }

    };
    set_client_on_message_callback(message_callback);

    // auto write_complete_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {};
    // set_client_on_write_complete_callback(write_complete_callback);

    m_tcp_client.start();

    if (execute_cmd(cmd) == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }
    
    return -1;
}

