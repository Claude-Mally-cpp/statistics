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

To build and test on Windows release and debug:

```powershell
# terse output
./windowsBuildAndTest.ps1

# verbose output
./windowsBuildAndTest.ps1 --verbose
```

## Tooling Workflows

### WSL / Linux: LLVM 22 workflow

Use this path when you want local Clang, `clang-format`, and `clang-tidy` behavior
to match the Docker image and Linux CI.

Install LLVM 22 from the same apt repository used by `Dockerfile.clang`:

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

Verify the installed tools:

```bash
clang-22 --version
clang++-22 --version
clang-format-22 --version
clang-tidy-22 --version
```

The Linux Clang presets already pin the versioned compiler names:

```bash
cmake --preset linux-clang-debug
cmake --build --preset linux-clang-debug
ctest --preset linux-clang-debug
```

To run local checks with the same binaries CI expects:

```bash
CLANG_FORMAT=clang-format-22 bash ./check-format.sh
bash ./clang-tidy-prepare.sh
CLANG_TIDY_BIN=clang-tidy-22 bash ./clang-tidy-run-checks.sh
```

> **Note:** The commands above target Ubuntu 24.04 Noble, matching
> `Dockerfile.clang`. Adjust `noble` / `llvm-toolchain-noble-22` if your local
> Ubuntu release differs.

### Windows: recommended workflow

Use Windows primarily for native MSVC configure/build/test. Use WSL or Docker for
Linux Clang parity.

Recommended local Windows setup:

1. Install [Microsoft C++ Build Tools / Visual Studio](https://visualstudio.microsoft.com/) and ensure `cl.exe` is available.
2. Install the VS Code CMake Tools extension if you work from the editor.
3. Optionally install LLVM tools if you want local `clang-format` or ad hoc Clang tooling.

For a simple LLVM install on Windows:

```powershell
winget install LLVM.LLVM
```

Then verify what Windows resolves:

```powershell
where clang
where clang-format
where clang-tidy
clang-format --version
```

Preferred VS Code + CMake Tools flow:

1. Open the repository in VS Code.
2. Select `msvc-x64-debug` or `msvc-x64-release`.
3. Run Configure, Build, and Test from CMake Tools.

Equivalent PowerShell commands:

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

### Practical contributor split

- Windows native: MSVC build/test
- WSL or Linux: LLVM 22 format/tidy/build workflow
- Docker: closest match to Linux CI
- GitHub Actions: final clean-runner authority

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
