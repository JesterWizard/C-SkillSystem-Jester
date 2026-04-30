---
name: save-suspend-ram-viability
description: "Use when planning or designing features that may consume normal save RAM or suspend RAM. Run make ramcheck and make savecheck, report used/total/free for NormalSave and SuspendSave, and judge whether the feature is viable before implementation."
---

# Save and Suspend RAM Viability

Use this skill when a proposed feature depends on persistent save data or suspend-state storage and you need to verify whether there is enough room before implementation.

## Use This Skill When

- The user is planning a new feature that stores data in normal save RAM.
- The user is planning a new feature that stores data in suspend RAM.
- The user wants a viability check before designing the data layout.
- The user needs to know whether the remaining space is sufficient for a proposed allocation.

## Workflow

1. Run `make ramcheck`.
2. Run `make savecheck`.
3. Record the reported `Total`, `Used`, `Free`, and `Usage` values for both regions.
4. Treat `Free` as the usable remaining capacity for planning purposes.
5. Compare the free space against the feature's expected footprint.
6. Return a clear verdict: viable as-is, viable only after reducing scope, or not viable without reclaiming space.

## Reporting Format

- Summarize `NormalSave` as `used / total / free`.
- Summarize `SuspendSave` as `used / total / free`.
- Include the usage percentage for each region.
- Call out which region is the blocker if the feature does not fit.
- If either command fails, report the failure and stop.

## Decision Rule

- If the feature fits within the reported free space in the relevant region, it is viable.
- If the feature exceeds the reported free space in either region, it is not viable without a layout change or space reclamation.
- If both regions are relevant, both checks must pass before the feature is considered viable.