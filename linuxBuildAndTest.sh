#!/usr/bin/env bash
set -euo pipefail

verbose=0
if [[ "${1:-}" == "-v" || "${1:-}" == "--verbose" ]]; then
  verbose=1
fi

run_step() {
  local label="$1"
  shift

  printf '=== %s ===\n' "$label"
  if [[ "$verbose" -eq 1 ]]; then
    "$@"
  else
    if ! output="$("$@" 2>&1)"; then
      printf '%s: FAILURE\n' "$label"
      printf '%s\n' "$output"
      return 1
    fi
  fi
  printf '%s: SUCCESS\n' "$label"
}

run_step "Release Configure" cmake --preset linux-gcc-release
run_step "Release Build" cmake --build --preset linux-gcc-release
run_step "Release Test" ctest --preset linux-gcc-release

run_step "Debug Configure" cmake --preset linux-gcc-debug
run_step "Debug Build" cmake --build --preset linux-gcc-debug
run_step "Debug Test" ctest --preset linux-gcc-debug
