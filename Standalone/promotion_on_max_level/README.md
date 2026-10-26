# Promotion On Max Level

Right after a battle in which an unpromoted unit levels up to 20, the vanilla promotion screen opens. The promoted unit starts at level 1 with 0 experience, and does not spend a use of an inventory item. It works with map animations and with battle animations.

An optional setting also promotes trainees (Ross, Amelia, Ewan, or any unit whose class or character has the "max level 10" attribute) right after the battle in which they reach level 10, which is the same as [promote_trainees_in_chapter](../promote_trainees_in_chapter/). Classes with no entry in the vanilla promotion table leave the screen immediately. Arena and staff level-ups don't trigger a promotion. Integrated `designer-config.c` defaults this flag to `true`.

### Why it runs after the battle

The promotion starts from the `BATTLE_HandleCombatDeaths` step of vanilla `sProcScr_CombatAction`, after the battle results have been applied to the real units. Earlier versions of this patch (and the integrated C Skill System code) started it from the map level-up window (`ManimLevelUp_ScrollOut`). On vanilla that window closes while the battle is still animating, before the results are applied, so the unit lost its level-up gains and the weapon use, and got the promotion bonuses twice.

## Configuration

Edit the settings at the top of `Installer.event`, then reinstall the patch. No C recompile is needed.

| Constant | Default | Meaning |
|----------|---------|---------|
| `PROMOTION_ON_MAX_LEVEL` | `1` | Set to `0` to leave level 20 units unpromoted |
| `PROMOTE_TRAINEES_AT_LEVEL_10` | `0` | Set to `1` to also promote trainees when they reach level 10 |

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: the `BATTLE_HandleCombatDeaths` step of vanilla `sProcScr_CombatAction` (routine pointer at `$59DB10`, 4 bytes, repointed with `POIN`)
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

- Any patch that repoints or rewrites `sProcScr_CombatAction` (`$59DABC` .. `$59DB24`), in particular the `BATTLE_HandleCombatDeaths` pointer at `$59DB10`, including the full C Skill System.
- [promote_trainees_in_chapter](../promote_trainees_in_chapter/) repoints the same pointer, so only one of the two can be installed. Use `PROMOTE_TRAINEES_AT_LEVEL_10` here to get both behaviours.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

The promotion handler's item slot is set to `-1` after `StartBmPromotion`, so vanilla's `UnitUpdateUsedItem` doesn't spend a use of the slot the battle left in `gActionData`.

`Installer.event` uses `PROTECT` on both the hook site (`$59DB10`, 4 bytes) and the free-space body (`$1000000` through end of install). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
