#include "host_serial_command.h"

#include <algorithm>

#include "android/stringprintf.h"
#include "adb_protocol.h"

using namespace std::chrono_literals;

void HostSerialCommand::set_client_on_connection_callback(std::function<void(const TSocketChannelPtr&)> callback) {
    m_tcp_client.onConnection = callback;
}

void HostSerialCommand::set_client_on_message_callback(
    std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) {
    m_tcp_client.onMessage = callback;
}

void HostSerialCommand::set_client_on_write_complete_callback(
    std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) {
    m_tcp_client.onWriteComplete = callback;
}

void HostSerialCommand::defualt_on_connection_callback(const hv::SocketChannelPtr& channel) {
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

void HostSerialCommand::defualt_on_message_callback(const hv::SocketChannelPtr& channel) { /*TODO: fixme*/
}

std::string HostSerialCommand::error_message() { return m_error; }

int HostSerialCommand::execute_cmd(std::string_view cmd) { return 0; }

HostSerialCommand::HostSerialCommand() {}

HostSerialCommand::~HostSerialCommand() {}

int HostSerialCommand::forward(std::string_view serial, std::string_view local, std::string_view remote,
                               bool norebind) {
    int status = -1;

    if (norebind) {
        m_command = std::format("host-serial:{0}:forward:norebind:{1};{2}", serial, local, remote);
    } else {
        m_command = std::format("host-serial:{0}:forward:{1};{2}", serial, local, remote);
    }

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&HostSerialCommand::defualt_on_connection_callback, this, std::placeholders::_1);
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
        ADB_LOGE("Failed in execution %s\n", m_command.c_str());
    }

    return status;
}

int HostSerialCommand::list_forward(std::string_view ARGS_IN serial, std::string& ARGS_OUT forward_list) {
    int status = -1;
    m_command = std::format("host-serial:{0}:list-forward", serial);

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&HostSerialCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        status = read_protocol_string(buf, forward_list, m_error);

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGE("Failed in execution %s\n", m_command.c_str());
    }

    return status;
}

int HostSerialCommand::kill_forward(std::string_view ARGS_IN serial, std::string_view ARGS_IN local) {
    int status = -1;
    m_command = std::format("host-serial:{0}:killforward:{1}", serial, local);

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&HostSerialCommand::defualt_on_connection_callback, this, std::placeholders::_1);
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
        ADB_LOGE("Failed in execution %s\n", m_command.c_str());
    }

    return status;
}

int HostSerialCommand::kill_forward_all(std::string_view ARGS_IN serial) {
    int status = -1;
    m_command = std::format("host-serial:{0}:killforward-all", serial);

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&HostSerialCommand::defualt_on_connection_callback, this, std::placeholders::_1);
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
        ADB_LOGE("Failed in execution %s\n", m_command.c_str());
    }

    return status;
}

int HostSerialCommand::get_device_path(std::string_view ARGS_IN serial, std::string& ARGS_OUT device_path) {
    int status = -1;
    m_command = std::format("host-serial:{0}:get-devpath", serial);

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&HostSerialCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        status = read_protocol_string(buf, device_path, m_error);

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGE("Failed in execution %s\n", m_command.c_str());
    }

    return status;
}

int HostSerialCommand::get_serial_no(std::string_view ARGS_IN serial, std::string& ARGS_OUT serial_no) {
    int status = -1;
    m_command = std::format("host-serial:{0}:get-serialno", serial);

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&HostSerialCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        status = read_protocol_string(buf, serial_no, m_error);

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGE("Failed in execution %s\n", m_command.c_str());
    }

    return status;
}

int HostSerialCommand::get_state(std::string_view ARGS_IN serial, std::string& ARGS_OUT state) {
    int status = -1;
    m_command = std::format("host-serial:{0}:get-state", serial);

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&HostSerialCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        status = read_protocol_string(buf, state, m_error);

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGE("Failed in execution %s\n", m_command.c_str());
    }

    return status;
}