# Hit Decrease On Range

When the unit that starts a battle attacks from 3 or more tiles away, its displayed and real hit rate drops by 20 for every 2 tiles of distance: -20 at 3-4 tiles, -40 at 5-6 tiles, and so on. The penalty is applied to the effective hit (hit minus the target's avoid) before the 0-100 clamp, so it shows in the battle forecast. Counter-attacks are never penalised, and ballista attacks are exempt.

This is the integrated C Skill System `hit_decrease_on_range` designer-config flag (default `false` there), always on here.

## Configuration

Edit the setting at the top of `Installer.event`, then reinstall the patch. No C recompile is needed.

| Constant | Default | Meaning |
|----------|---------|---------|
| `HIT_DECREASE_ON_RANGE_PENALTY` | `20` | Hit lost per 2 tiles of distance |

## Target ROM

- **FE8U (USA)** clean ROM

## Build

No build step is required to install the patch. `Source/HitDecreaseOnRange.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/HitDecreaseOnRange.c`:

```bash
make -C Standalone/hit_decrease_on_range
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `hit_decrease_on_range` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-hit-decrease-on-range.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: settings, addresses, hooks, free-space placement |
| `Source/HitDecreaseOnRange.c` | C implementation |
| `Source/HitDecreaseOnRange.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `ComputeBattleUnitEffectiveHitRate` (`0x0802AC60`) | `$2AC60` | 8 bytes (`jumpToHack`) |

- Free space: `$1000000` (about 0x64 bytes: code + 1 setting byte). No RAM.
- `Installer.event` uses `PROTECT` on the hook site and on the free-space body.

## Conflicts

- Any patch that replaces vanilla `ComputeBattleUnitEffectiveHitRate` (`$2AC60`), including the full C Skill System battle rewrite.
- Works with [last_weapon_hit_crit](../last_weapon_hit_crit/) (hooks the next function, `ComputeBattleUnitEffectiveCritRate` at `$2AC90`) and [biorhythm_mechanic](../biorhythm_mechanic/) (hooks `ComputeBattleUnitHitRate`/`ComputeBattleUnitAvoidRate`; its bonus is part of the hit this patch starts from).
- The five battle tweaks extracted with this one ([anima_weapon_triangle](../anima_weapon_triangle/), [hit_decrease_on_range](../hit_decrease_on_range/), [battle_max_damage](../battle_max_damage/), [s_rank_weapon_no_weight](../s_rank_weapon_no_weight/), [modular_staff_exp](../modular_staff_exp/)) hook different functions and can all be installed together (give each body its own free space).
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `FE8U_StandaloneFreeSpace` to the next free region after any already-installed standalone code.

## Limitations

- The integrated code skips the penalty for units with the Magic Eye skill and shows a one-time tutorial popup; both are Skill System features and are not part of this patch.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
