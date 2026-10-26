# FE7 Mode Select

<p align="center">
  <img src="./Feature - FE7 Mode Select.gif" alt="FE7 Mode Select" width="480"/>
</p>

Replaces FE8's New Game difficulty menu (Easy / Normal / Difficult) with FE7's **Mode Select** screen: the lords spin on a carousel of battle animations over a spell circle, with their portrait, name and weapon type, and a Normal / Hard choice.

| Button | Action |
|--------|--------|
| Left / L, Right / R | Turn the carousel (Eirika, Ephraim, Lyon) |
| Up / Down | Normal or Hard |
| A / Start | Start the game (then pick a save slot, as in vanilla) |
| B | Back to the main menu |

- **Normal** starts the game on FE8 Normal and **Hard** on FE8 Difficult (both settable, see [Settings](#settings)). The save menu then writes the new game exactly as after vanilla's menu.
- The lord on the carousel is for show: FE8's route is still chosen in Chapter 8.

This is the integrated C Skill System `fe7_mode_select` feature, made to work on **clean FE8U** without the kernel. In the current kernel this screen is switched off; see [Differences from the C Skill System](#differences-from-the-c-skill-system).

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: the `NewNewGameDifficultySelect` step of vanilla `ProcScr_SaveMenu` (routine pointer at `$A20164`)
- Free space: `$1000000` (42,037 bytes including graphics, `$1000000` .. `$100A435`)

## Settings

Set in `Installer.event`, read at run time:

| Define | Default | Meaning |
|--------|---------|---------|
| `FE7_MODE_SELECT_UNLOCKS` | `0x1F` | Which carousel entries and Hard rows exist: `0x01` Eirika Hard, `0x02` Ephraim, `0x04` Ephraim Hard, `0x08` Lyon, `0x10` Lyon Hard. Eirika is always there. `0x1F` is everything (the C Skill System value). |
| `FE7_MODE_SELECT_NORMAL_ROW` | `1` | FE8 difficulty for the Normal row: `0` Easy, `1` Normal, `2` Difficult |
| `FE7_MODE_SELECT_HARD_ROW` | `2` | FE8 difficulty for the Hard row |

FE8's Easy mode (with tutorials) isn't on this screen by default; set `FE7_MODE_SELECT_NORMAL_ROW` to `0` if the Normal row should start Easy.

## Text and graphics

- Names and rows use vanilla FE8U text IDs: `0x212` "Eirika", `0x220` "Ephraim", `0x234` "Lyon", `0x53E` "Normal", `0x53F` "Hard". Edit them in FEBuilder's text editor to rename.
- "Main character:", "Weapon:" and "Swd" / "Lnc" / "Mag" are ASCII strings in `Source/ModeSelect.c`.
- The background, frame, "Mode Select" / "Press Start" / "Change" sprites and the chapter banners are FE7U graphics, in `Data/` (file names keep their FE7U ROM addresses). The banners read FE7's "Prologue ~ Ch.10" (Eirika) and "Ch.11 ~ Final" (Ephraim, Lyon); replace `Data/Img_08415BE8.bin` .. `Data/Img_08416220.bin` (LZ77 4bpp tiles) to change them.
- Battle animations: `lorf_sw1` (Eirika), `lorm_sp1` (Ephraim), `necm_mg1` (Lyon), by vanilla animation ID (`sLordBanim` in `Source/ModeSelect.c`). Portraits: vanilla Eirika, Ephraim and Lyon (`0x46`).

## Build

No build step is required to install the patch. `Source/ModeSelect.lyn.event` and the `Data/*.bin` graphics are checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/ModeSelect.c`:

```bash
make -C Standalone/fe7_mode_select
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder's bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `fe7_mode_select` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-fe7-mode-select.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: hook, settings, graphics and free-space placement |
| `Source/ModeSelect.c` | C implementation (port of FE7U's mode select) |
| `Source/ModeSelect.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `Data/*.bin` | FE7U graphics, palettes and tile maps |
| `makefile` | Compiles C to `.lyn.event` |
| `Feature - FE7 Mode Select.gif` | Preview |

## Hooks & Free Space

| Location | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `ProcScr_SaveMenu` (`$A200B8`), command `PROC_CALL(NewNewGameDifficultySelect)` at `$A20160` | `$A20164` | 4 bytes (routine pointer, repointed with `POIN`) |

`StartModeSelect` starts the mode select proc as a blocking child of the save menu, like vanilla's menu. It reports the choice with vanilla `SaveMenu_SetDifficultyChoice` (`0` Easy, `1` Normal, `2` Difficult, `3` back), so the rest of the save menu (`SaveMenu_ReloadScreenFormDifficulty`, `SaveMenu_PostDifficultHandler`, slot selection, `WriteNewGameSave`) runs unchanged. On exit the screen restores the save menu's background layout and graphics, the default portrait VRAM layout and the H-blank handler, then fades to black as vanilla's menu does.

**Free space:** `$1000000`.

**RAM:** no permanent RAM. While the screen is open it uses the EWRAM overlay at `0x020000F4` .. `0x0201EC68` (the same buffers FE7 uses: three battle animation buffers, text, palette backups). Vanilla only uses that area for battle animations, the preparation screens, the ending and similar screens that can't run at the same time. The save menu keeps a little data of its own there (`0x020007A0` .. `0x02000968`: slot palette, falling sparkles, sub-box text); it is copied to `0x0201EAA0` while the screen is open and put back on exit.

## Conflicts

- Any patch that changes the New Game difficulty step of `ProcScr_SaveMenu` (`$A20160` .. `$A20168`), for example patches that skip or replace FE8's difficulty menu, and the full C Skill System.
- A patch that keeps data of its own in the EWRAM overlay listed above while the title/save menu is open.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or change `FE8U_StandaloneFreeSpace` in `Installer.event` to an offset after any already-installed standalone code.
- No other package in `Standalone/` hooks the difficulty step. [chapter_names](../chapter_names/), [skip_intro](../skip_intro/) and [konami_style_bonus_screen](../konami_style_bonus_screen/) hook other parts of the title/save menu flow.

`Installer.event` uses `PROTECT` on both the hook site (`$A20164`, 4 bytes) and the free-space body. If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Differences from the C Skill System

In the current kernel `fe7_mode_select` does nothing: `FE7ModeSelect_Installer.event` writes the vanilla `NewNewGameDifficultySelect` pointer back, and `StartModeSelect` just calls vanilla, because the screen "crashed in mGBA". This package ports what the flag used to do (start `ProcScr_ModeSelect` in new-game mode from the save menu) and fixes what kept it from working:

- **Difficulty.** The integrated screen set `PLAY_FLAG_HARD` and `chapterModeIndex` in `gPlaySt` directly. The save menu rebuilds `gPlaySt` when it writes the new game (`WriteNewGameSave`), so both were lost, and the save menu's own difficulty value was left stale. This package reports the choice with `SaveMenu_SetDifficultyChoice`, like vanilla's menu.
- **Lord choice.** The integrated screen meant Eirika and Ephraim to preset the route (`chapterModeIndex` 2 / 3). Besides being overwritten (above), a route set before the Chapter 8 split would change vanilla behaviour in the common chapters (several systems check for `chapterModeIndex == 1`), so the carousel is cosmetic here.
- **Back button.** The integrated new-game mode ignored B, so there was no way back to the main menu. B now returns to it.
- **Leaving the screen.** The integrated exit re-ran `SaveMenu_Init`, `SaveMenu_InitScreen` and `SaveMenu_LoadExtraMenuGraphics` from inside the save menu, starting a second save-draw proc and resetting the menu state. This package restores only what it changed (background layout, graphics including BG2/BG3, portrait VRAM layout, H-blank handler and the borrowed EWRAM) and fades out, then lets the save menu continue as after vanilla's menu.
- **Palette.** The integrated installer copied 256 bytes from a 32-byte inline palette (`Pal_084138F0`), reading whatever followed it. This package uses the full 256-byte FE7 palette (`data/Pal_084138F0.bin`, which the kernel ships but doesn't use).
- **Settings.** The unlock bits and the difficulty each row starts on are settings here; the integrated code had everything unlocked and only Normal / Hard.
- The kernel's replacement of `SaveMenu_ResetLcdFormDifficulty` is not ported; it does the same as vanilla.
- The small work structures that the kernel put in its own heap use the FE7 overlay addresses above instead.

## Limitations

- Hard rows are unlocked by the setting only; FE7's unlock-by-clear-data isn't ported (the integrated code had it commented out too).
- The banners are FE7's chapter ranges (see [Text and graphics](#text-and-graphics)).
- Tested on clean FE8U in mGBA: the screen opens from New Game, the carousel turns both ways, Normal and Hard give the same `gPlaySt` difficulty and option bits as vanilla's Normal and Difficult, the game starts and reaches the Prologue map, B returns to the main menu and New Game can be opened again, and the save slot screen after it is drawn correctly. Easy (`FE7_MODE_SELECT_NORMAL_ROW 0`) and the unlock bits were assembled but not runtime-tested.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester), whose code is a port of FE7U's mode select (FE7U addresses are noted in `Source/ModeSelect.c`). Graphics are FE7U assets.
