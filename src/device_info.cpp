#include "device_info.h"

std::string state_to_string(ConnectionState state) {
    switch (state) {
        case kCsOffline:
            return "offline";
        case kCsBootloader:
            return "bootloader";
        case kCsDevice:
            return "device";
        case kCsHost:
            return "host";
        case kCsRecovery:
            return "recovery";
        case kCsRescue:
            return "rescue";
        case kCsNoPerm:
            return "no permissions";    // changed
        case kCsSideload:
            return "sideload";
        case kCsUnauthorized:
            return "unauthorized";
        case kCsAuthorizing:
            return "authorizing";
        case kCsConnecting:
            return "connecting";
        default:
            return "unknown";
    }
}