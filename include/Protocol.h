#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <string_view>

std::string_view OKAY = "OKAY"sv;
std::string_view FAIL = "FAIL"sv;
std::string_view STAT = "STAT"sv;
std::string_view LIST = "LIST"sv;
std::string_view DENT = "DENT"sv;
std::string_view RECV = "RECV"sv;
std::string_view DATA = "DATA"sv;
std::string_view DONE = "DONE"sv;
std::string_view SEND = "SEND"sv;
std::string_view QUIT = "QUIT"sv;

// TODO: Generate this file by protobuf according to https://cs.android.com/android/platform/superproject/main/+/main:packages/modules/adb/proto/devices.proto?q=unauthorized&ss=android%2Fplatform%2Fsuperproject%2Fmain

#endif // PROTOCOL_H_