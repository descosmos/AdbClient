#include "adb_command.h"

AdbCommand::AdbCommand() {}

AdbCommand::~AdbCommand() {}

void AdbCommand::waits() {
    std::unique_lock<std::mutex> lk(m_mutex);
    m_cv.wait(lk, [&] { return m_command_finished == 1; });
}

void AdbCommand::weak_up() {
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_command_finished = 1;
    }
    m_cv.notify_all();
}

void AdbCommand::start() {
    m_command_finished = 0;
    m_tcp_client.startConnect();
    m_tcp_client.start();
}

void AdbCommand::resume() {
    m_command_finished = 0;
}