cmake_minimum_required(VERSION 3.16)
project(unittest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
if(WIN32)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

enable_testing()

include_directories(../include)
include_directories(../third_party/libhv/include/hv)

add_executable(
  ${PROJECT_NAME}
  command_test.cpp
)

if(WIN32)
target_link_libraries(
  ${PROJECT_NAME}
  GTest::gtest_main
  adbClientStatic
)
else()
target_link_libraries(
  ${PROJECT_NAME}
  GTest::gtest_main
  adbClient
)
endif()

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})