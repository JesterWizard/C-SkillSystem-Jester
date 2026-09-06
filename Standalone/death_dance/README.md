# Death Dance

Porting a mechanic from FE6: when a rescuer dies on their own phase, the rescued unit can still move after being dropped.

Vanilla FE8U always marks a dropped ally as unselectable (`US_UNSELECTABLE`), so they cannot act again that turn even if the drop was caused by the rescuer's death. This patch only applies that gray-out when the rescuer is still alive (`curHP != 0`).

Manual drop while the rescuer is alive behaves like vanilla.

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: vanilla `UnitDrop` at `0x08018371` (Thumb entry `0x08018371`)
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/UnitDrop_DeathDance.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/UnitDrop_DeathDance.c`:

```bash
make -C Standalone/death_dance
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

### FEBuilderGBA

1. Download [FEBuilderGBA (Laqieer branch)](https://nightly.link/laqieer/FEBuilderGBA/workflows/msbuild/master).
2. In **Settings → Options → Path**, set **Event Assembler** to this repo's [ColorzCore](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) executable (see [Setup.md](../../Documentation/Setup.md)).
3. Open your project ROM in FEBuilder.
4. Go to **Advanced Editors → Insert EA**.
5. Click **Select File**, choose `Standalone/death_dance/Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`:

```bash
cp /path/to/fe8.gba /path/to/fe8-death-dance.gba
Tools/EventAssembler/ColorzCore A FE8 \
  -input:Standalone/death_dance/Installer.event \
  -output:/path/to/fe8-death-dance.gba
```

Run from the repository root so Event Assembler can resolve `EAstdlib.event`.

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hook, and free-space placement |
| `Source/UnitDrop_DeathDance.c` | C implementation |
| `Source/UnitDrop_DeathDance.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that replaces vanilla `UnitDrop` (`0x08018371`), including the full C Skill System `MiscFunctions` rewrite.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both the hook site (`$18371`, 8 bytes) and the free-space body (`$1000000` through end of install). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
