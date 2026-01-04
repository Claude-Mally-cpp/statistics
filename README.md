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

## Code Modernization (clang-tidy)

The modernization scripts are intended for WSL or Linux where `clang-tidy` is
available. They are not set up for Windows.

```sh
# configure to generate compile_commands.json
PRESET=linux-clang-release ./clang-tidy-prepare.sh

# dry run (report only)
PRESET=linux-clang-release ./clang-tidy-run-checks.sh

# apply fixes
PRESET=linux-clang-release ./clang-tidy-run-checks.sh --fix
```
