#!/usr/bin/env bash
set -euo pipefail

PRESET="${1:-linux-clang-coverage}"
BUILD_DIR="out/build/${PRESET}"
REPORT_DIR="out/coverage/${PRESET}"
RAW_PROFILE="${REPORT_DIR}/statistics_test.profraw"
MERGED_PROFILE="${REPORT_DIR}/statistics_test.profdata"
LCOV_REPORT="${REPORT_DIR}/coverage.lcov"
SUMMARY_REPORT="${REPORT_DIR}/summary.txt"
HTML_DIR="${REPORT_DIR}/html"
IGNORE_REGEX='(^|/)(out/build|build|_deps|third_party|external|vendor)/'

find_llvm_tool() {
  local versioned="$1"
  local fallback="$2"
  if command -v "$versioned" >/dev/null 2>&1; then
    echo "$versioned"
    return 0
  fi
  if command -v "$fallback" >/dev/null 2>&1; then
    echo "$fallback"
    return 0
  fi
  echo "Missing LLVM tool: tried '$versioned' and '$fallback'" >&2
  exit 1
}

LLVM_COV_BIN="${LLVM_COV_BIN:-$(find_llvm_tool llvm-cov-22 llvm-cov)}"
LLVM_PROFDATA_BIN="${LLVM_PROFDATA_BIN:-$(find_llvm_tool llvm-profdata-22 llvm-profdata)}"

echo "Using preset: ${PRESET}"
echo "Using llvm-cov: ${LLVM_COV_BIN}"
echo "Using llvm-profdata: ${LLVM_PROFDATA_BIN}"

cmake --preset "${PRESET}"
cmake --build "${BUILD_DIR}"

rm -rf "${REPORT_DIR}"
mkdir -p "${HTML_DIR}"

LLVM_PROFILE_FILE="${RAW_PROFILE}" "${BUILD_DIR}/statistics_test"

"${LLVM_PROFDATA_BIN}" merge -sparse "${RAW_PROFILE}" -o "${MERGED_PROFILE}"

"${LLVM_COV_BIN}" report "${BUILD_DIR}/statistics_test" \
  -instr-profile="${MERGED_PROFILE}" \
  -ignore-filename-regex="${IGNORE_REGEX}" \
  > "${SUMMARY_REPORT}"

"${LLVM_COV_BIN}" export "${BUILD_DIR}/statistics_test" \
  -instr-profile="${MERGED_PROFILE}" \
  -format=lcov \
  -ignore-filename-regex="${IGNORE_REGEX}" \
  > "${LCOV_REPORT}"

"${LLVM_COV_BIN}" show "${BUILD_DIR}/statistics_test" \
  -instr-profile="${MERGED_PROFILE}" \
  -format=html \
  -output-dir="${HTML_DIR}" \
  -ignore-filename-regex="${IGNORE_REGEX}" \
  >/dev/null

echo "Coverage summary: ${SUMMARY_REPORT}"
echo "Coverage LCOV: ${LCOV_REPORT}"
echo "Coverage HTML: ${HTML_DIR}/index.html"
