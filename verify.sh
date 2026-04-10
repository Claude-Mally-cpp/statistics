#!/usr/bin/env bash
set -euo pipefail

MODE="${1:-quick}"

have_cmd() {
  command -v "$1" >/dev/null 2>&1
}

run_step() {
  local label="$1"
  shift
  echo "==> $label"
  "$@"
}

run_cppcheck_local() {
  local build_dir=""

  if [ -d "out/build/msvc-x64-debug/_deps" ]; then
    build_dir="out/build/msvc-x64-debug"
  elif [ -d "out/build/linux-gcc-debug/_deps" ]; then
    build_dir="out/build/linux-gcc-debug"
  elif [ -d "out/build/linux-clang-debug/_deps" ]; then
    build_dir="out/build/linux-clang-debug"
  fi

  if [ -z "$build_dir" ]; then
    echo "Skipping cppcheck: no configured build tree with fetched deps found."
    echo "Run a configure step first so fmt/googletest headers exist under out/build/*/_deps."
    return 0
  fi

  run_step "cppcheck" \
    cppcheck include test \
      -I include \
      -I "$build_dir/_deps/fmt-src/include" \
      -I "$build_dir/_deps/googletest-src/googletest/include" \
      --language=c++ \
      --std=c++23 \
      --enable=warning,style,performance,portability,information,missingInclude \
      --inline-suppr \
      --suppress=missingIncludeSystem \
      -i "$build_dir/_deps" \
      -q
}

run_quick() {
  run_step "format check" bash ./format-check.sh
}

run_full() {
  run_quick

  if have_cmd doxygen; then
    run_step "doxygen verify" bash ./doxygen-docs.sh --verify-all
  else
    echo "Skipping Doxygen verification: doxygen not found."
  fi

  if [[ "${OS:-}" == "Windows_NT" ]]; then
    run_step "windows build and test" pwsh -File ./windowsBuildAndTest.ps1
  else
    echo "Skipping windowsBuildAndTest.ps1 on non-Windows host."
  fi

  if have_cmd cppcheck; then
    run_cppcheck_local
  else
    echo "Skipping cppcheck: command not found."
  fi

  if have_cmd docker; then
    run_step "rebuild docker images" bash ./rebuildDockerImages.sh
    run_step "docker linux build and test" bash ./dockerLinuxBuildAndTest.sh
  else
    echo "Skipping Docker checks: docker not found."
  fi
}

case "$MODE" in
  quick)
    run_quick
    ;;
  full)
    run_full
    ;;
  *)
    echo "Usage: ./verify.sh [quick|full]"
    exit 2
    ;;
esac
