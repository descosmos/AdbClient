#ifndef HOST_COMMAND_H_
#define HOST_COMMAND_H_

#include <vector>

#include "AdbCommand.h"

class HostCommand : public AdbCommand {
   public:
    DISALLOW_COPY_AND_ASSIGN(HostCommand);

    struct DevicesInfo {
       public:
        DevicesInfo() {}

        DevicesInfo(const std::string& _serial, const std::string& _status) {
            memcpy(serial, _serial.data(), _serial.size());
            memcpy(status, _status.data(), _status.size());
        }

        DevicesInfo(const DevicesInfo& info) {
            memcpy(serial, info.serial, std::size(info.serial));
            memcpy(status, info.status, std::size(info.status));
        }

        DevicesInfo(DevicesInfo&& info) {
            memcpy(serial, info.serial, std::size(info.serial));
            memcpy(status, info.status, std::size(info.status));
        }

        char serial[16] = {0};
        char status[16] = {0};
    };

    HostCommand();
    ~HostCommand();

    void set_client_on_connection_callback(std::function<void(const TSocketChannelPtr&)> callback) override;
    void set_client_on_message_callback(std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) override;
    void set_client_on_write_complete_callback(
        std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) override;

    int execute_cmd(std::string_view cmd) override;
    int get_version(int& ARGS_OUT version);
    int get_devices(std::vector<DevicesInfo>& ARGS_OUT devices_list);
};

#endif  // HOST_COMMAND_H_