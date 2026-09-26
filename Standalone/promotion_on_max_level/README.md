# Promotion On Max Level

When the map level-up window finishes scrolling out, an unpromoted unit at level 20 opens the vanilla promotion screen. Level and experience are set to 1 and 0 as that screen starts. The unit does not spend a use of an inventory item.

This runs for whoever the level-up window belongs to, including enemies. Classes with no entry in the vanilla promotion table leave the screen immediately. Trainee auto-promotion at level 10 is a separate feature and is not included.

The check is on the map level-up window (`ManimLevelUp_ScrollOut`). Battle-animation level-ups do not promote. Integrated `designer-config.c` defaults this flag to `true`.

## Configuration

Edit `PROMOTION_ON_MAX_LEVEL` at the top of `Installer.event`, then reinstall the patch. No C recompile is needed.

| Constant | Default | Meaning |
|----------|---------|---------|
| `PROMOTION_ON_MAX_LEVEL` | `1` | Set to `0` to leave level 20 units unpromoted |

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: vanilla `ManimLevelUp_ScrollOut` at `0x0807F355` (installer ORG `$7F354`, 8 bytes)
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/PromotionOnMaxLevel.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/PromotionOnMaxLevel.c`:

```bash
make -C Standalone/promotion_on_max_level
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `promotion_on_max_level` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-promotion-on-max-level.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hook, and free-space placement |
| `Source/PromotionOnMaxLevel.c` | C implementation |
| `Source/PromotionOnMaxLevel.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that replaces vanilla `ManimLevelUp_ScrollOut` (`$7F354` / `0x0807F355`), including the full C Skill System map level-up rewrite.
- `Standalone/talk_on_level_up/` calls this same function from its own level-up proc, so the promotion still runs if both bodies are installed at different free-space addresses.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

The integrated hook leaves `gActionData.itemSlotIndex` alone. Vanilla promotion then calls `UnitUpdateUsedItem` on that slot, which can spend a use of the weapon just used in battle. This patch sets the promotion handler's item slot to `-1` after `StartBmPromotion` so no item is consumed.

`Installer.event` uses `PROTECT` on both the hook site (`$7F354`, 8 bytes) and the free-space body (`$1000000` through end of install). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
