---
name: validate-build-after-each-code-change
description: Use when making code changes in this repository. Run make after every code edit and do not continue to another edit until the build succeeds or the failure is understood.
---

# Validate Build After Each Code Change

## When To Use

Use this skill whenever you edit code in the repository.

## Required Workflow

1. Make one code change.
2. Run `make` immediately after that change.
3. Do not apply the next code change until the build passes or the build failure has been investigated.
4. If the build fails, fix the failure at the source before making any further edits.
5. Repeat this cycle for every subsequent code change.

## Notes

- This requirement applies to all code edits, including small refactors and debug-only changes.
- Prefer the smallest possible edit so build validation stays fast.
- If a change cannot be validated with `make`, explain why before proceeding.