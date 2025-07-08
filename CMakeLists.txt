# CMakeList.txt : CMake project for statistics, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.16)

project("statistics")

if (MSVC)
  if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
  endif()
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Detect Clang
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

# Use FetchContent to get fmt
include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.2.1 # Or latest stable release
)
FetchContent_MakeAvailable(fmt)

add_executable(statistics "statistics.cpp" "statistics.hpp")
target_link_libraries(statistics PRIVATE fmt::fmt)

# Link libc++/libc++abi only with Clang
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_libraries(statistics PRIVATE c++ c++abi)
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET statistics PROPERTY CXX_STANDARD 23)
endif()

# TODO: Add tests and install targets if needed.
