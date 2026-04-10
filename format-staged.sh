#!/usr/bin/env bash
set -euo pipefail

. "$(dirname "$0")/format-common.sh"

CLANG_FORMAT_BIN="$(format_bin)"

mapfile -d '' FILES < <(
  git diff --cached --name-only -z --diff-filter=ACMR |
    grep -zE '\.(c|cc|cxx|cpp|h|hh|hpp|hxx)$' || true
)

if (( ${#FILES[@]} == 0 )); then
  echo "No staged C/C++ files to format-check."
  exit 0
fi

echo "Checking clang-format on staged files..."
"$CLANG_FORMAT_BIN" --version
printf '%s\0' "${FILES[@]}" | xargs -0 "$CLANG_FORMAT_BIN" --dry-run --Werror
echo "Staged files are properly formatted."
