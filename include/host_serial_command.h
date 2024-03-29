#ifndef HOST_SERICAL_COMMAND_H_
#define HOST_SERICAL_COMMAND_H_

#include "adb_command.h"

class HostSerialCommand : public AdbCommand {
   public:
    DISALLOW_COPY_AND_ASSIGN(HostSerialCommand);

    HostSerialCommand();
    ~HostSerialCommand();

    void set_client_on_connection_callback(std::function<void(const TSocketChannelPtr&)> callback) final;
    void set_client_on_message_callback(std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) final;
    void set_client_on_write_complete_callback(
        std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) final;
    void defualt_on_connection_callback(const hv::SocketChannelPtr& channel) final;
    void defualt_on_message_callback(const hv::SocketChannelPtr& channel) final;
    std::string error_message() final;

    int execute_cmd(std::string_view cmd) final;
    int forward(std::string_view serial, std::string_view local, std::string_view remote, bool norebind = false);
    int list_forward(std::string_view ARGS_IN serial, std::string& ARGS_OUT forward_list);
    int kill_forward(std::string_view ARGS_IN serial, std::string_view ARGS_IN local);
    int kill_forward_all(std::string_view ARGS_IN serial);
    int get_device_path(std::string_view ARGS_IN serial, std::string& ARGS_OUT device_path);
    int get_serial_no(std::string_view ARGS_IN serial, std::string& ARGS_OUT serial_no);
    int get_state(std::string_view ARGS_IN serial, std::string& ARGS_OUT state);
};

#endif  // HOST_SERICAL_COMMAND_H_