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
  local image="cpp-ci-${compiler}"

  # Pass flags into the container; bind mount repo
  local DOCKER=(docker run --rm \
    -e DEBUG="$DEBUG" -e VERBOSE="$VERBOSE" \
    -v "${HOST_PWD}:/project:rw" -w /project "$image")

  run_and_report "${compiler^} ${buildType^} Configure" \
    "${DOCKER[@]}" cmake --preset "$preset"

  run_and_report "${compiler^} ${buildType^} Build" \
    "${DOCKER[@]}" cmake --build "$buildDir"

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

        if [[ -f "$BUILD_DIR/compile_commands.json" ]]; then
          if command -v jq >/dev/null 2>&1; then
            [ "${DEBUG:-0}" -eq 1 ] && echo "Using filtered compile_commands.json"
            jq '"'"'[ .[] | select(.file | test("^(?:/project/)?(include|test)/")) ]'"'"' \
              "$BUILD_DIR/compile_commands.json" > "$BUILD_DIR/compile_commands.local.json"

            COUNT="$(jq -r "length" "$BUILD_DIR/compile_commands.local.json")"
            [ "${DEBUG:-0}" -eq 1 ] && echo "Filtered compile DB entries: $COUNT"
            XML="$BUILD_DIR/cppcheck.xml"

            if [ "$COUNT" -eq 0 ]; then
              [ "${DEBUG:-0}" -eq 1 ] && echo "Filtered DB empty; falling back to dir scan"
              set +e
              cppcheck include test -I include --language=c++ --std=c++20 \
                --enable=warning,style,performance,portability,information,missingInclude \
                --inline-suppr --suppress=missingIncludeSystem \
                --quiet --xml --xml-version=2 2> "$XML"
              rc=$?; set -e
              [ "${DEBUG:-0}" -eq 1 ] && echo "cppcheck rc (fallback dir scan): $rc"
            else
              set +e
              cppcheck \
                --project="$BUILD_DIR/compile_commands.local.json" \
                --enable=warning,style,performance,portability,information,missingInclude \
                --inline-suppr --std=c++20 \
                --suppress=missingIncludeSystem \
                --quiet --xml --xml-version=2 2> "$XML"
              rc=$?; set -e
              [ "${DEBUG:-0}" -eq 1 ] && echo "cppcheck rc (project mode): $rc"
            fi

            if grep -q '\''severity="error"'\'' "$XML"; then
              echo "cppcheck: found severity=error issues"; exit 2
            else
              exit 0
            fi
          else
            [ "${DEBUG:-0}" -eq 1 ] && echo "jq not found; using full compile_commands.json"
            XML="$BUILD_DIR/cppcheck.xml"
            set +e
            cppcheck \
              --project="$BUILD_DIR/compile_commands.json" \
              --enable=warning,style,performance,portability,information,missingInclude \
              --inline-suppr --std=c++20 \
              --suppress=missingIncludeSystem \
              --quiet --xml --xml-version=2 2> "$XML"
            rc=$?; set -e
            [ "${DEBUG:-0}" -eq 1 ] && echo "cppcheck rc (unfiltered project): $rc"
            if grep -q '\''severity="error"'\'' "$XML"; then
              echo "cppcheck: found severity=error issues (unfiltered project)"; exit 2
            else
              exit 0
            fi
          fi
        else
          [ "${DEBUG:-0}" -eq 1 ] && echo "No compile_commands.json; scanning source dirs"
          XML="$BUILD_DIR/cppcheck.xml"
          set +e
          cppcheck include source test -I include --language=c++ --std=c++20 \
            --suppress="*:*/_deps/*" --suppress="*:*/third_party/*" --suppress="*:*/external/*" \
            --enable=warning,style,performance,portability,information,missingInclude \
            --inline-suppr --suppress=missingIncludeSystem \
            --quiet --xml --xml-version=2 2> "$XML"
          rc=$?; set -e
          [ "${DEBUG:-0}" -eq 1 ] && echo "cppcheck rc (no project, dir scan): $rc"
          if grep -q '\''severity="error"'\'' "$XML"; then
            echo "cppcheck: found severity=error issues (no project, dir scan)"; exit 2
          else
            exit 0
          fi
        fi
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
