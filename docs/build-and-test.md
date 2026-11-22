# Build and test

Run these workflows from the repository root. They use the configure, build, and
test presets in `CMakePresets.json`, so the same selections work in VS Code and
from a terminal.

## Choose an environment

Use a C++23 toolchain, CMake 3.25 or newer, and Ninja. Native MSVC builds also
require the Windows SDK. See [LLVM and platform setup](llvm-setup.md) for compiler
installation and tool selection.

| Environment | Debug preset | Release preset |
| --- | --- | --- |
| Linux / WSL, Clang | `linux-clang-debug` | `linux-clang-release` |
| Linux / WSL, GCC | `linux-gcc-debug` | `linux-gcc-release` |
| Windows, MSVC x64 | `msvc-x64-debug` | `msvc-x64-release` |
| Windows, Clang x64 | `windows-clang-x64-debug` | `windows-clang-x64-release` |

For each workflow, select the same preset name for configure, build, and test.
The build files and executables are under `out/build/<preset>`. Each platform and
configuration has its own directory.

## Open the project in VS Code

Install Microsoft's **CMake Tools** (`ms-vscode.cmake-tools`) extension. The
**C/C++** extension (`ms-vscode.cpptools`) provides C++ editing and debugging.

For Linux development through WSL, install Microsoft's **WSL** extension in
Windows VS Code. Open an Ubuntu terminal, navigate to the checkout, and run:

```bash
code .
```

Confirm the window's remote indicator shows the intended WSL distribution.
Install CMake Tools and C/C++ in that WSL environment when prompted. The build
tools must be available inside that distribution. See the
[VS Code WSL guide](https://code.visualstudio.com/docs/remote/wsl).

For native Windows development, open the checkout in a local VS Code window.
An x64 Visual Studio Developer PowerShell initializes the compiler, linker, and
SDK environment. Running `code .` there gives a newly started VS Code process
that environment. An already running VS Code process can retain its earlier
environment. CMake Tools also supports Visual C++ environment discovery.

## Configure, build, and test in VS Code

Open the Command Palette with **Ctrl+Shift+P** and follow this sequence. The
example selections below use Linux Clang Debug. For Windows MSVC Debug, select
`msvc-x64-debug` in all three preset commands.

| Command | Selection or result |
| --- | --- |
| `CMake: Select Configure Preset` | `linux-clang-debug` |
| `CMake: Select Build Preset` | `linux-clang-debug` |
| `CMake: Select Test Preset` | `linux-clang-debug` |
| `CMake: Configure` | Generate build files and obtain dependencies on the first run |
| `CMake: Set Build Target` | Select `all` for the full project |
| `CMake: Build` | Compile the library consumers, including `statistics_test` |
| `CMake: Run Tests` | Run the configured CTest suite |

The CMake sidebar's Project Status view shows the active configure, build, and
test presets. CMake Tools normally detects `CMakePresets.json` automatically.
If needed, set `cmake.useCMakePresets` to `always` in VS Code settings.

After a successful build, open the **Testing** view (the beaker icon). The
repository uses `gtest_discover_tests` to expose its GoogleTests through CTest.
Run the whole suite or use the run button beside a test. Read failures in Test
Results. Use **CMake: Refresh Tests** if discovery needs refreshing.

After source edits, save, build, and run the relevant tests. After changing CMake
configuration, configure again before building. This repository sets
`cmake.configureOnEdit` to `false`. Repeat the sequence with the matching Release
presets to check that configuration. See
[CMake Tools preset integration](https://github.com/microsoft/vscode-cmake-tools/blob/main/docs/cmake-presets.md).

## Equivalent terminal commands

Linux / WSL with the configured Clang toolchain:

```bash
cmake --preset linux-clang-debug
cmake --build --preset linux-clang-debug
ctest --preset linux-clang-debug --output-on-failure
```

Native Windows in an initialized x64 developer shell:

```powershell
cmake --preset msvc-x64-debug
cmake --build --preset msvc-x64-debug
ctest --preset msvc-x64-debug --output-on-failure
```

Use the corresponding names from the table for Release, GCC, or Windows Clang.
Configuration must complete successfully before building, and the test executable
must be built before running CTest. A successful test run reports zero failures.

## Run individual tests

List the tests registered for the active build:

```bash
ctest --preset linux-clang-debug -N
```

Then filter by a test name. The regular expression below selects one existing
test:

```bash
ctest --preset linux-clang-debug -R '^StatisticsTest.Summary_ForwardList$' --output-on-failure
```

For native Windows, use the same `-N` or `-R` arguments with `msvc-x64-debug`.

The `linux-clang-debug-cli` and `linux-clang-release-cli` build presets target
only the CLI. Use the regular build preset or the `statistics_test` target when
preparing to run the unit tests.

## Related workflows

- [LLVM and platform setup](llvm-setup.md) includes the optional `import std`
  example, which has additional CMake requirements.
- The [README](../README.md) documents Docker builds, sanitizers, coverage, and
  repository verification helpers.

When changing compilers or moving between host and container source paths,
configure a separate build directory so CMake discovers the intended toolchain
and source location.
