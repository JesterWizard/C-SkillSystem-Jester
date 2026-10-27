# RAM allocation and ROM placement

> Code: `include/link/config-memmap.s`, `include/configs/config-memmap.h`, `Tools/FE-PyTools/`  ·  Entry points: `_kernel_malloc` macros (`include/link/config-memmap.s:18`), `gpKernelArmFunConfig` (`Kernel/Wizardry/ArmFunc/Source/ArmFuncConf.c:3`)  ·  Existing docs: `Documentation/Memory.md` (partly stale, see Gotchas), [SaveData.md](SaveData.md)

## What it covers

How to add a global variable that lives in kernel EWRAM, where IWRAM room for ARM code comes from, and which ROM region new data and code land in. Examples: `gKernelLanguage` (boot language, `_kernel_malloc2`), `gPlayStExpa` (chapter flag bits, `_kernel_malloc`), `gSnekHighScore` (minigame score, `_kernel_malloc`). If the value must survive a save or suspend, add the save chunk as well ([SaveData.md](SaveData.md)).

## How the engine calls it

### EWRAM pools

`include/link/config-memmap.s` is the only place RAM addresses are chosen. Each pool has a `SET_DATA` top and bottom (`:14-16` for the main pool) and a macro that moves the used pointer by `size` and sets the new symbol to that address (`_kernel_malloc`, `:18-21`). The macro does not clear memory and does not pad (see Gotchas). The symbols are plain `.global` objects, so C refers to them with `extern`.

| Macro (`config-memmap.s`) | Pool and range | Grows | Used at writing (`ram-memory-usage.py`) | Safe for |
|---|---|---|---|---|
| `_kernel_malloc` (`:18`) | `FreeRamSpaceTop` `0x02026E30` to `FreeRamSpaceBottom` `0x02028E58` (`0x2028`) | down, from the bottom | 89.4% | Largest and most crowded. Kernel state, including save-chunk variables such as `gDeadUnits` (`:102`) and `gPlayStExpa` (`:91`). Avoid for new work when a pool with more room fits. |
| `_kernel_malloc2` (`:36`) | `FreeRamSpace2Top` `0x0203AAA4` to `FreeRamSpace2Bottom` `0x0203DDE0` (`0x333C`) | up, from the top | 39.1% | General state. Holds `gKernelLanguage` (`:288`), `sGaidenMagicUsesPool` (`:291`) and `sChatLogState` (`:263`). |
| `_kernel_malloc3` (`:45`) | `FreeRamSpace3Top` `0x02026AD0` to `FreeRamSpace3Bottom` `0x02026E30` (`0x360`) | down, from the bottom | 61.1% | Comment at `:215` says this space is taken from the icon display. Holds `gBattleHitArrayRe` and `gAnimRoundDataRe`. Which lifetimes are safe was not traced (unverified). |
| `_kernel_malloc_overlay0` (`:54`) | `EwramOverlay0_FreeRamSpaceTop` `0x0201F200` to `EwramOverlay0_FreeRamSpaceBottom` `0x02020188` (`0xF88`) | down, from the bottom | 37.5% | Shares time with banim. The comment at `:223-228` says it is risky to use rashly. Only for data that is not live during banim or battle animations. |
| `_kernel_malloc_demo` (`:27`) | `FreeDemoRamSpaceTop` `0x0203F150` to `FreeDemoRamSpaceBottom` `0x02040000` (`0xEB0`) | down, from the bottom | 0% (no allocations in the tree) | Nothing in this tree allocates from it or reads it. Its top is tied to the load-unit buffer, see Gotchas. Whether anything else writes it is unverified. |

The "used" column is the output of `Tools/FE-PyTools/ram-memory-usage.py` run on the tree at writing. It changes as allocations are added.

### IWRAM

IWRAM has no allocation macro. The file has fixed `dat start, name` / `dat end, nameEnd` pairs for relocated ARM functions (`include/link/config-memmap.s:319-329`). At boot, `GameInit_CopyArmFuncs` (`Kernel/Wizardry/ArmFunc/Source/copy-ram.c:4`, registered at `Kernel/Wizardry/GameInitHook/data.event:9`) copies each entry of `gKernelArmFunConfig` from ROM into its IWRAM window. It asserts that the window is at least as large as the source (`copy-ram.c:16`). `ram-memory-usage.py` (`Tools/FE-PyTools/ram-memory-usage.py:22-25`) reports two windows, one at `0x03003CAC`-`0x03004150` and one at `0x03004838`-`0x03004960`. Both were 100% used at writing. The comment at `config-memmap.s:309` lists a `__free__` gap from the end of IRQ to `0x03004838`. Its size is not verified.

### Boot check and build

1. `config-memmap.o` is a reference object for every `lyn` run (`makefile:164`; rules at `makefile:187-191` and `makefile:211-215`). A C `extern` resolves to the address in that object.
2. `GameInit_OverflowDetection` (`Kernel/Wizardry/OverflowDetection/Source/OverflowDetection.c:12`, registered at `Kernel/Wizardry/GameInitHook/data.event:8`) asserts that the used pointer of the main, 2, 3 and overlay0 pools (`FreeRamSpaceTop`, `FreeRamSpace2Top`, `FreeRamSpace3Top`, `EwramOverlay0_FreeRamSpaceTop`) has not crossed the far end (`OverflowDetection.c:21-24`). It does not check the demo pool (`FreeDemoRamSpaceTop`) or the IWRAM windows. There is no per-variable size check.
3. `make ramcheck` (`makefile:498-502`) prints the usage table. It does not fail the build.

### ROM placement

New code and data go into one of the regions below. Addresses are from `include/configs/config-memmap.h`. Which ORG block each region is emitted from is from `main.event` and `Kernel/kernel.event`.

| ROM address | Size | Contents | Where it is emitted |
|---|---|---|---|
| `0x0E8414` | `0x7C000` | Kernel `.text`, must stay in BL range (`KernelSpace_Text`) | `Kernel/kernel.event:30-41` (`WIZARDRY_TEXT` block of `Kernel/Wizardry/wizardry.event:41-127`) |
| `0x9875AC` | `0xF000` | Kernel `.data`: magic, reloc, then KData (`KernelSpace_Data`) | `Kernel/kernel.event:21-28` (`WIZARDRY_DATA` block, `wizardry.event:1-39`) |
| `0xB2A604` | `0xD59FC` | Common, non-secure: magic, reloc, `Data/Data.event`, message table at the end (`FreeSpaceCommon`) | `main.event:15-37` |
| `0xEFB2E0` | `0xE4D20` | Font (`FreeSpaceFont`) | `main.event:39-43` |
| `0x1000000` | not set | `Contents/` and the `WIZARDRY_CUSTOM` block of `wizardry.event:129-191` (`FreeSpaceDEMO`) | `main.event:57-61` |

Non-hot features go in the `WIZARDRY_CUSTOM` block. Kernel text is nearly full (`CLAUDE.md`), so check the `ASSERT` in `Kernel/kernel.event:41` after a build.

## Where to look

| File | Role |
|---|---|
| `include/link/config-memmap.s` | Pool bounds, the `_kernel_malloc*` macros, the allocation list, IWRAM `dat` windows |
| `include/configs/config-memmap.h` | ROM regions (`KernelSpace_*`, `FreeSpace*`) and the rodata layout |
| `Documentation/Memory.md` | Human-readable map. Stale in two places (Gotchas) |
| `include/kernel/kernel-lib.h` | `_BIT_SET`, `_BIT_CLR`, `_BIT_CHK` for flag bits (`:447-466`) |
| `Tools/FE-PyTools/ram-memory-usage.py` | Backend of `make ramcheck`; also reports IWRAM windows and SRAM |
| `Kernel/Wizardry/OverflowDetection/Source/OverflowDetection.c` | Boot-time pool asserts |
| `Kernel/Wizardry/ArmFunc/Source/ArmFuncConf.c`, `copy-ram.c` | IWRAM copy table and copy routine |
| `Kernel/Wizardry/LoadUnitFix/Source/LoadUnitFix.c` | `GetLoadUnitsAmount` clamp that protects the demo pool top (`:3`, `:27-28`, `:37-38`) |
| `Kernel/Wizardry/GameInitHook/data.event` | Boot hooks (overflow check `:8`, ARM copy `:9`) |
| `Kernel/kernel.event`, `main.event` | ORG blocks that set the ROM placement |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Pick the pool from the table above. Prefer `_kernel_malloc2` for general state, because `FreeRamSpace` is the most used pool.
3. Add one line at the end of that pool's block in `include/link/config-memmap.s`, for example `_kernel_malloc gExampleState, 2` (`size` is in bytes). Insert at the end, not in the middle (Gotchas).
4. Keep the running total aligned. A `u16` needs an even address, a `u32` or pointer needs a 4-byte address. Add a pad line if the next allocation would be odd (Gotchas).
5. Declare the variable `extern` in a header under `include/kernel/` (or `include/jester_headers/`). Do not define it in C. The definition is the `.s` symbol.
6. Include the header from the `.c` that uses it. Flag bits: use `_BIT_SET` / `_BIT_CHK`, or a `PlayStExpa_*`-style macro over a `u32[]` pool.
7. Run `make ramcheck` and read the row for your pool. Overflow is caught only at boot, by `GameInit_OverflowDetection`.
8. New ROM code or data goes into the `WIZARDRY_CUSTOM` block unless it is hot (ROM section above).
9. If the variable must persist, go on to [SaveData.md](SaveData.md).

## Template

Adapted from the Snek high score (`Kernel/Wizardry/Minigames/Snek/Snek.h:37-38` for the `extern` pair, `include/link/config-memmap.s:148-149` for the allocation).

```asm
// include/link/config-memmap.s, at the end of the _kernel_malloc block
_kernel_malloc gExampleState, 2
_kernel_malloc gExampleStateMagic, 2
```

```c
/* include/kernel/example.h */
#pragma once

#include "common-chax.h"

/* Both are defined by include/link/config-memmap.s. Do not define them in C. */
extern u16 gExampleState;
extern u16 gExampleStateMagic;
```

```c
/* Kernel/Wizardry/Example/Source/Example.c */
#include "common-chax.h"
#include "example.h"

void Example_SetState(u16 value)
{
	gExampleState = value;
}
```

## Worked examples

- `Kernel/Wizardry/Minigames/Snek/Snek.h:37-38`: two `extern u16` for the high score and its magic word, allocated as two 2-byte `_kernel_malloc` lines (`include/link/config-memmap.s:148-149`).
- `include/kernel/playst-expa.h:45`: `extern u32 gPlayStExpa[4];`, 16 bytes (`include/link/config-memmap.s:91`), used as a bit field through `PlayStExpa_SetBit` (`:47-48`).
- `Kernel/Wizardry/LanguageSelect/Source/LanguageSelect.c:21`: `extern u8 gKernelLanguage;` declared in the `.c` instead of a header. The 4-byte allocation is `_kernel_malloc2` (`include/link/config-memmap.s:288`).
- `include/link/config-memmap.s:277-278`: `sPortrait32AlignPad` is a 2-byte pad that puts `sPortrait32State` on a 4-byte address. It is the only explicit alignment pad in the file.

## Gotchas

- **Alignment is not checked and not padded.** The macros only move a pointer (`config-memmap.s:18-21`). The file says "do not assign any even numbered amount of bytes if the next address would be odd" (`:179-180`), and `Documentation/Memory.md` says allocations "must always be even numbered". Its example says "32-bit aligned" instead. Checked by running `ram-memory-usage.py --debug` on the unchanged tree: 10 one-byte allocations start at odd addresses (for example `sKernelHookSkippingFlag`, `:94`), and many arrays start at 2 mod 4 (for example `gDeadUnits`, 0x32 bytes). That is fine for `u8` and `u16`, but a `u32` or pointer placed after one of them is misaligned. Pair odd sizes as the file does (`sWmChapterIdOverride` and `sWmMapReloadPending`, `:275-276`).
- **Line order sets the addresses.** The first line of a pool gets the highest address (`config-memmap.s:59`). Adding a line at the end of a pool moves nothing. Adding one in the middle moves every later allocation in the same pool. Anything that hard-codes a kernel RAM address then breaks. Whether any patch does so was not checked.
- **`_kernel_malloc_align4_pad` is commented out** (`config-memmap.s:212`). There is no aligning macro. Use an explicit pad line.
- **Demo pool.** Nothing in the tree allocates from the demo pool (`_kernel_malloc_demo` has no uses). Its top is `FreeDemoRamSpaceTop = gLoadUnitBuffer + 0x14 * count` (`config-memmap.s:23`, comment in `Kernel/Wizardry/LoadUnitFix/Source/LoadUnitFix.c:22-23`). `GetLoadUnitsAmount` clamps `count` to 20 (`LoadUnitFix.c:3`, `:27-28`), which keeps the top at `0x0203F150`. The numeric address of `gLoadUnitBuffer` is not in this repo (unverified).
- **`Documentation/Memory.md` is stale.** Its demo size `0x0C40` (line 52) is wrong: `0x02040000 - 0x0203F150 = 0xEB0` (`config-memmap.s:23-24`). Its IWRAM table puts `ARM_UnitList` at `0x0300428C`; `config-memmap.s:326` puts it at `0x03004838`.
- **`config-memmap.o` is a dependency of every `.lyn.event`.** Editing `config-memmap.s` re-runs every `lyn` rule that lists `$(LYN_REF)` (`makefile:164`, `:187-191`, `:211-215`). Slow, but expected.
- **Pool headroom.** At writing, `FreeRamSpace` had 876 bytes free and `FreeRamSpace2` had 7988. Put new general state in `_kernel_malloc2` unless it needs to be in `FreeRamSpace`.
- **IWRAM is full.** Both windows were 100% used at writing. Adding ARM code needs room made first. Whether `__free__` (`config-memmap.s:309`) can take a new window was not verified.
- **`EWRAM_DATA` and `IWRAM_DATA`** are section attributes in `Tools/FE-CLib-Mokha/include/gba/defines.h:9-10`. The build passes no linker script (`makefile:164-222`), and nothing in the repo maps the `ewram_data` or `iwram_data` sections. Whether they place data anywhere is unverified. Use `_kernel_malloc`.
- **ROM headroom is not asserted for the custom block.** `main.event` has `ASSERT`s after the font and common regions (`:37`, `:43`), and `Kernel/kernel.event` has them after data and text (`:28`, `:41`). None follows the `WIZARDRY_CUSTOM` block (`main.event:57-61`).
