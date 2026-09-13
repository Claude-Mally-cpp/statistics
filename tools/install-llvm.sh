#!/usr/bin/env bash
set -euo pipefail

# Install the stable release, rather than apt.llvm.org's moving branch snapshots.
# Requires root, curl, xz-utils, ca-certificates and the normal C++ runtime.
version=23.1.1
prefix="/opt/llvm-${version}"
case "$(uname -m)" in
  x86_64)
    platform=X64
    sha256=832aeb58d105de1cabc7b982dd2c65de0610f7377df48ae8fc2dd8e97420a15c
    triple=x86_64-unknown-linux-gnu
    ;;
  aarch64)
    platform=ARM64
    sha256=3fbaaa6a1f147557a4095b9911f8dc2d4c745a11863982f76ee20e248c190a80
    triple=aarch64-unknown-linux-gnu
    ;;
  *) echo "Unsupported LLVM release architecture: $(uname -m)" >&2; exit 1 ;;
esac

archive="$(mktemp)"
trap 'rm -f "$archive"' EXIT
curl --fail --location --silent --show-error --retry 3 \
  "https://github.com/llvm/llvm-project/releases/download/llvmorg-${version}/LLVM-${version}-Linux-${platform}.tar.xz" \
  --output "$archive"
printf '%s  %s\n' "$sha256" "$archive" | sha256sum --check -

# Keep the compiler, analysis/coverage tools and C++/sanitizer runtimes, without
# the archive's unrelated LLVM/MLIR/Fortran development libraries and executables.
tools=(clang clang++ clang-23 clang-format clang-tidy clang-apply-replacements
       clang-scan-deps llvm-ar llvm-ranlib llvm-nm llvm-cov llvm-profdata
       llvm-symbolizer lld ld.lld)
members=("*/include/c++" "*/include/${triple}" "*/lib/clang" "*/lib/${triple}")
for tool in "${tools[@]}"; do members+=("*/bin/${tool}"); done
mkdir -p "$prefix"
tar -xJf "$archive" --strip-components=1 --wildcards --no-anchored \
  -C "$prefix" "${members[@]}"

for tool in "${tools[@]}"; do
  ln -sfn "$prefix/bin/$tool" "/usr/local/bin/$tool"
done
for tool in clang++ clang-format clang-tidy llvm-cov llvm-profdata; do
  ln -sfn "$tool" "$prefix/bin/${tool}-23"
  ln -sfn "$prefix/bin/$tool" "/usr/local/bin/${tool}-23"
done
# clang-tidy locates libc++ relative to the compiler path in the compilation
# database. Prefer paths inside the installation over /usr/local/bin links.
printf 'export PATH="%s/bin:$PATH"\n' "$prefix" > /etc/profile.d/llvm-23.sh
printf '%s\n' "$prefix/lib/$triple" > /etc/ld.so.conf.d/llvm-23.conf
ldconfig
clang-23 --version
