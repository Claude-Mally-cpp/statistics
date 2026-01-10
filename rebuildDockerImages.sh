#!/usr/bin/env bash
set -euo pipefail

show_usage() {
  cat <<'EOF'
Usage: bash ./rebuildDockerImages.sh [--keepcache]

Build the GCC and Clang Docker images. By default, removes out/build and
out/install to avoid stale CMake cache paths.

Options:
  --keepcache   Preserve out/build and out/install
  -h, --help    Show this help and exit
EOF
}

keep_cache=false
for arg in "$@"; do
  if [[ "$arg" == "--keepcache" ]]; then
    keep_cache=true
  elif [[ "$arg" == "-h" || "$arg" == "--help" ]]; then
    show_usage
    exit 0
  else
    echo "Unknown option: $arg" >&2
    show_usage >&2
    exit 2
  fi
done

if [[ "$keep_cache" == "false" ]]; then
  rm -rf out/build out/install
fi

docker build -f Dockerfile.gcc -t cpp-ci-gcc .
docker build -f Dockerfile.clang -t cpp-ci-clang .
