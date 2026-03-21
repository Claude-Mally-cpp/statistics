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

If you later want stricter enforcement:
modify your script to exit non-zero on warnings
👉 Do this only after your warnings are stable.

## Future improvements

- run on push to main
- upload results as artifact
- run only on changed files
- cache build directory
