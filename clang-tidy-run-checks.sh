#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: bash ./clang-tidy-run-checks.sh [--fix]

Runs clang-tidy checks over the repo. Pass --fix to apply suggested changes.
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

PRESET=${PRESET:-linux-clang-release}
DB=${DB:-"out/build/${PRESET}"}
if [ ! -f "$DB/compile_commands.json" ]; then
  echo "No compile_commands.json. Run: bash ./clang-tidy-prepare.sh"
  exit 0
fi

PROJECT_DIRS=${PROJECT_DIRS:-"include test"}
read -r -a DIRS <<< "$PROJECT_DIRS"

mapfile -d '' ALL_FILES < <(git ls-files -z \
  '*.c' '*.cc' '*.cxx' '*.cpp' '*.h' '*.hh' '*.hxx' '*.hpp' || true)

FILES=()
for f in "${ALL_FILES[@]}"; do
  for d in "${DIRS[@]}"; do
    if [[ "$f" == "$d/"* ]]; then
      FILES+=("$f")
      break
    fi
  done
done

(( ${#FILES[@]} )) || exit 0

CHECKS=${CHECKS:-""}

ROOT=${ROOT:-"$(pwd)"}
if command -v cygpath >/dev/null 2>&1; then
  ROOT="$(cygpath -m "$ROOT")"
fi
dir_regex=$(IFS='|'; printf '%s' "${DIRS[*]}")
HEADER_FILTER=${HEADER_FILTER:-"^${ROOT}/(${dir_regex})/"}

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

if [ -n "$CHECKS" ]; then
  echo "Running clang-tidy with checks: ${CHECKS}"
else
  echo "Running clang-tidy with checks from .clang-tidy"
fi
if [ "$APPLY_FIX" = "1" ]; then
  echo "Applying fixes..."
fi

USE_RUN_CLANG_TIDY=0
if command -v cygpath >/dev/null 2>&1; then
  # run-clang-tidy treats file args as regex; Windows paths are a bad fit.
  USE_RUN_CLANG_TIDY=0
fi

CHECKS_ARGS=()
if [ -n "$CHECKS" ]; then
  CHECKS_ARGS+=("-checks=$CHECKS")
fi

if [ "$USE_RUN_CLANG_TIDY" = "1" ] && command -v run-clang-tidy >/dev/null 2>&1; then
  # run-clang-tidy accepts file paths as positional arguments.
  printf '%s\0' "${WIN_FILES[@]}" | \
    xargs -0 run-clang-tidy -p "$DB" "${CHECKS_ARGS[@]}" -header-filter="$HEADER_FILTER" "${EXTRA_ARGS[@]}"
else
  for f in "${WIN_FILES[@]}"; do
    clang-tidy --quiet "$f" -p "$DB" "${CHECKS_ARGS[@]}" -header-filter="$HEADER_FILTER" "${EXTRA_ARGS[@]}"
  done
fi
