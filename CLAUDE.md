# CLAUDE.md

FE8U C-SkillSystem buildfile. C is built with devkitARM, turned into `.lyn.event` by `lyn`, and assembled by Event Assembler.

## Build

- `make` builds `fe8-kernel-dev.gba` from a clean `fe8.gba` at the repo root. It needs `DEVKITPRO`/`DEVKITARM` set (see `Documentation/Setup.md`).
- Cloud sessions: `.claude/hooks/session-start.sh` sets up a devkitPro stand-in, the `gbarun` emulator runner and `fe8.gba` (from the user's uploads). Build with `Tools/cloud/build.sh`, not plain `make`. See `Tools/cloud/README.md`.
- Build flags: `configs.mk`. Compile-time switches: `include/configs/configs.h`. Runtime toggles: `Data/DesignerConfig/designer-config.c` (struct name is spelled `KernelDesigerConfig`).
- To compile one file by hand, use `$DEVKITARM/bin/arm-none-eabi-gcc`, not the system gcc.
- After a change, run `python3 Tools/cloud/smoketest.py --quick` (emulator smoke tests of boot, battle, saves, a district, the world map, the stub chapters and the postgame; it runs tests side by side, `--jobs`; see `Tools/cloud/README.md`). The full run (no option) takes about 30 seconds.
- After adding a feature, run `python3 Tools/audit/unreachable.py`: it lists entry points, messages and items that nothing in the game reaches (see `Tools/audit/README.md`).

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
- New text messages go at the end of `Contents/Texts/Source/texts/Districts.txt`, the last file `kernel.txt` includes. A message added anywhere else (or a `# NAME` comment turned into a `## NAME` message) shifts every later message ID, and committed `.lyn.event` files that are not rebuilt keep the old IDs: wrong lines in game (a level-up quote showed a defeat quote). The smoke test's level-up quote check catches it.
- Rebuild before testing: a `Data/` `.c` edit is only in the ROM after `Tools/cloud/build.sh`. A header change does not regenerate the `.lyn.event` files that include it: run `make -B <path>.lyn.event` for each (a `struct` layout change touches every user of the struct).
- World-map nodes are 4 bytes each: `gGMData.nodes[n]` is at `+0x30 + 4*n` (`openPaths` at `+0xA4`, `current_node` at `+0xC8`, `unk_cd` at `+0xCD`). FE-CLib's `struct GMNode` was 1 byte until #690, and `struct GMapData` lacked the padding after `openPaths` until #699: C reads and writes hit the wrong bytes. Check a vanilla struct's offsets against vanilla code before trusting the FE-CLib comments. State bit 1 is "shown", bit 2 is "next" (not "cleared", whatever FE-CLib calls it). See `Documentation/Features/WorldMapStoryChapters.md`.
- A story chapter's world-map `SET_NODE` script must reveal its node as the destination (`WM_REVEAL_DEST` / `WM_SETDESTINATION`); `WM_CREATENEXTDESTINATION` alone does not. See `Documentation/Features/StoryStubs.md`.
- Compare story chapters by story order (`WmStoryReached`, `GetWmStoryChapterRank` in `Kernel/Wizardry/WorldMap/WmStoryChapter.c`), never by id: `CHAPTER_11` is `0x3D`.
- The district override (`sWmChapterIdOverride`) must live until the district chapter starts. The chapter is read after the world map ends, so clearing it on its first read or in `WorldMap_Destruct` starts the story chapter instead. A chapter-init hook clears it. See `Documentation/Features/Districts.md`, "Entering".
- Emulator tests that save: SRAM is not in a save state, and after a soft reset the game treats SRAM it never set up as empty. Boot first (`wait 600`), then `load` the state, in the same run (`test_wmsave`).
- A district's `events/traps.c` is `#include`d by its `events.c`: after editing it, run `make CustomCampaign/Chapters/<Name>/events/events.lyn.event` before `Tools/cloud/build.sh`, which does not rebuild it.
