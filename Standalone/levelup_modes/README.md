# Level-Up Modes

Picks how stat gains are rolled on a battle level-up, separately for each difficulty (Easy, Normal, Hard):

| Mode | Name | Stat gains |
|------|------|------------|
| `0` | Vanilla | Random growths through vanilla `GetStatIncrease`. If nothing is gained, vanilla re-rolls stat by stat up to twice. |
| `1` | Random (separate RNG) | Random growths, rolled on a separate level-up RNG stream. Level-ups don't use up battle RNs, so they can't be steered by battle RNG manipulation and they don't change the RNs of later battles. |
| `2` | Fixed | No randomness. Each stat gains `(growth + (growth * ref) mod 100) / 100`, where `ref` is the unit's level before the level-up minus 1 (+19 if promoted), plus 5 per stat (in HP, Str, Skl, Spd, Lck, Def, Res order). |
| `3` | 100% | +1 to every stat. |
| `4` | 0% | No stat gains. |

Stat caps apply in every mode, and the Metis' Tome +5% growth bonus applies to modes 0-2. Level and EXP handling are vanilla.

This matches the integrated C Skill System `lvup_mode_easy`, `lvup_mode_normal` and `lvup_mode_hard` designer-config flags (the `UnitLvupCore` dispatcher), but installs on **clean FE8U** without the kernel.

## Configuration

Edit the settings at the top of `Installer.event`, then reinstall the patch. No C recompile is needed.

| Constant | Default | Meaning |
|----------|---------|---------|
| `LVUP_MODE_EASY` | `0` | Mode used in Easy mode |
| `LVUP_MODE_NORMAL` | `0` | Mode used in Normal mode |
| `LVUP_MODE_HARD` | `0` | Mode used in Difficult mode |
| `LVUP_MODES_GUARANTEED` | `0` | `1`: when a level-up gains nothing, re-roll up to 10 times at +10% growth (the [guaranteed_lvup](../guaranteed_lvup/) behaviour, in any mode). This replaces vanilla's two stat-by-stat retries. |

The defaults match the integrated `designer-config.c` (all `0`), so the patch changes nothing until you pick a mode. Values above `4` fall back to `0`.

The difficulty is read when the level-up happens: Difficult if the chapter state has the hard-mode bit, Easy if vanilla `TUTORIAL_MODE()` is true, Normal otherwise.

## RAM

Mode `1` keeps its RNG state in **8 bytes** of EWRAM, set in `Source/RamAlloc.s`:

| Symbol | Address range | Size | Use |
|--------|---------------|------|-----|
| `sLvupModesRandSeeds` | `0x0203AE20` .. `0x0203AE27` | 8 | Three 16-bit seeds and a check value |

The slot sits in FreeRamSpace2 (`0x0203AAA4` .. `0x0203DDE0`). Other modes don't read or write it. To move it, change the address in `Source/RamAlloc.s` and run `make`.

The seeds are not saved. When the check value doesn't match (after power-on, or after something else wrote to the slot), the next mode 1 level-up re-draws the seeds from three vanilla RNs. See [Differences from the C Skill System](#differences-from-the-c-skill-system).

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: vanilla `CheckBattleUnitLevelUp` at `0x0802BA28` (Thumb entry `0x0802BA29`)
- Free space: `$1000000` (about 1.3 KB, `$1000000` .. `$1000524`)

## Build

No build step is required to install the patch. `Source/CheckBattleUnitLevelUp_Modes.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/CheckBattleUnitLevelUp_Modes.c` or `Source/RamAlloc.s`:

```bash
make -C Standalone/levelup_modes
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder's bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `levelup_modes` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-levelup-modes.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: settings, hook and free-space placement |
| `Source/CheckBattleUnitLevelUp_Modes.c` | C implementation |
| `Source/RamAlloc.s` | EWRAM address for the mode 1 RNG state |
| `Source/CheckBattleUnitLevelUp_Modes.lyn.event` | Checked-in lyn output (rebuild with `make` after editing the source) |
| `makefile` | Compiles C and the RAM object to `.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `CheckBattleUnitLevelUp` | `$2BA28` | 8 bytes |

**Free space:** `$1000000` (code, then the 4 setting bytes `LvupModesConfig`).

## Conflicts

- **[guaranteed_lvup](../guaranteed_lvup/)** replaces the same function (`CheckBattleUnitLevelUp`, `$2BA28`), so the two can't be installed together. Install only this package and set `LVUP_MODES_GUARANTEED` to `1` to get both behaviours.
- Any other patch that replaces vanilla `CheckBattleUnitLevelUp`, including the full C Skill System level-up rewrite.
- Works with [two_random_number_growths](../two_random_number_growths/) (it hooks `GetStatIncrease`, which mode `0` still calls, so mode `0` rolls 2RN; modes `1`-`4` don't use `GetStatIncrease`) and with [restore_hp_on_level_up](../restore_hp_on_level_up/) (it hooks `BattleApplyUnitUpdates`). Each needs its own free-space address: they all default to `$1000000`.
- **EWRAM** `0x0203AE20` .. `0x0203AE27` overlaps with other hacks using FreeRamSpace2. Change `Source/RamAlloc.s` if needed.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both the hook site (`$2BA28`, 8 bytes) and the free-space body. If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Differences from the C Skill System

- **Difficulty selection.** The integrated `UnitLvupCore` overwrites the Easy choice with a second, unconditional `if`, and treats the vanilla controller bit as "hard". In practice it uses `lvup_mode_normal` in Easy mode and `lvup_mode_hard` in both Normal and Difficult mode, and never uses `lvup_mode_easy`. This patch uses each setting for its own difficulty. With the default settings (all `0`) there is no difference.
- **Mode 0** is vanilla FE8U, including vanilla's two stat-by-stat retries when nothing is gained. The integrated mode 0 is the kernel's own growth routine (skills, multi-level gains, magic), which has no retry.
- **Mode 1** uses the same generator as the integrated `RandC` stream, but the seeds live only in RAM. The integrated kernel seeds them at chapter start and stores them in its expanded save data. Here they are re-drawn from the vanilla RNG on the first mode 1 level-up after power-on, so the stream is still reproducible from a save but isn't the same sequence the kernel would produce.
- Growths are the vanilla character growths. The integrated getters can add class growths and skill bonuses; there is no magic stat.
- The `talk_on_level_up` and `restore_hp_on_level_up` event-slot writes inside the integrated mode functions belong to those features and are not included.

## Limitations

- The mode applies wherever vanilla calls `CheckBattleUnitLevelUp`: combat, arena, staff/dance/item EXP, and vanilla's realistic autolevel (`UnitAutolevelRealistic`, `AutolevelSecondaryLord`). The integrated kernel replaces the same function, so this matches it.
- Stat gains on the level-up screen are capped as in vanilla (`CheckBattleUnitStatCaps`); mode `3` shows `+1` only for stats below their cap.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
