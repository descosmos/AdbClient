#include "AdbCommand.h"

AdbCommand::AdbCommand() {}

AdbCommand::~AdbCommand() {}

int AdbCommand::create_socket(int remote_port, const char* remote_addr) {
    return m_tcp_client.createsocket(remote_port, remote_addr);
}

void AdbCommand::close_socket() {
    m_tcp_client.closesocket();
}

void AdbCommand::on_connection() {}

void AdbCommand::on_message() {}