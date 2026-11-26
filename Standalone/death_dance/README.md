# Death Dance

Porting a mechanic from FE6: when a rescuer dies on their own phase, the rescued unit can still move after being dropped.

Vanilla FE8U always marks a dropped ally as unselectable (`US_UNSELECTABLE`), so they cannot act again that turn even if the drop was caused by the rescuer's death. This patch only applies that gray-out when the rescuer is still alive (`curHP != 0`).

Manual drop while the rescuer is alive behaves like vanilla.

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: vanilla `UnitDrop` at `0x08018371` (installer ORG `$18370`)
- Free space: wherever Insert EA (or your `ORG`) puts it; see [Free space](../README.md#free-space)
## Build

No build step is required to install the patch. `Source/UnitDrop_DeathDance.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/UnitDrop_DeathDance.c`:

```bash
make -C Standalone/death_dance
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `death_dance` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
# ORG: where the code goes. $1000000 is free on a clean ROM; in an edited ROM, use free space.
printf 'ORG $1000000\n#include "Installer.event"\n' > _install.event
ColorzCore A FE8 -input:_install.event -output:/path/to/fe8-death-dance.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hook, and free-space placement |
| `Source/UnitDrop_DeathDance.c` | C implementation |
| `Source/UnitDrop_DeathDance.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that replaces vanilla `UnitDrop` (`$18370` / `0x08018371`), including the full C Skill System `MiscFunctions` rewrite.
- Each Insert EA puts the body in its own free space, so it doesn't overlap other standalone patches or data FEBuilder has added (see [Free space](../README.md#free-space)).

`Installer.event` uses `PROTECT` on both the hook site (`$18371`, 8 bytes) and the free-space body (from its start through the end of install). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
