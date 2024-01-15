#ifndef HOST_COMMAND_H_
#define HOST_COMMAND_H_

#include "AdbCommand.h"
#include <vector>


class HostCommand : public AdbCommand {
public:
    DISALLOW_COPY_AND_ASSIGN(HostCommand);
    
    struct DevicesInfo {
        char serial[16];
        char status[16];
    };

    HostCommand();
    ~HostCommand();

    void set_client_on_connection_callback(std::function<void(const TSocketChannelPtr&)> callback) override;
    void set_client_on_message_callback(std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) override;
    void set_client_on_write_complete_callback(std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) override;

    int execute_cmd(std::string_view cmd) override;
    int get_version(std::string& ARGS_OUT res);
    int get_devices(std::vector<DevicesInfo>& ARGS_OUT devices_list);

};


#endif // HOST_COMMAND_H_