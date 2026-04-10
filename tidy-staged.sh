#!/usr/bin/env bash
set -euo pipefail

# Toggle to enforce (1 = block on diagnostics; 0 = just warn)
ENFORCE=${ENFORCE:-0}
DEFAULT_DB="out/build/linux-clang-debug"
FALLBACK_DB="build/linux-clang-debug"
DB=${DB:-}
ROOT=${ROOT:-"$(pwd)"}
CLANG_TIDY_BIN=${CLANG_TIDY_BIN:-clang-tidy}
HEADER_FILTER=${HEADER_FILTER:-"^${ROOT}/(include|test)/"}
EXCLUDE_HEADER_FILTER=${EXCLUDE_HEADER_FILTER:-'(^|.*/)(out/build|build|_deps|third_party|external|vendor)/'}

check_compile_db() {
  local db="$1"
  local compile_db="$db/compile_commands.json"

  if [ ! -f "$compile_db" ]; then
    return 1
  fi

  # Reject compile databases generated in a different checkout or container path
  # such as /project/... because clang-tidy will abort when it cannot chdir there.
  if grep -Fq '"/project/' "$compile_db"; then
    echo "stale:$compile_db"
    return 2
  fi

  return 0
}

resolve_compile_db() {
  local candidates=()

  if [ -n "$DB" ]; then
    candidates+=("$DB")
  else
    candidates+=("$DEFAULT_DB" "$FALLBACK_DB")
  fi

  local stale_db=""
  local candidate=""
  for candidate in "${candidates[@]}"; do
    local status=0
    local output=""
    output=$(check_compile_db "$candidate") || status=$?
    if [ "$status" -eq 0 ]; then
      DB="$candidate"
      return 0
    fi
    if [ "$status" -eq 2 ] && [ -z "$stale_db" ]; then
      stale_db="${output#stale:}"
    fi
  done

  if [ -n "$stale_db" ]; then
    echo "Skipping clang-tidy: $stale_db was generated for /project/ and is stale in this checkout."
    echo "Reconfigure the local build tree first with: bash ./tidy-prepare.sh"
    return 1
  fi

  echo "No usable compile_commands.json. Run: bash ./tidy-prepare.sh"
  return 1
}

HEADER_FILTER_ARGS=("-header-filter=$HEADER_FILTER")
if clang-tidy --help 2>&1 | grep -q -- '--exclude-header-filter'; then
  HEADER_FILTER_ARGS+=("-exclude-header-filter=$EXCLUDE_HEADER_FILTER")
else
  echo "clang-tidy does not support --exclude-header-filter; continuing without it"
fi

TIDY_ARGS=()
if [ "${ENFORCE}" = "1" ]; then
  TIDY_ARGS+=("--warnings-as-errors=*")
fi

if ! resolve_compile_db; then
  exit 0
fi

# Collect staged C/C++ files
mapfile -d '' FILES < <(git diff --cached --name-only -z --diff-filter=ACMR | \
  grep -zE '\.(c|cc|cxx|cpp|h|hh|hpp|hxx)$' || true)

(( ${#FILES[@]} )) || exit 0

echo "Running clang-tidy on staged files..."
STATUS=0
for f in "${FILES[@]}"; do
  "$CLANG_TIDY_BIN" "$f" -p "$DB" -quiet "${TIDY_ARGS[@]}" "${HEADER_FILTER_ARGS[@]}" || STATUS=1
done

if [ "${ENFORCE}" = "1" ] && [ "${STATUS:-0}" -ne 0 ]; then
  echo "❌ clang-tidy found issues. Fix or set ENFORCE=0 to warn only."
  exit 1
fi

echo "✅ clang-tidy finished (ENFORCE=${ENFORCE})."
