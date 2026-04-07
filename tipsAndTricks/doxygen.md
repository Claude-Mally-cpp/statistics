# Doxygen Workflow

This note describes a practical Doxygen workflow for this repository.

The goal is to make documentation generation deterministic and easy to verify
locally and in CI.

## 1. Local installation

### Linux / WSL

Use the same Ubuntu LLVM 22 setup already used by `Dockerfile.clang`, then add
the documentation tools:

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
  libc++abi-22-dev \
  doxygen \
  graphviz
```

Verify the documentation tools:

```bash
doxygen --version
dot -V
```

### Windows

Windows is fine for writing and previewing documentation, but Linux/Docker
should remain the canonical docs-generation environment.

Recommended Windows installs:

```powershell
winget install LLVM.LLVM
winget install DimitriVanHeesch.Doxygen
winget install Graphviz.Graphviz
```

Verify:

```powershell
doxygen --version
dot -V
```

If `doxygen --version` works but `dot -V` reports that `dot` is not recognized,
Graphviz is either missing from `PATH` or the Windows package registration is
stale.

Check the expected install path first:

```powershell
Test-Path 'C:\Program Files\Graphviz\bin\dot.exe'
& 'C:\Program Files\Graphviz\bin\dot.exe' -V
where.exe dot
```

If `Test-Path` returns `False`, do not keep retrying `winget install
Graphviz.Graphviz`. A common failure mode is that Windows and `winget` still
show Graphviz as installed, but the real files are gone and the uninstall entry
points at a missing file such as `C:\Program Files\Graphviz\Uninstall.exe`.

In that case, use the official Windows installer from the Graphviz release
page:

- <https://gitlab.com/graphviz/graphviz/-/releases>

Download the Windows `.exe` installer asset, not a source archive. For example:

- `windows_10_cmake_Release_graphviz-install-14.1.4-win64.exe`

Verify the download checksum before running it. If the release also provides a
`.sha256` file, compute the local SHA-256 and compare it with the published
value:

```powershell
Get-FileHash .\windows_10_cmake_Release_graphviz-install-14.1.4-win64.exe -Algorithm SHA256
```

Example expected hash from the matching `.sha256` file:

```text
48512fbb2fbdf6e98400de8ad325e34be6b242b5a4fd412da62fd42115948bae
```

Example full comparison:

```powershell
$actual = (Get-FileHash .\windows_10_cmake_Release_graphviz-install-14.1.4-win64.exe -Algorithm SHA256).Hash.ToLower()
$expected = (Get-Content .\windows_10_cmake_Release_graphviz-install-14.1.4-win64.exe.sha256).Split()[0].ToLower()
$actual
$expected
$actual -eq $expected
```

If the last command returns `True`, run the installer and then verify again:

```powershell
Test-Path 'C:\Program Files\Graphviz\bin\dot.exe'
& 'C:\Program Files\Graphviz\bin\dot.exe' -V
```

If `dot.exe` exists but `dot -V` still fails, add Graphviz to the user `PATH`,
restart PowerShell, and verify again:

```powershell
[Environment]::SetEnvironmentVariable(
  'Path',
  [Environment]::GetEnvironmentVariable('Path','User') + ';C:\Program Files\Graphviz\bin',
  'User'
)
dot -V
where.exe dot
```

If you only need Doxygen to find Graphviz, set the path explicitly in
`Doxyfile`:

```text
HAVE_DOT = YES
DOT_PATH = C:\Program Files\Graphviz\bin
```

## 2. Bash entrypoint

This repo should use one script, `doxygen-docs.sh`, for the common paths:

- default: generate docs into `out/docs/doxygen` using the repo `Doxyfile`
- `--verify`: run the same documentation generation as an explicit verification step for local checks and CI
- `--clean`: remove generated docs output

With the current repo `Doxyfile`, warnings are treated as errors (`WARN_AS_ERROR = YES`), so both the default command and `--verify` fail if Doxygen emits warnings. `--verify` is still useful as the dedicated “validation” mode for CI and pre-push checks.
Typical usage:

```bash
bash ./doxygen-docs.sh
bash ./doxygen-docs.sh --verify
bash ./doxygen-docs.sh --clean
```

The script should:

- print the Doxygen version
- create output directories as needed
- read the repo `Doxyfile`
- use the same output location locally, in Docker, and in CI

## 3. Docker and CI changes

### Docker

Extend `Dockerfile.clang` to install:

- `doxygen`
- `graphviz`

That keeps docs generation in the same Linux toolchain image already used for:

- `clang-format`
- `clang-tidy`
- Linux Clang builds

### CI

Add a dedicated workflow at `.github/workflows/doxygen.yml`.

Recommended behavior:

1. Build `Dockerfile.clang`
2. Run `bash ./doxygen-docs.sh --verify` inside the container
3. Upload generated HTML as an artifact on `workflow_dispatch`

Recommended trigger policy:

- `pull_request`: verify docs config and warning-free generation
- `workflow_dispatch`: verify and upload generated HTML artifact

## 4. Badge plan

Start with a workflow badge in `README.md` for the Doxygen workflow, the same
way the repo already exposes format, tidy, and build badges.

Later options:

- keep the workflow badge only
- add a second badge for published docs if GitHub Pages is enabled
- replace the workflow badge with a published-docs badge if the docs site
  becomes the primary user-facing signal

## Additional notes

- The first version should focus on deterministic generation and warning
  visibility, not on publishing a polished docs site.
- unconditional widening to `long double` and related numeric-policy choices should stay out
  of the initial Doxygen rollout. It is worth documenting as a design-review
  question, but it should not block docs generation.
