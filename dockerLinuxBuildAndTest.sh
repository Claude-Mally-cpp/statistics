#!/usr/bin/env bash
set -euo pipefail

# --- options -----------------------------------------------------------------
VERBOSE=0
if [[ "${1:-}" == "--verbose" ]]; then VERBOSE=1; shift; fi

# --- colors ------------------------------------------------------------------
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'
summary=""

# --- helpers -----------------------------------------------------------------
run_and_report() {
  local label="$1"; shift
  echo -e "${YELLOW}==> $label${NC}"

  set +e
  if (( VERBOSE )); then
    "$@"
  else
    "$@" >/dev/null 2>&1
  fi
  local rc=$?
  set -e

  if (( rc == 0 )); then
    echo -e "${GREEN}$label: SUCCESS${NC}"
    summary+="$label: SUCCESS\n"
  else
    echo -e "${RED}$label: FAILURE${NC}"
    summary+="$label: FAILURE\n"
  fi
  return $rc
}

# --- docker path handling (fix for Git Bash on Windows) ----------------------
export MSYS_NO_PATHCONV=1
export MSYS2_ARG_CONV_EXCL="*"

HOST_PWD="$PWD"
case "${OSTYPE:-}" in
  msys*|cygwin*)
    if command -v pwd >/dev/null 2>&1; then
      HOST_PWD="$(pwd -W 2>/dev/null | sed 's#\\#/#g')"
    fi
    ;;
esac

# --- main runner -------------------------------------------------------------
run_build_and_test() {
  local compiler="$1"    # clang | gcc
  local buildType="$2"   # debug | release
  local preset="linux-${compiler}-${buildType}"
  local buildDir="out/build/$preset"
  local image="cpp-ci-${compiler}"

  local DOCKER=(docker run --rm -v "${HOST_PWD}:/project" -w /project "$image")

  run_and_report "${compiler^} ${buildType^} Configure" \
    "${DOCKER[@]}" cmake --preset "$preset"

  run_and_report "${compiler^} ${buildType^} Build" \
    "${DOCKER[@]}" cmake --build "$buildDir"

  run_and_report "${compiler^} ${buildType^} Test" \
    "${DOCKER[@]}" "$buildDir/statistics_test"
}

# --- calls -------------------------------------------------------------------
run_build_and_test clang debug
run_build_and_test clang release
run_build_and_test gcc debug
run_build_and_test gcc release

echo -e "\n========== SUMMARY =========="
echo -e "$summary"
