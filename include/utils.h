// #pragma once
#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>


#define DISALLOW_COPY_AND_ASSIGN(TypeName)   \
  void operator=(const TypeName &) = delete; \
  void operator=(TypeName &&) = delete;      \
  TypeName(const TypeName &) = delete;       \
  TypeName(TypeName &&) = delete

#ifndef ARGS_ATTR
#if defined(__clang__)
#define ARGS_OUT
#elif defined(__GNUC__) || defined(__GNUG__)
#define ARGS_OUT
#elif defined(_MSC_VER)
#define ARGS_OUT _Out_
#endif

#endif // ARGS_ATTR

#ifndef ADB_LOG
#define ADB_LOGI(fmt, ...) \
  do { \
    printf("%s (%I32d) : "fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    fflush(stdout); \
  } while(0)

#endif // ADB_LOG


constexpr int DEFAULT_ADB_PORT = 5037;

int add_t(int a, int b);
std::vector<std::string> string_split(const std::string &str, char delimiter);

std::string unique_character(std::string &str);

#endif // UTILS_H_