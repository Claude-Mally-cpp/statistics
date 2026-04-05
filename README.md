# statistics

[![clang-tidy](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/clang-tidy.yml/badge.svg)](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/clang-tidy.yml)
[![clang-format](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/clang-format.yml/badge.svg)](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/clang-format.yml)
[![msvc](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/msvc.yml/badge.svg)](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/msvc.yml)
[![gcc](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/gcc.yml/badge.svg)](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/gcc.yml)
[![clang](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/clang.yml/badge.svg)](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/clang.yml)
[![cppcheck](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/cppcheck.yml/badge.svg)](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/cppcheck.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/Claude-Mally-cpp/statistics/blob/main/LICENSE)

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

#### Recommended flow (preferred): VS Code + CMake Tools

Use this path when working in the editor (matches what you already tested):

1. Install [Microsoft C++ Build Tools / Visual Studio](https://visualstudio.microsoft.com/) and ensure `cl.exe` is available in your shell.
2. Install the CMake Tools extension.
3. Open this repository in VS Code.
4. Select `msvc-x64-debug` (or `msvc-x64-release`) from:
   - status bar, or `Ctrl+Shift+P` -> `CMake: Select Configure Preset`
5. Use CMake Tools commands: Configure, Build, then Run Tests.

#### Alternative: terminal/manual preset commands

Use these only if you prefer explicit commands in PowerShell:

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

## Local Clang Toolchain Setup (WSL / Linux)

To match the Docker image and CI exactly, install LLVM 22 on your local WSL or Linux
machine using the same repository that `Dockerfile.clang` uses:

```bash
sudo apt-get update && sudo apt-get install -y wget gpg
wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key \
  | gpg --dearmor \
  | sudo tee /usr/share/keyrings/llvm-archive-keyring.gpg >/dev/null
echo "deb [signed-by=/usr/share/keyrings/llvm-archive-keyring.gpg] \
http://apt.llvm.org/noble/ llvm-toolchain-noble-22 main" \
  | sudo tee /etc/apt/sources.list.d/llvm.list
sudo apt-get update
sudo apt-get install -y \
  clang-22 \
  clang-format-22 \
  clang-tidy-22 \
  libc++-22-dev \
  libc++abi-22-dev
```

> **Note:** The commands above target Ubuntu 24.04 Noble (the same base image as
> `Dockerfile.clang`). Adjust `noble` / `llvm-toolchain-noble-22` if you are on a
> different Ubuntu release.

After installation, verify that the versions match:

```bash
clang-22 --version
clang++-22 --version
clang-format-22 --version
clang-tidy-22 --version
```

CMake presets (`linux-clang-*`) already pin `CMAKE_C_COMPILER=clang-22` and
`CMAKE_CXX_COMPILER=clang++-22`, so no extra configuration is needed.

To run `clang-tidy` locally with the same settings as CI:

```bash
bash ./clang-tidy-prepare.sh
bash ./clang-tidy-run-checks.sh
```

To run the format check locally with the same settings as CI:

```bash
CLANG_FORMAT=clang-format-22 bash ./check-format.sh
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

- To run static analysis locally (and in CI) after configuring a build, use:

```sh
cppcheck include test \
  -I include \
  -I out/build/msvc-x64-debug/_deps/fmt-src/include \
  -I out/build/msvc-x64-debug/_deps/googletest-src/googletest/include \
  --language=c++ --std=c++23 \
  --enable=warning,style,performance,portability,information,missingInclude \
  --inline-suppr --suppress=missingIncludeSystem \
  -i out/build/msvc-x64-debug/_deps
```

Make sure `cppcheck` is available on the runner (for Windows use `choco install cppcheck -y`).

## Local Verification

Use the repo verification wrapper for the common local paths:

```bash
# fast local check
./verify.sh quick

# broader pre-CI pass
./verify.sh full
```

`quick` runs the repo-wide format check.

`full` runs:
- format check
- Windows build/test on Windows hosts
- `cppcheck` when available and when a configured build tree with fetched deps already exists
- Docker image rebuild plus Linux build/test when Docker is available

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

### 3. Use the tracked pre-commit hook

```bash
chmod +x .githooks/pre-commit check-format-staged.sh verify.sh
```

### 4. Test it

Edit any .cpp or .hpp file to break formatting and try:

```bash
git add .
git commit -m "test hook"
```
