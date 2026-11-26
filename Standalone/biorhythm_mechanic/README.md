# Biorhythm

Playable characters cycle through five hit and avoid modifiers, one step per chapter turn:

| Turn | Hit | Avoid |
|------|-----|-------|
| 1 | -15 | -15 |
| 2 | -5 | -5 |
| 3 | 0 | 0 |
| 4 | +5 | +5 |
| 5 | +15 | +15 |
| 6 | -15 | -15 |

The same modifier is added to both hit and avoid. Avoid is clamped to 0 after the modifier, using the same order as the integrated battle calc: vanilla avoid is clamped first, then the biorhythm value is applied, then avoid is clamped again.

Characters with an all-zero row get no modifier. The shipped table covers the recruitable cast and creature-campaign bosses. Generic enemies are zero and are unchanged.

`startOffset` shifts where turn 1 begins in the cycle. Integrated `designer-config.c` defaults this flag to `false`. This patch defaults to on.

## Configuration

Edit `BIORHYTHM_MECHANIC` at the top of `Installer.event`, then reinstall the patch. No C recompile is needed.

| Constant | Default | Meaning |
|----------|---------|---------|
| `BIORHYTHM_MECHANIC` | `1` | Set to `0` to keep vanilla hit and avoid |

To change a character's cycle or starting step, edit `gBiorhythmPInfoConfigList` in `Source/Biorhythm.c` and rebuild with `make`.

## Target ROM

- **FE8U (USA)** clean ROM
- Hooks (8 bytes each, Thumb entry is the address + 1):
  - `ComputeBattleUnitHitRate` at `0x0802ABAC`
  - `ComputeBattleUnitAvoidRate` at `0x0802ABE4`
- Free space: about `0x18D0` bytes, including the 256-entry character table, wherever Insert EA (or your `ORG`) puts it; see [Free space](../README.md#free-space)
## Build

No build step is required to install the patch. `Source/Biorhythm.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/Biorhythm.c`:

```bash
make -C Standalone/biorhythm_mechanic
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `biorhythm_mechanic` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
# ORG: where the code goes. $1000000 is free on a clean ROM; in an edited ROM, use free space.
printf 'ORG $1000000\n#include "Installer.event"\n' > _install.event
ColorzCore A FE8 -input:_install.event -output:/path/to/fe8-biorhythm.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hooks, and free-space placement |
| `Source/Biorhythm.c` | C implementation and per-character cycle table |
| `Source/Biorhythm.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that replaces vanilla `ComputeBattleUnitHitRate` (`0x0802ABAC`) or `ComputeBattleUnitAvoidRate` (`0x0802ABE4`), including the full C Skill System pre-battle calc rewrite.
- Each Insert EA puts the body in its own free space, so it doesn't overlap other standalone patches or data FEBuilder has added (see [Free space](../README.md#free-space)).

`Installer.event` uses `PROTECT` on each hook site (8 bytes) and on the free-space body (from its start through the end of install). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
