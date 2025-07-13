cmake_minimum_required(VERSION 3.16)

project(statistics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set policy for MSVC debug info (optional)
if (MSVC AND POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Header-only library target
add_library(statistics INTERFACE)
target_include_directories(statistics INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Use FetchContent to get fmt (for test only)
include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(fmt)

# Test executable
add_executable(statistics_test statistics.cpp)
target_link_libraries(statistics_test PRIVATE statistics fmt::fmt)

# Link libc++ for Clang if necessary
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_link_libraries(statistics_test PRIVATE c++ c++abi)
endif()
