# Windows Clang Notes For Contributors

This note is for contributors working on this repository from Windows.

## Recommended approach

The safest way to contribute from Windows is:

1. Match the repository's `clang-format` major version locally.
2. Run local formatting checks and local MSVC builds/tests.
3. Use Docker for Linux `gcc` / `clang` validation when available.
4. Let GitHub Actions be the final clean-runner authority.

## What matters most

The most important tool to match exactly is `clang-format`.

Formatting disagreements are usually caused by version drift, so contributors should keep their local `clang-format` on the same major version used by CI and Docker. For this repository, `19.x` is the intended target.

## Minimum Windows setup

Contributors on Windows should aim to have:

- `clang-format` 19.x
- MSVC build tools
- CMake and Ninja

With that setup, you can already do useful local validation even without a full LLVM installation.

## Local checks that are expected to work well

These are the most useful local checks for Windows contributors:

- formatting checks
- MSVC configure/build/test
- repository helper scripts such as `./verify.sh quick`

If Docker is available on your machine, also run:

- `./rebuildDockerImages.sh`
- `./dockerLinuxBuildAndTest.sh`

## About clang-tidy on Windows

`clang-tidy` is optional for local Windows contributors.

It depends on:

- LLVM tooling being installed locally
- a usable compile database for the relevant build tree

If you do not have those tools set up yet, that is acceptable. In that case, rely on:

- local formatting checks
- local MSVC build/tests
- Docker Linux checks when available
- CI for the remaining Clang-oriented validation

## LLVM installation note

You do not need Visual Studio Professional to use LLVM tools. Contributors can install LLVM separately on Windows, including when using Visual Studio Community.

Common options are:

- install standalone LLVM and add it to `PATH`
- use Visual Studio's LLVM/Clang tools if they are already present

## Check your local toolchain

```bash
clang-format --version
clang --version
clang-tidy --version
```

If only one of these is going to match CI reliably, make it `clang-format`.

## Practical contributor policy

- Exact-match target: `clang-format` major version
- Good local baseline: MSVC build/test plus formatting
- Preferred cross-platform validation: Docker Linux build/test
- Final authority: GitHub Actions
