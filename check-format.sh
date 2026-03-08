#!/usr/bin/env bash
set -euo pipefail
FILES=$(git ls-files '*.cpp' '*.hpp' '*.h' || true)
if [ -z "$FILES" ]; then
  echo "No source files found."
  exit 0
fi
echo "Checking clang-format..."
clang-format --version
clang-format --dry-run --Werror $FILES
echo "All files are properly formatted ✅"
