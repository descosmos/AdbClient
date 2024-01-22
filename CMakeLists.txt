cmake_minimum_required(VERSION 3.16)
project(adbClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# subordinate directory
# dependency
add_subdirectory(./third_party/libhv)

# third_party
# libhv
include_directories(${PROJECT_SOURCE_DIR}/third_party/libhv/include/hv)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SRC
    ${PROJECT_SOURCE_DIR}/src/utils.cpp
    ${PROJECT_SOURCE_DIR}/src/android/stringprintf.cpp
    ${PROJECT_SOURCE_DIR}/src/AdbCommand.cpp
    ${PROJECT_SOURCE_DIR}/src/HostCommand.cpp
    ${PROJECT_SOURCE_DIR}/src/LocalCommand.cpp
    ${PROJECT_SOURCE_DIR}/src/HostSerialCommand.cpp
)

# library
if(WIN32)
    add_library(${PROJECT_NAME} SHARED ${SRC})
    add_library(${PROJECT_NAME}Static STATIC ${SRC})
else()
    add_library(${PROJECT_NAME} SHARED ${SRC})
endif()

# link
if(WIN32)
    target_link_libraries(${PROJECT_NAME}Static PUBLIC hv_static)
    target_link_libraries(${PROJECT_NAME} PUBLIC hv_static)
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC hv_static)
endif()

# testing

# if(BUILD_UNITTEST)
enable_testing()
add_subdirectory(./unittest)

# endif()

# executable
add_executable(${PROJECT_NAME}Exe main.cpp)
target_link_libraries(${PROJECT_NAME}Exe ${PROJECT_NAME}Static)
