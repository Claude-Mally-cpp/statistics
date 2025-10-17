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

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(ProjectWarnings)

# Header-only library
add_library(statistics INTERFACE)
target_include_directories(statistics INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

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

# Fetch CLI11 v2.5.0
FetchContent_Declare(
  CLI11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG v2.5.0
)
FetchContent_MakeAvailable(CLI11)

enable_testing()

# Test executable
add_executable(statistics_test test/test_statistics.cpp)
target_link_libraries(statistics_test PRIVATE statistics fmt::fmt gtest_main project_warnings)

include(GoogleTest)
gtest_discover_tests(statistics_test)

add_executable(statistics-cli statistics-cli/statistics-cli.cpp)

# name the produced binary just "statistics"
set_target_properties(statistics-cli PROPERTIES OUTPUT_NAME statistics)

target_link_libraries(statistics-cli
  PRIVATE
    CLI11::CLI11
    statistics
    fmt::fmt
    project_warnings
)

# Install the CLI executable
install(TARGETS statistics-cli
        RUNTIME DESTINATION bin
        BUNDLE DESTINATION bin
        LIBRARY DESTINATION lib
)

# Install headers (all .hpp files under include/)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp"
)

# Optional: install the test runner for CI/local use
install(TARGETS statistics_test
        RUNTIME DESTINATION bin/tests
        OPTIONAL
)
