# Battle Max Damage

Sets the most damage a single hit can deal (vanilla: 127). Every hit, critical included, is capped to `BATTLE_MAX_DAMAGE`, and a silencer (lethality) hit deals exactly that much. The battle forecast and the AI's battle simulation use the same cap: each side's attack is lowered so that attack minus defense never exceeds it.

This is the integrated C Skill System `battle_max_damage` designer-config value. Its default there, and here, is `127`, which is the vanilla cap: lower it to use the patch.

## Configuration

Edit the setting at the top of `Installer.event`, then reinstall the patch. No C recompile is needed.

| Constant | Default | Meaning |
|----------|---------|---------|
| `BATTLE_MAX_DAMAGE` | `127` | Highest damage of one hit, `0`-`127` |

Values above 127 are rejected by an `ASSERT`: vanilla stores each hit's HP change in a signed byte, so it can't record more.

## Target ROM

- **FE8U (USA)** clean ROM

## Build

No build step is required to install the patch. `Source/BattleMaxDamage.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/BattleMaxDamage.c`:

```bash
make -C Standalone/battle_max_damage
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `battle_max_damage` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-battle-max-damage.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: settings, addresses, hooks, free-space placement |
| `Source/BattleMaxDamage.c` | C implementation |
| `Source/BattleMaxDamage.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Hooks & Free Space

| Site | `ORG` | Overwritten |
|------|-------|-------------|
| `BattleGenerateHitAttributes`: silencer damage `movs r0, #127` | `$2B4E4` | 1 byte (immediate) |
| `BattleGenerateHitAttributes`: cap `cmp r0, #127` | `$2B532` | 1 byte (immediate) |
| `BattleGenerateHitAttributes`: cap `movs r0, #127` | `$2B536` | 1 byte (immediate) |
| `BattleGenerate` (`0x0802A398`), forecast cap | `$2A398` | 8 bytes (`jumpToHack`) |

- Free space: `$1000000` (about 0xBC bytes: code + 1 setting byte). No RAM.
- `Installer.event` uses `PROTECT` on every patched site and on the free-space body.

## Conflicts

- Any patch that replaces `BattleGenerateHitAttributes` (`$2B3EC`) or `BattleGenerate` (`$2A398`), including the full C Skill System battle rewrite.
- [expanded_hp](../expanded_hp/) (HP up to 254) installs alongside it; hits are still capped at 127 or lower.
- The five battle tweaks extracted with this one ([anima_weapon_triangle](../anima_weapon_triangle/), [hit_decrease_on_range](../hit_decrease_on_range/), [battle_max_damage](../battle_max_damage/), [s_rank_weapon_no_weight](../s_rank_weapon_no_weight/), [modular_staff_exp](../modular_staff_exp/)) hook different functions and can all be installed together (give each body its own free space).
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `FE8U_StandaloneFreeSpace` to the next free region after any already-installed standalone code.

## Limitations

- Scripted battles (`gActionData.scriptedBattleHits`, event-defined damage) keep vanilla's 127 cap, as in the integrated code.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
