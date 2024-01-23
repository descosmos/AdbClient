#ifndef LOCAL_COMMAND_H_
#define LOCAL_COMMAND_H_

#include "AdbCommand.h"

class LocalCommand : public AdbCommand {
   public:
    DISALLOW_COPY_AND_ASSIGN(LocalCommand);

    LocalCommand();
    ~LocalCommand();

    void set_client_on_connection_callback(std::function<void(const TSocketChannelPtr&)> callback) override;
    void set_client_on_message_callback(std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) override;
    void set_client_on_write_complete_callback(
        std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) override;

    int execute_cmd(std::string_view cmd) override;
    int transport(std::string_view ARGS_IN serial);
    int shell(std::string_view ARGS_IN serial, std::string_view ARGS_IN command,
              std::vector<std::string>& ARGS_OUT lines /*TODO: handle to for shell*/);  // TODO: fixme
    int sync();
    int screencap();
    int list_packages(std::string_view ARGS_IN serial, std::vector<std::string>& ARGS_OUT lines);
    int local();
    int tcpip();
    int logcat();
    int get_properties(std::string_view ARGS_IN serial, std::vector<std::string>& ARGS_OUT lines);
};

#endif  // LOCAL_COMMAND_H_