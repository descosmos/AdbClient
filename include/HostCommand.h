#ifndef HOST_COMMAND_H_
#define HOST_COMMAND_H_

#include <vector>

#include "AdbCommand.h"

class HostCommand : public AdbCommand {
   public:
    DISALLOW_ASSIGN(HostCommand);

    struct DevicesInfo {
       public:
        DevicesInfo() {}

        DevicesInfo(const std::string& _serial, const std::string& _status) {
            memcpy(serial, _serial.data(), _serial.size());
            memcpy(status, _status.data(), _status.size());
        }

        DevicesInfo(const std::string& _serial, const std::string& _status, const std::string& _product,
                    const std::string& _model, const std::string& _device, const std::string& _transport_id) {
            memcpy(serial, _serial.data(), _serial.size());
            memcpy(status, _status.data(), _status.size());
            memcpy(product, _product.data(), _product.size());
            memcpy(model, _model.data(), _model.size());
            memcpy(device, _device.data(), _device.size());
            memcpy(transport_id, _transport_id.data(), _transport_id.size());
        }

        DevicesInfo(const DevicesInfo& info) {
            memcpy(serial, info.serial, std::size(info.serial));
            memcpy(status, info.status, std::size(info.status));
            memcpy(product, info.product, std::size(info.product));
            memcpy(model, info.model, std::size(info.model));
            memcpy(device, info.device, std::size(info.device));
            memcpy(transport_id, info.transport_id, std::size(info.transport_id));
        }

        DevicesInfo(DevicesInfo&& info) {
            memcpy(serial, info.serial, std::size(info.serial));
            memcpy(status, info.status, std::size(info.status));
            memcpy(product, info.product, std::size(info.product));
            memcpy(model, info.model, std::size(info.model));
            memcpy(device, info.device, std::size(info.device));
            memcpy(transport_id, info.transport_id, std::size(info.transport_id));
        }

        char serial[16] = {0};
        char status[16] = {0};
        char product[16] = {0};
        char model[16] = {0};
        char device[16] = {0};
        char transport_id[16] = {0};
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
    int get_devices_with_path(std::vector<DevicesInfo>& ARGS_OUT devices_list);
    int kill();
    
    // TODO: fixme
    int track_devices();
    int connect();
    int disconnect();
};

#endif  // HOST_COMMAND_H_