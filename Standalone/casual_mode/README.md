# Casual Mode

A player unit that is defeated is not lost: it leaves the map for the rest of the chapter and is back, at full HP, from the next chapter on. Its supports are kept.

- Applies to every way a player unit dies (battle, arena, poison/traps, event kills), because it is done in vanilla `UnitKill`.
- Summoned phantoms still vanish as in vanilla.
- Lords still end the game: vanilla FE8 triggers Game Over from the lord's death quote event, which this patch doesn't touch. If every player unit is lost, vanilla's "no units left" Game Over still happens.
- Casual mode can be always on (default), or on only while an event flag is set, so a hack can let the player choose (see [Setting](#setting)).

This matches the integrated C Skill System `casual_mode` designer-config flag, but installs on **clean FE8U** without the kernel. See [Differences from the C Skill System](#differences-from-the-c-skill-system).

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: the player-unit branch of vanilla `UnitKill` at `$18418`
- Free space: `$1000000` (98 bytes, `$1000000` .. `$1000062`)

## Setting

`Installer.event` has one setting, read at run time:

```
#define CASUAL_MODE_FLAG 0
```

| Value | Behaviour |
|-------|-----------|
| `0` | Casual mode is always on (the C Skill System default when the flag is on). |
| any event flag ID | Casual mode is on only while that flag is set. |

To let the player pick, use a **permanent** event flag (`0x64` / 100 or higher; FE8 keeps those in the save file), for example `0xB0` if your hack doesn't use it, and set it from an event (for example after a Yes/No choice in the Prologue's opening event) with `ENUT 0xB0`. Clear it with `ENUF 0xB0` to go back to classic mode. Nothing else is stored, so the save layout is unchanged.

## Build

No build step is required to install the patch. `Source/CasualMode.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/CasualMode.c` or `Source/CasualModeHook.s`:

```bash
make -C Standalone/casual_mode
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder's bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `casual_mode` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-casual-mode.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: hook, setting and free-space placement |
| `Source/CasualMode.c` | C implementation (`CasualMode_KillPlayerUnit`) |
| `Source/CasualModeHook.s` | Hook stub placed at `$18418` |
| `Source/CasualMode.lyn.event` | Checked-in lyn output (rebuild with `make` after editing the sources) |
| `makefile` | Compiles the sources to `.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `UnitKill` (`$183FC`), player unit that isn't a phantom | `$18418` | 8 bytes (`$18418` .. `$1841F`, `jumpToHack`) |

Vanilla `UnitKill` does `unit->state |= US_DEAD | US_HIDDEN; InitUnitsupports(unit);` for a dying player unit. The hook calls `CasualMode_KillPlayerUnit`, which does exactly that when casual mode is off, and otherwise only sets `US_HIDDEN | US_NOT_DEPLOYED`, then returns through the vanilla epilogue at `$1842C`. Vanilla `ChapterChangeUnitCleanup` keeps only `US_DEAD` from those bits, so the unit is available again from the next chapter.

**Free space:** `$1000000`. **RAM:** none.

## Conflicts

- Any patch that changes vanilla `UnitKill` (`$183FC` .. `$18430`) in the player-unit branch, in particular circleseverywhere's older Casual Mode hack (it hooks the same `$18418`) and the full C Skill System.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or change `FE8U_StandaloneFreeSpace` in `Installer.event` to an offset after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both the hook site (8 bytes) and the free-space body. If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

### With other standalone patches

| Patch | Together? |
|-------|-----------|
| [collect_dead_units](../collect_dead_units/) | Yes. It hooks the start of `UnitKill` and hands player units back to vanilla code, which then reaches this hook. Tested together. |
| [send_inventory_on_death](../send_inventory_on_death/) | Yes. A defeated unit's items go to the convoy and are removed from the unit, so a unit that comes back doesn't duplicate them; items that didn't fit stay with it. Tested together. |
| [death_dance](../death_dance/), [rescue_drop_move_again](../rescue_drop_move_again/), [promote_enemy_on_kill](../promote_enemy_on_kill/) | Yes, different functions. A rescued unit is dropped before its rescuer is removed, as in vanilla. |

To install several, give each its own free space (for example `$1000000`, `$1000200`, `$1000400`, ...).

## Differences from the C Skill System

- **Unit state.** The integrated code sets only `US_HIDDEN`. This patch also sets `US_NOT_DEPLOYED` (as circleseverywhere's older Casual Mode hack does): vanilla treats a hidden-but-deployed unit as present in several places (`GetPhaseAbleUnitCount`, so the player phase no longer ends by itself; `CountAvailableBlueUnits`, so losing every unit no longer ends the game; terrain healing and status recovery lists). A side effect is that vanilla remembers the casualty as "not deployed last chapter" (unit state bit 21), which only matters to events that restore the previous deployment.
- **Choosing the mode.** The integrated behaviour is controlled only by the designer-config constant. The kernel also has a Casual Mode entry in its custom Options menu (stored in its extended `gPlaySt.config` bits), but integrated `UnitKill` never reads it. That menu and the extended config bits are part of the kernel and are not ported; the event-flag setting replaces them.
- **Difficulty menu.** The integrated casual mode has no link with `fe7_mode_select` or the difficulty menu, and neither does this patch.

## Limitations

- During the chapter the casualty is still in the unit array (hidden, not deployed) at the tile where it fell. Events that address it by character still find it, and vanilla's "any player unit in this area" check (`CheckAnyBlueUnitArea`, which only skips `US_DEAD`) still counts it there. Its death quote and any event flags it sets are unchanged.
- Tested on clean FE8U in mGBA: Seth defeated in the Prologue left the map (state `US_HIDDEN | US_NOT_DEPLOYED`, supports kept), the chapter continued, and in Chapter 1 he was alive with full HP. The flag-controlled setting was assembled and byte-checked but not runtime-tested.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester). The `US_NOT_DEPLOYED` approach follows circleseverywhere's Casual Mode hack.
