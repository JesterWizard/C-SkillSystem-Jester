# Talk On Level Up

Standalone FE8U patch extracted from the C Skill System designer config flag `talk_on_level_up`.

## Behavior

After the level-up stat gains are shown, the unit speaks a short quote based on how many stats increased:

| Stat gains | Quote tier |
|------------|------------|
| 0–2 | Poor |
| 3–5 | Good |
| 6–7 | Great |

Quotes are included for the main playable cast plus several bosses (Lyon, Orson, Glen, Valter, and others). Characters without a table entry skip the speech step.

The patch hooks `StartManimLevelUp` to run a custom proc script that inserts `DisplayCharacterSpeech` after the stat-gain animation, matching the integrated C Skill System behavior on vanilla FE8U growth rolls.

## Configuration

| File | What to edit |
|------|----------------|
| `Installer.event` | `LVUP_TEXT_BASE` (default `$1000`) if the default text ID range conflicts with your project |
| `LevelUpQuotes.event` | Quote strings (IDs are assigned sequentially from `LVUP_TEXT_BASE`) |
| `LevelUpQuoteTable.event` | Which character IDs map to which quote row |

Default text IDs: `LVUP_TEXT_BASE` through `LVUP_TEXT_BASE + LVUP_QUOTE_COUNT - 1` (135 entries). Change only `LVUP_TEXT_BASE` in `Installer.event` to relocate the block.

## Target ROM

- **FE8U (USA)** clean ROM
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

### FEBuilderGBA

1. Download [FEBuilderGBA (Laqieer branch)](https://nightly.link/laqieer/FEBuilderGBA/workflows/msbuild/master).
2. In **Settings → Options → Path**, set **Event Assembler** to this repo's [ColorzCore](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) executable (see [Setup.md](../../Documentation/Setup.md)).
3. Open your project ROM in FEBuilder.
4. Go to **Advanced Editors → Insert EA**.
5. Click **Select File**, choose `Standalone/talk_on_level_up/Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`:

```bash
cp /path/to/fe8.gba /path/to/fe8-talk-on-level-up.gba
Tools/EventAssembler/ColorzCore A FE8 \
  -input:Standalone/talk_on_level_up/Installer.event \
  -output:/path/to/fe8-talk-on-level-up.gba
```

Run from the repository root so Event Assembler can resolve `EAstdlib.event` and `Tools/Tool Helpers.txt`.

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

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester). Quote text from `Data/CustomCampaign/Text/LevelUpQuotes/LevelUpQuotes.txt`.
