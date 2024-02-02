cmake_minimum_required(VERSION 3.16)
project(adbClient LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAFS "-g")
set(CMAKE_CXX_FLAFS "-Wall")
set(CMAKE_CXX_FLAFS "-Wextra")
set(CMAKE_CXX_FLAFS "-Werror")

if(NOT WIN32)
    set(CMAKE_REQUIRED_LIBRARIES "-pthread")
    set(CMAKE_CXX_FLAFS "-shared")
    set(CMAKE_CXX_FLAFS "-fPIC")
endif()


set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

# subordinate directory
# dependency
add_subdirectory(./third_party/libhv)

# third_party
# libhv
include_directories(${PROJECT_SOURCE_DIR}/third_party/libhv/include/hv)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SRC
    ${PROJECT_SOURCE_DIR}/src/utils.cpp
    ${PROJECT_SOURCE_DIR}/src/device_info.cpp
    ${PROJECT_SOURCE_DIR}/src/android/stringprintf.cpp
    ${PROJECT_SOURCE_DIR}/src/adb_command.cpp
    ${PROJECT_SOURCE_DIR}/src/host_command.cpp
    ${PROJECT_SOURCE_DIR}/src/local_command.cpp
    ${PROJECT_SOURCE_DIR}/src/host_serial_command.cpp
    ${PROJECT_SOURCE_DIR}/src/adb_protocol.cpp
)

# add library
if(WIN32)
    add_library(${PROJECT_NAME} SHARED ${SRC})
    add_library(${PROJECT_NAME}Static STATIC ${SRC})
else()
    add_library(${PROJECT_NAME} SHARED ${SRC})
endif()

# link third_party
if(WIN32)
    target_link_libraries(${PROJECT_NAME}Static PUBLIC hv_static)
    target_link_libraries(${PROJECT_NAME} PUBLIC hv_static)
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC hv)
endif()

# testing
add_subdirectory(./unittest)

# executable
add_executable(${PROJECT_NAME}Exe main.cpp)

# target_link
if(WIN32)
    target_link_libraries(${PROJECT_NAME}Exe ${PROJECT_NAME}Static)
else()
    target_link_libraries(${PROJECT_NAME}Exe ${PROJECT_NAME})
endif()
