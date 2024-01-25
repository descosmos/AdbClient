/**
 * The file is based on https://cs.android.com/android/platform/superproject/main/+/main:packages/modules/adb/proto/devices.proto.
 * https://cs.android.com/android/platform/superproject/main/+/main:system/core/diagnose_usb/diagnose_usb.cpp
*/

#ifndef DEVICEINFO_H_
#define DEVICEINFO_H_

#include <string>
#include <ctype.h>

enum ConnectionState {
    kCsAny = -1,

    kCsConnecting = 0,  // Haven't received a response from the device yet.
    kCsAuthorizing,     // Authorizing with keys from ADB_VENDOR_KEYS.
    kCsUnauthorized,    // ADB_VENDOR_KEYS exhausted, fell back to user prompt.
    kCsNoPerm,          // Insufficient permissions to communicate with the device.
    kCsDetached,        // USB device that's detached from the adb server.
    kCsOffline,

    // After CNXN packet, the ConnectionState describes not a state but the type of service
    // on the other end of the transport.
    kCsBootloader,  // Device running fastboot OS (fastboot) or userspace fastboot (fastbootd).
    kCsDevice,      // Device running Android OS (adbd).
    kCsHost,        // What a device sees from its end of a Transport (adb host).
    kCsRecovery,    // Device with bootloader loaded but no ROM OS loaded (adbd).
    kCsSideload,    // Device running Android OS Sideload mode (minadbd sideload mode).
    kCsRescue,      // Device running Android OS Rescue mode (minadbd rescue mode).
};

enum ConnectionType {
    UNKNOWN = 0,
    USB = 1,
    WIFI_SOCKET = 2
};

struct DeviceInfo {
    std::string serial;
    std::string state;  //  changed the type of ConnectionState to std::string
    std::string bus_address;
    std::string product;
    std::string model;
    std::string device;
    ConnectionType connection_type = UNKNOWN;
    int64_t negotiated_speed;
    int64_t max_speed;
    int64_t transport_id = -1;
};

std::string state_to_string(ConnectionState state);

#endif // DEVICEINFO_H_