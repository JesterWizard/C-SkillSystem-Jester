# Restore HP on Level Up

When a battle level-up is committed, the player unit in that battle is refilled to max HP.

- Player phase: the acting unit
- Enemy or NPC phase: the target

This happens if either side leveled up, including a 0-stat level-up. Dead units are left dead. Enemies who level up are not refilled. HP is written when battle results are applied, so there is no separate heal popup.

Integrated `designer-config.c` defaults this flag to `false`. This patch defaults to on.

## Configuration

Edit `RESTORE_HP_ON_LEVEL_UP` at the top of `Installer.event`, then reinstall the patch. No C recompile is needed.

| Constant | Default | Meaning |
|----------|---------|---------|
| `RESTORE_HP_ON_LEVEL_UP` | `1` | Set to `0` to keep vanilla HP on level-up |

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: vanilla `BattleApplyUnitUpdates` at `0x0802C028` (Thumb entry `0x0802C029`)
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/RestoreHpOnLevelUp.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/RestoreHpOnLevelUp.c`:

```bash
make -C Standalone/restore_hp_on_level_up
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `restore_hp_on_level_up` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-restore-hp-on-level-up.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hook, and free-space placement |
| `Source/RestoreHpOnLevelUp.c` | C implementation |
| `Source/RestoreHpOnLevelUp.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that replaces vanilla `BattleApplyUnitUpdates` (`0x0802C028` / `0x0802C029`), including the full C Skill System battle-result rewrite.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

`Installer.event` uses `PROTECT` on the hook site (`$2C028`, 8 bytes) and on the free-space body (`$1000000` through end of install). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
