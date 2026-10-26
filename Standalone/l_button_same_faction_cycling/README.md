# L Button Same Faction Cycling

On the player-phase map, **L** jumps the cursor to the next unit of the **same faction as the unit under the cursor**:

| Cursor is on | L cycles through |
|--------------|------------------|
| An enemy (red, `0x80`..`0xBF`) | Enemies |
| An NPC (green, `0x40`..`0x7F`) | NPCs |
| A player unit or an empty tile | Player units (vanilla behaviour) |

Vanilla always jumps back to the player army. This patch makes it easy to look over every enemy on the map one by one. The units the cycle skips are the same ones vanilla skips (`TrySetCursorOn`: dead, hidden, greyed-out, asleep or berserk units).

This matches the integrated C Skill System behaviour for the `l_button_same_faction_cycling` flag, but installs on **clean FE8U** without the kernel.

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: vanilla `TrySwitchViewedUnit` at `0x0801DB4C` (Thumb entry `0x0801DB4D`)
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/TrySwitchViewedUnit_SameFaction.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/TrySwitchViewedUnit_SameFaction.c`:

```bash
make -C Standalone/l_button_same_faction_cycling
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `l_button_same_faction_cycling` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-l-button.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, hook, and free-space placement |
| `Source/TrySwitchViewedUnit_SameFaction.c` | C implementation |
| `Source/TrySwitchViewedUnit_SameFaction.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `TrySwitchViewedUnit` | `$1DB4C` | 8 bytes (`jumpToHack`) |

**Free space:** `$1000000`, 188 bytes. No RAM.

## Conflicts

- Any patch that replaces vanilla `TrySwitchViewedUnit` (`0x0801DB4C`), including the full C Skill System.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both the hook site (`$1DB4C`, 8 bytes) and the free-space body. If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Limitations

- Like the integrated version, the cycle does not check fog of war: pressing L on a visible enemy can move the cursor (and camera) onto an enemy that is hidden in fog.
- Only the L button (`PlayerPhase_MainIdle`) uses `TrySwitchViewedUnit`; other unit-cycling screens (stat screen L/R, prep screens) are unchanged.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
