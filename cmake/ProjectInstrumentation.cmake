cmake_minimum_required(VERSION 3.16)

option(STATISTICS_ENABLE_ASAN "Enable AddressSanitizer for supported builds" OFF)
option(STATISTICS_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer for supported builds" OFF)
option(STATISTICS_ENABLE_COVERAGE "Enable source-based coverage instrumentation for supported builds" OFF)

add_library(project_instrumentation INTERFACE)

set(_statistics_instrumentation_compile_flags "")
set(_statistics_instrumentation_link_flags "")

if (STATISTICS_ENABLE_COVERAGE)
  if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "STATISTICS_ENABLE_COVERAGE requires Clang in this project.")
  endif()
  list(APPEND _statistics_instrumentation_compile_flags
    -fprofile-instr-generate
    -fcoverage-mapping
  )
  list(APPEND _statistics_instrumentation_link_flags
    -fprofile-instr-generate
  )
endif()

set(_statistics_sanitizer_names "")
if (STATISTICS_ENABLE_ASAN)
  list(APPEND _statistics_sanitizer_names address)
endif()
if (STATISTICS_ENABLE_UBSAN)
  list(APPEND _statistics_sanitizer_names undefined)
endif()

if (_statistics_sanitizer_names)
  if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    message(FATAL_ERROR "STATISTICS_ENABLE_ASAN/UBSAN require Clang or GCC in this project.")
  endif()
  string(JOIN "," _statistics_sanitizers ${_statistics_sanitizer_names})
  list(APPEND _statistics_instrumentation_compile_flags
    -fsanitize=${_statistics_sanitizers}
    -fno-omit-frame-pointer
  )
  list(APPEND _statistics_instrumentation_link_flags
    -fsanitize=${_statistics_sanitizers}
  )
endif()

if (_statistics_instrumentation_compile_flags)
  target_compile_options(project_instrumentation INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:${_statistics_instrumentation_compile_flags}>
  )
endif()

if (_statistics_instrumentation_link_flags)
  target_link_options(project_instrumentation INTERFACE
    ${_statistics_instrumentation_link_flags}
  )
endif()
