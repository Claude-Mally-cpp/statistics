clang-tidy.md

to fix all my files for 1 type of check (e.g. misc-include-cleaner)

```bash
CHECKS='-*,misc-include-cleaner' bash ./tidy-run-checks.sh --fix
```

to allow warnings locally without failing the script

```bash
WARNINGS_AS_ERRORS='' bash ./tidy-run-checks.sh
```
