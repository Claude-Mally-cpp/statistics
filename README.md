# statistics

[![clang-tidy](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/clang-tidy.yml/badge.svg)](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/clang-tidy.yml)
[![clang-format](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/clang-format.yml/badge.svg)](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/clang-format.yml)
[![msvc](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/msvc.yml/badge.svg)](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/msvc.yml)
[![gcc](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/gcc.yml/badge.svg)](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/gcc.yml)
[![clang](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/clang.yml/badge.svg)](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/clang.yml)
[![sanitizers](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/sanitizers.yml)
[![cppcheck](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/cppcheck.yml/badge.svg)](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/cppcheck.yml)
[![doxygen](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/doxygen.yml/badge.svg)](https://github.com/Claude-Mally-cpp/statistics/actions/workflows/doxygen.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/Claude-Mally-cpp/statistics/blob/main/LICENSE)

Modern C++ statistics library and CLI for small descriptive-statistics workflows.

This repository is for people who want a compact C++23 project that computes common summary statistics, exposes a reusable library, and keeps build and analysis tooling visible.

If you prefer an editor-driven workflow, the CMake presets also work well from VS Code with CMake Tools.

Right now the project focuses on:

- descriptive statistics such as mean, median, quartiles, variance, standard deviation, range, MAD, z-scores, covariance, correlation, and modes
- a header-only library target for use from other C++ code
- a small CLI that computes summary statistics from comma-separated input
- multi-toolchain verification with Clang, GCC, MSVC, sanitizers, `clang-format`, `clang-tidy`, and `cppcheck`

> NOTE: This project documents the quartile convention in `CHANGELOG.md` (Tukey hinges, exclusive-median by default; small-sample exception for size==3).

## Quick Start

Linux / WSL:

```bash
cmake --preset linux-clang-debug
cmake --build --preset linux-clang-debug
ctest --preset linux-clang-debug
./verify.sh quick
```

Windows PowerShell:

```powershell
cmake --preset msvc-x64-debug
cmake --build --preset msvc-x64-debug
ctest --preset msvc-x64-debug
```

VS Code:

- open the folder
- select a CMake preset such as `linux-clang-debug` or `msvc-x64-debug`
- run Configure, Build, and Test from CMake Tools

If you just want to build the CLI:

```bash
cmake --build --preset linux-clang-debug-cli
./out/build/linux-clang-debug/statistics summary --data 1,2,2,3,5
```

## Example

The CLI currently exposes a `summary` subcommand:

```bash
./out/build/linux-clang-debug/statistics summary --data 1,2,2,3,5
```

Expected output format:

```text
Computed summary: n=5, min=1, q1=1.5, median=2, q3=4, max=5, mean=2.6
```

## Result Type Policy

Public result types are grouped by behavior:

- Natural value type: `minMaxValue`
- Widened integral result for integral inputs: `sum`, `product`, `sumSquared`
- Natural input value type inside `std::expected<std::vector<T>, std::string>`: `modes`
- Statistical/public result policy: `average`, `median`, `quartiles`, `summary`, `variance`, `standardDeviation`, `medianAbsoluteDeviation`, `zScores`, `correlationCoefficient`, `covariance`
- Widened arithmetic difference for integral inputs: `range`

Examples:

- `sum(range<int>)` returns a widened integral type rather than `int`
- `product(range<int>)` and `sumSquared(range<int>)` also return widened integral types
- `minMaxValue(range<int>)` preserves the input value type
- `modes(range<int>)` returns repeated modes as `std::vector<int>` on success
- `average(range<int>)`, `median(range<int>)`, and other statistical outputs follow the library's statistical public result policy
- `range(range<int>)` returns a widened integral difference rather than `int`

Variance and standard deviation accept `VarianceKind` with `VarianceKind::sample` as the default. `medianAbsoluteDeviation` currently returns the raw MAD, with no robustness scaling constant applied.

Input behavior for the newer descriptive-statistics APIs:

- `variance(range)` returns an error for empty input, and sample variance also returns an error when fewer than 2 values are provided
- `standardDeviation(range)` propagates `variance(...)` input errors
- `range(range)` returns an error for empty input
- `medianAbsoluteDeviation(range)` returns an error for empty input
- `zScores(range)` returns an error for empty input or when the standard deviation is zero

Internal calculation may widen independently from the public result type. For example, a function may accumulate in a wider type for stability while still returning either a natural value type, a widened integral helper type, or a statistical/public result type.

## Tooling and Platform Setup

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
