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

file(MAKE_DIRECTORY "${STATISTICS_GENERATED_INCLUDE_DIR}")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/statistics_version.hpp.in"
  "${STATISTICS_GENERATED_VERSION_HEADER}"
  @ONLY
)

# Use libc++ for non-Windows Clang builds. On Windows, Clang typically targets
# the MSVC runtime/toolchain instead, and -stdlib=libc++ is rejected.
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT WIN32)
  add_compile_options(-stdlib=libc++)
  add_link_options(-stdlib=libc++ -lc++ -lc++abi)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(ProjectWarnings)
include(ProjectInstrumentation)

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

# Find fmt: prefer a pre-installed package (produces an IMPORTED target that
# can be referenced in CMake install exports); fall back to FetchContent for
# development builds where fmt is not pre-installed.
include(FetchContent)
find_package(fmt 12 CONFIG QUIET)
if (NOT fmt_FOUND)
  FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 12.1.0
  )
  FetchContent_MakeAvailable(fmt)
endif()

# Propagate fmt as a public (INTERFACE) dependency so consumers of the installed
# statistics package automatically get the fmt include paths and link requirement
# without having to add it manually.  When fmt was resolved via find_package the
# fmt::fmt target is IMPORTED, which CMake allows in an install export set and
# which consumers receive automatically via find_dependency(fmt) in
# statisticsConfig.cmake.  When fmt was resolved via FetchContent (fallback for
# development), BUILD_INTERFACE restricts the reference to the local build only
# and avoids the "target not in any export set" CMake error; consumers of the
# installed package still obtain fmt through find_dependency(fmt).
if (fmt_FOUND)
  target_link_libraries(statistics INTERFACE fmt::fmt)
else()
  target_link_libraries(statistics INTERFACE $<BUILD_INTERFACE:fmt::fmt>)
endif()

# Treat fmt headers as system includes so clang-tidy/analyzers do not report
# third-party diagnostics from template instantiations in fetched sources.
set(FMT_INCLUDE_DIRS $<TARGET_PROPERTY:fmt::fmt,INTERFACE_INCLUDE_DIRECTORIES>)

# Fetch GoogleTest
if (WIN32)
  set(gtest_disable_pthreads ON CACHE BOOL "Disable pthread usage in GoogleTest on Windows" FORCE)
endif()

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
set_target_properties(statistics_test PROPERTIES BUILD_WITH_INSTALL_RPATH ON)
target_link_libraries(statistics_test PRIVATE statistics fmt::fmt gtest_main project_warnings project_instrumentation)
target_include_directories(statistics_test SYSTEM PRIVATE ${FMT_INCLUDE_DIRS})

if (STATISTICS_ENABLE_ASAN)
  # LeakSanitizer aborts in this environment when the test binary is launched
  # through CTest/GoogleTest discovery. Run through `cmake -E env` and disable
  # leak detection so ASan/UBSan coverage remains usable.
  set_property(
    TARGET statistics_test
    PROPERTY CROSSCOMPILING_EMULATOR
      "${CMAKE_COMMAND};-E;env;LSAN_OPTIONS=detect_leaks=0"
  )
endif()

include(GoogleTest)
set(_statistics_gtest_discovery_mode POST_BUILD)
if (STATISTICS_ENABLE_ASAN OR STATISTICS_ENABLE_UBSAN)
  # LeakSanitizer can abort when GoogleTest enumerates tests during the build.
  # Discovering at CTest time avoids that build-time execution path.
  set(_statistics_gtest_discovery_mode PRE_TEST)
endif()
gtest_discover_tests(
  statistics_test
  DISCOVERY_MODE ${_statistics_gtest_discovery_mode}
)

add_executable(statistics-cli statistics-cli/statistics-cli.cpp)

# name the produced binary just "statistics"
set_target_properties(statistics-cli PROPERTIES
  OUTPUT_NAME statistics
  BUILD_WITH_INSTALL_RPATH ON
)

target_link_libraries(statistics-cli
  PRIVATE
    CLI11::CLI11
    statistics
    fmt::fmt
    project_warnings
    project_instrumentation
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
