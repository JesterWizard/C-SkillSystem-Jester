# Talk On Level Up

<p align="center">
  <img src="./Feature - Talk on Level Up.gif" alt="Feature - Talk on Level Up" width="600"/>
</p>

After the level-up stat gains are shown, the unit speaks a short quote based on how many stats increased:

| Stat gains | Quote tier |
|------------|------------|
| 0–2 | Poor |
| 3–5 | Good |
| 6–7 | Great |

Quotes are included for the main playable cast plus several bosses (Lyon, Orson, Glen, Valter, and others). Characters without a table entry skip the speech step.

The patch hooks `StartManimLevelUp` to run a custom proc script. Once the stat gains have finished, the level-up window scrolls away while the unit's portrait stays where it is. The quote then opens in the standard talk bubble, spoken through that same portrait, so no second portrait loads. The quote closes on its own after 90 frames, or earlier on A or B. After that the portrait slides off screen as it does in vanilla.

Speech uses the vanilla talk engine on BG0/BG1, the same layers as the level-up window, so the window has to clear before the bubble can open.

## Configuration

| File | What to edit |
|------|----------------|
| `Installer.event` | `LVUP_TEXT_BASE` (default `$1000`) if the default text ID range conflicts with your project |
| `LevelUpQuotes.event` | Quote strings (IDs are assigned sequentially from `LVUP_TEXT_BASE`). Plain ASCII, no `[OpenX]` / `[LoadFace]` codes. Split long quotes over two lines with `BYTE $01` (`[NL]`); keep each line to about 30 characters. Leave a quote empty to skip that tier |
| `LevelUpQuoteTable.event` | Which character IDs map to which quote row |

Default text IDs: `LVUP_TEXT_BASE` through `LVUP_TEXT_BASE + LVUP_QUOTE_COUNT - 1` (135 entries). Change only `LVUP_TEXT_BASE` in `Installer.event` to relocate the block.

## Target ROM

- **FE8U (USA)** clean ROM
- Quote text is stored uncompressed (`pointer | 0x80000000`), so the ROM needs the anti-Huffman patch (most FEBuilder projects already have it). A raw ColorzCore build on clean FE8U will not display the quotes.
- Hook: vanilla `StartManimLevelUp` at `0x0807F10C` (8-byte `jumpToHack`; Thumb entry `0x0807F10D`)
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/TalkOnLevelUp.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/TalkOnLevelUp.c`:

```bash
make -C Standalone/talk_on_level_up
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `talk_on_level_up` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-talk-on-level-up.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, hook, free-space placement, and `LVUP_TEXT_BASE` |
| `LevelUpQuotes.event` | Quote text data and `setText` pointers |
| `LevelUpQuoteTable.event` | Character → quote text ID mapping |
| `Source/TalkOnLevelUp.c` | Level-up proc hook and speech display |
| `Source/TalkOnLevelUp.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that replaces vanilla `StartManimLevelUp` (`0x0807F10C` / `0x0807F10D`), including the full C Skill System Lvupfx module.
- Text IDs `LVUP_TEXT_BASE` through `LVUP_TEXT_BASE + 134` if your project already uses that range (change `LVUP_TEXT_BASE` in `Installer.event`).
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

`Installer.event` uses `PROTECT` on the hook site and the free-space body. Overlapping patches should fail assembly with a clear EA error.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester). Quote text from `CustomCampaign/Text/LevelUpQuotes/LevelUpQuotes.txt`.
