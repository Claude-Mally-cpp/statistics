#!/usr/bin/env bash

format_bin() {
  printf '%s\n' "${CLANG_FORMAT:-clang-format}"
}

collect_tracked_cpp_files() {
  git ls-files '*.c' '*.cc' '*.cxx' '*.cpp' '*.h' '*.hh' '*.hpp' '*.hxx' || true
}
