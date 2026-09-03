---
name: extract-standalone-feature
description: Extract one C Skill System feature into a self-contained FE8U FEBuilder/Event Assembler patch under Standalone/. Use when the user asks to extract, standalone-ize, or package a feature for FEBuilder installation without the full kernel.
---

# Extract Standalone Feature

Use this skill when turning one integrated C Skill System feature into a standalone FEBuilder patch.

## When to use

- The user names a feature, config flag, or source path to extract.
- The deliverable is a folder under `Standalone/<feature_name>/`.
- The patch must install on **clean FE8U** through Event Assembler / FEBuilder.

## Workflow

1. **Trace the feature**
   - Start from config/data references and find the runtime entry point.
   - Read the integrated implementation and list every shared dependency.
   - Decide whether a **leaf vanilla hook** is enough. Prefer that over copying a whole subsystem.

2. **Define the package boundary**
   - Include only the code/data/assets required for the feature behavior.
   - Exclude Skill System core, relocation tables, designer config, and unrelated hooks.
   - If the integrated code uses `gpKernelDesignerConfig`, replace that with standalone behavior or a local constant/table.

3. **Create `Standalone/<feature_name>/`**
   - Follow the contract in [checklist.md](checklist.md).
   - Use `Standalone/two_random_number_growths/` as the reference implementation.

4. **Write the installer and C source**
   - `Installer.event` must assemble standalone with:
     - `#include "EAstdlib.event"`
     - `#include "Extensions/Hack Installation.txt"`
     - address `#define`s at the top of the installer
   - Apply the hook with `PUSH` / `ORG` / `POP`.
   - Place generated code at **`ORG $1000000`**. Standalone free space always starts here; later patches continue after the previous body using `CURRENTOFFSET`.
   - Include `Source/<Feature>.lyn.event` from the local build.
   - Keep hook alignment valid: if `jumpToHack`'s `POIN` would land on an odd offset, hook at the preceding even address.
   - Add a local **`makefile`** that compiles `Source/*.c` with FE-CLib and `lyn`.

5. **Document**
   - README must cover behavior, target ROM, install steps, hook address, free-space address, and conflicts.

## Implementation heuristics

- **Config flag only affects one vanilla helper** → hook that helper directly.
- **Feature lives in a hook table** → either hook the table owner or install a private dispatcher; do not require kernel relocation tables.
- **Feature is visual/data-only** → prefer `#incbin`, tables, and palette swaps with no C build.
- **Feature has C source** → put it in `Source/*.c`, compile with a local `makefile` to `*.lyn.event`, and include the generated file from `Installer.event`.
- **Never place standalone code at ROM offset 0**; always start free space at **`$1000000`**.

## Approval gates

Stop and ask before proceeding if:

- The feature cannot work without Skill System core, save format changes, or fixed kernel RAM.
- Multiple valid hook strategies would change gameplay differently.
- The only available free-space region is already used by common FEBuilder patches.
- Extracting the feature would silently diverge from integrated behavior.

## Reference example

`Standalone/two_random_number_growths/`:

- Integrated source: `Kernel/Wizardry/Core/Lvup/Source/Levelup.c`
- Config flag: `two_random_number_growths`
- Standalone strategy: replace vanilla `GetStatIncrease` only
- C source: `Source/GetStatIncrease_2RN.c`
- Local `makefile` builds `Source/GetStatIncrease_2RN.lyn.event`
- Addresses and hook live in `Installer.event`
- Free space: `$1000000`

## Output expectations

When extraction is complete, report:

- Standalone folder path
- Hook address and overwritten range
- Free-space address used
- Known conflicts

For dry-run requests, produce the audit and planned package layout without creating files.
