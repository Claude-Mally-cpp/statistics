#!/usr/bin/env bash
set -euo pipefail
cmake --preset linux-clang-debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
echo "compile_commands.json at out/build/linux-clang-debug/"
