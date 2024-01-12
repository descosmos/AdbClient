#ifndef ADB_COMMAND_H_
#define ADB_COMMAND_H_

#include "utils.h"
#include "libhv_evpp/TcpServer.h"
#include "libhv_evpp/TcpClient.h"

class AdbCommand {
public:
    DISALLOW_COPY_AND_ASSIGN(AdbCommand);
    
    AdbCommand();
    ~AdbCommand();

    int create_socket(int remote_port, const char* remote_addr);
    void close_socket();
    void on_connection();   // TODO
    void on_message();  // TODO


    hv::TcpClient m_tcp_client;
    hv::TcpServer m_tcp_server;
private:
};

#endif // ADB_COMMAND_H_