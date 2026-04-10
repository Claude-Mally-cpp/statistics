#!/usr/bin/env bash
set -euo pipefail

PRESET=${PRESET:-linux-clang-debug}
PRESET="$PRESET" bash ./clang-tidy-prepare.sh
