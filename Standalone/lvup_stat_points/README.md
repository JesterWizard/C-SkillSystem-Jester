# Level-Up Stat Points

Player units no longer roll growths when they level up. Instead, a menu lets the player decide the gains, in one of two modes:

| Mode | Name | How gains are chosen |
|------|------|----------------------|
| `0` | Stat points | The unit gets `LVUP_STAT_POINTS` points per level and spends them in a menu (HP, Str, Skl, Spd, Lck, Def, Res). Each stat takes at most one point per round; once every stat that can still grow has had its point, a new round starts. Points can't push a stat past its cap. The menu closes when all points are spent (or nothing can grow any more). |
| `1` | Timing minigame | Each stat row has a bar with a moving cursor. The bar has one green segment per 10% growth (character growth, +5% with Metis' Tome). Press **A** while the cursor is on green to gain +1 in that stat; the cursor then jumps to the next stat. A growth of 100% or more gets one more pass per extra 100%. The menu closes by itself one second after the last stat. |

The gains are written into the battle copy of the unit, exactly where vanilla puts its rolled growths, so vanilla applies them to the unit after the battle, and caps, HP and the level/EXP handling stay vanilla. Enemy and NPC level-ups, and the realistic autolevels vanilla uses when some units join, still roll growths as usual.

Where the menu appears:

- **Map animations:** in place of the map level-up window, during the battle.
- **Battle animations:** the level-up screen is skipped, and the menu opens on the map as soon as the animation has ended (the menu can't be drawn over the battle scene). The game waits for the menu before applying the battle results. In the arena the menu opens on a black screen between the fight and the results screen.

This is the integrated C Skill System `lvup_stat_points` / `lvup_stat_timing` designer-config flags (`Data/UnitMenu/Source/StatPoints.c`), installed on **clean FE8U** without the kernel. Both modes are in this one package; pick one with a setting.

## Configuration

Edit the settings at the top of `Installer.event`, then reinstall the patch. No C recompile is needed.

| Constant | Default | Meaning |
|----------|---------|---------|
| `LVUP_STAT_MODE` | `0` | `0` = stat points, `1` = timing minigame |
| `LVUP_STAT_POINTS` | `3` | Points per level in mode `0` (1-255). Ignored in mode `1`. |

The integrated designer config defaults both flags to off, so there is no integrated default to copy; `3` points is this package's choice.

## Save data

None. The points are spent in the same level-up that earns them, so nothing is pending between battles and nothing is saved. No RAM is reserved either: the menu state lives in its own proc.

## Target ROM

- **FE8U (USA)** clean ROM
- Hooks: vanilla `NewEkrLevelup` at `0x08073988` and `StartManimLevelUp` at `0x0807F10C`
- Free space: `$1000000` (about 4.4 KB, `$1000000` .. `$100115E`)

## Build

No build step is required to install the patch. `Source/LvupStatPoints.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/LvupStatPoints.c`:

```bash
make -C Standalone/lvup_stat_points
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder's bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `lvup_stat_points` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-lvup-stat-points.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: settings, hooks and free-space placement |
| `Source/LvupStatPoints.c` | C implementation |
| `Source/LvupStatPoints.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` (`lyn -nohook`) |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `NewEkrLevelup` | `$73988` | 8 bytes |
| `StartManimLevelUp` | `$7F10C` | 8 bytes |

Both replacements fall back to the vanilla body for non-player units (and for the promotion screen, which also goes through `NewEkrLevelup`).

**Free space:** `$1000000` (code, then the 2 setting bytes `gLvupStatPointsConfig`).

## Conflicts

- **[talk_on_level_up](../talk_on_level_up/)** replaces the same function (`StartManimLevelUp`, `$7F10C`), so the two can't be installed together. (The integrated kernel also lets the stat menu take priority over the level-up talk.)
- Works with **[levelup_modes](../levelup_modes/)**, **[guaranteed_lvup](../guaranteed_lvup/)** and **[two_random_number_growths](../two_random_number_growths/)**: they hook `CheckBattleUnitLevelUp` / `GetStatIncrease`, which still roll for every unit; this patch discards the roll for player units only when it opens the menu, so enemy and NPC level-ups keep using them.
- Works with **[restore_hp_on_level_up](../restore_hp_on_level_up/)** (it hooks `BattleApplyUnitUpdates` and only checks that the level went up) and **[stat_gain_frame_speed](../stat_gain_frame_speed/)** (it speeds up the map level-up window, which now only shows for non-player units).
- Any other patch that replaces vanilla `NewEkrLevelup` or `StartManimLevelUp`, including the full C Skill System.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both hook sites and the free-space body. If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Differences from the C Skill System

- **Battle animations.** The integrated kernel opens the menu over the battle scene. On vanilla FE8U the battle scene uses the same VRAM as the menu's font, window and hand cursor, so it shows up garbled; here the battle-animation level-up screen is skipped and the menu opens on the map once the animation has ended, with the game held (`LockGame`) until it closes.
- **Only player units.** The integrated kernel skips growth rolls for every unit (enemy and NPC level-ups gain nothing, as do realistic autolevels for joining units). Here only player units that reach the menu are affected.
- **Stats.** No separate Magic stat (vanilla has none); growths are vanilla character growths (+5% Metis' Tome) and caps are the vanilla caps (60 HP, class maximums, 30 Lck). The integrated kernel uses its skill-aware growth getters and Limit Breaker caps.
- **Points mode rounds.** The integrated menu only starts a new round once all 8 stats received a point, so a unit with a capped stat could get stuck with unspendable points. Here a new round starts once every stat that can still grow has its point.
- **Timing mode close.** The integrated timing menu ends without clearing its text from BG0; this one clears it.
- **No R-button help.** The menu rows have no help text (the integrated ones point at kernel text IDs).
- **Prep-screen BEXP** level-ups are a kernel feature and are not part of this package.

## Limitations

- Tested on FE8U with mGBA: points and timing mode with battle animations, points mode with map animations, and a level-up in an arena fight. Staff, dance and scripted event battle level-ups use the same two hooks but were not tested in-game.
- In mode `1` the result depends on timing, so the gains are not reproducible from a save the way vanilla RNG level-ups are.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
