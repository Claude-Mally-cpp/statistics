#!/usr/bin/env bash
set -euo pipefail

# --- options -----------------------------------------------------------------
VERBOSE=0
DEBUG=0
while [[ $# -gt 0 ]]; do
  case "$1" in
    --verbose|-v) VERBOSE=1; shift ;;
    --debug) DEBUG=1; shift ;;
    *) break ;;
  esac
done

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

# --- docker path handling (Windows/MINGW/MSYS/Cygwin) ------------------------
HOST_PWD="$PWD"
case "$(uname -s)" in
  MINGW*|MSYS*|CYGWIN*)
    export MSYS_NO_PATHCONV=1
    export MSYS2_ARG_CONV_EXCL="*"
    HOST_PWD="$(cygpath -w "$PWD")"
    ;;
esac

# --- main runner -------------------------------------------------------------
run_build_and_test() {
  local compiler="$1"    # clang | gcc
  local buildType="$2"   # debug | release
  local preset="linux-${compiler}-${buildType}"
  local buildDir="out/build/$preset"
  local installDir="out/install/$preset"
  local image="cpp-ci-${compiler}"

  # Pass flags into the container; bind mount repo
  local DOCKER=(docker run --rm \
    -e DEBUG="$DEBUG" -e VERBOSE="$VERBOSE" \
    -v "${HOST_PWD}:/project:rw" -w /project "$image")

  # CMake caches absolute source/build paths. A cache created on the host under
  # /home/... cannot be safely reused in the container under /project.
  run_and_report "${compiler^} ${buildType^} Cache Check" \
    "${DOCKER[@]}" bash -lc '
      set -e
      BUILD_DIR="'"$buildDir"'"
      INSTALL_DIR="'"$installDir"'"
      CACHE_FILE="$BUILD_DIR/CMakeCache.txt"
      EXPECTED_SOURCE_DIR="/project"
      EXPECTED_BUILD_DIR="/project/$BUILD_DIR"

      if [[ ! -f "$CACHE_FILE" ]]; then
        exit 0
      fi

      CACHE_SOURCE_DIR="$(sed -n "s/^CMAKE_HOME_DIRECTORY:INTERNAL=//p" "$CACHE_FILE" | head -n1)"
      CACHE_BUILD_DIR="$(sed -n "s/^CMAKE_CACHEFILE_DIR:INTERNAL=//p" "$CACHE_FILE" | head -n1)"

      if [[ "$CACHE_SOURCE_DIR" == "$EXPECTED_SOURCE_DIR" && "$CACHE_BUILD_DIR" == "$EXPECTED_BUILD_DIR" ]]; then
        exit 0
      fi

      echo "Removing stale CMake cache for container path remap: $BUILD_DIR"
      echo "  cached source: ${CACHE_SOURCE_DIR:-<missing>}"
      echo "  cached build:  ${CACHE_BUILD_DIR:-<missing>}"
      rm -rf "$BUILD_DIR" "$INSTALL_DIR"
    '

  run_and_report "${compiler^} ${buildType^} Configure" \
    "${DOCKER[@]}" cmake --preset "$preset"

  run_and_report "${compiler^} ${buildType^} Build" \
    "${DOCKER[@]}" cmake --build "$buildDir"

  if [[ "$compiler" == "clang" ]]; then
    run_and_report "${compiler^} ${buildType^} Clang-Tidy" \
      "${DOCKER[@]}" bash -lc '
        set -e
        if ! command -v clang-tidy >/dev/null 2>&1; then
          echo "clang-tidy not found in image"; exit 1
        fi
        PRESET="'"$preset"'" DB="'"$buildDir"'" bash ./tidy-run-checks.sh
      '
  fi

  # --- Cppcheck --------------------------------------------------------------
  if [[ "$compiler" == "gcc" ]]; then
    run_and_report "${compiler^} ${buildType^} Cppcheck" \
      "${DOCKER[@]}" bash -lc '
        # debug helper
        if [ "${DEBUG:-0}" -eq 1 ]; then
          echo "Mount info:"
          mount | grep " /project " || true
          echo "HEAD: $(git -C /project rev-parse --short HEAD 2>/dev/null || echo n/a)"
          echo "test/test_statistics.cpp (60-70):"
          sed -n "60,70p" test/test_statistics.cpp || true
        fi
        set -e
        BUILD_DIR="'"$buildDir"'"

        if ! command -v cppcheck >/dev/null 2>&1; then
          echo "cppcheck not found in image"; exit 1
        fi

        # Exclude common vendored locations (kept for safety with dir scan)
        EXCLUDES=()
        [[ -d "$BUILD_DIR/_deps" ]] && EXCLUDES+=( -i "$BUILD_DIR/_deps" )
        [[ -d "$BUILD_DIR/_cpm"  ]] && EXCLUDES+=( -i "$BUILD_DIR/_cpm" )
        [[ -d "third_party"     ]] && EXCLUDES+=( -i "third_party" )
        [[ -d "external"        ]] && EXCLUDES+=( -i "external" )

        XML="$BUILD_DIR/cppcheck.xml"
        set +e
        cppcheck include test -I include --language=c++ --std=c++20 \
          "${EXCLUDES[@]}" \
          --enable=warning,style,performance,portability,information,missingInclude \
          --inline-suppr --suppress=missingIncludeSystem \
          --quiet --xml --xml-version=2 2> "$XML"
        rc=$?; set -e
        [ "${DEBUG:-0}" -eq 1 ] && echo "cppcheck rc (gcc dir scan): $rc"
        if grep -q '\''severity="error"'\'' "$XML"; then
          echo "cppcheck: found severity=error issues (gcc dir scan)"; exit 2
        else
          exit 0
        fi
      '
  else
    run_and_report "${compiler^} ${buildType^} Cppcheck" \
      "${DOCKER[@]}" bash -lc '
        set -e
        BUILD_DIR="'"$buildDir"'"

        if ! command -v cppcheck >/dev/null 2>&1; then
          echo "cppcheck not found in image"; exit 1
        fi

        # Use compile_commands.json if available; fall back to dir scan
        if [[ -f "$BUILD_DIR/compile_commands.json" ]]; then
          set +e
          cppcheck --project="$BUILD_DIR/compile_commands.json" \
            --enable=warning,style,performance,portability,information,missingInclude \
            --inline-suppr --std=c++20 --suppress=missingIncludeSystem --quiet
          rc=$?; set -e
        else
          set +e
          cppcheck include test -I include --language=c++ --std=c++20 \
            --enable=warning,style,performance,portability,information,missingInclude \
            --inline-suppr --suppress=missingIncludeSystem --quiet
          rc=$?; set -e
        fi

        [ $rc -ne 0 ] && exit $rc || exit 0
      '
  fi

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
