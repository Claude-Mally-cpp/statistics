#!/usr/bin/env bash

# dockerLinuxBuildAndTest.sh
# This script builds and tests the project in Docker containers for multiple Linux toolchains.
# It supports a --verbose argument for detailed output.
#
# Usage:
#   ./dockerLinuxBuildAndTest.sh [--verbose]
#
# If running under Git Bash or MSYS2 on Windows, MSYS_NO_PATHCONV=1 is set automatically.
# The script prints a summary of all build and test results at the end.

set -e

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

summary=""
VERBOSE=0

# Parse arguments
for arg in "$@"; do
    if [[ "$arg" == "--verbose" ]]; then
        VERBOSE=1
    fi
done

# Only export MSYS_NO_PATHCONV=1 if running under Git Bash/MSYS2
if [[ "$MSYSTEM" == MINGW* || "$MSYSTEM" == MSYS* ]]; then
    export MSYS_NO_PATHCONV=1
fi

run_and_report() {
    label="$1"
    shift
    if [[ $VERBOSE -eq 1 ]]; then
        echo "=== $label ==="
        if "$@"; then
            echo -e "${GREEN}$label: SUCCESS${NC}"
            summary="$summary\n$label: SUCCESS"
        else
            echo -e "${RED}$label: FAILURE${NC}"
            summary="$summary\n$label: FAILURE"
        fi
    else
        if output=$("$@" 2>&1); then
            echo -e "${GREEN}$label: SUCCESS${NC}"
            summary="$summary\n$label: SUCCESS"
        else
            echo -e "${RED}$label: FAILURE${NC}"
            echo "$output"
            summary="$summary\n$label: FAILURE"
        fi
    fi
}

# GCC14 Debug
run_and_report "GCC14 Debug Build" docker run --rm -v "$PWD:/project" -w /project cpp-ci-gcc14 cmake --preset linux-gcc14-debug
run_and_report "GCC14 Debug Test"  docker run --rm -v "$PWD:/project" -w /project cpp-ci-gcc14 ./out/build/linux-gcc14-debug/statistics_test

# GCC14 Release
run_and_report "GCC14 Release Build" docker run --rm -v "$PWD:/project" -w /project cpp-ci-gcc14 cmake --preset linux-gcc14-release
run_and_report "GCC14 Release Test"  docker run --rm -v "$PWD:/project" -w /project cpp-ci-gcc14 ./out/build/linux-gcc14-release/statistics_test

# Clang18 Debug
run_and_report "Clang18 Debug Build" docker run --rm -v "$PWD:/project" -w /project cpp-ci-clang18 cmake --preset linux-clang18-debug
run_and_report "Clang18 Debug Test"  docker run --rm -v "$PWD:/project" -w /project cpp-ci-clang18 ./out/build/linux-clang18-debug/statistics_test

# Clang18 Release
run_and_report "Clang18 Release Build" docker run --rm -v "$PWD:/project" -w /project cpp-ci-clang18 cmake --preset linux-clang18-release
run_and_report "Clang18 Release Test"  docker run --rm -v "$PWD:/project" -w /project cpp-ci-clang18 ./out/build/linux-clang18-release/statistics_test

echo -e "\n========== SUMMARY =========="
echo -e "$summary"
