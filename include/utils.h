// #pragma once
#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <string.h>

#include <chrono>
#include <string>
#include <vector>

#include "device_info.h"

#ifndef NDEBUG
#define ADB_DEBUG
#endif  // NDEBUG

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

#define ADB_LOGE(fmt, ...)                                                                                           \
    do {                                                                                                             \
        auto now = std::chrono::system_clock::now();                                                                 \
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);                                  \
        auto value = now_ms.time_since_epoch().count();                                                              \
        auto milliseconds = value % 1000;                                                                            \
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);                                               \
        char timeString[80];                                                                                         \
        std::strftime(timeString, sizeof(timeString), "%H:%M:%S", std::localtime(&now_c));                           \
        fprintf(stderr, "[%s.%I64d] %s (%I32d) E : " fmt, timeString, milliseconds, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#ifdef ADB_DEBUG
#define ADB_LOGI(fmt, ...)                                                                                           \
    do {                                                                                                             \
        auto now = std::chrono::system_clock::now();                                                                 \
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);                                  \
        auto value = now_ms.time_since_epoch().count();                                                              \
        auto milliseconds = value % 1000;                                                                            \
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);                                               \
        char timeString[80];                                                                                         \
        std::strftime(timeString, sizeof(timeString), "%H:%M:%S", std::localtime(&now_c));                           \
        fprintf(stdout, "[%s.%I64d] %s (%I32d) I : " fmt, timeString, milliseconds, __FILE__, __LINE__, ##__VA_ARGS__); \
        fflush(stdout);                                                                                              \
    } while (0)

#else
#define ADB_LOGI(fmt, ...) \
    do {                   \
    } while (0)

#endif  // ADB_DEBUG

#endif  // ADB_LOG

#ifndef STRING_CONCAT
// MSVC is ok with a##b when both of them are const char*,
// while gcc and clang doesn't support it.
// https://stackoverflow.com/questions/1206624/differences-in-macro-concatenation-operator-between-visual-c-and-gcc
#if defined(_MSC_VER)
#define STRING_CONCAT(a, b) a##b
#else
#define STRING_CONCAT(a, b) (a b)
#endif

#endif  // STRING_CONCAT

constexpr int DEFAULT_ADB_PORT = 5037;

std::vector<std::string> string_split(const std::string &str, char delimiter);
void unique_spaces(std::string &str);
void get_device_info_from_buf(std::vector<DeviceInfo> &ARGS_OUT devices_list, const std::string &ARGS_IN buf);
void get_lines_from_buf(std::vector<std::string> &ARGS_OUT lines, const std::string &ARGS_IN buf);

#endif  // UTILS_H_