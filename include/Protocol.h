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


#endif // PROTOCOL_H_