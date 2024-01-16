#ifndef ADB_COMMAND_H_
#define ADB_COMMAND_H_

#include <functional>
#include <string_view>

#include "libhv_evpp/TcpClient.h"
#include "libhv_evpp/TcpServer.h"
#include "utils.h"

typedef std::shared_ptr<hv::SocketChannel> TSocketChannelPtr;

class AdbCommand {
   public:
    DISALLOW_COPY_AND_ASSIGN(AdbCommand);

    AdbCommand();
    ~AdbCommand();

    virtual void set_client_on_connection_callback(std::function<void(const TSocketChannelPtr&)> callback) = 0;
    virtual void set_client_on_message_callback(
        std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) = 0;
    virtual void set_client_on_write_complete_callback(
        std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) = 0;

    virtual int execute_cmd(std::string_view cmd) = 0;

    hv::TcpClient m_tcp_client;
    hv::TcpServer m_tcp_server;
};

#endif  // ADB_COMMAND_H_