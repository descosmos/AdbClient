#include "HostCommand.h"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <thread>

#include "android/stringprintf.h"

#define STRING_CONCAT(a, b) a##b

using namespace std::chrono_literals;

std::condition_variable cv;
std::mutex _mutex;
int finished = false;

void waits() {
    std::unique_lock<std::mutex> lk(_mutex);
    cv.wait(lk, [&] { return finished == 1; });
}

void weak_up() {
    {
        std::lock_guard<std::mutex> lk(_mutex);
        finished = 1;
    }
    cv.notify_all();
}

HostCommand::HostCommand() {}

HostCommand::~HostCommand() {}

void HostCommand::set_client_on_connection_callback(std::function<void(const TSocketChannelPtr&)> callback) {
    m_tcp_client.onConnection = callback;
}

void HostCommand::set_client_on_message_callback(std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) {
    m_tcp_client.onMessage = callback;
}

void HostCommand::set_client_on_write_complete_callback(
    std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) {
    m_tcp_client.onWriteComplete = callback;
}

int HostCommand::execute_cmd(std::string_view cmd) { return 0; }

int HostCommand::get_version(int& ARGS_OUT version) {
    std::string_view cmd = STRING_CONCAT("host", ":version");
    int status = -1;

    auto connection_callback = [&](const hv::SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            auto str = android::base::StringPrintf("%04x", cmd.length()).append(cmd);
            channel->write(str);
            ADB_LOGI("connect to %s! connfd=%d\n", peeraddr.c_str(), channel->fd());
        } else {
            ADB_LOGI("disconnected to %s! connfd=%d\n", peeraddr.c_str(), channel->fd());
            weak_up();
        }
        if (m_tcp_client.isReconnect()) {
            ADB_LOGI("reconnect cnt=%d, delay=%d\n", m_tcp_client.reconn_setting->cur_retry_cnt,
                     m_tcp_client.reconn_setting->cur_delay);
        }
    };
    set_client_on_connection_callback(connection_callback);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        if (buf->size() > 4) {
            if (strstr((char*)buf->data(), "OKAY") != NULL) {
                std::string data = (char*)buf->data() + 8;
                version = std::stoi(data, nullptr, 16);
            } else {
                std::string data = (char*)buf->data() + 4;
                version = std::stoi(data, nullptr, 16);
            }
            status = 0;
        } else if (buf->size() == 4) {
            if (strcmp((char*)buf->data(), "OKAY") == 0) {
                // channel->write("host:devices");
            } else {
                status = -1;
            }
        } else {
            // TODO: buf-size < 4
        }

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    // auto write_complete_callback = [&](const hv::SocketChannelPtr& channel,
    // hv::Buffer* buf) {};
    // set_client_on_write_complete_callback(write_complete_callback);

    finished = 0;

    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}

void get_device_info_from_buf(std::vector<HostCommand::DevicesInfo>& ARGS_OUT devices_list,
                              const std::string& ARGS_IN buf) {
    std::vector<std::string> devices_list_tmp = string_split(buf, '\n');
    devices_list_tmp.pop_back();  // pop null

#ifdef ADB_DEBUG
    ADB_LOGI("devices_list_tmp.size: %d\n", devices_list_tmp.size());
#endif

    for (auto& device : devices_list_tmp) {
        ADB_LOGI("devices_list_tmp it: %s\n", device.c_str());
        std::vector<std::string> devices_info = string_split(device, '\t');
#ifdef ADB_DEBUG
        ADB_LOGI("devices_info.size: %d\n", devices_info.size());
        for (auto& info : devices_info) {
            ADB_LOGI("devices_info it: %s\n", info.c_str());
        }
#endif

        devices_list.push_back(HostCommand::DevicesInfo(devices_info[0], devices_info[1]));
    }
}

int HostCommand::get_devices(std::vector<DevicesInfo>& ARGS_OUT devices_list) {
    std::string_view cmd = STRING_CONCAT("host", ":devices");
    int status = -1;

    auto connection_callback = [&](const hv::SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            auto str = android::base::StringPrintf("%04x", cmd.length()).append(cmd);
            channel->write(str);
            ADB_LOGI("connect to %s! connfd=%d\n", peeraddr.c_str(), channel->fd());
        } else {
            ADB_LOGI("disconnected to %s! connfd=%d\n", peeraddr.c_str(), channel->fd());
            weak_up();
        }
        if (m_tcp_client.isReconnect()) {
            ADB_LOGI("reconnect cnt=%d, delay=%d\n", m_tcp_client.reconn_setting->cur_retry_cnt,
                     m_tcp_client.reconn_setting->cur_delay);
        }
    };
    set_client_on_connection_callback(connection_callback);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        if (channel->isConnected()) {
            if (buf->size() > 4) {
                if (strstr((char*)buf->data(), "OKAY") != NULL) {
                    get_device_info_from_buf(devices_list, std::string((char*)buf->data() + 8));
                } else {
                    get_device_info_from_buf(devices_list, std::string((char*)buf->data() + 4));
                }
                status = 0;
            } else if (buf->size() == 4) {
                if (strcmp((char*)buf->data(), "OKAY") == 0) {
                    // channel->write("host:devices");
                } else {
                    status = -1;
                }
            } else {
                // TODO: buf-size < 4
            }
        } else {
            ADB_LOGI("disconnect.\n");
        }
        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    // auto write_complete_callback = [&](const hv::SocketChannelPtr& channel,
    // hv::Buffer* buf) {};
    // set_client_on_write_complete_callback(write_complete_callback);

    if (!m_tcp_client.isConnected()) {
        m_tcp_client.startConnect();
    }
    finished = 0;
    
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}
