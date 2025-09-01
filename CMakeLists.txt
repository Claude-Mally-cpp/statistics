cmake_minimum_required(VERSION 3.25)
project(statistics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use libc++ globally when compiling with Clang
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(-stdlib=libc++)
  add_link_options(-stdlib=libc++ -lc++ -lc++abi)
endif()

# Header-only library
add_library(statistics INTERFACE)
target_include_directories(statistics INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Fetch fmt
include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(fmt)

# Fetch GoogleTest
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Test executable
add_executable(statistics_test test_statistics.cpp)
target_link_libraries(statistics_test PRIVATE statistics fmt::fmt gtest_main)

include(GoogleTest)
gtest_discover_tests(statistics_test)
