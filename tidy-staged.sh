#!/usr/bin/env bash
set -euo pipefail

# Toggle to enforce (1 = block on diagnostics; 0 = just warn)
ENFORCE=${ENFORCE:-0}
DB="out/build/linux-clang-debug"
ROOT=${ROOT:-"$(pwd)"}
HEADER_FILTER=${HEADER_FILTER:-"^${ROOT}/(include|test)/"}
EXCLUDE_HEADER_FILTER=${EXCLUDE_HEADER_FILTER:-'(^|.*/)(out/build|build|_deps|third_party|external|vendor)/'}

if [ ! -f "$DB/compile_commands.json" ]; then
  echo "No compile_commands.json. Run: ./tidy-prepare.sh"
  exit 0
fi

# Collect staged C/C++ files
mapfile -d '' FILES < <(git diff --cached --name-only -z --diff-filter=ACMR | \
  grep -zE '\.(c|cc|cxx|cpp|h|hh|hpp|hxx)$' || true)

(( ${#FILES[@]} )) || exit 0

echo "Running clang-tidy on staged files..."
if command -v run-clang-tidy >/dev/null 2>&1; then
  # Limit to staged files
  printf '%s\n' "${FILES[@]}" > .tidyfiles
  run-clang-tidy -p "$DB" -quiet -vf=.tidyfiles -header-filter="$HEADER_FILTER" \
    -exclude-header-filter="$EXCLUDE_HEADER_FILTER" || STATUS=$?
  rm -f .tidyfiles
else
  STATUS=0
  for f in "${FILES[@]}"; do
    clang-tidy "$f" -p "$DB" -quiet -header-filter="$HEADER_FILTER" \
      -exclude-header-filter="$EXCLUDE_HEADER_FILTER" || STATUS=1
  done
fi

if [ "${ENFORCE}" = "1" ] && [ "${STATUS:-0}" -ne 0 ]; then
  echo "❌ clang-tidy found issues. Fix or set ENFORCE=0 to warn only."
  exit 1
fi

echo "✅ clang-tidy finished (ENFORCE=${ENFORCE})."
