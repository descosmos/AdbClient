#ifndef LOCAL_COMMAND_H_
#define LOCAL_COMMAND_H_

#include "adb_command.h"

class LocalCommand : public AdbCommand {
   public:
    DISALLOW_COPY_AND_ASSIGN(LocalCommand);

    LocalCommand();
    ~LocalCommand();

    void set_client_on_connection_callback(std::function<void(const TSocketChannelPtr&)> callback) final;
    void set_client_on_message_callback(std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) final;
    void set_client_on_write_complete_callback(
        std::function<void(const TSocketChannelPtr&, hv::Buffer*)> callback) final;
    void defualt_on_connection_callback(const hv::SocketChannelPtr& channel) final;
    void defualt_on_message_callback(const hv::SocketChannelPtr& channel) final;
    std::string error_message() final;

    int execute_cmd(std::string_view cmd) final;
    int transport(std::string_view ARGS_IN serial);
    int shell(std::string_view ARGS_IN serial, std::string_view ARGS_IN command,
              std::string& ARGS_OUT data /*TODO: handle to for shell*/);  // TODO: fixme
    int sync(std::string_view ARGS_IN serial);
    int screencap(std::string_view ARGS_IN serial, std::string& ARGS_OUT data);
    int list_packages(std::string_view ARGS_IN serial, std::string& ARGS_OUT packages);
    int tcpip(std::string_view ARGS_IN serial, uint32_t ARGS_IN port);
    int usb(std::string_view ARGS_IN serial);
    int logcat();
    int get_properties(std::string_view ARGS_IN serial, std::string& ARGS_OUT properties);
    int root(std::string_view ARGS_IN serial);
    int reverse(std::string_view serial, std::string_view local, std::string_view remote, bool norebind = false);
    int list_reverse(std::string_view ARGS_IN serial, std::string& ARGS_OUT forward_list);
    int kill_reverse(std::string_view ARGS_IN serial, std::string_view ARGS_IN local);
    int kill_reverse_all(std::string_view ARGS_IN serial);

   private:
    std::string m_serial;
};

#endif  // LOCAL_COMMAND_H_