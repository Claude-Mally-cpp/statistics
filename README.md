# statistics
playing around with statistics

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

- Quartile convention: this project uses the Tukey hinge approach (median included in both halves)
	when computing Q1 and Q3. Unit tests in `test/test_statistics.cpp` assert this behavior.

- To run static analysis locally (and in CI) on the reorganized layout use:

```sh
cppcheck ./include/* ./test/*
```

Make sure `cppcheck` is available on the runner (for Windows use `choco install cppcheck -y`).
