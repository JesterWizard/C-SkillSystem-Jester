# Anima Weapon Triangle

Adds a triangle between anima tomes on top of the vanilla weapon triangle:

- **Fire** and **Elfire** beat **Fimbulvetr** (ice) and **Excalibur** (wind)
- **Fimbulvetr** and **Excalibur** beat **Thunder** and **Bolting**
- **Thunder** and **Bolting** beat **Fire** and **Elfire**

The winning side gets +3 damage and +15 hit, the losing side -3 damage and -15 hit, for both the attacker and the defender. It goes through vanilla's weapon triangle bonus fields, so the battle forecast shows the usual up/down triangle arrows. The vanilla sword/lance/axe and anima/light/dark triangle and the reaver weapons are unchanged.

This is the integrated C Skill System `anima_weapon_triangle` designer-config flag (default `false` there), always on here.

## Configuration

Edit `Installer.event`, then reinstall the patch. No C recompile is needed.

| Constant | Default | Meaning |
|----------|---------|---------|
| `ANIMA_TRIANGLE_ATK_BONUS` | `3` | Damage gained by the winning tome (and lost by the losing one) |
| `ANIMA_TRIANGLE_HIT_BONUS` | `15` | Hit gained by the winning tome (and lost by the losing one) |

`AnimaTriangleTable` has one `AnimaTriangleEntry(tome, beats, beats)` row per tome (use `0` for an unused slot) and ends with `AnimaTriangleEnd`. Any item ID works, so you can add custom tomes or use the same rule for other weapons.

## Target ROM

- **FE8U (USA)** clean ROM

## Build

No build step is required to install the patch. `Source/AnimaWeaponTriangle.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/AnimaWeaponTriangle.c`:

```bash
make -C Standalone/anima_weapon_triangle
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `anima_weapon_triangle` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-anima-weapon-triangle.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: settings, addresses, hooks, free-space placement, tome table |
| `Source/AnimaWeaponTriangle.c` | C implementation |
| `Source/AnimaWeaponTriangle.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `BattleApplyWeaponTriangleEffect` (`0x0802C7C0`) | `$2C7C0` | 8 bytes (`jumpToHack`) |

The replacement runs the vanilla triangle first. It reads the vanilla rule table through the function's own literal at `0x0802C7CC` (not overwritten), so a weapon triangle edited or repointed in FEBuilder still applies.

- Free space: `$1000000` (about 0x13C bytes: code, 2 setting bytes, table). No RAM.
- `Installer.event` uses `PROTECT` on the hook site and on the free-space body.

## Conflicts

- Any patch that replaces `BattleApplyWeaponTriangleEffect` (`$2C7C0`), including the full C Skill System battle rewrite.
- Patches that edit the vanilla weapon triangle table are fine (see above).
- The five battle tweaks extracted with this one ([anima_weapon_triangle](../anima_weapon_triangle/), [hit_decrease_on_range](../hit_decrease_on_range/), [battle_max_damage](../battle_max_damage/), [s_rank_weapon_no_weight](../s_rank_weapon_no_weight/), [modular_staff_exp](../modular_staff_exp/)) hook different functions and can all be installed together (give each body its own free space).
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `FE8U_StandaloneFreeSpace` to the next free region after any already-installed standalone code.

## Limitations

- Like vanilla weapon-triangle damage, the bonus is added to weapon might before effectiveness, so an effective tome (Excalibur against fliers) doubles it. In the integrated code it is added after. No vanilla flier uses anima, so this only matters with custom data.
- Divergences from the integrated code, which has two data quirks: its per-weapon table gives the six tomes weapon type `0` (sword), so a tome user got +3/+15 against every sword user even with the flag off; and the "losing tome" half of the rule ran even with the flag off. This patch applies only the intended tome-against-tome rule.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
