# Remove Move Path

Removes the blue movement path arrow while a player unit is choosing where to move. When the player confirms a destination, the unit walks the best (shortest) route to the tile under the cursor instead of the route the player traced with the D-pad.

This matches the integrated C Skill System `remove_move_path` config flag, but installs on **clean FE8U** without the kernel.

## Target ROM

- **FE8U (USA)** clean ROM
- Hooks:
  - `PlayerPhase_DisplayUnitMovement` at `0x0801D624` (installer ORG `$1D624`): `jumpToHack` to a version that calls `GenerateBestMovementScript(cursor x, cursor y, gWorkingMovementScript)` instead of `GetMovementScriptFromPath()`
  - `DrawUpdatedPathArrow` at `0x08033248` (installer ORG `$33248`): first instruction replaced with `bx lr`, so the arrow is never updated or drawn
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/RemoveMovePath.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/RemoveMovePath.c`:

```bash
make -C Standalone/remove_move_path
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `remove_move_path` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-remove-move-path.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, hooks, and free-space placement |
| `Source/RemoveMovePath.c` | Replacement for `PlayerPhase_DisplayUnitMovement` |
| `Source/RemoveMovePath.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `PlayerPhase_DisplayUnitMovement` | `$1D624` | 8 bytes (`jumpToHack`) |
| `DrawUpdatedPathArrow` | `$33248` | 2 bytes (`bx lr`); 8 bytes `PROTECT`ed |

**Free space:** `$1000000` (84 bytes; body continues at `CURRENTOFFSET` after install).

No RAM is used.

## Conflicts

- **[`alpha_blend_movement_sprites`](../alpha_blend_movement_sprites/)** also hooks `DrawUpdatedPathArrow` (`$33248`). The two packages are **alternatives**: install one or the other. Whichever is installed last owns `$33248`:
  - `remove_move_path` last: no arrow and no ghost sprite.
  - `alpha_blend_movement_sprites` last: the arrow and ghost come back, but units still ignore the traced arrow and walk the shortest route.
  In the integrated C Skill System, `remove_move_path` simply turns the ghost off together with the arrow.
- Any other patch that hooks or replaces `PlayerPhase_DisplayUnitMovement` (`$1D624`) or `DrawUpdatedPathArrow` (`$33248`), including the full C Skill System `RemoveMovePath` module.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both hook sites and the free-space body. Overlapping ROM writes should fail assembly with a clear EA error.

## Limitations

- When several shortest routes exist, the game's own pathfinder (`GenerateBestMovementScript`, the one AI units use) picks the route; the player cannot choose a detour.
- Only player-phase move selection is affected (`DrawUpdatedPathArrow` is only called from `PlayerPhase_RangeDisplayIdle`, and `PlayerPhase_DisplayUnitMovement` is the only caller of `GetMovementScriptFromPath`). AI and event movement are unchanged.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester) (`Kernel/Wizardry/RemoveMovePath`).
