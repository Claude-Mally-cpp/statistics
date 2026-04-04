#!/usr/bin/env bash
set -euo pipefail
CLANG_FORMAT_BIN="${CLANG_FORMAT:-clang-format}"
FILES=$(git ls-files '*.cpp' '*.hpp' '*.h' || true)
if [ -z "$FILES" ]; then
  echo "No source files found."
  exit 0
fi
echo "Checking clang-format..."
"$CLANG_FORMAT_BIN" --version
"$CLANG_FORMAT_BIN" --dry-run --Werror $FILES
echo "All files are properly formatted ✅"
