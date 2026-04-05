# Windows Clang Notes For Contributors

This note is for contributors working on this repository from Windows.

## Recommended approach

The safest way to contribute from Windows is:

1. Run local MSVC builds/tests natively on Windows.
2. Match the repository's LLVM 22 toolchain for formatting and optional local Clang tools.
3. Use WSL or Docker for Linux `clang` / `gcc` validation when available.
4. Let GitHub Actions be the final clean-runner authority.

## What matters most

The most important Windows-side tool to match is `clang-format`.

Formatting disagreements are usually caused by version drift, so contributors should keep their local `clang-format` on the same major version used by CI and Docker. For this repository, the intended target is LLVM 22.

## Minimum Windows setup

Contributors on Windows should aim to have:

- `clang-format` 22.x
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

If WSL is available, use it for the closest Linux Clang match to CI.

## About clang-tidy on Windows

`clang-tidy` is optional for local Windows contributors.

It depends on:

- LLVM tooling being installed locally
- a usable compile database for the relevant build tree

If you do not have those tools set up yet, that is acceptable. In that case, rely on:

- local formatting checks
- local MSVC build/tests
- WSL or Docker Linux checks when available
- CI for the remaining Clang-oriented validation

## LLVM installation note

You do not need Visual Studio Professional to use LLVM tools. Contributors can install LLVM separately on Windows, including when using Visual Studio Community.

Common options are:

- install standalone LLVM and add it to `PATH`
- use Visual Studio's LLVM/Clang tools if they are already present
- install LLVM with `winget install LLVM.LLVM`

## Check your local toolchain

```powershell
where clang
where clang-format
where clang-tidy
clang-format --version
clang --version
clang-tidy --version
```

If only one of these is going to match CI reliably, make it `clang-format`.

## Practical contributor policy

- Exact-match target: LLVM 22 tool versions, especially `clang-format`
- Good local baseline: MSVC build/test plus formatting
- Preferred cross-platform validation: WSL or Docker Linux build/test
- Final authority: GitHub Actions
