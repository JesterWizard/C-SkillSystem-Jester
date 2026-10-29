# Send Inventory On Death

When a player unit is defeated in battle or in the arena, its items are sent to the convoy before it dies, so weapons and items aren't lost with the unit.

- Items go in inventory order while the convoy has room (vanilla holds 100). Items that don't fit stay with the unit.
- Sent items are removed from the unit's inventory.
- Enemies, NPCs and summoned phantoms are not affected.

This matches the integrated C Skill System `send_inventory_on_death` designer-config flag, but installs on **clean FE8U** without the kernel. See [Differences from the C Skill System](#differences-from-the-c-skill-system).

## Target ROM

- **FE8U (USA)** clean ROM
- Hooks: vanilla `KillUnitOnCombatDeath` at `$32728` and `KillUnitOnArenaDeathMaybe` at `$32750`
- Free space: `$1000000` (248 bytes, `$1000000` .. `$10000F8`)

## Build

No build step is required to install the patch. `Source/SendInventoryOnDeath.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/SendInventoryOnDeath.c`:

```bash
make -C Standalone/send_inventory_on_death
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder's bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `send_inventory_on_death` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-send-inventory.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: hooks and free-space placement |
| `Source/SendInventoryOnDeath.c` | C implementation |
| `Source/SendInventoryOnDeath.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `KillUnitOnCombatDeath` | `$32728` | 8 bytes (`jumpToHack`) |
| `KillUnitOnArenaDeathMaybe` | `$32750` | 8 bytes (`jumpToHack`) |

Both replacements keep the vanilla steps (HP check, `PidStatsRecordDefeatInfo`, vanilla `UnitKill` in the same order) and send the inventory first. They run from `BATTLE_HandleCombatDeaths` and the arena's death handler, after the battle results (weapon uses, broken weapons) have been applied to the unit.

**Free space:** `$1000000`. **RAM:** none.

## Conflicts

- Any patch that changes vanilla `KillUnitOnCombatDeath` (`$32728`) or `KillUnitOnArenaDeathMaybe` (`$32750`), including the full C Skill System.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or change `FE8U_StandaloneFreeSpace` in `Installer.event` to an offset after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both hook sites (8 bytes each) and the free-space body. If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

### With other standalone patches

| Patch | Together? |
|-------|-----------|
| [casual_mode](../casual_mode/) | Yes. Because sent items are removed from the unit, a unit that comes back next chapter doesn't duplicate them. Tested together. |
| [collect_dead_units](../collect_dead_units/) | Yes. Tested together. |
| [promote_trainees_in_chapter](../promote_trainees_in_chapter/), [promotion_on_max_level](../promotion_on_max_level/) | Yes. They wrap `BATTLE_HandleCombatDeaths`, which still calls this patch's `KillUnitOnCombatDeath`. |
| [death_dance](../death_dance/), [rescue_drop_move_again](../rescue_drop_move_again/), [promote_enemy_on_kill](../promote_enemy_on_kill/) | Yes, different functions. Tested with `death_dance` and `promote_enemy_on_kill`. |

To install several, give each its own free space (for example `$1000000`, `$1000200`, `$1000400`, ...).

## Differences from the C Skill System

- **Sent items are removed from the unit.** The integrated code copies them, which is harmless while the dead unit stays dead, but duplicates them if the unit comes back (Casual Mode or a revival event).
- **Phantoms** (Ewan's summons) are skipped, so a summon can't be used to farm its weapon into the convoy.
- **Convoy size** is vanilla's 100; the kernel uses its expanded convoy size.

## Limitations

- Like the integrated code, only battle and arena deaths send items. Deaths from poison, traps or event kill commands go straight to vanilla `UnitKill` and the items are lost as in vanilla.
- Tested on clean FE8U in mGBA: Seth, defeated by a Prologue soldier, sent his three items to the empty convoy and was left with an empty inventory; enemy deaths were unchanged. The arena path and a full convoy were not runtime-tested.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
