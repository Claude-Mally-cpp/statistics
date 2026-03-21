# CMake tips and tricks

## Clearing the docker cache

When CMake starts complaining that CMakeCache.txt was created from a different source or build directory, clear the cached build directory and reconfigure from scratch. In this repo, that usually means deleting out/build/<preset> and, if needed, out/install/<preset>, then rerunning the preset, for example rm -rf out/build/linux-clang-debug out/install/linux-clang-debug && cmake --preset linux-clang-debug. This is the safest fix when switching between local builds and Docker paths like /home/... vs /project.