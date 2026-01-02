#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: ./tidy-modernize.sh [--fix]

Runs clang-tidy modernize checks over the repo to surface C++23
modernization opportunities. Pass --fix to apply suggested changes.
EOF
}

if [ "${1:-}" = "-h" ] || [ "${1:-}" = "--help" ]; then
  usage
  exit 0
fi

APPLY_FIX=0
if [ "${1:-}" = "--fix" ] || [ "${1:-}" = "--apply" ]; then
  APPLY_FIX=1
  shift
fi

if [ $# -gt 0 ]; then
  echo "Unknown argument: $1"
  usage
  exit 2
fi

DB=${DB:-"out/build/msvc-x64-debug"}
if [ ! -f "$DB/compile_commands.json" ]; then
  echo "No compile_commands.json. Run: ./tidy-prepare.sh"
  exit 0
fi

mapfile -d '' FILES < <(git ls-files -z \
  '*.c' '*.cc' '*.cxx' '*.cpp' '*.h' '*.hh' '*.hxx' '*.hpp' || true)

(( ${#FILES[@]} )) || exit 0

CHECKS=${CHECKS:-"modernize-*"}

EXTRA_ARGS=()
if [ "$APPLY_FIX" = "1" ]; then
  EXTRA_ARGS+=("-fix")
fi

WIN_FILES=()
for f in "${FILES[@]}"; do
  if command -v cygpath >/dev/null 2>&1; then
    # Use forward slashes to avoid regex escape issues in run-clang-tidy.
    WIN_FILES+=("$(cygpath -m "$f")")
  elif command -v realpath >/dev/null 2>&1; then
    WIN_FILES+=("$(realpath "$f")")
  else
    WIN_FILES+=("$f")
  fi
done

echo "Running clang-tidy with checks: ${CHECKS}"
if [ "$APPLY_FIX" = "1" ]; then
  echo "Applying fixes..."
fi

USE_RUN_CLANG_TIDY=1
if command -v cygpath >/dev/null 2>&1; then
  # run-clang-tidy treats file args as regex; Windows paths are a bad fit.
  USE_RUN_CLANG_TIDY=0
fi

if [ "$USE_RUN_CLANG_TIDY" = "1" ] && command -v run-clang-tidy >/dev/null 2>&1; then
  # run-clang-tidy accepts file paths as positional arguments.
  printf '%s\0' "${WIN_FILES[@]}" | \
    xargs -0 run-clang-tidy -p "$DB" -checks="$CHECKS" "${EXTRA_ARGS[@]}"
else
  for f in "${WIN_FILES[@]}"; do
    clang-tidy "$f" -p "$DB" -checks="$CHECKS" "${EXTRA_ARGS[@]}"
  done
fi
