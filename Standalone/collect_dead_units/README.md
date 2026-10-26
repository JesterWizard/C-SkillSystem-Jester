# Collect Dead Units

In vanilla FE8 a defeated enemy or NPC is erased from the unit array on the spot. With this patch it stays in its slot until the end of the chapter, marked dead and hidden, the same way vanilla already keeps defeated player units. Events, ASMCs and other patches can then still find a defeated unit and read its data (class, items, stats), or bring it back.

- Applies to every non-player death that goes through vanilla `UnitKill` (battle, arena, poison/traps, event kills).
- Nothing changes on screen: dead units are hidden and off the map, the AI, healing and rout/"defeat all" checks already skip `US_DEAD` units, and vanilla clears all non-player units at the end of the chapter.
- If a map loads more enemies (50) or NPCs (20) over the chapter than the unit array holds, a new unit takes the slot of a dead one instead of failing.

This matches the integrated C Skill System `collect_dead_units` designer-config flag, but installs on **clean FE8U** without the kernel. See [Differences from the C Skill System](#differences-from-the-c-skill-system).

## Target ROM

- **FE8U (USA)** clean ROM
- Hooks: vanilla `UnitKill` at `$183FC` and `GetFreeUnit` at `$17838`
- Free space: `$1000000` (164 bytes, `$1000000` .. `$10000A4`)

## Build

No build step is required to install the patch. `Source/CollectDeadUnits.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/CollectDeadUnits.c` or `Source/CollectDeadUnitsHook.s`:

```bash
make -C Standalone/collect_dead_units
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder's bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `collect_dead_units` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-collect-dead-units.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: hooks and free-space placement |
| `Source/CollectDeadUnitsHook.s` | `UnitKill` entry stub |
| `Source/CollectDeadUnits.c` | `GetFreeUnit` replacement |
| `Source/CollectDeadUnits.lyn.event` | Checked-in lyn output (rebuild with `make` after editing the sources) |
| `makefile` | Compiles the sources to `.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `UnitKill` | `$183FC` | 8 bytes (`$183FC` .. `$18403`, `jumpToHack`) |
| `GetFreeUnit` | `$17838` | 8 bytes (`$17838` .. `$1783F`, `jumpToHack`) |

`UnitKill_CollectDeadUnits` redoes the four overwritten vanilla instructions and checks the unit's faction. A non-player unit gets `state |= US_DEAD | US_HIDDEN` in place of `pCharacterData = NULL`. A player unit continues into vanilla `UnitKill` at `$1840C`, unchanged, so a patch that hooks the player branch further down (such as [casual_mode](../casual_mode/) at `$18418`) still runs.

`GetFreeUnit_CollectDeadUnits` returns the first empty slot of the faction like vanilla. If there is none (non-player factions only), it clears the first dead unit of that faction and returns its slot. It also skips the unused unit table entries past the end of each faction's array, which vanilla reads through a null pointer.

**Free space:** `$1000000`. **RAM:** none.

## Conflicts

- Any patch that changes the start of vanilla `UnitKill` (`$183FC` .. `$18403`) or `GetFreeUnit` (`$17838` .. `$1783F`), including the full C Skill System.
- A patch that hooks the non-player branch of `UnitKill` (`$18428`) is bypassed for non-player units.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or change `FE8U_StandaloneFreeSpace` in `Installer.event` to an offset after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both hook sites (8 bytes each) and the free-space body. If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

### With other standalone patches

| Patch | Together? |
|-------|-----------|
| [casual_mode](../casual_mode/) | Yes. Player units are passed on to vanilla code and reach Casual Mode's hook. Tested together. |
| [send_inventory_on_death](../send_inventory_on_death/) | Yes, it only handles player units. Tested together. |
| [death_dance](../death_dance/), [rescue_drop_move_again](../rescue_drop_move_again/), [promote_enemy_on_kill](../promote_enemy_on_kill/) | Yes, different functions. A dead enemy's rescued unit is dropped before the enemy is marked dead, as in vanilla. Tested with `death_dance` and `promote_enemy_on_kill`. |

To install several, give each its own free space (for example `$1000000`, `$1000200`, `$1000400`, ...).

## Differences from the C Skill System

- **Hook point.** The integrated code replaces the whole `UnitKill`. This patch hooks its start and hands player units back to vanilla code, so it combines with Casual Mode. (Hooking the non-player branch at `$18428` doesn't work: its last 4 bytes are the epilogue that the player branches jump to.)
- **Full unit array.** The integrated code has no fallback when all 50 enemy (or 20 NPC) slots are taken; a load would then fail. This patch reuses a dead unit's slot.
- **Dead player unit list.** The kernel also records defeated player units in its own `gDeadUnits` array (saved with the game and cleared each chapter) for its Necromancy skill. That needs the kernel's save extension and skill, and is not ported.

## Limitations

- Vanilla looks units up by character ID (`GetUnitFromCharId`, used by many event commands) without skipping dead ones, so an event aimed at a generic enemy character can now pick a dead copy that sits in an earlier slot. Boss and named enemies are unaffected unless the same character is loaded twice.
- A defeated enemy keeps its items; they are not dropped unless vanilla drops them (the item drop flag).
- Tested on clean FE8U in mGBA: both Prologue soldiers, killed on Eirika's counterattack, stayed in their slots with `US_DEAD | US_HIDDEN`; the "All that's left is their leader" event still fired; player deaths were unchanged; no other unit data changed. The slot-reuse path of `GetFreeUnit` was not reached in testing (the Prologue never fills the array); it is checked by reading only.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
