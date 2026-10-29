# Konami-Style Bonus Screen

Press **↑ ↑ ↓ ↓ ← → ← → B A** on the title screen ("Press START") to unlock a secret bonus:

1. A random **stat booster** (Angelic Robe, Energy Ring, Secret Book, Speedwings, Goddess Icon, Dragonshield, Talisman, Boots or Body Ring) is added to the game's vanilla **Bonus Claim** data, named "Mokha bonus". It shows up as **Extras > Transfer Data**, where it can be sent to a save file.
2. A text screen (debug font) congratulates the player. After about two seconds, pressing A, B, L, R, START or SELECT soft-resets the game.

Everything else on the title screen is vanilla: **A** or **START** (when it is not the last key of the code) starts the game, and leaving the title idle still starts the class reel demo.

This matches the integrated C Skill System behaviour of `konami_style_bonus_screen`, but installs on **clean FE8U** without the kernel. The bonus only uses vanilla save data (the Bonus Claim block that vanilla uses for e-Reader/link bonuses), so no save-format changes are needed.

## RAM

This patch uses **2 bytes** of EWRAM (1 used, 1 padding) for `gKonamiComboStep`, the number of code keys entered so far.

| Symbol | Address range | Size | Region |
|--------|---------------|------|--------|
| `gKonamiComboStep` | `0x0203ADC0` .. `0x0203ADC1` | 2 bytes | FreeRamSpace2 (`0x0203AAA4` .. `0x0203DDE0`) |

To use a different address, edit `Source/RamAlloc.s`, then run `make` and reinstall.

While the bonus is being granted and shown (on the title screen, until the soft reset), the patch also borrows two vanilla buffers: `gGenericBuffer` (`0x02020188`, 0x140 bytes for the Bonus Claim data) and the vanilla debug-print text buffer at `0x02026E30` used by `PrintStringToDBG`. Some hacks use the latter as "FreeRamSpace"; nothing from a running game is live on the title screen, and the game resets right after.

## Target ROM

- **FE8U (USA)** clean ROM
- Hooks: vanilla `Title_IDLE` at `0x080C6354` and `LoadSavedEid8A` at `0x080A5290`
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/KonamiCode.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` if you edit `Source/KonamiCode.c` (for example the bonus text `KONAMI_BONUS_NOTE` or the item list) or `Source/RamAlloc.s`:

```bash
make -C Standalone/konami_style_bonus_screen
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder's bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `konami_style_bonus_screen` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-konami.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hooks, and free-space placement |
| `Source/KonamiCode.c` | Title screen listener, bonus grant, bonus screen, `LoadSavedEid8A` replacement |
| `Source/RamAlloc.s` | EWRAM address for `gKonamiComboStep` |
| `Source/KonamiCode.lyn.event` | Checked-in lyn output (rebuild with `make` after editing) |
| `makefile` | Compiles C + RAM object to `.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten | Replacement |
|----------|--------------|-------------|-------------|
| `Title_IDLE` (`0x080C6354`) | `$C6354` | 8 bytes | Vanilla title idle loop (START/A, idle timeout to class reel) plus the code listener |
| `LoadSavedEid8A` (`0x080A5290`) | `$A5290` | 8 bytes | Always `TRUE`: every save file may receive Transfer Data (Bonus Claim) items |

The integrated kernel swaps the `Title_IDLE` pointer inside `gProcScr_TitleScreen` (`0x08AA6B64`); this package hooks the function itself instead, which has the same effect.

**Free space:** `$1000000`, 0x3F5 bytes (body continues at `CURRENTOFFSET` after install).

## Conflicts

- Any patch that replaces `Title_IDLE` (`$C6354`) or edits the `Title_IDLE` entry of `gProcScr_TitleScreen` (`$AA6B64`), including the C Skill System `EasterEgg` module.
- Any patch that replaces `LoadSavedEid8A` (`$A5290`).
- Patches that use the Bonus Claim save block for their own data.
- **EWRAM** `0x0203ADC0` .. `0x0203ADC1` overlaps with other hacks using FreeRamSpace2. Change `Source/RamAlloc.s` if needed.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both hook sites and the free-space body. Overlapping ROM writes should fail assembly with a clear EA error.

## Limitations

- `LoadSavedEid8A` is replaced for all Transfer Data use, not only for this bonus: in vanilla a save file can only receive Bonus Claim items once its event flag `0x8A` is set (the menu says "Send data from Chapter 2+"). The kernel makes the same change.
- If the Bonus Claim data already holds bonus content that did not come from this patch, no item is added (the screen still shows). Each code entry adds one more booster while free slots remain (16 in total).
- The bonus screen uses the vanilla debug font, so empty lines in the text collapse and letters are upper-case.
- A wrong key resets the code; it is not re-checked as the first key, so for example ↑ ↑ ↑ ↓ ↓ … does not count (same as the kernel).

### Differences from the integrated kernel

- Kernel `GiveKernelBonus` picks `iid_bonus[NextRN_N(9) - 1]`, which can read one byte before the item list and never picks the Body Ring. This package uses `NextRN_N(9)` so all nine boosters can be picked.
- Kernel `KernelTitle_IDLE` dropped vanilla's idle timeout to the class reel demo. This package keeps it.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester) (`Kernel/Wizardry/EasterEgg/`, originally from Mokha's [fe8u-cskillsys](https://github.com/FireEmblemUniverse/fe8u-cskillsys)). See [KonamiCode.md](../../Documentation/Features/KonamiCode.md).
