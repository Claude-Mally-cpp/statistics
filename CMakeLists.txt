cmake_minimum_required(VERSION 3.25)
project(
  statistics
  VERSION 0.0.1
  DESCRIPTION "Modern C++ statistics library and CLI"
  LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(STATISTICS_GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/include")
set(STATISTICS_GENERATED_VERSION_HEADER "${STATISTICS_GENERATED_INCLUDE_DIR}/statistics_version.hpp")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/statistics_version.hpp.in"
  "${STATISTICS_GENERATED_VERSION_HEADER}"
  @ONLY
)

# Use libc++ globally when compiling with Clang
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(-stdlib=libc++)
  add_link_options(-stdlib=libc++ -lc++ -lc++abi)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(ProjectWarnings)

# Header-only library
add_library(statistics INTERFACE)
add_library(statistics::statistics ALIAS statistics)
set_target_properties(statistics PROPERTIES EXPORT_NAME statistics)
target_include_directories(
  statistics
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${STATISTICS_GENERATED_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Fetch fmt
include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 12.1.0
)
FetchContent_MakeAvailable(fmt)

# Treat fmt headers as system includes so clang-tidy/analyzers do not report
# third-party diagnostics from template instantiations in fetched sources.
set(FMT_INCLUDE_DIRS $<TARGET_PROPERTY:fmt::fmt,INTERFACE_INCLUDE_DIRECTORIES>)

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
target_include_directories(statistics_test SYSTEM PRIVATE ${FMT_INCLUDE_DIRS})

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
target_include_directories(statistics-cli SYSTEM PRIVATE ${FMT_INCLUDE_DIRS})

# Install the CLI executable
install(
  TARGETS statistics statistics-cli
  EXPORT statisticsTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers (all .hpp files under include/)
install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp"
)

install(
  FILES "${STATISTICS_GENERATED_VERSION_HEADER}"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/statisticsConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY ExactVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/statisticsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/statisticsConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/statistics"
)

install(
  EXPORT statisticsTargets
  FILE statisticsTargets.cmake
  NAMESPACE statistics::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/statistics"
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/statisticsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/statisticsConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/statistics"
)

# Optional: install the test runner for CI/local use
install(
  TARGETS statistics_test
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
  OPTIONAL
)
