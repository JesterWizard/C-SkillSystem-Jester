# Critical Cut-in

<p align="center">
  <img src="./Feature - Critical Cut-in.gif" alt="Critical Cut-in" width="600"/>
</p>

In the style of Lex Talionis' FE7 remake: before a critical hit by a unit with its own portrait, an electric band opens across the middle of the battle screen with the attacker's portrait held still in it. The battle sprites are hidden under the band while it plays (about one second), then the attack carries on.

- Only **battle animations** show it (map animations are unchanged).
- Only units with a **character portrait** (`portraitId` in their character data) get a cut-in. Generic units, which only have a class card, attack as usual.
- Only in close and far range battles: not in the far-far (long range) mini animation, and not in promotion or other scripted animations.
- Vanilla **Sure Shot / Great Shield / Pierce** activation backgrounds still play, right after the cut-in.

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: entry 7 (banim command C07, attack start) of the case table that vanilla `BattleAIS_ExecCommands` jumps through, at `0x08058C60` (table base `0x08058C44`)
- Free space: `$1000000`

## How it works

Vanilla battle animation scripts run command C07 when the attacker starts its attack. `BattleAIS_ExecCommands` dispatches each queued command with `mov pc` through a table of case addresses; the installer repoints entry 7 to `CritCutin_C07Thunk` (`Source/CritCutinThunk.s`). The thunk calls `CritCutin_OnC07` (`Source/CritCutin.c`) and then either resumes the dispatcher at `0x080596CC` (the command is handled for now) or falls through to the vanilla C07 case at `0x08059048`.

When the attacker's round is a critical (`ANIM_ROUND_CRIT_CLOSE` / `ANIM_ROUND_CRIT_FAR`) and the attacking unit has a portrait, `CritCutin_OnC07` blocks the script on C07 (it rewinds the script pointer, the same way the C Skill System's `BanimHack` does) and starts a proc that:

1. freezes the attacker's anims and hides all four battle anims,
2. plays the band as a spell BG animation on BG1 (29 frames of 2 ticks: open, four lightning loops, close),
3. shows the portrait in a free face slot (96x80, flipped for the left side) while the band is fully open,
4. restores the face palette, BG1 and the anims, and releases the block.

On the next C07 run the block is cleared and the vanilla C07 case runs as normal.

## Settings

In `Installer.event`:

| Define | Default | Meaning |
|--------|---------|---------|
| `CritCutinSfx` | `0x3D1` | Sound played when the band opens (vanilla skill activation sound); `0` for none |

The band art is in `Gfx/`. `CritCutin_*.png` are the source frames (made by `generate_crit_cutin.py`) and the `*.img.bin` / `*.map.bin` / `*.pal.bin` files are their grit output (`grit X.png -gB 4 -gzl -m -mLf -mR4 -mzl -pn 16 -ftb -fh!`, LZ77 tiles and tilemap). `Gfx/CritCutin.event` lists the frames and their durations. A frame holds at most 256 tiles and 16 colours (colour 0 clear).

## Build

No build step is required to install the patch. `Source/CritCutin.lyn.event` and the graphics are checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/CritCutin.c` or `Source/CritCutinThunk.s`:

```bash
make -C Standalone/crit_cutin
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder's bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `crit_cutin` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-crit-cutin.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, settings, table repoint and free-space placement |
| `Source/CritCutin.c` | C07 handler and the cut-in proc |
| `Source/CritCutinThunk.s` | C07 case entry: calls the handler, then resumes the dispatcher or runs vanilla C07 |
| `Source/CritCutin.lyn.event` | Checked-in lyn output |
| `Gfx/CritCutin.event` | Band frame table (`CritCutinConf`) and graphics includes |
| `Gfx/CritCutin_*.png` | Band frame sources |
| `Gfx/CritCutin_*.{img,map,pal}.bin` | Band frame graphics (grit output) |
| `Gfx/generate_crit_cutin.py` | Generates the band frame PNGs |
| `makefile` | Compiles C + asm to `.lyn.event` |

## Hooks & Free Space

| Site | `ORG` | Overwritten |
|------|-------|-------------|
| `BattleAIS_ExecCommands` case table, entry 7 (C07) | `$58C60` | 4 bytes (pointer) |

**Free space:** `$1000000`, about 25 KB (code ~1.3 KB, band frames ~23 KB). No RAM is used.

## Conflicts

- Any patch that repoints banim command C07 (table entry at `$58C60`), including the C Skill System's `BanimHack` (`InjectBanimCmd(0x07, ...)`). Patches that only change other banim commands are fine.
- No other package in `Standalone/` touches the banim command table or C07. `text_engine` hooks face blinking (`$5FE0`, `$6134`) and works alongside this (the cut-in portrait is a normal face proc).
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.

`Installer.event` uses `PROTECT` on the table entry and the free-space body, so overlapping ROM writes fail assembly with a clear EA error.

## Limitations

- The integrated version also checks the C Skill System's combo attacks (`engage_combo_attack`) to pick the attacking unit; there are no combo attacks without the kernel, so the standalone always uses the attacker on the anim's side.
- The integrated C07 handler runs HP-cost and skill activation boxes before the cut-in; those are Skill System features and are not part of this package. Vanilla skill backgrounds (Sure Shot, Great Shield, Pierce) play after the cut-in instead.
- Critical detection uses the battle anim round type (crit close / crit far) rather than the kernel's expanded battle-hit array, so it also works with patches that change how hits are stored, as long as they keep vanilla round types.
- The integrated version only skips far-far battles; this one also skips the promotion and other non-battle animation distances (`gEkrDistanceType` other than close / far), where no attack crits.
- Needs a free face slot (4 in vanilla); battle quotes are not shown at C07, so one is always free in practice.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester) (`Kernel/Wizardry/CritCutin`, `Kernel/Wizardry/BanimHack`). Cut-in design after Lex Talionis' FE7 remake.
