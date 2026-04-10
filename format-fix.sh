#!/usr/bin/env bash
set -euo pipefail

. "$(dirname "$0")/format-common.sh"

CLANG_FORMAT_BIN="$(format_bin)"
FILES="$(collect_tracked_cpp_files)"

if [ -z "$FILES" ]; then
  echo "No source files found."
  exit 0
fi

echo "Applying clang-format..."
"$CLANG_FORMAT_BIN" --version
"$CLANG_FORMAT_BIN" -i $FILES
echo "All files are properly formatted."
