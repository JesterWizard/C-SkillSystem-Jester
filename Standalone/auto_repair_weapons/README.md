# Auto Repair Weapons

Standalone FE8U patch extracted from the C Skill System designer config flag `auto_repair_weapons`.

## Behavior

At the end of each map, when the game runs chapter-transition unit cleanup, unbroken weapons in each player unit's inventory are restored to full durability.

Broken weapons are left unchanged. This matches the integrated C Skill System mechanic described in the in-game guide.

## Configuration

Edit `AUTO_REPAIR_WEAPONS` at the top of `Installer.event`, then reinstall the patch. No C recompile is needed.

| Constant | Default | Meaning |
|----------|---------|---------|
| `AUTO_REPAIR_WEAPONS` | `1` | Set to `0` to disable the feature |

Integrated `designer-config.c` defaults this to `false`; set the constant to `1` when installing this standalone patch.

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: vanilla `ChapterChangeUnitCleanup` at `0x08031215` (Thumb entry `0x08031215`)
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/AutoRepairWeapons.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/AutoRepairWeapons.c`:

```bash
make -C Standalone/auto_repair_weapons
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

### FEBuilderGBA

1. Download [FEBuilderGBA (Laqieer branch)](https://nightly.link/laqieer/FEBuilderGBA/workflows/msbuild/master).
2. In **Settings → Options → Path**, set **Event Assembler** to this repo's [ColorzCore](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) executable (see [Setup.md](../../Documentation/Setup.md)).
3. Open your project ROM in FEBuilder.
4. Go to **Advanced Editors → Insert EA**.
5. Click **Select File**, choose `Standalone/auto_repair_weapons/Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`:

```bash
cp /path/to/fe8.gba /path/to/fe8-auto-repair-weapons.gba
Tools/EventAssembler/ColorzCore A FE8 \
  -input:Standalone/auto_repair_weapons/Installer.event \
  -output:/path/to/fe8-auto-repair-weapons.gba
```

Run from the repository root so Event Assembler can resolve `EAstdlib.event`.

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hook, and free-space placement |
| `Source/AutoRepairWeapons.c` | C implementation |
| `Source/AutoRepairWeapons.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that replaces vanilla `ChapterChangeUnitCleanup` (`0x08031215`), including the full C Skill System `UnitRefrain.c` rewrite.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both the hook site (`$31215`, 8 bytes) and the free-space body (`$1000000` through end of install). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
