# Rescue Drop Move Again

A unit that is **dropped** (Drop, or released when its rescuer dies) is no longer greyed out automatically. If the dropped unit had not acted yet this turn, it can still move and act after landing.

- A unit that already acted (Wait, attack, etc.) before it was rescued keeps its own greyed-out state, so it cannot act twice.
- A unit that is in the middle of a Canto move (`US_HAS_MOVED`) is still greyed out, as in vanilla.
- Enemies and NPCs follow the same rule during their own phase.

Vanilla `UnitDrop` greys out every dropped unit that belongs to the active faction.

This matches the integrated C Skill System behaviour for the `rescue_drop_move_again` flag, but installs on **clean FE8U** without the kernel.

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: vanilla `UnitDrop` at `0x08018370` (Thumb entry `0x08018371`)
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/UnitDrop_MoveAgain.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/UnitDrop_MoveAgain.c`:

```bash
make -C Standalone/rescue_drop_move_again
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `rescue_drop_move_again` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-rescue-drop.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, hook, and free-space placement |
| `Source/UnitDrop_MoveAgain.c` | C implementation (vanilla `UnitDrop` plus the Canto check) |
| `Source/UnitDrop_MoveAgain.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `UnitDrop` | `$18370` | 8 bytes (`jumpToHack`) |

**Free space:** `$1000000`, 96 bytes. No RAM.

## Conflicts

- **`Standalone/death_dance`** also replaces `UnitDrop` at `$18370`; the two packages cannot be installed together (EA reports the `PROTECT` overlap). The integrated kernel combines both in one `UnitDrop`. To get both, merge the rules by hand in `Source/UnitDrop_MoveAgain.c` (grey out only when `US_HAS_MOVED` is set **and** the rescuer is alive, `actor->curHP != 0`), rebuild, and install only this package.
- Any other patch that replaces vanilla `UnitDrop` (`0x08018370`), including the full C Skill System.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both the hook site (`$18370`, 8 bytes) and the free-space body. If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Limitations

- The integrated `UnitDrop` also kills a dropped unit that has 0 HP (part of the kernel's capture feature) and has the Death Dance rule. Both are left out; this package only carries the move-again rule.
- `UnitGive` also calls `UnitDrop` (then immediately re-rescues the unit), so Give is unaffected.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
