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

has_usable_compile_db() {
  local db="$1"
  local compile_db="$db/compile_commands.json"

  if [ ! -f "$compile_db" ]; then
    echo "No compile_commands.json. Run: bash ./clang-tidy-prepare.sh"
    return 1
  fi

  # Reject compile databases generated in a different checkout or container path
  # such as /project/... because clang-tidy will abort when it cannot chdir there.
  if grep -Fq '"/project/' "$compile_db"; then
    echo "Skipping clang-tidy: $compile_db was generated for /project/ and is stale in this checkout."
    echo "Reconfigure a local build tree first, for example: rm -rf $db && PRESET=$PRESET bash ./clang-tidy-prepare.sh"
    return 1
  fi

  return 0
}

if ! has_usable_compile_db "$DB"; then
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
WARNINGS_AS_ERRORS=${WARNINGS_AS_ERRORS:-"*"}
CLANG_TIDY_BIN=${CLANG_TIDY_BIN:-clang-tidy}

ROOT=${ROOT:-"$(pwd)"}
if command -v cygpath >/dev/null 2>&1; then
  ROOT="$(cygpath -m "$ROOT")"
fi
dir_regex=$(IFS='|'; printf '%s' "${DIRS[*]}")
HEADER_FILTER=${HEADER_FILTER:-"^${ROOT}/(${dir_regex})/"}
EXCLUDE_HEADER_FILTER=${EXCLUDE_HEADER_FILTER:-'(^|.*/)(out/build|build|_deps|third_party|external|vendor)/'}

HEADER_FILTER_ARGS=("-header-filter=$HEADER_FILTER")
if "$CLANG_TIDY_BIN" --help 2>&1 | grep -q -- '--exclude-header-filter'; then
  HEADER_FILTER_ARGS+=("-exclude-header-filter=$EXCLUDE_HEADER_FILTER")
else
  echo "$CLANG_TIDY_BIN does not support --exclude-header-filter; continuing without it"
fi

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
if [ -n "$WARNINGS_AS_ERRORS" ]; then
  CHECKS_ARGS+=("--warnings-as-errors=$WARNINGS_AS_ERRORS")
fi

if [ "$USE_RUN_CLANG_TIDY" = "1" ] && command -v run-clang-tidy >/dev/null 2>&1; then
  # run-clang-tidy accepts file paths as positional arguments.
  printf '%s\0' "${WIN_FILES[@]}" | \
    xargs -0 run-clang-tidy -p "$DB" "${CHECKS_ARGS[@]}" "${HEADER_FILTER_ARGS[@]}" \
      "${EXTRA_ARGS[@]}"
else
  for f in "${WIN_FILES[@]}"; do
    "$CLANG_TIDY_BIN" --quiet "$f" -p "$DB" "${CHECKS_ARGS[@]}" "${HEADER_FILTER_ARGS[@]}" \
      "${EXTRA_ARGS[@]}"
  done
fi
