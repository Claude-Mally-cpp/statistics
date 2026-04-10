#!/usr/bin/env bash

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
  local prepare_cmd="$1"
  shift

  local candidates=("$@")
  local stale_db=""
  local candidate=""
  for candidate in "${candidates[@]}"; do
    local status=0
    local output=""
    output=$(check_compile_db "$candidate") || status=$?
    if [ "$status" -eq 0 ]; then
      RESOLVED_CLANG_TIDY_DB="$candidate"
      return 0
    fi
    if [ "$status" -eq 2 ] && [ -z "$stale_db" ]; then
      stale_db="${output#stale:}"
    fi
  done

  if [ -n "$stale_db" ]; then
    echo "Skipping clang-tidy: $stale_db was generated for /project/ and is stale in this checkout."
    echo "Reconfigure the local build tree first with: $prepare_cmd"
    return 1
  fi

  echo "No usable compile_commands.json. Run: $prepare_cmd"
  return 1
}
