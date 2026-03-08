# statistics
playing around with statistics

> NOTE: This project documents the quartile convention in `CHANGELOG.md` (Tukey hinges, exclusive-median by default; small-sample exception for size==3).

## Running Linux Builds and Tests with Docker

### 1. Build the Docker Images

Before running the build and test script, you need to build the Docker images for each toolchain:

```sh
docker build -f Dockerfile.gcc -t cpp-ci-gcc .
docker build -f Dockerfile.clang -t cpp-ci-clang .
```

### 2. Build and Test the Project

To build and test this project for multiple Linux toolchains using Docker, use the provided script:

```sh
# terse output
./dockerLinuxBuildAndTest.sh

# verbose output
./dockerLinuxBuildAndTest.sh --verbose
```

To buld and test on windows release and debug

```powershell
# terse output
./windowsBuildAndTest.ps1

# verbose output
./windowsBuildAndTest.ps1 --verbose
```

### VS Code + MSVC workflow (Windows)

Recommended workflow when using Visual Studio/C++ toolchain in VS Code:

1. Install [Microsoft C++ Build Tools / Visual Studio](https://visualstudio.microsoft.com/) and ensure `cl.exe` is on PATH for your terminal session.
2. Install the CMake Tools extension in VS Code.
3. Open this repository in VS Code.
4. In the status bar or command palette, choose:

```text
CMake: Select Configure Preset -> msvc-x64-debug
```

5. Configure, build, and test with either CMake Tools or terminal:

```powershell
cmake --preset msvc-x64-debug
cmake --build --preset msvc-x64-debug
ctest --preset msvc-x64-debug
```

For release:

```powershell
cmake --preset msvc-x64-release
cmake --build --preset msvc-x64-release
ctest --preset msvc-x64-release
```

## Conventions and notes


- Quartile convention: this project uses the Tukey hinge approach. By default we use the
  exclusive-median variant (the median element is excluded from both lower and upper halves
  when computing Q1 and Q3). For very small samples there is a special-case: when the sample
  size is exactly 3 the implementation follows the common textbook convention and includes
  the median in both halves.

  Unit tests in `test/test_statistics.cpp` were written to match this behavior. If you
  depend on a different quartile convention you should materialize and compute quartiles
  explicitly (the library exposes constexpr array-based helpers for `std::array` inputs).

- To run static analysis locally (and in CI) on the reorganized layout use:

```sh
cppcheck ./include/* ./test/*
```

Make sure `cppcheck` is available on the runner (for Windows use `choco install cppcheck -y`).

## Enforcing Code Formatting Before Commit

This project uses **clang-format** to ensure consistent code style.  
You can install the formatting check as a local **Git pre-commit hook** so that
any incorrectly formatted code is caught before you commit.

### 1. Verify clang-format is installed
On Windows (Git Bash, PowerShell, or CMD):
```bash
clang-format --version
```

If not found, install LLVM (https://github.com/llvm/llvm-project/releases
) and
select “Add LLVM to system PATH”.

### 2. Enable the pre-commit hook

Run once in the repository root:

```bash
mkdir -p .githooks
git config core.hooksPath .githooks
```

### 3. Create .githooks/pre-commit

```bash
#!/usr/bin/env bash
set -euo pipefail

# Collect staged C/C++ files
mapfile -d '' FILES < <(git diff --cached --name-only -z --diff-filter=ACMR | \
  grep -zE '\.(c|cc|cxx|cpp|h|hh|hpp|hxx)$' || true)

# Skip if nothing relevant is staged
if (( ${#FILES[@]} == 0 )); then
  exit 0
fi

echo "Running clang-format check on staged files..."
if ! printf '%s\0' "${FILES[@]}" | xargs -0 clang-format --dry-run --Werror >/dev/null; then
  echo
  echo "❌ Formatting required. Run: clang-format -i <files> (or ./auto-format.sh)"
  echo "   Then re-stage and commit again."
  exit 1
fi

echo "✅ Formatting looks good."
```

### 4. Test it

Edit any .cpp or .hpp file to break formatting and try:

```bash
git add .
git commit -m "test hook"
```
