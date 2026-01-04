#!/usr/bin/env bash
set -euo pipefail

exec ./clang-tidy-run-checks.sh "$@"
