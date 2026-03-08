#!/usr/bin/env bash
#
# Prepare a compile_commands.json for clang-tidy.
set -euo pipefail

PRESET=${PRESET:-linux-clang-release}
cmake --preset "$PRESET"
echo "compile_commands.json at out/build/${PRESET}/"
