// #pragma once
#ifndef UTILS_H_
#define UTILS_H_

#include "device_info.h"

#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#ifndef NDEBUG
#define ADB_DEBUG
#endif // NDEBUG

#define DISALLOW_ASSIGN(TypeName)              \
    void operator=(const TypeName &) = delete; \
    void operator=(TypeName &&) = delete;

#define DISALLOW_COPY_AND_ASSIGN(TypeName)     \
    void operator=(const TypeName &) = delete; \
    void operator=(TypeName &&) = delete;      \
    TypeName(const TypeName &) = delete;       \
    TypeName(TypeName &&) = delete

// https://stackoverflow.com/questions/1537964/visual-c-equivalent-of-gccs-attribute-packed
#ifdef _MSC_VER
#define PACK(__Declaration__) __pragma(pack(push, 1)) __Declaration__ __pragma(pack(pop))
#else
#define PACK(__Declaration__) __Declaration__ __attribute__((__packed__))
#endif

#ifndef ARGS_ATTR
#if defined(__clang__)
#define ARGS_OUT
#define ARGS_IN
#elif defined(__GNUC__) || defined(__GNUG__)
#define ARGS_OUT
#define ARGS_IN
#elif defined(_MSC_VER)
#define ARGS_OUT _Out_
#define ARGS_IN _In_
#endif

#endif  // ARGS_ATTR

#ifndef ADB_LOG

#ifdef ADB_DEBUG
#define ADB_LOGI(fmt, ...)                                              \
    do {                                                                \
        printf("%s (%I32d) : " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
        fflush(stdout);                                                 \
    } while (0)

#else 
#define ADB_LOGI(fmt, ...)                                              \
    do {} while (0)

#endif // ADB_DEBUG




#endif  // ADB_LOG

#ifndef STRING_CONCAT
#define STRING_CONCAT(a, b) a##b
#endif  // STRING_CONCAT

constexpr int DEFAULT_ADB_PORT = 5037;

std::vector<std::string> string_split(const std::string &str, char delimiter);
void unique_spaces(std::string &str);
void get_device_info_from_buf(std::vector<DeviceInfo>& ARGS_OUT devices_list, const std::string& ARGS_IN buf);
void get_lines_from_buf(std::vector<std::string>& ARGS_OUT lines, const std::string& ARGS_IN buf);

#endif  // UTILS_H_