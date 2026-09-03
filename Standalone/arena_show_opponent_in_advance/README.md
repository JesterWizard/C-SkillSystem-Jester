# Arena Show Opponent In Advance

Standalone FE8U patch extracted from the C Skill System designer config flag `arena_show_opponent_in_advance`.

## Behavior

Vanilla FE8U only reveals the arena opponent after the player confirms the gold wager. This patch shows opponent details **before** the wager prompt, and adds a matchup quality label:

| Label | Condition |
|-------|-----------|
| Good match | Player power weight is at least 20 above the opponent |
| Okay match | Opponent is not more than 20 above the player |
| Bad match | Opponent is more than 20 above the player |

The details window is taller than vanilla (8 rows instead of 6) to fit the matchup label. Opponent info is still redrawn after the wager is confirmed, matching integrated C Skill System behavior.

## Target ROM

- **FE8U (USA)** clean ROM
- Hooks:
  - vanilla `ArenaUi_WagerGoldDialogue` at `0x080B59CC` (Thumb entry `0x080B59CD`)
  - vanilla `DrawArenaOpponentDetailsText` at `0x080B5C48` (Thumb entry `0x080B5C49`)
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/ArenaShowOpponentInAdvance.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/ArenaShowOpponentInAdvance.c`:

```bash
make -C Standalone/arena_show_opponent_in_advance
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

### FEBuilderGBA

1. Download [FEBuilderGBA (Laqieer branch)](https://nightly.link/laqieer/FEBuilderGBA/workflows/msbuild/master).
2. In **Settings → Options → Path**, set **Event Assembler** to this repo's [ColorzCore](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) executable (see [Setup.md](../../Documentation/Setup.md)).
3. Open your project ROM in FEBuilder.
4. Go to **Advanced Editors → Insert EA**.
5. Click **Select File**, choose `Standalone/arena_show_opponent_in_advance/Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`:

```bash
cp /path/to/fe8.gba /path/to/fe8-arena-preview.gba
Tools/EventAssembler/ColorzCore A FE8 \
  -input:Standalone/arena_show_opponent_in_advance/Installer.event \
  -output:/path/to/fe8-arena-preview.gba
```

Run from the repository root so Event Assembler can resolve `EAstdlib.event`.

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hooks, and free-space placement |
| `Source/ArenaShowOpponentInAdvance.c` | C implementation |
| `Source/ArenaShowOpponentInAdvance.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that replaces vanilla `ArenaUi_WagerGoldDialogue` (`0x080B59CC`) or `DrawArenaOpponentDetailsText` (`0x080B5C48`), including the full C Skill System arena rewrite or arena roster menu.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both hook sites and the free-space body. If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
