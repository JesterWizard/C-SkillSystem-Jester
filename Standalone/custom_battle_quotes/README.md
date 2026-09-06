# Custom Battle Quotes

Replaces vanilla `GetBattleQuoteEntry` with extended lookup logic that matches battle quotes on **both combat participants**, not just one character.

Each table entry can restrict by:

| Field | Purpose |
|-------|---------|
| `pidA` | First character ID (`0` = wildcard for `pidB`-only entries) |
| `pidB` | Second character ID (`0` = wildcard for `pidA`-only entries) |
| `chapter` | Chapter index, `CHAPTER_FF` for any chapter, `0xFE` for triangle attacks |
| `flag` | Permanent flag set after the quote plays; skipped if already set |
| `msg` | Text ID (`0` = use `event` instead) |
| `event` | Custom battle event script (used when `msg` is `0`) |

Example: O'Neill can have different pre-battle dialogue when fighting Eirika vs Seth.

With `USE_VANILLA_FALLBACK` enabled (default), vanilla `gBattleTalkList` entries are searched after your custom table, so clean FE8U quotes keep working until you override them.

## Configuration

Edit `BattleTalkTable.event`, then reinstall the patch. No C recompile is needed.

| Setting | Default | Meaning |
|---------|---------|---------|
| `USE_VANILLA_FALLBACK` | `1` | Also search vanilla `gBattleTalkList` when no custom entry matches |

Example table entry:

```text
CustomBattleQuoteEntry($01, $68, $00, $01, $XXXX, 0)
```

`$01` = Eirika, `$68` = O'Neill, `$00` = prologue, `$01` = battle-quote flag, `$XXXX` = your text ID from FEBuilder.

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: vanilla `GetBattleQuoteEntry` at `0x0808464C`
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/CustomBattleQuotes.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/CustomBattleQuotes.c`:

```bash
make -C Standalone/custom_battle_quotes
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

### FEBuilderGBA

1. Download [FEBuilderGBA (Laqieer branch)](https://nightly.link/laqieer/FEBuilderGBA/workflows/msbuild/master).
2. In **Settings → Options → Path**, set **Event Assembler** to this repo's [ColorzCore](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) executable (see [Setup.md](../../Documentation/Setup.md)).
3. Open your project ROM in FEBuilder.
4. Go to **Advanced Editors → Insert EA**.
5. Click **Select File**, choose `Standalone/custom_battle_quotes/Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`:

```bash
cp /path/to/fe8.gba /path/to/fe8-custom-battle-quotes.gba
Tools/EventAssembler/ColorzCore A FE8 \
  -input:Standalone/custom_battle_quotes/Installer.event \
  -output:/path/to/fe8-custom-battle-quotes.gba
```

Run from the repository root so Event Assembler can resolve `EAstdlib.event`.

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, hook, and free-space placement |
| `BattleTalkTable.event` | Fallback toggle and editable quote table (no C rebuild needed) |
| `Source/CustomBattleQuotes.c` | Lookup logic |
| `Source/CustomBattleQuotes.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that replaces vanilla `GetBattleQuoteEntry` (`0x0808464C`), including the full C Skill System Quotes module.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

`Installer.event` uses `PROTECT` on the hook site and the free-space body. Overlapping patches should fail assembly with a clear EA error.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
