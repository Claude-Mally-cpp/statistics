#!/usr/bin/env bash
set -euo pipefail

# Toggle to enforce (1 = block on diagnostics; 0 = just warn)
ENFORCE=${ENFORCE:-0}
DB="out/build/linux-clang-debug"
ROOT=${ROOT:-"$(pwd)"}
CLANG_TIDY_BIN=${CLANG_TIDY_BIN:-clang-tidy}
HEADER_FILTER=${HEADER_FILTER:-"^${ROOT}/(include|test)/"}
EXCLUDE_HEADER_FILTER=${EXCLUDE_HEADER_FILTER:-'(^|.*/)(out/build|build|_deps|third_party|external|vendor)/'}

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

if [ ! -f "$DB/compile_commands.json" ]; then
  echo "No compile_commands.json. Run: ./tidy-prepare.sh"
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
