#ifndef HOST_COMMAND_H_
#define HOST_COMMAND_H_

#include <vector>

#include "adb_command.h"
#include "device_info.h"

class HostCommand : public AdbCommand {
   public:
    DISALLOW_ASSIGN(HostCommand);

    HostCommand();
    ~HostCommand();

    void set_client_on_connection_callback(std::function<void(const TSocketChannelPtr&)> callback) final;
    void set_client_on_message_callback(std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) final;
    void set_client_on_write_complete_callback(
        std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) final;
    void defualt_on_connection_callback(const hv::SocketChannelPtr& channel) final;
    void defualt_on_message_callback(const hv::SocketChannelPtr& channel) final;
    std::string error_message() final;

    int execute_cmd(std::string_view cmd) final;
    int get_version(int& ARGS_OUT version);
    int get_devices(std::string& ARGS_OUT devices_list);
    int get_devices_with_path(std::string& ARGS_OUT devices_list);
    int kill();
    int connect(std::string_view ARGS_IN host, std::string_view ARGS_IN port);
    int disconnect(std::string_view ARGS_IN host, std::string_view ARGS_IN port);
    int track_devices();

    std::string get_tracked_devices();

   private:
    std::string m_tracked_device;
};

#endif  // HOST_COMMAND_H_