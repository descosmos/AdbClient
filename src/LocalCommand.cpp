#include "LocalCommand.h"

#include <algorithm>

#include "Protocol.h"
#include "android/stringprintf.h"

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

int LocalCommand::execute_cmd(std::string_view cmd) { return 0; }

int LocalCommand::transport(std::string_view ARGS_IN serial) {
    std::string cmd = std::format("host:transport:{0}", serial);

    int status = -1;

    auto connection_callback = [&](const hv::SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            auto str = android::base::StringPrintf("%04x", cmd.length()).append(cmd);
            channel->write(str);
            ADB_LOGI("connect to %s! connfd=%d\n", peeraddr.c_str(), channel->fd());
        } else {
            ADB_LOGI("disconnected to %s! connfd=%d\n", peeraddr.c_str(), channel->fd());
            // weak_up();
        }
        if (m_tcp_client.isReconnect()) {
            ADB_LOGI("reconnect cnt=%d, delay=%d\n", m_tcp_client.reconn_setting->cur_retry_cnt,
                     m_tcp_client.reconn_setting->cur_delay);
        }
    };
    set_client_on_connection_callback(connection_callback);

    auto message_callback = [&](const hv::SocketChannelPtr& channel, hv::Buffer* buf) {
        ADB_LOGI("buf->data(): %s\n", (char*)buf->data());
        if (buf->size() == 4 && strcmp((char*)buf->data(), "OKAY") == 0) {
            cmd = std::format("shell:{0}", "getprop ro.build.version.release");
            auto str = android::base::StringPrintf("%04x", cmd.size()).append(cmd);
            channel->write(str);
            status = 0;
        } else {
            weak_up();
        }

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    // auto write_complete_callback = [&](const hv::SocketChannelPtr& channel,
    // hv::Buffer* buf) {};
    // set_client_on_write_complete_callback(write_complete_callback);

    m_command_finished = 0;

    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();
    // while (true) {
    //     waits();
    //     m_command_finished = 0;
    // }

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}

void get_lines_from_buf(std::vector<std::string>& ARGS_OUT lines, const std::string& ARGS_IN buf) {
    std::vector<std::string> lines_tmp = string_split(buf, '\n');
    lines_tmp.pop_back();  // pop null

#ifdef ADB_DEBUG
    ADB_LOGI("lines_tmp.size: %d\n", lines_tmp.size());
#endif

    for (auto& line : lines_tmp) {
        lines.push_back(line);
    }
}

int LocalCommand::shell(std::string_view ARGS_IN serial, std::string_view ARGS_IN command,
                        std::vector<std::string>& ARGS_OUT lines) {
    std::string cmd = std::format("shell:{0}", command);
    int status = -1;

    auto connection_callback = [&](const hv::SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            std::string transport_cmd = std::format("host:transport:{0}", serial);
            auto str = android::base::StringPrintf("%04x", transport_cmd.length()).append(transport_cmd);
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
            std::string tmp = std::string((char*)buf->data());
            unique_spaces(tmp);
            get_lines_from_buf(lines, tmp);
        }

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    // auto write_complete_callback = [&](const hv::SocketChannelPtr& channel,
    // hv::Buffer* buf) {};
    // set_client_on_write_complete_callback(write_complete_callback);

    m_command_finished = 0;

    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}

int LocalCommand::sync(std::string_view ARGS_IN serial) {
    // Detail about [adb sync]: https://android.googlesource.com/platform/packages/modules/adb/+/HEAD/SYNC.TXT
    std::string_view cmd = "sync:";
    int status = -1;

    auto connection_callback = [&](const hv::SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            std::string transport_cmd = std::format("host:transport:{0}", serial);
            auto str = android::base::StringPrintf("%04x", transport_cmd.length()).append(transport_cmd);
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
            // TODO:
        }

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    // auto write_complete_callback = [&](const hv::SocketChannelPtr& channel,
    // hv::Buffer* buf) {};
    // set_client_on_write_complete_callback(write_complete_callback);

    m_command_finished = 0;

    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}

int LocalCommand::screencap(std::string_view ARGS_IN serial, std::string& ARGS_OUT data) {
    std::string_view cmd = "shell:/system/bin/screencap -p";
    int status = -1;

    auto connection_callback = [&](const hv::SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            std::string transport_cmd = std::format("host:transport:{0}", serial);
            auto str = android::base::StringPrintf("%04x", transport_cmd.length()).append(transport_cmd);
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

    // auto write_complete_callback = [&](const hv::SocketChannelPtr& channel,
    // hv::Buffer* buf) {};
    // set_client_on_write_complete_callback(write_complete_callback);

    m_command_finished = 0;

    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}

int LocalCommand::list_packages(std::string_view ARGS_IN serial, std::vector<std::string>& ARGS_OUT lines) {
    std::string_view command = "pm list packages";
    return shell(serial, command, lines);
}

int LocalCommand::tcpip(std::string_view ARGS_IN serial, uint32_t ARGS_IN port) {
    std::string cmd = std::format("tcpip:{0}", port);
    int status = -1;

    auto connection_callback = [&](const hv::SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            std::string transport_cmd = std::format("host:transport:{0}", serial);
            auto str = android::base::StringPrintf("%04x", transport_cmd.length()).append(transport_cmd);
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
            // TODO:
        }

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    // auto write_complete_callback = [&](const hv::SocketChannelPtr& channel,
    // hv::Buffer* buf) {};
    // set_client_on_write_complete_callback(write_complete_callback);

    m_command_finished = 0;

    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}

int LocalCommand::usb(std::string_view ARGS_IN serial) {
    std::string_view cmd = "usb:";
    int status = -1;

    auto connection_callback = [&](const hv::SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            std::string transport_cmd = std::format("host:transport:{0}", serial);
            auto str = android::base::StringPrintf("%04x", transport_cmd.length()).append(transport_cmd);
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
            // TODO:
        }

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    // auto write_complete_callback = [&](const hv::SocketChannelPtr& channel,
    // hv::Buffer* buf) {};
    // set_client_on_write_complete_callback(write_complete_callback);

    m_command_finished = 0;

    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}

int LocalCommand::logcat() { return 0; }

int LocalCommand::get_properties(std::string_view ARGS_IN serial, std::vector<std::string>& ARGS_OUT lines) {
    std::string_view command = "getprop";
    return shell(serial, command, lines);
}

int LocalCommand::root(std::string_view ARGS_IN serial) {
    std::string_view cmd = "root:";
    int status = -1;

    auto connection_callback = [&](const hv::SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            std::string transport_cmd = std::format("host:transport:{0}", serial);
            auto str = android::base::StringPrintf("%04x", transport_cmd.length()).append(transport_cmd);
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
            // TODO:
        }

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    // auto write_complete_callback = [&](const hv::SocketChannelPtr& channel,
    // hv::Buffer* buf) {};
    // set_client_on_write_complete_callback(write_complete_callback);

    m_command_finished = 0;

    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}

int LocalCommand::reverse(std::string_view serial, std::string_view local, std::string_view remote, bool norebind) {
    std::string cmd;
    if (norebind) {
        cmd = std::format("reverse:forward:norebind:{0};{1}", local, remote);
    } else {
        cmd = std::format("reverse:forward:{0};{1}", local, remote);
    }
    int status = -1;

    auto connection_callback = [&](const hv::SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            std::string transport_cmd = std::format("host:transport:{0}", serial);
            auto str = android::base::StringPrintf("%04x", transport_cmd.length()).append(transport_cmd);
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
            // TODO:
        }

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    // auto write_complete_callback = [&](const hv::SocketChannelPtr& channel,
    // hv::Buffer* buf) {};
    // set_client_on_write_complete_callback(write_complete_callback);

    m_command_finished = 0;

    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}

void get_reverse_list_from_buf(std::vector<std::string>& ARGS_OUT forward_list, const std::string& ARGS_IN buf) {
    std::vector<std::string> forward_list_tmp = string_split(buf, '\n');
    forward_list_tmp.pop_back();  // pop null

#ifdef ADB_DEBUG
    ADB_LOGI("forward_list_tmp.size: %d\n", forward_list_tmp.size());
#endif

    for (auto& forward : forward_list_tmp) {
#ifdef ADB_DEBUG
        ADB_LOGI("forward_list_tmp it: %s\n", forward.c_str());
#endif
        forward_list.push_back(forward);
    }
}

int LocalCommand::list_reverse(std::string_view ARGS_IN serial, std::vector<std::string>& ARGS_OUT forward_list) {
    std::string_view cmd = "reverse:list-forward";
    int status = -1;

    auto connection_callback = [&](const hv::SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            std::string transport_cmd = std::format("host:transport:{0}", serial);
            auto str = android::base::StringPrintf("%04x", transport_cmd.length()).append(transport_cmd);
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
        if (buf->size() == 4) {
            if (strcmp((char*)buf->data(), "OKAY") == 0) {
                auto str = android::base::StringPrintf("%04x", cmd.size()).append(cmd);
                channel->write(str);
                status = 0;
            } else if (strcmp((char*)buf->data(), "FAIL") == 0) {
                status = -1;
            } else {
                status = 0;
                // TODO: handle others ID
                // https://cs.android.com/android/platform/superproject/main/+/main:packages/modules/adb/file_sync_protocol.h?q=%22OKAY%22&ss=android%2Fplatform%2Fsuperproject%2Fmain
            }
        } else if (buf->size() > 4) {
            if (strstr((char*)buf->data(), "OKAY") != NULL) {
                get_reverse_list_from_buf(forward_list, std::string((char*)buf->data() + 8));
            } else {
                get_reverse_list_from_buf(forward_list, std::string((char*)buf->data() + 4));
            }
        } else {
            // TODO:
        }

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    // auto write_complete_callback = [&](const hv::SocketChannelPtr& channel,
    // hv::Buffer* buf) {};
    // set_client_on_write_complete_callback(write_complete_callback);

    m_command_finished = 0;

    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}

int LocalCommand::kill_reverse(std::string_view ARGS_IN serial, std::string_view ARGS_IN local) {
    std::string cmd = std::format("reverse:killforward:{0}", local);
    int status = -1;

    auto connection_callback = [&](const hv::SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            std::string transport_cmd = std::format("host:transport:{0}", serial);
            auto str = android::base::StringPrintf("%04x", transport_cmd.length()).append(transport_cmd);
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
            // TODO:
        }

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    // auto write_complete_callback = [&](const hv::SocketChannelPtr& channel,
    // hv::Buffer* buf) {};
    // set_client_on_write_complete_callback(write_complete_callback);

    m_command_finished = 0;

    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}

int LocalCommand::kill_reverse_all(std::string_view ARGS_IN serial) {
    std::string_view cmd = "reverse:killforward-all";
    int status = -1;

    auto connection_callback = [&](const hv::SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            std::string transport_cmd = std::format("host:transport:{0}", serial);
            auto str = android::base::StringPrintf("%04x", transport_cmd.length()).append(transport_cmd);
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
            // TODO:
        }

        memset(buf->data(), 0, buf->size());
    };
    set_client_on_message_callback(message_callback);

    // auto write_complete_callback = [&](const hv::SocketChannelPtr& channel,
    // hv::Buffer* buf) {};
    // set_client_on_write_complete_callback(write_complete_callback);

    m_command_finished = 0;

    m_tcp_client.startConnect();
    m_tcp_client.start();

    waits();

    if (status == -1) {
        ADB_LOGI("%s %s(%u): execute_cmd Failed\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return status;
}