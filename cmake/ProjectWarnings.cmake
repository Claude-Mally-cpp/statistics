# cmake/ProjectWarnings.cmake
cmake_minimum_required(VERSION 3.16)

# Turn this ON in CI: -DWARNINGS_AS_ERRORS=ON
option(WARNINGS_AS_ERRORS "Treat all warnings as errors" ON)

add_library(project_warnings INTERFACE)

# GCC/Clang/AppleClang warnings
target_compile_options(project_warnings INTERFACE
  $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>>:
    -Wall -Wextra -Wpedantic
    -Wconversion -Wfloat-conversion -Wsign-conversion
    -Wshadow -Wold-style-cast -Woverloaded-virtual
    -Wnon-virtual-dtor -Wnull-dereference -Wdouble-promotion
    -Wformat=2 -Wimplicit-fallthrough -Wextra-semi
    -Wcast-qual -Wcast-align=strict -Wmissing-declarations
  >
  $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:
    /W4 /permissive- /sdl /Zc:__cplusplus
    /w14242 /w14254 /w14263 /w14265 /w14287
    /we4244 /we4305
    /w14296 /w14311
    /w14545 /w14546 /w14547 /w14549 /w14555
    /w14640 /w14826 /w14928
  >
)

# Warnings-as-errors toggle
target_compile_options(project_warnings INTERFACE
  $<$<AND:$<BOOL:${WARNINGS_AS_ERRORS}>,$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-Werror>
  $<$<AND:$<BOOL:${WARNINGS_AS_ERRORS}>,$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/WX>
)
