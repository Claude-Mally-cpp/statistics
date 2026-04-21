# Release Process

This repository uses a small manual release flow. Release intent stays human-controlled, while version editing is automated enough to avoid typo-prone ad hoc changes.

## Versioning

The project follows a lightweight SemVer-style policy:

- major: breaking API or behavior change
- minor: backward-compatible feature
- patch: bug fix, docs-only maintenance release, or small tooling cleanup

Current source of truth:

- [`CMakeLists.txt`](../CMakeLists.txt) `project(... VERSION ...)`

The generated version header is derived from that CMake version during configure and should not be edited directly.

## Release Payload

Initial releases should stay small and maintainable.

Expected release payload:

- Git tag
- GitHub Release
- release notes
- updated `CHANGELOG.md`
- generated Doxygen HTML artifact

Coverage is treated as a CI/review artifact, not a formal release artifact.

Current repo support:

- `.github/workflows/doxygen.yml` already uploads a `doxygen-html` artifact and publishes docs from `main`
- `.github/workflows/coverage.yml` already uploads a coverage artifact and summary
- `.github/workflows/clang.yml` and `.github/workflows/msvc.yml` already archive release-oriented CLI package artifacts on `main` or `workflow_dispatch`

Optional later:

- packaged CLI binaries for selected platforms

## Pre-release Checklist

Use the smallest checklist that still gives confidence in the release.

Recommended checklist:

1. Run a quick formatting/verification pass.
2. Build and test on at least one main supported path.
3. Generate a coverage report and inspect obvious blind spots.
4. Generate documentation.
5. Run one manual CLI smoke test.
6. Update the version and changelog.
7. Tag the release and publish GitHub release notes.

Suggested commands:

```powershell
# Quick verification
pwsh -File .\verify.ps1 quick

# Main Windows path
cmake --preset msvc-x64-release
cmake --build --preset msvc-x64-release
ctest --preset msvc-x64-release

# Coverage
pwsh -File .\coverage-report.ps1

# Optional docs verification/build
bash ./doxygen-docs.sh --verify
bash ./doxygen-docs.sh

# CLI smoke test
.\out\build\msvc-x64-release\statistics.exe summary --data 1,2,2,3,5
```

Linux / WSL maintainers can substitute the equivalent documented Linux or Docker paths where appropriate.

If you adapt this repository into a different project, rewrite the CLI smoke-test
command to match the renamed executable and its real user-facing command surface.

## Version Bump Flow

Choose the release type manually, then use the helper to apply the actual version change.

Requirements:

- Python 3.9 or newer
- a working `python` command in your current shell, or the Windows `py -3` launcher

Examples:

```powershell
python .\tools\version_bump.py patch
python .\tools\version_bump.py minor
python .\tools\version_bump.py 1.2.3
python .\tools\version_bump.py --dry-run patch
```

Expected flow:

1. Decide whether the release is major, minor, or patch.
2. Run the version bump helper.
3. Add a changelog entry under the released version.
4. Re-run the relevant verification path.
5. Commit the version/changelog changes.
6. Tag the release, for example `v1.2.3`.
7. Create the GitHub Release and attach release notes.

The helper updates only `CMakeLists.txt`. Changelog content stays manual on purpose.

## Changelog and Tagging

Before tagging:

- move relevant entries from `## [Unreleased]` into a versioned section
- confirm the release notes match the actual merged work
- make sure the tag matches the bumped CMake version

Suggested tag shape:

- `vMAJOR.MINOR.PATCH`

## Coverage and Additional Tests

Coverage is a signal, not a target to game.

Add more tests when coverage highlights meaningful blind spots such as:

- edge cases in newer statistics APIs
- degenerate or empty-input handling
- surprising result-type behavior
- release-critical CLI behavior

Do not add tests only to inflate a percentage with low-value assertions.

## Notes

- Keep the release process lightweight and repeatable.
- Avoid full release automation until the repository genuinely needs it.
- Prefer documenting the current reliable workflow over inventing parallel tooling.
