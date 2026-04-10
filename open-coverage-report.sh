#!/usr/bin/env bash
set -euo pipefail

PRESET="${1:-linux-clang-coverage}"
REPORT_PATH="out/coverage/${PRESET}/html/index.html"

if [[ ! -f "${REPORT_PATH}" ]]; then
  echo "Coverage report not found: ${REPORT_PATH}" >&2
  exit 1
fi

case "$(uname -s)" in
  MINGW*|MSYS*|CYGWIN*)
    if command -v cygpath >/dev/null 2>&1; then
      WIN_PATH="$(cygpath -w "${REPORT_PATH}")"
      cmd.exe /c start "" "${WIN_PATH}" >/dev/null 2>&1
      exit 0
    fi
    ;;
esac

if command -v xdg-open >/dev/null 2>&1; then
  xdg-open "${REPORT_PATH}" >/dev/null 2>&1 &
  exit 0
fi

if command -v open >/dev/null 2>&1; then
  open "${REPORT_PATH}"
  exit 0
fi

echo "No supported opener found for ${REPORT_PATH}" >&2
exit 1
