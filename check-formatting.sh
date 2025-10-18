#!/usr/bin/env bash
git ls-files '*.cpp' '*.hpp' '*.h' | xargs clang-format --dry-run --Werror
