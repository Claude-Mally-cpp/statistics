# Template Guide

This repository can work as a starting point for a new Modern C++ project, but it is not a drop-in generic template yet.

Use this guide when your goal is:

- clone the repo
- rename it
- keep the useful build/test/tooling structure
- remove or rewrite the statistics-specific parts

## Who This Template Is For

This repo is a good starting point if you want:

- a C++23 project with CMake presets
- a reusable library target
- optional CLI support
- unit tests with GoogleTest
- CI for Clang, GCC, and MSVC
- optional formatting, tidy, coverage, and Doxygen workflows

This repo is a weaker fit if you want:

- a minimal single-toolchain starter
- no CLI at all
- no Docker-based Linux workflow
- no install/export/package config support

## What You Get Out Of The Box

Template-worthy pieces already here:

- a modern CMake project layout
- CMake presets for Windows, Linux, GCC, Clang, sanitizers, and coverage
- a header-only library target
- CLI11-based command-line executable
- GoogleTest-based unit tests
- GitHub Actions workflows for build/test/tooling checks
- formatting, tidy, coverage, Docker, and Windows helper scripts
- Doxygen documentation support
- a lightweight release-process document and version-bump helper

## Smallest Successful Path

If you clone this repo as the base of a new project, aim for the smallest successful bootstrap first:

1. Rename the project in `CMakeLists.txt`.
2. Rewrite the README title and top-level description.
3. Decide whether you are keeping the CLI.
4. Decide whether you are keeping the current namespace structure.
5. Build and test one primary path successfully.

Suggested first verification path:

```powershell
cmake --preset msvc-x64-debug
cmake --build --preset msvc-x64-debug
ctest --preset msvc-x64-debug
```

Linux / WSL alternative:

```bash
cmake --preset linux-clang-debug
cmake --build --preset linux-clang-debug
ctest --preset linux-clang-debug
```

Do not try to rename everything in one pass before you have one successful configure/build/test cycle.

## Rename / Customize Checklist

### Rename Early

These are the highest-priority project-specific names:

- project name in [`CMakeLists.txt`](../CMakeLists.txt)
- README title and repository description
- Doxygen project name / brief in [`Doxyfile`](../Doxyfile)
- CLI executable name and version string in [`statistics-cli/statistics-cli.cpp`](../statistics-cli/statistics-cli.cpp)
- package/export naming:
  - `statistics`
  - `statistics::statistics`
  - `statisticsTargets`
  - `statisticsConfig.cmake`
- generated version header naming if you do not want `statistics_version.hpp`

### Review Carefully

These are valid to keep briefly during local bootstrap, but should be reviewed before you call the derived repo “ready”:

- namespace `mally::statlib`
- workflow artifact names or package labels that still use the current project name
- release-process examples that mention the current CLI binary name
- changelog wording that still describes this as the statistics project
- documentation examples that show statistics-specific commands or outputs

### Remove If Unneeded

Not every derived project should keep every helper:

- `statistics-cli/` if the new project is library-only
- if you remove the CLI, also remove or update its references in
  [`CMakeLists.txt`](../CMakeLists.txt),
  [`CMakePresets.json`](../CMakePresets.json), and any CI workflows or artifact
  names that still assume the CLI target exists
- Docker helper scripts if Linux container parity is unnecessary
- coverage scripts/workflow if coverage is not part of your initial maintenance model
- Doxygen workflow if API docs are out of scope
- clang-tidy / cppcheck / sanitizer workflows if you want a leaner starting point

## Project Name And Namespace Guidance

Treat naming in two layers:

### First Pass

Document and decide:

- project name
- public package/export name
- CLI name
- namespace direction

Do not try to refactor every occurrence immediately.

### Second Pass

Clean up the remaining old names only after the new project already builds and tests successfully.

Practical recommendation:

- rename the namespace early if the derived project intends to ship a library publicly
- defer deeper symbol/file naming cleanup until after one successful bootstrap pass

## What Is Statistics-Specific Versus Template-Worthy

Statistics-specific:

- README framing around descriptive statistics
- CLI command text for summary statistics
- examples and outputs centered on numeric/statistics workflows
- names such as `statistics`, `statlib`, and `statistics_version.hpp`

Template-worthy:

- preset structure
- multi-toolchain CI layout
- format/tidy/cppcheck workflow pattern
- Doxygen workflow pattern
- coverage-report workflow pattern
- release-process skeleton

## Optional Tooling To Keep Or Drop

Usually worth keeping:

- CMake preset structure
- unit-test wiring
- CI build/test workflows
- release-process doc and version bump helper

Keep only if they match the new project’s maintenance style:

- CLI support
- Doxygen
- coverage workflow
- Docker Linux workflow
- sanitizer workflow
- clang-tidy / cppcheck

## Suggested Bootstrap Order

1. Rename the project and README.
2. Decide whether the CLI stays.
3. Decide whether the namespace stays.
4. Get one build/test path green.
5. Rename package/export/install-facing names.
6. Remove optional tooling you do not want.
7. Re-run the main verification path.
8. Only then broaden into deeper cleanup.

## Verification Checklist

After cloning and adapting:

1. `cmake --list-presets`
2. configure one main preset
3. build that preset
4. run tests for that preset
5. if you keep the CLI, run one CLI smoke command
6. if you keep docs/tooling workflows, run only the smallest relevant local check

## Notes

- Prefer a clean, minimal derived project over inheriting every helper by default.
- The goal is not to preserve this repo’s identity; it is to reuse the useful scaffolding safely.
- If this guide exposes too many statistics-specific leftovers, treat that as a follow-up cleanup task rather than broadening the first documentation pass.
