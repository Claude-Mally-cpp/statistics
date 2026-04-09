# Agents

## Change Loop

1. Identify the next scoped task from the requested source.
2. Create a short local note if the task needs temporary tracking.
3. Decide whether a new branch is needed.
4. Implement the smallest coherent change.
5. Run the verification steps that match the change type.
6. Stage only files relevant to that task.
7. Commit with a concise message when the user wants a commit or when the pass is complete and self-contained.
8. Report the branch name, commit SHA, verification status, and next suggested task.

## Task Sources

Tasks may come from:

- `worklist.md`
- GitHub issues
- PR review comments
- failing CI
- direct user requests
- local notes or scratch files created for the current task

## Verification Policy

Default policy:

1. Choose verification that matches the files changed.
2. Prefer the smallest targeted check that exercises the edited behavior.
3. Escalate to broader verification only when the change or risk justifies it.

Typical mapping:

- code changes: targeted build/test first, then broader repo checks if warranted
- workflow changes: YAML sanity review and, when practical, the smallest local validation available
- docs-only changes: proofread, link/path checks, and no compile/test unless the docs affect executable examples
- CI-failure follow-ups: reproduce or patch the reported failure first before running unrelated checks

If full verification is blocked by environment, permissions, network limits, or missing tools, report the blocker and use the best local fallback.

## Scoping Rules

- Keep each pass narrow.
- Do not stage unrelated edits.
- Treat local task `.md` files as temporary unless the user wants them kept.
- Prefer reusing existing helper aliases or policies over duplicating logic.
- Prefer staying on the user's current branch unless the user asks for a new branch or the task is clearly a separate, self-contained pass.
- If a new branch is created, use a descriptive name and say whether it is based on the current branch or intended to be cleanly rebased from `main`.
- For temporary PR or scratch artifacts, create or update them only when requested and avoid committing them unless asked.

## Default Shortcuts

If the user says `next pass`, the default is:

- prepare the next task note if needed
- create a feature branch if the current branch is not already the intended work branch
- implement the change
- run verification
- stage and commit
