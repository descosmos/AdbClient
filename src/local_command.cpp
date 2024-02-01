#include "local_command.h"

#include <algorithm>

#include "android/stringprintf.h"
#include "protocol.h"

using namespace std::chrono_literals;

LocalCommand::LocalCommand() {}

LocalCommand::~LocalCommand() {}

void LocalCommand::set_client_on_connection_callback(std::function<void(const TSocketChannelPtr&)> callback) {
    m_tcp_client.onConnection = callback;
}

void LocalCommand::set_client_on_message_callback(std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) {
    m_tcp_client.onMessage = callback;
}

void LocalCommand::set_client_on_write_complete_callback(
    std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) {
    m_tcp_client.onWriteComplete = callback;
}

void LocalCommand::defualt_on_connection_callback(const hv::SocketChannelPtr& channel) {
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

void LocalCommand::defualt_on_message_callback(const hv::SocketChannelPtr& channel) { /*TODO: fixme*/ }

std::string LocalCommand::error_message() { return m_error; }

int LocalCommand::execute_cmd(std::string_view cmd) { return 0; }

int LocalCommand::transport(std::string_view ARGS_IN serial) { return -1; /* Remove it temporarily */ }

int LocalCommand::shell(std::string_view ARGS_IN serial, std::string_view ARGS_IN command) {
    int status = 0;
    std::string cmd = std::format("shell:{0}", command);
    m_command = std::format("host:transport:{0}", serial);

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&LocalCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        status = respond_transport_command(channel, buf, cmd);
        if (status != -1 && strncmp((char*)buf->data(), "OKAY", 4) != 0) {
            m_shell_data = std::string((char*)buf->data());
        }

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGE("Failed in execution %s\n", cmd.data());
    }

    return status;
}

int LocalCommand::sync(std::string_view ARGS_IN serial) {
    // Detail about [adb sync]: https://android.googlesource.com/platform/packages/modules/adb/+/HEAD/SYNC.TXT
    int status = -1;
    std::string cmd = "sync:";
    m_command = std::format("host:transport:{0}", serial);

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&LocalCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        status = respond_transport_command(channel, buf, cmd);
        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGE("Failed in execution %s\n", cmd.data());
    }

    return status;
}

int LocalCommand::screencap(std::string_view ARGS_IN serial, std::string& ARGS_OUT data) {
    int status = -1;
    std::string cmd = "shell:/system/bin/screencap -p";
    m_command = std::format("host:transport:{0}", serial);

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&LocalCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data().size: %d\n", (char*)buf->size());
        if (buf->size() == 4) {
            if (strcmp((char*)buf->data(), "OKAY") == 0) {
                auto str = android::base::StringPrintf("%04x", cmd.size()).append(cmd);
                channel->write(str);
                status = 0;
            } else if (strcmp((char*)buf->data(), "FAIL")) {
                status = -1;
            } else {
                // TODO: handle others ID
                // https://cs.android.com/android/platform/superproject/main/+/main:packages/modules/adb/file_sync_protocol.h?q=%22OKAY%22&ss=android%2Fplatform%2Fsuperproject%2Fmain
            }
        } else {
            std::string payload((char*)buf->data(), (char*)buf->data() + buf->size());
            data.append(payload);
        }

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGE("Failed in execution %s\n", cmd.data());
    }

    return status;
}

int LocalCommand::list_packages(std::string_view ARGS_IN serial, std::string& ARGS_OUT packages) {
    std::string_view command = "pm list packages";
    int status = shell(serial, command);
    packages = m_shell_data;
    return status;
}

int LocalCommand::tcpip(std::string_view ARGS_IN serial, uint32_t ARGS_IN port) {
    int status = -1;
    std::string cmd = std::format("tcpip:{0}", port);
    m_command = std::format("host:transport:{0}", serial);

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&LocalCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        status = respond_transport_command(channel, buf, cmd);
        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGE("Failed in execution %s\n", cmd.data());
    }

    return status;
}

int LocalCommand::usb(std::string_view ARGS_IN serial) {
    int status = -1;
    std::string_view cmd = "usb:";
    m_command = std::format("host:transport:{0}", serial);

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&LocalCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        status = respond_transport_command(channel, buf, cmd);
        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGE("Failed in execution %s\n", cmd.data());
    }

    return status;
}

int LocalCommand::logcat(std::string_view serial) {
    std::string_view command = "logcat";
    int status = shell(serial, command);
    return status;
}

int LocalCommand::get_properties(std::string_view ARGS_IN serial, std::string& ARGS_OUT properties) {
    std::string_view command = "getprop";
    int status = shell(serial, command);
    properties = m_shell_data;
    return status;
}

int LocalCommand::root(std::string_view ARGS_IN serial) {
    int status = -1;
    std::string_view cmd = "root:";
    m_command = std::format("host:transport:{0}", serial);

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&LocalCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        status = respond_transport_command(channel, buf, cmd);
        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGE("Failed in execution %s\n", cmd.data());
    }

    return status;
}

int LocalCommand::reverse(std::string_view serial, std::string_view local, std::string_view remote, bool norebind) {
    int status = -1;
    std::string cmd;
    if (norebind) {
        cmd = std::format("reverse:forward:norebind:{0};{1}", local, remote);
    } else {
        cmd = std::format("reverse:forward:{0};{1}", local, remote);
    }
    m_command = std::format("host:transport:{0}", serial);

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&LocalCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        status = respond_transport_command(channel, buf, cmd);
        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGE("Failed in execution %s\n", cmd.data());
    }

    return status;
}

int LocalCommand::list_reverse(std::string_view ARGS_IN serial, std::string& ARGS_OUT forward_list) {
    int status = -1;
    std::string_view cmd = "reverse:list-forward";
    m_command = std::format("host:transport:{0}", serial);

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&LocalCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        status = respond_transport_command(channel, buf, cmd);
        if (status != -1) {
            if (strstr((char*)buf->data(), "OKAY") != NULL) {
                forward_list.append(std::string((char*)buf->data() + 8));
            } else {
                forward_list.append(std::string((char*)buf->data() + 4));
            }
        }

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGE("Failed in execution %s\n", cmd.data());
    }

    return status;
}

int LocalCommand::kill_reverse(std::string_view ARGS_IN serial, std::string_view ARGS_IN local) {
    int status = -1;
    std::string cmd = std::format("reverse:killforward:{0}", local);
    m_command = std::format("host:transport:{0}", serial);

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&LocalCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        status = respond_transport_command(channel, buf, cmd);
        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGE("Failed in execution %s\n", cmd.data());
    }

    return status;
}

int LocalCommand::kill_reverse_all(std::string_view ARGS_IN serial) {
    int status = -1;
    std::string_view cmd = "reverse:killforward-all";
    m_command = std::format("host:transport:{0}", serial);

    std::function<void(const TSocketChannelPtr&)> func =
        std::bind(&LocalCommand::defualt_on_connection_callback, this, std::placeholders::_1);
    set_client_on_connection_callback(func);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        status = respond_transport_command(channel, buf, cmd);
        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGE("Failed in execution %s\n", cmd.data());
    }

    return status;
}

std::string LocalCommand::get_shell_data() { return m_shell_data; }