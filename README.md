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
