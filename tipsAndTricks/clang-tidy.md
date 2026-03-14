clang-tidy.md

to fix all my files for 1 type of check (e.g. misc-include-cleaner)

```bash
CHECKS='-*,misc-include-cleaner' bash ./clang-tidy-run-checks.sh --fix
```
