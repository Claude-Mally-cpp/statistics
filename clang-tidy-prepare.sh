#!/usr/bin/env bash
#
# Prepare a compile_commands.json for clang-tidy.
set -euo pipefail

PRESET=${PRESET:-linux-clang-release}
ROOT=${ROOT:-"$(pwd)"}
DEFAULT_BUILD_DIR="out/build/${PRESET}"
FALLBACK_BUILD_DIR=${FALLBACK_BUILD_DIR:-"build/${PRESET}"}
BUILD_DIR=${BUILD_DIR:-"$DEFAULT_BUILD_DIR"}

stale_build_tree_reason() {
  local build_dir="$1"
  local compile_db="$build_dir/compile_commands.json"
  local cache_file="$build_dir/CMakeCache.txt"

  if [ -f "$cache_file" ]; then
    local cached_source
    cached_source=$(sed -n 's/^CMAKE_HOME_DIRECTORY:INTERNAL=//p' "$cache_file" | head -n 1)
    if [ -n "$cached_source" ] && [ "$cached_source" != "$ROOT" ]; then
      echo "CMake cache points at $cached_source instead of $ROOT"
      return 0
    fi
  fi

  if [ -f "$compile_db" ] && grep -Fq '"/project/' "$compile_db"; then
    echo "compile_commands.json was generated for /project"
    return 0
  fi

  return 1
}

select_build_dir() {
  local reason=""

  if reason=$(stale_build_tree_reason "$BUILD_DIR"); then
    if rm -rf "$BUILD_DIR" 2>/dev/null; then
      echo "Removed stale build tree at $BUILD_DIR"
      echo "Reason: $reason"
      return 0
    fi

    if [ "$BUILD_DIR" != "$FALLBACK_BUILD_DIR" ]; then
      echo "Unable to reset stale build tree at $BUILD_DIR"
      echo "Reason: $reason"
      echo "Falling back to writable build tree at $FALLBACK_BUILD_DIR"
      BUILD_DIR="$FALLBACK_BUILD_DIR"
    fi
  fi
}

select_build_dir
cmake --preset "$PRESET" -B "$BUILD_DIR"
echo "compile_commands.json at ${BUILD_DIR}/"
