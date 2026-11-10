# CLAUDE.md

FE8U C-SkillSystem buildfile. C is built with devkitARM, turned into `.lyn.event` by `lyn`, and assembled by Event Assembler.

## Build

- `make` builds `fe8-kernel-dev.gba` from a clean `fe8.gba` at the repo root. It needs `DEVKITPRO`/`DEVKITARM` set (see `Documentation/Setup.md`).
- Cloud sessions: `.claude/hooks/session-start.sh` sets up a devkitPro stand-in, the `gbarun` emulator runner and `fe8.gba` (from the user's uploads). Build with `Tools/cloud/build.sh`, not plain `make`. See `Tools/cloud/README.md`.
- Build flags: `configs.mk`. Compile-time switches: `include/configs/configs.h`. Runtime toggles: `Data/DesignerConfig/designer-config.c` (struct name is spelled `KernelDesigerConfig`).
- To compile one file by hand, use `$DEVKITARM/bin/arm-none-eabi-gcc`, not the system gcc.
- After a change, run `python3 Tools/cloud/smoketest.py --quick` (emulator smoke tests of boot, battle, saves and a district, about 20 seconds; see `Tools/cloud/README.md`).

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

- Signed and unsigned `%` are broken: `__aeabi_idivmod` is aliased to FE8's `__modsi3`, and `__aeabi_uidivmod` to its `__umodsi3`, which return the remainder in the wrong register. `r - (r / n) * n` on unsigned values becomes `__aeabi_uidivmod` too. Use `DivRem`/`Div` or a loop (e.g. `while (r >= n) r -= n;`).
- Kernel text has about 92 KB free since the stat screen, misc functions, arena, world map, convoy, UTF-8, popups, debug kit and start-map effects moved out of it (`kernel.event:41 Assertion error: -N` means N bytes over). Still put non-hot code in the `#ifdef WIZARDRY_CUSTOM` block of `Kernel/Wizardry/wizardry.event`, tables in `Data/`, and use `NOINLINE` on helpers that bloat callers.
- SIDs are positional: append new ones to the end of `include/constants/skills-equip.enum.txt`. That file is the only skill enum the build reads.
- Generated files, don't edit: `include/constants/skills.h`, `skills.event`, `Contents/Texts/build/msgs.h`, `Contents/Gfx/GfxDefs.h`, `Patches/combo.skills*.txt`.
- The `SkillTable-{person,job,item,weapon}.c` tables have no SID guards; wrap any row you add.
- `.lyn.event` files are committed build output, and some have hand edits (e.g. `Kernel/Wizardry/Lvup/Source/Levelup.lyn.event`). Check before regenerating.
- `.claude/worktrees/` holds stale copies; grep with `--exclude-dir=.claude`. The shell's `grep` may skip gitignored files; use `/bin/grep` for generated headers.
- Two `.lyn.event` files need a hand fix after regenerating: `Kernel/Wizardry/MirrorMapSprites/MirrorSprites.lyn.event` line 53 must be `WORD $8049789` (not `POIN sub_8049788`), and `Kernel/Wizardry/ItemSys/Source/StaffHeal.lyn.event` line 85 must be `WORD $8033259` (not `POIN GetPlayerLeaderUnitId`).
- Don't add code to `MiscFunctions.c`: its regenerated `.lyn.event` gains a `Sprite_PrepInformation` hook that `Tools/scripts/lynjump-detector.sh` rejects. Put new code in its own file.
- Emulator save states are tied to one ROM build; boot again after rebuilding.
- A district's `events/traps.c` is `#include`d by its `events.c`: after editing it, run `make CustomCampaign/Chapters/<Name>/events/events.lyn.event` before `Tools/cloud/build.sh`, which does not rebuild it.
