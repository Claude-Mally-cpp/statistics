# Add a clang-tidy GitHub Action

## Goal

Create a separate GitHub Actions workflow that runs your existing scripts on every push and pull request. GitHub Actions workflows live under `.github/workflows/`, and using `actions/checkout@v4` is the standard way to check out your repository in a job.

## Add workflow `.github/workflows/clang-tidy.yml`

```yaml
name: clang-tidy

on:
  pull_request:

jobs:
  clang-tidy:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4

      - name: Install tools
        run: |
          sudo apt-get update
          sudo apt-get install -y clang-tidy cmake ninja-build

      - name: Prepare compile_commands.json
        run: bash ./clang-tidy-prepare.sh

      - name: Run clang-tidy
        run: bash ./clang-tidy-run-checks.sh
```

## Fail on warnings

`clang-tidy-run-checks.sh` now passes `--warnings-as-errors=*` by default, so the workflow fails on any reported warning.

If you need a softer local run, override it like this:

```bash
WARNINGS_AS_ERRORS='' bash ./clang-tidy-run-checks.sh
```

## Future improvements

- run on push to main
- upload results as artifact
- run only on changed files
- cache build directory
