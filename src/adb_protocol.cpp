#include "adb_protocol.h"
#include "android/stringprintf.h"

/*
message layout:

| -- 4 -- | -- 4 -- |  -- len -- |
   status     len       message

*/
int read_protocol_string(hv::Buffer* ARGS_IN buf, std::string& ARGS_OUT result, std::string& ARGS_OUT error) {
    int status = 0;
    int cursor = 0;
    int buf_size = buf->size();
    if (buf_size > 4) {
        if (strncmp((char*)buf->data(), "OKAY", 4) == 0) {
            cursor += 4;  // status
            cursor += 4;  // len
            status = 0;
            result.append(std::string((char*)buf->data() + cursor));
        } else if (strncmp((char*)buf->data(), "FAIL", 4) == 0) {
            cursor += 4;  // status
            cursor += 4;  // len
            status = -1;
            error.append(std::string((char*)buf->data() + cursor));
        } else {
            // Other conditions
            // TODO: handle others ID
            // https://cs.android.com/android/platform/superproject/main/+/main:packages/modules/adb/file_sync_protocol.h?q=%22OKAY%22&ss=android%2Fplatform%2Fsuperproject%2Fmain
            cursor += 4;  // len
            result.append(std::string((char*)buf->data() + cursor));
        }
    } else if (buf_size == 4) {
        if (strncmp((char*)buf->data(), "OKAY", 4) == 0) {
            status = 0;
        } else if (strncmp((char*)buf->data(), "FAIL", 4) == 0) {
            status = -1;
        } else {
            // Other conditions
            status = 0;
        }
    } else {
        status = 0;
        result.append(std::string((char*)buf->data()));
    }

    return status;
}

int respond_transport_command(const hv::SocketChannelPtr& channel, hv::Buffer* ARGS_IN buf, std::string_view transport_cmd) {
    int status = 0;
    int buf_size = buf->size();
    if (buf_size == 4) {
        if (strncmp((char*)buf->data(), "OKAY", 4) == 0) {
            auto str = android::base::StringPrintf("%04x", transport_cmd.size()).append(transport_cmd);
            channel->write(str);
            status = 0;
        } else if (strncmp((char*)buf->data(), "FAIL", 4) == 0) {
            status = -1;
        } else {
            // Other conditions
            status = 0;
        }
    }

    return status;
}