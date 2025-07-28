cmake_minimum_required(VERSION 3.25)
project(statistics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.2.1
)

FetchContent_MakeAvailable(fmt)

add_executable(statistics_test statistics.cpp)

target_link_libraries(statistics_test PRIVATE fmt)

# Use libc++ when compiling with Clang
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(statistics_test PRIVATE -stdlib=libc++)
  target_link_libraries(statistics_test PRIVATE c++ c++abi)
endif()
