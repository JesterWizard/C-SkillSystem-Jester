# S Rank Weapon No Weight

A weapon whose type the unit has at S rank weighs nothing for attack speed: the unit's attack speed is its full Speed, whatever the weapon weight and its Constitution. Other weapons keep the vanilla rule (weight minus Constitution, never below 0). This applies to the battle forecast, the stat screen's battle stats and real battles.

This is the integrated C Skill System `s_rank_weapon_no_weight` designer-config flag (default `true` there), always on here.

## Target ROM

- **FE8U (USA)** clean ROM

## Build

No build step is required to install the patch. `Source/SRankWeaponNoWeight.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/SRankWeaponNoWeight.c`:

```bash
make -C Standalone/s_rank_weapon_no_weight
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `s_rank_weapon_no_weight` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-s-rank-no-weight.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: settings, addresses, hooks, free-space placement |
| `Source/SRankWeaponNoWeight.c` | C implementation |
| `Source/SRankWeaponNoWeight.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `ComputeBattleUnitSpeed` (`0x0802AB74`) | `$2AB74` | 8 bytes (`jumpToHack`) |

- Free space: `$1000000` (about 0x70 bytes). No RAM.
- `Installer.event` uses `PROTECT` on the hook site and on the free-space body.

## Conflicts

- Any patch that replaces `ComputeBattleUnitSpeed` (`$2AB74`), including the full C Skill System battle rewrite.
- [dynamic_weapon_slots](../dynamic_weapon_slots/): this patch reads the rank in the vanilla slot of the weapon's type (`ranks[type]`, types 0-7). With a class that moves a type to another slot, S rank in that moved slot isn't seen. Weapon types 8 and above are never weightless.
- The five battle tweaks extracted with this one ([anima_weapon_triangle](../anima_weapon_triangle/), [hit_decrease_on_range](../hit_decrease_on_range/), [battle_max_damage](../battle_max_damage/), [s_rank_weapon_no_weight](../s_rank_weapon_no_weight/), [modular_staff_exp](../modular_staff_exp/)) hook different functions and can all be installed together (give each body its own free space).
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `FE8U_StandaloneFreeSpace` to the next free region after any already-installed standalone code.

## Limitations

- The integrated code also adds a share of attack to Constitution (`as_calc_atk_perc`, a separate kernel setting) and has the Graceful Wielder skill; neither is part of this patch.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
