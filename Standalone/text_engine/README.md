# Text Engine

<p align="center">
  <img src="./Feature - Text Engine.gif" alt="Text Engine" width="600"/>
</p>

<p align="center">
  <img src="./Feature - Text Engine Effects.gif" alt="Text engine features: letter effects, nameplates, rainbow text, portrait motion, emotion marks, screen effects, big text, popups, scene layout and ash dissolve" width="600"/>
</p>

The C Skill System's dialogue text engine for clean FE8U, a C port of Tequila's text engine hack (with help from Zahlman). Vanilla text keeps working unchanged. It adds:

- **Text style:** fonts, colours, box shapes and palettes, box height, print speed, boop pitch.
- **Speaker nameplates** and a cutscene **scene layout** (one full-width box with the speaker's name on a tab).
- **Letter effects:** shake, bounce, wavy, scramble, drip and ghost printing, **big text** and **rainbow text**.
- **Screen effects:** background wave, screen tear, static, earthquake and impact flash.
- **Portraits:** flipped / eyes-closed loads, variable-speed moves, remapped positions, jumping, vibrating, shimmying, an ash dissolve, and sweat and anger marks.
- **Popups:** "Objection!"-style cut-ins.
- **Auto-advance:** boxes that move on by themselves.
- **Listener dimming:** portraits of characters who aren't speaking dim slightly (on by default).
- **Conversation log** (optional): SELECT during dialogue opens a scrollback log of the chapter's dialogue.
- **Half-body and 32-colour portraits** (optional): the formats from [`portrait_formats`](../portrait_formats/), shown in dialogue and menus.

## Installation

### FEBuilderGBA

1. Open a clean FE8U ROM in FEBuilder.
2. Optionally, change the [settings](#settings) at the top of `Installer.event` in a text editor.
3. Go to **Advanced Editors → Insert EA**, click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

```bash
EA_DIR=<repo>/Tools/EventAssembler
{
  echo '#include "Extensions/Hack Installation.txt"'
  echo '#include "Installer.event"'
} > _install.event
"$EA_DIR/ColorzCore" A FE8 -input:_install.event -I:"$EA_DIR" -output:/path/to/fe8.gba
```

The code and data go at `$1000000` and take about 115 KB, or about 126 KB with both options on.

## Settings

All settings are at the top of `Installer.event`. You don't need to compile anything to change them.

| Setting | Default | Meaning |
|---------|---------|---------|
| `TEXT_ENGINE_CHATLOG` | off | Remove the `//` in front of it to install the conversation log. |
| `TEXT_ENGINE_HALF_BODY_PORTRAITS` | off | Remove the `//` in front of it to install half-body and 32-colour portraits. |
| `TEXT_ENGINE_RAM` | `0x0203AB00` | Start of the 0x1A0 bytes of EWRAM the text engine uses. |
| `TEXT_ENGINE_CHATLOG_RAM` | `0x0203B000` | Start of the 0x600 bytes the conversation log uses. |
| `TEXT_ENGINE_PORTRAIT_RAM` | `0x0203B800` | Start of the 0xA0 bytes the portraits use. |
| `gTextEngineConfig` byte 0 | `1` | Listener dimming. `0` turns it off. |

The default RAM addresses are in FreeRamSpace2 (`0x0203AAA4` .. `0x0203DDE0`) and don't overlap any other `Standalone/` patch that can be installed with this one. If another patch in your ROM already uses one of these ranges, change the address to free EWRAM. Each address must be a multiple of 4. If it isn't, or the block doesn't fit in EWRAM, Event Assembler stops with an `Assertion error`.

If you include `Installer.event` from your own event file, you can also `#define` any of these settings before the `#include`.

**Conversation log:** SELECT opens it, UP/DOWN scroll and SELECT closes it. It keeps the last 17 lines and clears at each chapter start. It isn't saved. Don't also install [`Standalone/chatlog`](../chatlog/).

**Half-body and 32-colour portraits:** make and install the portraits with [`portrait_formats`](../portrait_formats/)' tools and `Tools/PortraitMacros.event`. Portraits in other formats stay as they are. Don't also install `Standalone/portrait_formats`.

## Using the codes

Every new code is the byte `0x80`, then the code, then its arguments. For example, big text is `0x80 0x5B` ... `0x80 0x5C`, and auto-advance after 60 frames is `0x80 0x5E 0x3C`. **Arguments can't be `0x00`**, because `0x00` ends the message.

[`textdefs.txt`](textdefs.txt) has an alias for every code for ParseFile and similar text tools, for example `[ToggleOnBigText]HALT![ToggleOffBigText]`.

| Code | Alias | Arguments | Summary |
|------|-------|-----------|---------|
| `0x26` | `[Font]` | `XX` font (`0x01`–`0xFF`) | Switches font (1 normal, 2 bold, 3 italics by default) |
| `0x27` | `[TextPalette]` | `XX` colour group, `YY` palette | Changes one 3-colour text group |
| `0x28` | `[ColorGroup]` | `XX` colour group (`0x01`–`0x05`) | Selects the colour group for new text |
| `0x29` | `[BoxBgPalette]` | `XX` palette | Swaps the text box background palette |
| `0x2A` | `[BoxType]` | `XX` type; bit `0x80` controls tails | Changes the box graphics before the next bubble opens |
| `0x2B` | `[BoxHeight]` | `XX` lines (`0x01`–`0x03`) | Sets the box height |
| `0x2C` | `[BoopPitch_*]` | `XX` pitch (`0x01`–`0x19`, default `0x0D`) | Sets the speaker's letter boop pitch, one octave down to one octave up |
| `0x2D` | `[PlaySound]` | four nybble bytes | Plays sound `0xABCD`, little-endian, each nybble OR'd with `0x80` |
| `0x2E` | `[MugLoc]` | `XX` slot (`0x01`–`0x08`), `YY` signed X tiles | Moves a face slot's X position (`YY = 0x80` is tile 0) |
| `0x2F` | `[LoadFaceFancy]` | options, attributes, portrait | Loads a face flipped and/or with eyes closed |
| `0x30`–`0x37` | `[MFL2]` … `[MFFR2]` | `YY` frames | Moves a portrait to that slot over `YY` frames |
| `0x38` | `[TextSpeed]` | `XX` frames, or `0xFF` | Sets the print delay; `0xFF` restores the Options speed |
| `0x39` / `0x3A` | `[ToggleOnJumping]` / `[ToggleOffJumping]` | none | Active portrait jumps |
| `0x3B` / `0x3C` | `[ToggleOnShakePrint]` / `[ToggleOffShakePrint]` | none | Each letter jitters the text as it prints |
| `0x3D` / `0x3E` | `[ToggleOnBouncePrint]` / `[ToggleOffBouncePrint]` | none | Each letter floats down into place |
| `0x3F` / `0x40` | `[ToggleOnWave]` / `[ToggleOffWave]` | none | Horizontal wave on the backgrounds |
| `0x41` / `0x42` | `[ToggleOnVibrating]` / `[ToggleOffVibrating]` | none | Active portrait vibrates up and down |
| `0x43` / `0x44` | `[ToggleOnShimmy]` / `[ToggleOffShimmy]` | none | Active portrait shimmies side to side |
| `0x45` / `0x46` | `[ToggleOnNameplate]` / `[ToggleOffNameplate]` | none | Speaker nameplates (off by default) |
| `0x47` / `0x48` | `[ToggleOnGlitch]` / `[ToggleOffGlitch]` | none | Screen tear |
| `0x49` / `0x4A` | `[ToggleOnStatic]` / `[ToggleOffStatic]` | none | Scanline static |
| `0x4B` | `[ClearFaceAsh]` / `[AshDissolve]` | none | Clears the active portrait by dissolving it into ash |
| `0x4C` / `0x4D` | `[ToggleOnWavyPrint]` / `[ToggleOffWavyPrint]` | none | Each letter wobbles into place |
| `0x4E` / `0x4F` | `[ToggleOnScramblePrint]` / `[ToggleOffScramblePrint]` | none | Each letter cycles random letters before it settles |
| `0x50` / `0x51` | `[ToggleOnDripPrint]` / `[ToggleOffDripPrint]` | none | Each letter hangs, then drops into place |
| `0x52` / `0x53` | `[ToggleOnGhostPrint]` / `[ToggleOffGhostPrint]` | none | Each letter appears with a trailing echo |
| `0x54` / `0x55` | `[ToggleOnEarthquake]` / `[ToggleOffEarthquake]` | none | Screen rumble |
| `0x56` | `[ImpactFlash]` | none | One white screen flash |
| `0x57` | `[Sweat]` | none | Sweat drop on the active portrait; the next text waits for it |
| `0x58` | `[Anger]` | none | Anger mark on the active portrait; the next text waits for it |
| `0x59` / `0x5A` | `[ToggleOnSceneLayout]` / `[ToggleOffSceneLayout]` | none | Full-width box along the bottom with a speaker name tab |
| `0x5B` / `0x5C` | `[ToggleOnBigText]` / `[ToggleOffBigText]` | none | Double-size letters over two lines |
| `0x5D` | `[Popup]` / `[Objection]` / `[HoldIt]` / `[TakeThat]` | `XX` popup (`0x01`+) | Shows popup `XX` from `TextEnginePopupTable` with a flash, sound and shake |
| `0x5E` | `[AutoAdvance]` | `XX` frames | Used instead of `[A]`: the box moves on by itself after `XX` frames |
| `0x5F` / `0x60` | `[ToggleOnAutoAdvance]` / `[ToggleOffAutoAdvance]` | `XX` frames / none | Every `[A]` moves on by itself after `XX` frames |
| `0x61` / `0x62` | `[ToggleOnRainbow]` / `[ToggleOffRainbow]` | none | Letters cycle through the colour wheel |

The screen effects can also be started from events with `ASMC` (`EnableScreenGlitch`, `DisableScreenGlitch`, `EnableScreenStatic`, `DisableScreenStatic`, `EnableScreenEarthquake`, `DisableScreenEarthquake`, `StartScreenImpactFlash`) when your events are assembled together with this installer.

[Text Engine Rework](../../Documentation/Features/TextEngineRework.md) describes each feature in more detail.

## Fonts, colours, boxes and popups

These are tables in [`Data/`](Data/):

- `Data/Tables.event`: fonts (`FontGlyphsPointerTable`), text and box palettes, box shapes and boop pitches.
- `Data/Popups.event`: `TextEnginePopupTable`. Each popup needs `*.img.bin`, `*.map.bin` and `*.pal.bin` from grit (`grit Name.png -gB 4 -gzl -m -mLf -mR4 -mzl -pn 16 -ftb -fh! -o Name.img.bin`).
- `Data/BoxGfx/*.png`: box shapes, converted to `.dmp` by `make`.

## Build

You only need to build if you change `Source/` or the PNGs in `Data/BoxGfx`. The `.lyn.event` and `.dmp` outputs are checked in. RAM addresses are settings, so they don't need a rebuild.

```bash
make -C Standalone/text_engine
```

This needs [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's tools (see [Documentation/Setup.md](../../Documentation/Setup.md)). `Source/Portraits.lyn.event` is built from [`../portrait_formats/Source/Portraits.c`](../portrait_formats/Source/Portraits.c), so that folder must be next to this one.

## Conflicts

- Other dialogue text engines (Tequila's original, Zeta's AutoNewLine) and any patch that hooks the same dialogue or face functions. The hooks are listed in `Installer.event`, and each one is `PROTECT`ed, so an overlapping patch fails with an EA error.
- Other standalone patches that also use `$1000000`. Install only one there, or move this patch's `ORG`.
- The full C Skill System, which already has this engine.

## Credits

Text engine by Tequila, with help from Zahlman. C port and extensions from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
