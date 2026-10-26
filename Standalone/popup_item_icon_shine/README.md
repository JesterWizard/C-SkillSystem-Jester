# Popup Item Icon Shine

<p align="center">
  <img src="./Feature - Popup Item Icon Shine.gif" alt="Popup Item Icon Shine" width="480"/>
</p>

When a popup shows the icon of an item the player has just got, a white glint sweeps across the icon so the new item catches the eye. That covers an item got from a village, chest, event or drop, and an item a player thief steals.

- The first sweep starts 8 frames after the popup's icon appears, then one sweep plays every 64 frames (about once a second) for as long as the popup stays up.
- The glint is a 3-pixel "\" band that crosses the icon from left to right in 19 frames. Most weapon icons point "/", so the glint runs up the blade from hilt to tip.
- Only the icon's opaque pixels light up. They take the brightest colour in the icon's palette, which is white in the vanilla item icon palette.
- Popups about an item the player isn't gaining keep a plain icon:

| Popup | Why |
|-------|-----|
| "[Item] broke!" | Its message is `0x003` |
| "[Item] was pilfered." / "[Item] was stolen." | Its message is `0x009` / `0x00B` |
| "[Enemy] pilfered/stole [item]" | The popup names a unit that isn't a player unit |

Popups that show a weapon-type icon (weapon rank up) also stay plain.

## Tuning

The timing and size are the enum at the top of `Source/PopupIconShine.c` (rebuild with `make` after editing):

| Constant | Default | Meaning |
|----------|---------|---------|
| `SHINE_DELAY` | `8` | Frames from the icon appearing to the first sweep |
| `SHINE_PERIOD` | `64` | Frames from one sweep to the next. Must be a power of two |
| `SHINE_WIDTH` | `3` | Width of the glint, in pixels |
| `SHINE_SPEED` | `2` | Pixels the glint moves each frame |

To keep another popup plain, add its message ID to `sPopupIconShineLossMsgs`.

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: vanilla `PopupIconUpdateProc_Loop` at `0x08011254` (the loop of `ProcScr_PopupUpdateIcon`)
- Free space: `$1000000` (about 0.5 KB)

## Build

No build step is required to install the patch. `Source/PopupIconShine.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/PopupIconShine.c`:

```bash
make -C Standalone/popup_item_icon_shine
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder's bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `popup_item_icon_shine` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
EA_DIR=<repo>/Tools/EventAssembler
{
  echo '#include "Extensions/Hack Installation.txt"'
  echo '#include "Installer.event"'
} > _install.event
"$EA_DIR/ColorzCore" A FE8 -input:_install.event -I:"$EA_DIR" -output:/path/to/fe8-popup-icon-shine.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, hook, and free-space placement |
| `Source/PopupIconShine.c` | Icon proc loop with the glint |
| `Source/PopupIconShine.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |
| `Feature - Popup Item Icon Shine.gif` | Preview |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `PopupIconUpdateProc_Loop` | `$11254` | 8 bytes |

**Free space:** `$1000000` (body continues at `CURRENTOFFSET` after install).

**RAM:** none. The icon proc's frame counter lives in the proc's own unused padding (`+0x34`..`+0x39`). Procs don't clear their memory, so the counter restarts whenever the proc wasn't updated in the last 4 frames.

The glint is drawn from the same item icon sheet vanilla `LoadIconObjectGraphics` uses: the patch reads the sheet pointer from that function's literal pool (`0x08003788`, normally `item_icon_tiles` at `0x085926F4`), so a repointed icon sheet is followed.

## Conflicts

- Any patch that replaces vanilla `PopupIconUpdateProc_Loop` (`$11254`), including the full C Skill System PopupIconShine module.
- Icon patches that rewrite `LoadIconObjectGraphics` to load icons from somewhere other than its `item_icon_tiles` pointer (for example multi-sheet icon expansions) would make the glint draw the wrong icon's pixels for a moment. A plain repoint of the icon sheet is fine.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

`Installer.event` uses `PROTECT` on the hook site and the free-space body. Overlapping patches should fail assembly with a clear EA error.

## Limitations

- Battle animation popups (`NewEkrPopup`, such as weapon broke or weapon rank up during a battle) draw their icon a different way and don't shine.
- The glint colour comes from the icon's own palette. An item icon palette with no light colour gives a dimmer glint.
- The glint writes straight to the icon's OBJ VRAM tiles, so another sprite drawn from the same tiles at the same time would shine too.
- Telling gains from losses goes by the popup's messages and unit, so a new popup about losing an item needs its message added to `sPopupIconShineLossMsgs`.

## Differences from the C Skill System

- The C Skill System reads icons through its icon rework (`GetIconGfx`); this patch reads vanilla's item icon sheet.
- The C Skill System also keeps its tonic "used" popup (`MSG_USED`) plain; that popup doesn't exist in vanilla, so the check is dropped.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester). See [PopupItemIconShine.md](../../Documentation/Features/PopupItemIconShine.md) for integrated documentation.
