# CMake 4.5 discovers standard-library modules when the compiler is enabled.
if (NOT CMAKE_CXX_STDLIB_MODULES_JSON OR NOT EXISTS "${CMAKE_CXX_STDLIB_MODULES_JSON}")
  message(FATAL_ERROR
    "The selected C++ toolchain has no standard-library module manifest. "
    "Use a toolchain with installed std module sources. For Linux Clang, "
    "run tools/install-llvm.sh and configure with -DCMAKE_CXX_FLAGS=-stdlib=libc++.")
endif()

get_filename_component(module_directory "${CMAKE_CXX_STDLIB_MODULES_JSON}" DIRECTORY)
file(READ "${CMAKE_CXX_STDLIB_MODULES_JSON}" module_manifest)
string(JSON module_count LENGTH "${module_manifest}" modules)
math(EXPR last_module "${module_count} - 1")
foreach (module_index RANGE ${last_module})
  string(JSON module_source GET "${module_manifest}" modules ${module_index} source-path)
  get_filename_component(module_source "${module_source}" ABSOLUTE BASE_DIR "${module_directory}")
  if (NOT EXISTS "${module_source}")
    message(FATAL_ERROR
      "Standard-library module source is missing: ${module_source}. "
      "For the repository's Linux LLVM toolchain, rerun the updated tools/install-llvm.sh. "
      "See docs/llvm-setup.md for the import std example requirements.")
  endif()
endforeach()

add_executable(statistics_std_module_example example.cpp)
target_link_libraries(statistics_std_module_example PRIVATE project_warnings)
set_target_properties(statistics_std_module_example PROPERTIES CXX_SCAN_FOR_MODULES ON)
add_test(NAME statistics_std_module_example COMMAND statistics_std_module_example)
set_tests_properties(statistics_std_module_example PROPERTIES
  PASS_REGULAR_EXPRESSION "C[+][+]23 modules: n=5, mean=2[.]6")
