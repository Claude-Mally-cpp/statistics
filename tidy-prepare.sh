#!/usr/bin/env bash
set -euo pipefail

PRESET=${PRESET:-linux-clang-release}
cmake --preset "$PRESET"
echo "compile_commands.json at out/build/${PRESET}/"
