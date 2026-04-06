#!/usr/bin/env bash
set -euo pipefail

MODE="generate"
if [[ $# -gt 1 ]]; then
  echo "Usage: $0 [--verify|--verify-all|--clean]"
  exit 2
fi

if [[ $# -eq 1 ]]; then
  case "$1" in
    --verify)
      MODE="verify"
      ;;
    --clean)
      MODE="clean"
      ;;
    --verify-all)
      MODE="verify-all"
      ;;
    *)
      echo "Usage: $0 [--verify|--verify-all|--clean]"
      exit 2
      ;;
  esac
fi

OUTPUT_DIR="out/docs/doxygen"
DOXYFILE="${DOXYFILE:-Doxyfile}"

if [[ "$MODE" == "clean" ]]; then
  rm -rf "$OUTPUT_DIR"
  echo "Removed $OUTPUT_DIR"
  exit 0
fi

if ! command -v doxygen >/dev/null 2>&1; then
  echo "doxygen not found in PATH"
  exit 1
fi

mkdir -p "$OUTPUT_DIR"

echo "Using Doxygen:"
doxygen --version

if [[ "$MODE" == "verify" ]]; then
  echo "Verifying Doxygen configuration and warnings"
elif [[ "$MODE" == "verify-all" ]]; then
  echo "Verifying Doxygen configuration and listing all warnings"
else
  echo "Generating Doxygen HTML documentation"
fi

if [[ "$MODE" == "verify-all" ]]; then
  WARN_LOG="$(mktemp)"
  set +e
  doxygen - <<EOF >/dev/null
@INCLUDE = $DOXYFILE
WARN_LOGFILE = $WARN_LOG
EOF
  DOXYGEN_EXIT_CODE=$?
  set -e

  HAS_WARNINGS=0
  if [[ -s "$WARN_LOG" ]]; then
    echo "Doxygen reported warnings:"
    cat "$WARN_LOG"
    HAS_WARNINGS=1
  fi

  rm -f "$WARN_LOG"

  if [[ $HAS_WARNINGS -ne 0 || $DOXYGEN_EXIT_CODE -ne 0 ]]; then
    exit 1
  fi
else
  doxygen "$DOXYFILE"
fi

if [[ "$MODE" == "verify" ]]; then
  echo "Doxygen verification passed"
elif [[ "$MODE" == "verify-all" ]]; then
  echo "Doxygen verification passed with no warnings"
else
  echo "Generated HTML docs under $OUTPUT_DIR/html"
fi
