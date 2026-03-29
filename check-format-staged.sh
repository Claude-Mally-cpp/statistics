#!/usr/bin/env bash
set -euo pipefail

mapfile -d '' FILES < <(
  git diff --cached --name-only -z --diff-filter=ACMR |
    grep -zE '\.(c|cc|cxx|cpp|h|hh|hpp|hxx)$' || true
)

if (( ${#FILES[@]} == 0 )); then
  echo "No staged C/C++ files to format-check."
  exit 0
fi

echo "Checking clang-format on staged files..."
clang-format --version
printf '%s\0' "${FILES[@]}" | xargs -0 clang-format --dry-run --Werror
echo "Staged files are properly formatted."
