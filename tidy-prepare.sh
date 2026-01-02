#!/usr/bin/env bash
set -euo pipefail

PRESET=${PRESET:-msvc-x64-debug}
cmake --preset "$PRESET"
echo "compile_commands.json at out/build/${PRESET}/"
