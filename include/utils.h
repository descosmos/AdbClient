// #pragma once
#ifndef UTILS_H_
#define UTILS_H_


#define DISALLOW_COPY_AND_ASSIGN(TypeName)                                    \
  void operator=(const TypeName&) = delete;                                   \
  void operator=(TypeName&&) = delete;                                        \
  TypeName(const TypeName&) = delete;                                         \
  TypeName(TypeName&&) = delete

constexpr int DEFAULT_ADB_PORT = 5037;


int add_t(int a, int b);



#endif // UTILS_H_