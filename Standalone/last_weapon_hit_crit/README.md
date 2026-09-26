# Last Weapon Hit Crit

A combat strike whose remaining weapon uses are 1 is a guaranteed critical. That covers starting a fight on the last use, and a later strike in the same battle once earlier hits have spent the weapon down to 1 use.

Unbreakable weapons and weapons with 255 uses are excluded. Each strike costs 1 use, which is vanilla durability. The forecast shows 100 crit only when the weapon already has 1 use when the preview is built. A follow-up that spends the last use still crits.

Negate-crit items and the monster stone do not block this critical. Integrated `designer-config.c` defaults this flag to `true`.

## Configuration

Edit `LAST_WEAPON_HIT_CRIT` at the top of `Installer.event`, then reinstall the patch. No C recompile is needed.

| Constant | Default | Meaning |
|----------|---------|---------|
| `LAST_WEAPON_HIT_CRIT` | `1` | Set to `0` to use vanilla crit rates |

## Target ROM

- **FE8U (USA)** clean ROM
- Hooks:
  - `ComputeBattleUnitEffectiveCritRate` at `0x0802AC91` (installer ORG `$2AC90`, 8 bytes) for the battle forecast
  - `BattleUpdateBattleStats` at `0x0802B1C5` (installer ORG `$2B1C4`, 8 bytes) for each strike
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/LastWeaponHitCrit.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/LastWeaponHitCrit.c`:

```bash
make -C Standalone/last_weapon_hit_crit
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `last_weapon_hit_crit` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-last-weapon-hit-crit.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hooks, and free-space placement |
| `Source/LastWeaponHitCrit.c` | C implementation |
| `Source/LastWeaponHitCrit.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that replaces `ComputeBattleUnitEffectiveCritRate` (`$2AC90`) or `BattleUpdateBattleStats` (`$2B1C4`), including the full C Skill System battle-stat rewrite.
- `Standalone/infinite_durability/` stops weapons from losing uses, so only a weapon that already has 1 use can trigger this critical.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

Combat-art durability costs and gaiden-magic weapon slots are skill-system only and are not part of this patch. On clean FE8U every breakable weapon spends 1 use per strike.

`Installer.event` uses `PROTECT` on both hook sites (8 bytes each) and the free-space body (`$1000000` through end of install). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
