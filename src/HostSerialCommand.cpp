#include "HostSerialCommand.h"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <format>
#include <thread>

#include "android/stringprintf.h"

#define STRING_CONCAT(a, b) a##b

using namespace std::chrono_literals;

static std::condition_variable cv;
static std::mutex _mutex;
static int finished = false;

static void waits() {
    std::unique_lock<std::mutex> lk(_mutex);
    cv.wait(lk, [&] { return finished == 1; });
}

static void weak_up() {
    {
        std::lock_guard<std::mutex> lk(_mutex);
        finished = 1;
    }
    cv.notify_all();
}

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

int HostSerialCommand::execute_cmd(std::string_view cmd) { return 0; }

HostSerialCommand::HostSerialCommand() {}

HostSerialCommand::~HostSerialCommand() {}

int HostSerialCommand::forward(std::string_view serial, std::string_view local, std::string_view remote,
                               bool norebind) {
    std::string cmd = std::format("host-serial:{0}:forward:norebind:{1};{2}", serial, local, remote);

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
                status = 0;
            } else {
                status = -1;
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

void get_forward_list_from_buf(std::vector<std::string>& ARGS_OUT forward_list, const std::string& ARGS_IN buf) {
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

int HostSerialCommand::list_forward(std::string_view ARGS_IN serial, std::vector<std::string>& ARGS_OUT forward_list) {
    std::string cmd = std::format("host-serial:{0}:list-forward", serial);
    ADB_LOGI("cmd: %s\n", cmd.c_str());
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
                    get_forward_list_from_buf(forward_list, std::string((char*)buf->data() + 8));
                } else {
                    get_forward_list_from_buf(forward_list, std::string((char*)buf->data() + 4));
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

int HostSerialCommand::kill_forward(std::string_view ARGS_IN serial, std::string_view ARGS_IN local) {
    std::string cmd = std::format("host-serial:{0}:killforward:{1}", serial, local);

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
                status = 0;
            } else {
                status = -1;
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

int HostSerialCommand::kill_forward_all(std::string_view ARGS_IN serial) {
    std::string cmd = std::format("host-serial:{0}:killforward-all", serial);

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
                status = 0;
            } else {
                status = -1;
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

int HostSerialCommand::get_device_path(std::string_view ARGS_IN serial, std::string& ARGS_OUT device_path) {
    std::string cmd = std::format("host-serial:{0}:get-devpath", serial);
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
                device_path = data;
            } else {
                std::string data = (char*)buf->data() + 4;
                device_path = data;
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

int HostSerialCommand::get_serial_no(std::string_view ARGS_IN serial, std::string& ARGS_OUT serial_no) {
    std::string cmd = std::format("host-serial:{0}:get-serialno", serial);
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
                serial_no = data;
            } else {
                std::string data = (char*)buf->data() + 4;
                serial_no = data;
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

int HostSerialCommand::get_state(std::string_view ARGS_IN serial, std::string& ARGS_OUT state) {
    std::string cmd = std::format("host-serial:{0}:get-state", serial);
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
                state = data;
            } else {
                std::string data = (char*)buf->data() + 4;
                state = data;
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