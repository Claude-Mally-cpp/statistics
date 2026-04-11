# LLVM Setup and Update Notes

This note documents the LLVM workflow used by this repository.

It is meant to help:

- current contributors working in this repo
- future users if this repo is reused as a template

## Purpose

Use this guide when you need to:

- install LLVM locally on Windows
- install matching LLVM tools on Linux or WSL
- verify which LLVM version the repo expects
- update the LLVM version used by the Docker image
- check which `clang`, `clang-format`, and `clang-tidy` binaries your shell is actually using

## Repo Expectation

The Linux Clang workflow in this repo is currently pinned to LLVM 22.

That version shows up in the main repo tooling here:

- `Dockerfile.clang`
- `CMakePresets.json`
- `.github/workflows/clang.yml`
- `.github/workflows/clang-format.yml`
- `.github/workflows/clang-tidy.yml`

In practice:

- Linux / WSL Clang presets expect `clang-22` and `clang++-22`
- local format and tidy examples expect `clang-format-22` and `clang-tidy-22`
- the Docker image is the cleanest reference for the expected Linux LLVM toolchain

## Supported Workflow Split

Use the toolchain that best matches the job:

- Windows native build/test: MSVC presets such as `msvc-x64-debug`
- WSL or Linux LLVM workflow: Clang 22 with versioned binaries
- Docker Linux workflow: closest match to Linux CI
- GitHub Actions: final clean-runner authority

On Windows, LLVM tools are optional for the main native build because the repo's Windows presets use `cl.exe`, not `clang`.

## Windows

### Recommended approach

For this repo, the simplest Windows split is:

- use Visual Studio or Build Tools for native configure/build/test
- optionally install standalone LLVM if you want local `clang-format`, `clang-tidy`, or ad hoc Clang checks
- use WSL or Docker when you want close parity with the Linux Clang CI path

### Install LLVM on Windows

Two common approaches:

1. Official LLVM installer
2. `winget install LLVM.LLVM`

The official release page is:

- https://github.com/llvm/llvm-project/releases

### Does installing a new Windows LLVM version remove the old one?

Usually, yes, if you use the official Windows installer.

The practical model is:

- the `LLVM-*.exe` installer is the normal Windows toolchain installer
- if an older LLVM version is already installed, the installer is meant to replace that managed install rather than keep a clean side-by-side toolchain stack
- in contrast, `clang+llvm-*.zip` archives can coexist in separate directories because they are just extracted files, not an installer-managed product

For contributors, the safest expectation is:

- installer-based LLVM on Windows should be treated as a replace-in-place install
- archive-based LLVM can be kept side-by-side if you deliberately want multiple versions

### Verify what Windows resolves

Run:

```powershell
where clang
where clang++
where clang-format
where clang-tidy
clang --version
clang-format --version
clang-tidy --version
```

This matters because Windows often has more than one LLVM/Clang source installed at the same time.

Common examples:

- standalone LLVM in `C:\Program Files\LLVM\bin`
- Visual Studio bundled LLVM under `VC\Tools\Llvm\...`
- other bundled toolchains such as Code::Blocks / MinGW

### Visual Studio and VS Code path behavior

If VS Code is launched from a Visual Studio Developer PowerShell or Developer Command Prompt, it will usually inherit the Visual Studio environment first.

That means:

- Visual Studio's own LLVM or MSVC tool paths can appear ahead of standalone LLVM on `PATH`
- `where clang` may resolve to Visual Studio's bundled tools even if standalone LLVM is installed
- the active shell environment matters as much as what is installed on disk

So if you want predictable behavior, verify from the same shell you actually use for:

- VS Code
- CMake configure/build
- `clang-format`
- `clang-tidy`

### Git Bash, Code::Blocks, and PATH order

The same path-order issue can also happen with Git Bash, Code::Blocks, or other Windows tool bundles.

Examples:

- Code::Blocks may put its bundled MinGW/Clang tools ahead of standalone LLVM
- Git Bash startup files may prepend paths that change which `clang` resolves first

If that happens, fix the order in your shell startup config, for example `~/.bashrc`, so the intended LLVM path appears first.

After changing shell startup config, open a fresh shell and re-run:

```bash
which clang
which clang-format
which clang-tidy
clang --version
```

### Windows native build/test for this repo

Use the MSVC presets:

```powershell
cmake --preset msvc-x64-debug
cmake --build --preset msvc-x64-debug
ctest --preset msvc-x64-debug
.\verify.ps1 quick
```

For release:

```powershell
cmake --preset msvc-x64-release
cmake --build --preset msvc-x64-release
ctest --preset msvc-x64-release
```

## Linux and WSL

### Install the expected LLVM version

This repo currently matches the LLVM 22 packages installed in `Dockerfile.clang`.

For Ubuntu 24.04 / Noble, install:

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
  llvm-22 \
  libc++-22-dev \
  libc++abi-22-dev
```

### Verify installed versions

```bash
clang-22 --version
clang++-22 --version
clang-format-22 --version
clang-tidy-22 --version
```

### Run repo commands with the expected binaries

The Linux Clang presets already pin `clang-22` and `clang++-22`:

```bash
cmake --preset linux-clang-debug
cmake --build --preset linux-clang-debug
ctest --preset linux-clang-debug
```

For local format/tidy runs:

```bash
CLANG_FORMAT=clang-format-22 bash ./format-check.sh
bash ./tidy-prepare.sh
CLANG_TIDY_BIN=clang-tidy-22 bash ./tidy-run-checks.sh
```

If your shell default `clang` points somewhere else, use the versioned binary names explicitly.

## Dockerfile LLVM Version Update Checklist

When changing the repo's expected LLVM version, update the full chain, not just one file.

### Required repo updates

1. Update `Dockerfile.clang`
2. Update `CMakePresets.json` compiler names if the presets pin a versioned `clang-N`
3. Update local workflow docs, especially this file and any README summary
4. Update GitHub Actions workflow commands that call versioned LLVM binaries
5. Rebuild the Docker image
6. Re-run the relevant build, test, format, tidy, sanitizer, and coverage paths

### Places to check in this repo

- `Dockerfile.clang`
- `CMakePresets.json`
- `README.md`
- `docs/llvm-setup.md`
- `.github/workflows/clang.yml`
- `.github/workflows/clang-format.yml`
- `.github/workflows/clang-tidy.yml`

### Example version-change review

If moving from LLVM 22 to LLVM 23, review:

- `clang-22` -> `clang-23`
- `clang++-22` -> `clang++-23`
- `clang-format-22` -> `clang-format-23`
- `clang-tidy-22` -> `clang-tidy-23`
- `llvm-toolchain-noble-22` -> `llvm-toolchain-noble-23`
- `llvm-22` package references -> `llvm-23`

## Local Verification After LLVM Changes

Use the smallest check that proves the edited path still works.

### Linux / WSL

```bash
clang-22 --version
clang-format-22 --version
clang-tidy-22 --version
cmake --preset linux-clang-debug
cmake --build --preset linux-clang-debug
ctest --preset linux-clang-debug
CLANG_FORMAT=clang-format-22 bash ./format-check.sh
bash ./tidy-prepare.sh
CLANG_TIDY_BIN=clang-tidy-22 bash ./tidy-run-checks.sh
```

### Docker

```bash
docker build -f Dockerfile.clang -t cpp-ci-clang .
```

Then run the repo's Linux build/test wrapper or the narrower command sequence you actually changed.

### Windows

```powershell
where clang
where clang-format
where clang-tidy
cmake --preset msvc-x64-debug
cmake --build --preset msvc-x64-debug
ctest --preset msvc-x64-debug
```

## Template Reuse Notes

The following parts are general enough to keep if this repo becomes a template:

- explicit LLVM install commands
- `where` / `which` verification steps
- PATH-order warnings for Windows shells and editors
- Dockerfile update checklist

The following parts are repo-specific and should be reviewed before reuse:

- pinned LLVM version number
- preset names such as `linux-clang-debug` and `msvc-x64-debug`
- exact workflow file names
- any repo-specific wrapper scripts such as `verify.sh`, `verify.ps1`, `format-check.sh`, and `tidy-run-checks.sh`
