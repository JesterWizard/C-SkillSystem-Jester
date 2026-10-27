# CLAUDE.md

FE8U C-SkillSystem buildfile. C is built with devkitARM, turned into `.lyn.event` by `lyn`, and assembled by Event Assembler.

## Build

- `make` builds `fe8-kernel-dev.gba` from a clean `fe8.gba` at the repo root. It needs `DEVKITPRO`/`DEVKITARM` set (see `Documentation/Setup.md`).
- Build flags: `configs.mk`. Compile-time switches: `include/configs/configs.h`. Runtime toggles: `Data/DesignerConfig/designer-config.c` (struct name is spelled `KernelDesigerConfig`).
- To compile one file by hand, use `$DEVKITARM/bin/arm-none-eabi-gcc`, not the system gcc.

## Adding a skill

Read `Documentation/SkillTypes/README.md`, then `Common.md`, then the type doc. Don't rediscover the process. `Documentation/Features/SkillSys.md` is partly stale.

## Adding a feature

Read `Documentation/FeatureTypes/README.md`, then `Common.md`, then the type doc(s) the feature touches.

## Rules

- Guard all skill code with `#if defined(SID_x) && (COMMON_SKILL_VALID(SID_x))`.
- A rewritten vanilla function gets `LYN_REPLACE_CHECK(Func);` plus an entry in a `LynJump.event` in the same directory.
- Linux kernel style; check with `./check_codingstyle.sh <file.c>`. Prefer C over ASM.
- Hot paths: battle calc, stat getters, `SkillTester`, map task, movement generation.
- Don't commit, branch or push unless asked.

## Pitfalls

- Signed `%` is broken: `__aeabi_idivmod` is aliased to FE8's `__modsi3`, which returns the remainder in the wrong register. Use `DivRem`/`Div` or a loop.
- Kernel text is nearly full (`kernel.event:41 Assertion error: -N` means N bytes over). Put large, non-hot code in the `#ifdef WIZARDRY_CUSTOM` block of `Kernel/Wizardry/wizardry.event`, tables in `Data/`, and use `NOINLINE` on helpers that bloat callers.
- SIDs are positional: append new ones to the end of `include/constants/skills-equip.enum.txt`. That file is the only skill enum the build reads.
- Generated files, don't edit: `include/constants/skills.h`, `skills.event`, `Contents/Texts/build/msgs.h`, `Contents/Gfx/GfxDefs.h`, `Patches/combo.skills*.txt`.
- The `SkillTable-{person,job,item,weapon}.c` tables have no SID guards; wrap any row you add.
- `.lyn.event` files are committed build output, and some have hand edits (e.g. `Kernel/Wizardry/Lvup/Source/Levelup.lyn.event`). Check before regenerating.
- `.claude/worktrees/` holds stale copies; grep with `--exclude-dir=.claude`. The shell's `grep` may skip gitignored files; use `/bin/grep` for generated headers.
- `make clean` deletes tracked `.4bpp` files under `Contents/Gfx/Sources/` (its `Contents/Gfx/makefile` clean recipe removes every non-PNG); restore with `git checkout -- Contents/Gfx/Sources`.
