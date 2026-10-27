# Standalone patches

> Code: `Standalone/<name>/`, `Commissions/<name>/`  ·  Entry points: `Installer.event` hook sites (`jumpToHack(Func)` at a vanilla address, or a `POIN` into a vanilla table), free space `$1000000`  ·  Existing docs: `Standalone/README.md`, `Commissions/README.md`, `.agents/skills/extract-standalone-feature/SKILL.md`, `.agents/skills/extract-standalone-feature/checklist.md`, `Documentation/Setup.md`

## What it covers

A standalone patch is one integrated kernel feature rebuilt as a self-contained FEBuilder "Insert EA" script. It installs on a clean FE8U (USA) ROM without the C Skill System kernel. There are 26 folders under `Standalone/` and one under `Commissions/` (same contract). Examples: `Standalone/two_random_number_growths/` (one hook, one C function), `Standalone/goal_timer/` (four hooks and a data table), `Standalone/custom_fog_sight/` (no C at all). This doc says how the folders are laid out and how they relate to the integrated feature. Per-patch behaviour is in each folder's `README.md`.

## How the engine calls it

Install time, then runtime through the vanilla call site.

### Install

1. FEBuilder "Insert EA" on `Installer.event` (or `ColorzCore A FE8 -input:Installer.event -output:<rom>`, `Standalone/two_random_number_growths/README.md:43-47`) assembles the patch onto a clean `fe8.gba`.
2. `jumpToHack(Func)` expands to `BYTE 0x00 0x4B 0x18 0x47; POIN (offset|0x1)` (the file `Hack Installation.txt` in `Tools/EventAssembler/Extensions/`, line 63). The four code bytes are `ldr r3, [pc, #0]` and `bx r3`, followed by a 4-byte pointer. The hook therefore overwrites 8 bytes at the vanilla function entry (`PROTECT ... + 0x8`).
3. The body is placed at `$1000000`, the standalone free space, in `ORG` offset form (`Standalone/two_random_number_growths/Installer.event:3`, `:11`). `Standalone/README.md` and `.agents/skills/extract-standalone-feature/SKILL.md` both give this address.
4. The body is `Source/<Name>.lyn.event`, which is checked in. It starts with `ORG CURRENTOFFSET+$1;<Func>:` and is followed by the compiled code as `WORD` values (`Standalone/two_random_number_growths/Source/GetStatIncrease_2RN.lyn.event`).
5. Data tables and config constants follow the body, under the same `PROTECT` range (`Standalone/goal_timer/Installer.event:26-29`, `Standalone/auto_repair_weapons/Installer.event:16-18`).

### Runtime

6. Vanilla callers are unchanged. They call the original address, which now starts with the `ldr`/`bx` pair and jumps into `$1000000`. The C function keeps the vanilla signature.
7. Values written into the installer are read from C as `extern const`. Example: `AutoRepairWeaponsEnabled` (`Standalone/auto_repair_weapons/Installer.event:5`, `:16-18`; `Standalone/auto_repair_weapons/Source/AutoRepairWeapons.c:8`).

### Relation to the integrated feature

8. Integrated code is gated by a runtime flag in `gpKernelDesignerConfig`. Examples: `two_random_number_growths` at `Kernel/Wizardry/Lvup/Source/Levelup.c:63`, `goal_timer` at `Kernel/Wizardry/Goals/Timer.c:55` and `Kernel/Wizardry/Goals/GoalDisplay.c:13`. Defaults are in `Data/DesignerConfig/designer-config.c` (`.two_random_number_growths` at `:160`, `.goal_timer` at `:69`).
9. The standalone copy has no designer config. Installing the patch is the switch (two_random has no toggle). Other patches use a constant in `Installer.event` (`AUTO_REPAIR_WEAPONS`, `BIORHYTHM_MECHANIC`) or a data file (`goal_timer`'s `ChapterTimers.event`).
10. The folder name equals the designer-config key for 25 of 26 patches. The exception is `refuge`, whose key is `.menu_option_refuge` (`Data/DesignerConfig/designer-config.c:98`).
11. Each README states the integrated doc it mirrors where one exists. Example: `Standalone/goal_timer/README.md:106` links `Documentation/Features/ChapterGoals/Timer.md`.

### Build and test

12. Build is optional. Each folder has a checked-in `Source/<Name>.lyn.event`, so installing `Installer.event` needs no compile. Only edits to `Source/*.c` need `make -C Standalone/<name>`. The makefile needs devkitARM and the repo's `Tools/FE-CLib-Mokha` and `Tools/EventAssembler` (`Documentation/Setup.md`). Its pattern is `$(LYN) -longcalls <o> $(FE8_REF)` followed by `Tools/scripts/lynjump-protector.sh` (`Standalone/two_random_number_growths/makefile:23-26`).
13. `lynjump-protector.sh` appends `PROTECT` lines for each `ORG $...` in the lyn output, sized from `Tools/FE-CLib-Mokha/reference/fireemblem8.sym` (`Tools/scripts/lynjump-protector.sh:9-30`).
14. No test target exists for standalone patches. The gates are in `.agents/skills/extract-standalone-feature/checklist.md`: `make` succeeds in the folder, the installer does not define `jumpToHack`, EA assembles from the folder, hook bytes and pointer are checked, and no write touches ROM offset `0`. If a patch uses RAM or save data, run `make ramcheck` / `make savecheck` in the repo root (`makefile:498`, `:504`) and keep allocations even-sized (checklist).

## Where to look

| File | Role |
|------|------|
| `Standalone/README.md` | Index table of patches. The table has 25 rows for 26 folders (`show_true_2rn` is missing). |
| `Commissions/README.md` | Index of commissions (one row). |
| `Standalone/<name>/Installer.event` | `#define` addresses, `jumpToHack` hook with `PROTECT`, free-space `ORG $1000000`, `#include` of the body, data, closing `PROTECT`. |
| `Standalone/<name>/Source/<Name>.c` | C body. Compiled only by `make`. |
| `Standalone/<name>/Source/<Name>.lyn.event` | Checked-in lyn output. The installer includes this file. |
| `Standalone/<name>/makefile` | Builds `Source/<Name>.lyn.event` from the `.c`. `REPO_ROOT := $(abspath ../..)` (`:1`). |
| `Standalone/<name>/README.md` | Target ROM, hook address, free-space address, install steps, conflicts. |
| `Standalone/<name>/*.event` (for example `ChapterTimers.event`, `ClassFogSightBonusTable.event`) | Data tables that are edited without compiling C. |
| `Commissions/<name>/` (for example `Commissions/wary_fighter/`) | Same layout and contract as `Standalone/`. |
| `.agents/skills/extract-standalone-feature/SKILL.md` | Extraction workflow, approval gates, reference example. |
| `.agents/skills/extract-standalone-feature/checklist.md` | Required layout, package rules, validation gates. |
| `Tools/EventAssembler/Extensions/` (file `Hack Installation.txt`) | Defines `jumpToHack` (line 63). Used by the CLI build only. The installer must not redefine it. |
| `Tools/scripts/lynjump-protector.sh` | Appends `PROTECT` lines to lyn output. |
| `Tools/FE-CLib-Mokha/reference/fireemblem8.sym` | Symbol table. Used to size `PROTECT` ranges and to look up vanilla addresses. |
| `Data/DesignerConfig/designer-config.c` | Integrated defaults that each standalone README compares against. |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Choose the hook. Prefer the smallest vanilla function that keeps unrelated behaviour (checklist, "Package rules"). Stop and ask first if the feature needs Skill System core, a save format change, fixed kernel RAM, or if the only free space is taken (SKILL.md, "Approval gates").
3. Create `Standalone/<name>/` in snake_case. Use the designer-config key name when one exists (see Relation, item 10).
4. Write `Source/<Name>.c`. Keep the vanilla signature. Include only FE-CLib headers, for example `global.h` and `rng.h` as in `Standalone/two_random_number_growths/Source/GetStatIncrease_2RN.c:1-2`. Add `-I $(REPO_ROOT)/include` only if the C uses repo headers (`Standalone/goal_timer/makefile:13`).
5. Copy `makefile` from `Standalone/two_random_number_growths/` and change `SOURCE`, `OBJECT` and `OUTPUT` (see Template).
6. Run `make -C Standalone/<name>`. Check in `Source/<Name>.lyn.event` and any `.dmp` graphics. Do not use `#incext Png2Dmp` in the installer (checklist).
7. Write `Installer.event` (see Template). Find each hook address by taking the ROM address and subtracting `0x08000000`. Example: `GetStatIncrease` at `0x0802B9A0` is `$2B9A0` (`Standalone/two_random_number_growths/README.md:15`).
8. Write `README.md` with the same headings as the existing patches: behaviour, target ROM, build, installation, files, hooks and free space, conflicts, credits.
9. Add a row to `Standalone/README.md` (or `Commissions/README.md`).
10. Run the validation gates in `checklist.md`. Build and test beyond these gates: not covered by this doc.

## Template

Adapted from `Standalone/two_random_number_growths/` (the reference patch named in SKILL.md).

`Installer.event`:

```
#define FE8U_Example                 $00000   /* vanilla ROM offset, no 0x08 prefix */
#define FE8U_StandaloneFreeSpace     $1000000

/* Edit without recompiling C. */
#define EXAMPLE_ENABLED 1

PUSH
ORG FE8U_Example
    jumpToHack(Example_Standalone)
    PROTECT FE8U_Example FE8U_Example + 0x8 // Example hook
POP

ORG FE8U_StandaloneFreeSpace
#include "Source/Example_Standalone.lyn.event"

ALIGN 4
ExampleEnabled:
BYTE EXAMPLE_ENABLED

PROTECT FE8U_StandaloneFreeSpace CURRENTOFFSET // Example body + config
```

`Source/Example_Standalone.c`:

```c
#include "global.h"

/* Replaces vanilla <Func>. Keep the vanilla signature. */
int Example_Standalone(int growth)
{
	return growth;
}
```

`makefile` (copy the whole file from `Standalone/two_random_number_growths/makefile` and change the three file names):

```make
SOURCE := Source/Example_Standalone.c
OBJECT := Source/Example_Standalone.o
OUTPUT := Source/Example_Standalone.lyn.event
```

## Worked examples

- `Standalone/two_random_number_growths/`: the reference. One hook (`$2B9A0`, 8 bytes), one C file, a `make`-built `.lyn.event`, and a README whose Conflicts section names the overwritten bytes. Integrated code: `Kernel/Wizardry/Lvup/Source/Levelup.c:63`.
- `Standalone/goal_timer/`: four `jumpToHack` hooks (`$8D288`, `$8D784`, `$79030`, `$26F2C`), one `POIN` into the `gProc_BMapMain` pointer (`$59A254`, `PROTECT ... + 0x4`, `Installer.event:21-23`), and a data table in `ChapterTimers.event`. Reads and writes `PlaySt` +0x48 for the countdown (`README.md:28`). Integrated docs: `Documentation/Features/ChapterGoals/Timer.md`.
- `Standalone/custom_fog_sight/`: a patch with no C. Its body is hand-assembled `SHORT` words in `Installer.event` followed by `WORD FE8U_gPlaySt` and `POIN ClassFogSightBonusTable`. It has no `Source/` folder and no `makefile`, which departs from `checklist.md` "Required layout". Use it as an example of the data-only route, not as a default.
- `Commissions/wary_fighter/`: a commission. Its hook is `BattleGetFollowUpOrder` at `$2AF90`. Its lists are in `WaryFighterUnits.event`, which is edited and inserted without `make` (`Commissions/wary_fighter/README.md`). Its C reads the lists as `extern const u8` (`Commissions/wary_fighter/Source/BattleGetFollowUpOrder_WaryFighter.c:17-19`). The integrated follow-up check that uses `SID_WaryFighter` is at `Kernel/Wizardry/BattleSys/Source/BattleOrder.c:139-140`. Whether the two match in every case is not checked here (unverified).

## Gotchas

- Shared free space. All 27 installers (26 standalone, 1 commission) use `$1000000`. `Standalone/goal_timer/README.md:94` says install only one body there, or move the `ORG`. `SKILL.md` step 4 says later patches should continue after the previous body with `CURRENTOFFSET`. The two instructions conflict, and which one EA accepts without an overlap error is unverified.
- Hook width. A `jumpToHack` hook is 8 bytes (`PROTECT ... + 0x8`). A bare `POIN` hook is 4 bytes (`PROTECT ... + 0x4`, `goal_timer/Installer.event:21-23`). A `PROTECT` range shorter than the bytes actually written leaves the rest unprotected, so overlaps there would not be reported.
- `REPO_ROOT := $(abspath ../..)` (`makefile:1`). The makefile works only from a folder two levels below the repo root, that is `Standalone/<name>/` or `Commissions/<name>/`. Copying the folder elsewhere breaks `make`, but the install still works because the `.lyn.event` is checked in.
- Do not hand-edit `Source/*.lyn.event`. `lynjump-protector.sh` regenerates the `PROTECT` lines from the `ORG $...` lines on every `make` (`Tools/scripts/lynjump-protector.sh:9-30`).
- Defaults differ from the integrated build. `restore_hp_on_level_up` is on in the standalone README (`Standalone/restore_hp_on_level_up/README.md:10`) and off in `designer-config.c:130`. `biorhythm_mechanic` is on in its README (`:18`) and off in `designer-config.c:22`. `auto_repair_weapons` installs with the constant `1` (`Standalone/auto_repair_weapons/Installer.event:5`) while `designer-config.c:17` is `false`. Check the README before comparing outputs.
- Do not install over the kernel. Each README that has an integrated counterpart says so. Examples: `Standalone/goal_timer/README.md:95`, `Standalone/world_map_thought_bubbles/README.md:99`, `Standalone/gameover_quotes/README.md:71`, `Standalone/alpha_blend_movement_sprites/README.md:43`.
- Index gap. `Standalone/show_true_2rn/` has an `Installer.event`, `makefile`, `README.md` and `Source/`, but no row in `Standalone/README.md`.
- Contract gap. `Standalone/custom_fog_sight/` has no `Source/` and no `makefile`, unlike the other 25 folders.
- `Standalone/refuge/` uses the key `menu_option_refuge`, not `refuge` (see Relation, item 10).
