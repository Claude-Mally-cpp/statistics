#!/usr/bin/env bash
set -euo pipefail

MODE="generate"
if [[ $# -gt 1 ]]; then
  echo "Usage: $0 [--verify|--clean]"
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
    *)
      echo "Usage: $0 [--verify|--clean]"
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
else
  echo "Generating Doxygen HTML documentation"
fi

doxygen "$DOXYFILE"

if [[ "$MODE" == "verify" ]]; then
  echo "Doxygen verification passed"
else
  echo "Generated HTML docs under $OUTPUT_DIR/html"
fi
