// #pragma once
#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#ifndef ADB_DEBUG
// #define ADB_DEBUG
#endif  // ADB_DEBUG

#define DISALLOW_ASSIGN(TypeName)     \
    void operator=(const TypeName &) = delete; \
    void operator=(TypeName &&) = delete;      \

#define DISALLOW_COPY_AND_ASSIGN(TypeName)     \
    void operator=(const TypeName &) = delete; \
    void operator=(TypeName &&) = delete;      \
    TypeName(const TypeName &) = delete;       \
    TypeName(TypeName &&) = delete

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
#define ADB_LOGI(fmt, ...)                                              \
    do {                                                                \
        printf("%s (%I32d) : " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
        fflush(stdout);                                                 \
    } while (0)

#endif  // ADB_LOG

constexpr int DEFAULT_ADB_PORT = 5037;

std::vector<std::string> string_split(const std::string &str, char delimiter);
void unique_character(std::string &str);

#endif  // UTILS_H_