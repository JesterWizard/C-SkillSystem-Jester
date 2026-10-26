# Language Select

<p align="center">
  <img src="./Language_Select.gif" alt="Language Select" width="600"/>
</p>

After the health & safety, Nintendo and Intelligent Systems screens, the game shows a language selector before the title screen: a 2x3 grid of flags.

| | | |
| :-: | :-: | :-: |
| English | French | German |
| Spanish | Italian | Japanese |

- The D-pad moves the cursor. Left and right wrap within a row; up and down switch rows.
- The focused flag is full colour with a pulsing gold frame. It grows, lifts and floats a little. The other flags are darkened.
- A or START confirms. The screen fades out and goes straight to the title screen. The opening cutscene still plays when the title screen is left idle.

The choice is saved in the SRAM global save header and is preselected on the next boot.

On its own the patch **only records the choice**, exactly like the integrated feature. FE8U has one set of text, so the game looks the same whichever flag is picked. To make the choice switch the game's text, turn on the optional text switch and supply the translations (see [Translated text](#translated-text)).

## Target ROM

- **FE8U (USA)** clean ROM
- Free space: `$1000000`

## Where the choice is stored

- **SRAM:** `GlobalSaveInfo.unk10` (offset `$10` of the global save header), as `language + 1` (`0` = never chosen, so English). Vanilla only writes `0` there when it creates a fresh save header and never reads it, so the save layout does not change. The integrated feature uses the same field.
- **EWRAM:** a 4-byte cache at `0x0203BC00` (`gLanguageSelectCache`, set in `Source/RamAlloc.s`): `$4C53` in the high half marks it valid, the low byte is the language (0 English, 1 French, 2 German, 3 Spanish, 4 Italian, 5 Japanese). It saves reading SRAM on every text lookup. Change the address in `Source/RamAlloc.s` and run `make` if your project already uses it.

## Translated text

Uncomment this line at the top of `Installer.event`:

```
#define LANGUAGE_SELECT_TEXT_SWITCH
```

Then vanilla `GetStringFromIndex` and `GetStringFromIndexInBuffer` read text ID **`id + offset`** instead of `id` for every `id` below `LANGUAGE_TEXT_COUNT`, where `offset` is the chosen language's entry in `LanguageText.event`. An offset of `0` keeps the original text.

What you must supply:

1. **A copy of the text for each language.** In FEBuilder, extend the text table and enter each translation at a fixed distance from the original. For example, with `LANGUAGE_TEXT_COUNT $1000` and offsets `$1000` (French), `$2000` (German), ...: the French version of text `$0123` goes in `$1123`, the German one in `$2123`, and so on. Every ID in the remapped range needs an entry in each translated block, even if it is just a copy of the English text.
2. **The offsets** in `LanguageText.event` (`gLanguageTextOffsets`, one `SHORT` per flag) and the range `LANGUAGE_TEXT_COUNT`. All offsets are `0` by default, so enabling the switch changes nothing until you fill them in. Never set an offset that points past the end of your text table: the game would read garbage pointers and crash.
3. **Fonts** for any characters FE8U's font does not have (accents need a font patch; Japanese needs the Japanese font).
4. **Graphics:** text drawn as images (title screen menus, chapter title cards, battle UI labels and so on) does not change with the text table. Translate those graphics separately; they stay the same for every language.

The remap uses the text table pointer from the vanilla functions' own literal pools (`$A26C` and `$A2A0`), so it still follows a text table that FEBuilder has moved. Picking a new language also clears vanilla's decoded-string cache.

## Build

No build step is required to install the patch. `Source/LanguageSelect.lyn.event` and `Gfx/Img_LanguageSelectFlags.dmp` are checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/LanguageSelect.c`, `Source/RamAlloc.s` or the flag sheet PNG:

```bash
make -C Standalone/language_select
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools (including `Png2Dmp`). See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

Edits to `Installer.event` or `LanguageText.event` do not need `make`.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder's bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `language_select` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-language-select.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hooks, text-switch option, and free-space placement |
| `LanguageText.event` | Per-language text ID offsets and the remapped range (used only with the text switch) |
| `Gfx/Img_LanguageSelectFlags.png` | Flag sheet: 256x128, six 64x64 cells, 16-colour indexed palette |
| `Gfx/Img_LanguageSelectFlags.dmp` | Checked-in `Png2Dmp --lz77` output of the PNG |
| `Source/LanguageSelect.c` | C implementation (selector screen, save, text remap) |
| `Source/RamAlloc.s` | EWRAM address of the 4-byte language cache |
| `Source/LanguageSelect.lyn.event` | Checked-in lyn output (rebuild with `make`) |
| `makefile` | Compiles C to `.lyn.event` and the PNG to `.dmp` |

The flag colours come from the `sFlagPalette` table in `Source/LanguageSelect.c`, which must match the PNG's indexed palette. Index 1 and 2 are the two-pixel frame that glows on the focused flag.

## Hooks & Free Space

| Hook | Address (`ORG`) | Overwritten |
|------|-----------------|-------------|
| `gProcScr_GameControl`: `PROC_START_CHILD_BLOCKING(ProcScr_GameEarlyStartUI)` pointer | `$591710` | 4 bytes, now `ProcScr_LanguageSelect` |
| `GetStringFromIndex` (only with `LANGUAGE_SELECT_TEXT_SWITCH`) | `$A240` | 8 bytes (`jumpToHack`) |
| `GetStringFromIndexInBuffer` (only with `LANGUAGE_SELECT_TEXT_SWITCH`) | `$A280` | 8 bytes (`jumpToHack`) |

`ProcScr_LanguageSelect` starts the vanilla logo sequence (`ProcScr_GameEarlyStartUI`) as a blocking child, then shows the selector and jumps `gProcScr_GameControl` to its title-screen label.

**Free space:** `$1000000` (code, flag graphics and text offsets continue at `CURRENTOFFSET`, about 4.4 KB).

**RAM:** `0x0203BC00` .. `0x0203BC03` (4 bytes).

**Save:** `GlobalSaveInfo.unk10` (2 bytes in the SRAM global header). No new save blocks.

## Conflicts

- Any patch that changes the boot sequence through the same `gProcScr_GameControl` pointer (`$591710`), or that replaces `ProcScr_GameEarlyStartUI`'s caller. Patches that skip the logos may skip the selector too.
- Any patch that uses `GlobalSaveInfo.unk10`.
- Any patch that uses EWRAM `0x0203BC00` .. `0x0203BC03`.
- With `LANGUAGE_SELECT_TEXT_SWITCH`: any patch that hooks `GetStringFromIndex` (`$A240`) or `GetStringFromIndexInBuffer` (`$A280`). Patches that change the Huffman decoder itself (`CallARM_DecompText`) rather than these two functions still work, because the remap only changes the text ID and then calls the decoder.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.
- Do not install this on a ROM that already has the C Skill System kernel with `language_select` enabled.

No other package in `Standalone/` uses these hook sites, `unk10` or this RAM. `Standalone/skip_intro` (opening monologue, chapter title cards) and `Standalone/konami_style_bonus_screen` (title screen idle) hook later steps of the boot and can be installed alongside this one after moving one body's `ORG`.

`Installer.event` uses `PROTECT` on every hook site and the free-space body. Overlapping ROM writes should fail assembly with a clear EA error.

## Differences from the integrated feature

- **No on/off flag:** the integrated selector skips itself when the designer-config flag is off; the standalone is always on.
- **Language cache:** the integrated build caches the language in kernel RAM (`gKernelLanguage`). The standalone uses `0x0203BC00` with a validity marker, so leftover RAM after a soft reset cannot be mistaken for a choice.
- **Text switch:** new and optional. The integrated build only exposes `GetKernelLanguage()` for its own C code; a standalone ROM has no such code, so the text-ID remap is the way to use the choice.
- **Graphics:** the flag sheet is converted with `Png2Dmp --lz77` instead of grit. The decompressed tiles are identical to the integrated `.lz77`, and the stream is VRAM-safe (no back-references closer than 2 bytes).

## Limitations

- FE8U only has an English font; non-English text needs font patches.
- Only text that goes through `GetStringFromIndex` / `GetStringFromIndexInBuffer` follows the language. Graphics with text do not.
- The selector shows on every boot.
- Tested: logos then selector, D-pad movement (right, and down wrapping to the other row), confirm goes to the title screen, the choice is cached in RAM and preselected on the next boot from the same save. With the text switch on (Italian offset set to 1 for the test), the difficulty-select help shows the next text ID's string, while the build without the switch shows the vanilla one.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester). See [LanguageSelect.md](../../Documentation/Features/LanguageSelect.md) for the integrated documentation.
