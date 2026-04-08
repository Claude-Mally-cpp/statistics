# Agents

## Change Loop

1. Identify the next scoped task from the requested source.
2. Create a short local note if the task needs temporary tracking.
3. Create a feature branch with a descriptive name.
4. Implement the smallest coherent change.
5. Run the default verification steps.
6. Stage only files relevant to that task.
7. Commit with a concise message.
8. Report the branch name, commit SHA, verification status, and next suggested task.

## Task Sources

Tasks may come from:

- `worklist.md`
- GitHub issues
- PR review comments
- failing CI
- direct user requests

## Verification Policy

Default order:

1. Run a targeted compile or test check when practical.
2. Run `./clang-tidy-run-checks.sh --fix`.
3. Run `./auto-clang.format.sh`.

If full verification is blocked by environment or network limits, report the blocker and use the best local fallback.

## Scoping Rules

- Keep each pass narrow.
- Do not stage unrelated edits.
- Treat local task `.md` files as temporary unless the user wants them kept.
- Prefer reusing existing helper aliases or policies over duplicating logic.

## Default Shortcuts

If the user says `next pass`, the default is:

- prepare the next task note if needed
- create the feature branch
- implement the change
- run verification
- stage and commit
