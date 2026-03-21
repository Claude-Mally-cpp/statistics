# Add a basic clang-tidy GitHub Action

## Goal

Create a separate GitHub Actions workflow that runs your existing scripts on every push and pull request. GitHub Actions workflows live under `.github/workflows/`, and using `actions/checkout@v4` is the standard way to check out your repository in a job. :contentReference[oaicite:0]{index=0}

## Step 1 — create the workflow file

Create this file:

```text
.github/workflows/clang-tidy.yml

name: clang-tidy

on:
  push:
    branches:
      - main
      - develop
      - 'feature/**'
  pull_request:

jobs:
  clang-tidy:
    runs-on: ubuntu-latest

    steps:
      - name: Checkout repository
        uses: actions/checkout@v4

      - name: Install clang-tidy and build tools
        run: |
          sudo apt-get update
          sudo apt-get install -y clang-tidy cmake ninja-build

      - name: Prepare compile_commands.json
        run: bash ./clang-tidy-prepare.sh

      - name: Run clang-tidy
        run: bash ./clang-tidy-run-checks.sh