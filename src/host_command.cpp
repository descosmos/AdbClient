#include "host_command.h"

#include <algorithm>

#include "android/stringprintf.h"
#include "protocol.h"

using namespace std::chrono_literals;

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

void HostCommand::defualt_on_connection_callback(const hv::SocketChannelPtr& channel) {
    std::string peeraddr = channel->peeraddr();
    if (channel->isConnected()) {
        auto str = android::base::StringPrintf("%04x", m_command.length()).append(m_command);
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
}

void HostCommand::defualt_on_message_callback(const hv::SocketChannelPtr& channel) { /*TODO: fixme*/
}

std::string HostCommand::error_message() { return m_error; }

int HostCommand::execute_cmd(std::string_view cmd) { return 0; }

int HostCommand::get_version(int& ARGS_OUT version) {
    m_command = STRING_CONCAT("host", ":version");
    int status = -1;

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&HostCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        std::string data;
        status = read_protocol_string(buf, data, m_error);
        if (status != -1) {
            version = std::stoi(data, nullptr, 16);
        }

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}

int HostCommand::get_devices(std::string& ARGS_OUT devices_list) {
    m_command = STRING_CONCAT("host", ":devices");
    int status = 0;

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&HostCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        status = read_protocol_string(buf, devices_list, m_error);

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}

int HostCommand::get_devices_with_path(std::string& ARGS_OUT devices_list) {
    m_command = STRING_CONCAT("host", ":devices-l");
    int status = -1;

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&HostCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        status = read_protocol_string(buf, devices_list, m_error);
        if (status != -1) {
            unique_spaces(devices_list);
        }

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}

int HostCommand::kill() {
    m_command = STRING_CONCAT("host", ":kill");
    int status = -1;

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&HostCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        std::string data;
        status = read_protocol_string(buf, data, m_error);

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}

int HostCommand::track_devices() {
    m_command = STRING_CONCAT("host", ":track-devices");
    int status = -1;

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&HostCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        if (buf->size() > 4) {
            if (strstr((char*)buf->data(), "OKAY") != NULL) {
                status = 0;
            } else {
                auto vec = string_split(std::string((char*)(buf->data())), '\t');
                printf("device: %s   state: %s", vec[0].c_str(), vec[1].c_str());
                status = 0;
            }
        } else if (buf->size() == 4) {
            if (strcmp((char*)buf->data(), "OKAY") == 0) {
                status = 0;
            } else {
                status = -1;
            }
        } else {
            // TODO: buf-size < 4
        }

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    while (true) {
        waits();
        m_command_finished = 0;
    }

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}

int HostCommand::connect(std::string_view ARGS_IN host, std::string_view ARGS_IN port) {
    m_command = std::format("host:connect:{0}:{1}", host, port);
    int status = -1;

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&HostCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        std::string data;
        status = read_protocol_string(buf, data, m_error);

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}

int HostCommand::disconnect(std::string_view ARGS_IN host, std::string_view ARGS_IN port) {
    m_command = std::format("host:disconnect:{0}:{1}", host, port);
    int status = -1;

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&HostCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        std::string data;
        status = read_protocol_string(buf, data, m_error);

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}