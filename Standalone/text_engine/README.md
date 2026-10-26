# Text Engine

<p align="center">
  <img src="./Feature - Text Engine.gif" alt="Text Engine" width="600"/>
</p>

The C Skill System's dialogue text engine for clean FE8U. It is a C port of Tequila's text engine hack (with help from Zahlman), and adds control codes for:

- **Text style:** fonts, colours, text box shapes and palettes, box height, print speed, boop pitch.
- **Speaker nameplates** and a cutscene **scene layout** (one full-width box with the speaker's name on a tab).
- **Letter effects:** shake, bounce, wavy, scramble, drip and ghost printing, Paper Mario-style **big text**, and **rainbow text**.
- **Screen effects:** a background wave, screen tear, static, earthquake and impact flash.
- **Portraits:** fancy face loads (flipped, eyes closed), variable-speed moves, remapped positions, jumping, vibrating and shimmying, an ash dissolve, and sweat and anger marks.
- **Popups:** "Objection!"-style cut-ins.
- **Auto-advance:** boxes that move on by themselves.
- **Listener dimming:** the portraits of characters who aren't speaking dim slightly (on by default).
- **Conversation log** (optional, off by default): SELECT during dialogue opens a scrollback log of the chapter's dialogue. See [Conversation log](#conversation-log).

Existing vanilla text keeps working unchanged.

## Using the codes

Every new code is the byte `0x80` followed by the code, then its arguments. For example, big text is `0x80 0x5B` ... `0x80 0x5C`, and auto-advance after 60 frames is `0x80 0x5E 0x3C`. **Arguments must not be `0x00`**, which ends the message.

[`textdefs.txt`](textdefs.txt) has an alias for every code in the `[Name] = byte, byte` format of Event Assembler's ParseFile and similar text tools, for example `[ToggleOnBigText]HALT![ToggleOffBigText]`.

| Code | Alias | Arguments | Summary |
|------|-------|-----------|---------|
| `0x26` | `[Font]` | `XX` font index (`0x01`–`0xFF`) | Switches the active font from `FontGlyphsPointerTable` (1 normal, 2 bold, 3 italics by default) |
| `0x27` | `[TextPalette]` | `XX` color group, `YY` palette index | Updates one 3-color text group in the active palette |
| `0x28` | `[ColorGroup]` | `XX` color group (`0x01`–`0x05`) | Selects which color group draws new text (`0x05` ≈ `[ToggleRed]`) |
| `0x29` | `[BoxBgPalette]` | `XX` palette index | Swaps the text-box background palette |
| `0x2A` | `[BoxType]` | `XX` type index; bit `0x80` controls tails | Changes box graphics / speech-tail behavior before the next bubble opens |
| `0x2B` | `[BoxHeight]` | `XX` lines (`0x01`–`0x03`) | Sets 1-, 2-, or 3-line box height |
| `0x2C` | `[BoopPitch_*]` | `XX` pitch (`0x01`–`0x19`, default `0x0D`) | Sets the active face's letter-blip pitch. `0x01` is one octave below the default, `0x19` is one octave above |
| `0x2D` | `[PlaySound]` | four nybble bytes | Plays sound `0xABCD` encoded little-endian with each nybble OR'd with `0x80` |
| `0x2E` | `[MugLoc]` | `XX` slot (`0x01`–`0x08`), `YY` signed X tiles | Remaps a face-slot X position (`YY = 0x80` means tile 0) |
| `0x2F` | `[LoadFaceFancy]` | options + attrs + portrait id | Loads a face with flip / eyes-closed options and explicit attributes |
| `0x30`–`0x37` | `[MFL2]` … `[MFFR2]` | `YY` frames | Moves a portrait to that slot over `YY` frames |
| `0x38` | `[TextSpeed]` | `XX` frames, or `0xFF` | Overrides print delay; `0xFF` restores the Options menu speed |
| `0x39` | `[ToggleOnJumping]` | none | Starts continuous jumping on the active portrait |
| `0x3A` | `[ToggleOffJumping]` | none | Stops jumping and restores the portrait's resting Y |
| `0x3B` | `[ToggleOnShakePrint]` | none | Each printed glyph briefly jitters the dialogue text layer |
| `0x3C` | `[ToggleOffShakePrint]` | none | Disables shake-on-print |
| `0x3D` | `[ToggleOnBouncePrint]` | none | Each letter floats down into place as an OBJ, then bakes into the text |
| `0x3E` | `[ToggleOffBouncePrint]` | none | Disables bounce-on-print |
| `0x3F` | `[ToggleOnWave]` | none | Applies a continuous horizontal wave to BG0–BG3 |
| `0x40` | `[ToggleOffWave]` | none | Restores the dialogue background layers' normal horizontal offsets |
| `0x41` | `[ToggleOnVibrating]` | none | Starts a fast, one-pixel vertical vibration on the active portrait |
| `0x42` | `[ToggleOffVibrating]` | none | Stops vibration and restores the portrait's resting Y |
| `0x43` | `[ToggleOnShimmy]` | none | Starts a fast, one-pixel horizontal shimmy on the active portrait |
| `0x44` | `[ToggleOffShimmy]` | none | Stops shimming and restores the portrait's resting X |
| `0x45` | `[ToggleOnNameplate]` | none | Enables speaker nameplates (default off); redraws if a box is already open |
| `0x46` | `[ToggleOffNameplate]` | none | Disables speaker nameplates and clears any active plate |
| `0x47` | `[ToggleOnGlitch]` | none | Starts a horizontal screen tear on BG0–BG3 (a rolling vsync split) |
| `0x48` | `[ToggleOffGlitch]` | none | Stops the screen tear |
| `0x49` | `[ToggleOnStatic]` | none | Starts white horizontal scanline static over the screen |
| `0x4A` | `[ToggleOffStatic]` | none | Stops the scanline static |
| `0x4B` | `[ClearFaceAsh]` / `[AshDissolve]` | none | ClearFace variant: the active portrait dissolves into drifting 16×16 chips, then fades |
| `0x4C` | `[ToggleOnWavyPrint]` | none | Each letter settles with a decaying sine wobble |
| `0x4D` | `[ToggleOffWavyPrint]` | none | Disables wavy-on-print |
| `0x4E` | `[ToggleOnScramblePrint]` | none | Each letter cycles random A–Z glyphs, then resolves to the real character |
| `0x4F` | `[ToggleOffScramblePrint]` | none | Disables scramble-on-print |
| `0x50` | `[ToggleOnDripPrint]` | none | Each letter hangs, then drops into place |
| `0x51` | `[ToggleOffDripPrint]` | none | Disables drip-on-print |
| `0x52` | `[ToggleOnGhostPrint]` | none | Each letter appears with a trailing ghost echo, then bakes |
| `0x53` | `[ToggleOffGhostPrint]` | none | Disables ghost-echo-on-print |
| `0x54` | `[ToggleOnEarthquake]` | none | Starts a 2px screen rumble on BG0–BG3 and matching OBJ |
| `0x55` | `[ToggleOffEarthquake]` | none | Stops the rumble and restores resting offsets |
| `0x56` | `[ImpactFlash]` | none | One-shot white screen flash that peaks for 2 frames, then fades over 10 more |
| `0x57` | `[Sweat]` | none | One-shot 16×16 sweat drop on the active portrait's cheek. Falls into place, holds, then blinks out. The next text waits until it finishes |
| `0x58` | `[Anger]` | none | One-shot 16×16 anger mark on the active portrait's forehead. Grows and shrinks four times, then the next text box continues |
| `0x59` | `[ToggleOnSceneLayout]` | none | Starts the scene layout: a full-width box along the bottom of the screen, portraits behind it, and a speaker name tab on the box's top-left edge. Closes any open bubble so the next text reopens in the new layout |
| `0x5A` | `[ToggleOffSceneLayout]` | none | Returns to normal speech bubbles, moves portraits back to their normal height, and restores the nameplate setting from before the layout was turned on |
| `0x5B` | `[ToggleOnBigText]` | none | Following letters print at twice their size, filling the active line and the line below it. Each one lands explosively, and printing slows to at least 5 frames per letter |
| `0x5C` | `[ToggleOffBigText]` | none | Returns to normal-size letters and the previous print speed. The rest of that line stays on the big letters' baseline |
| `0x5D` | `[Popup]` / `[Objection]` / `[HoldIt]` / `[TakeThat]` | `XX` popup index (`0x01`+) | Closes the box and shows popup `XX` from `TextEnginePopupTable` with a flash, sound and shake for 60 frames (A or B skips after 24), then the next text opens a new box |
| `0x5E` | `[AutoAdvance]` | `XX` frames (`0x01`–`0xFF`) | Used in place of `[A]`: no arrow, and the box moves on by itself after `XX` frames (a button press moves on sooner) |
| `0x5F` | `[ToggleOnAutoAdvance]` | `XX` frames (`0x01`–`0xFF`) | Every following `[A]` shows its arrow as usual but moves on by itself after `XX` frames |
| `0x60` | `[ToggleOffAutoAdvance]` | none | `[A]` waits for the player again |
| `0x61` | `[ToggleOnRainbow]` | none | Following letters take three colour groups in turn, and the groups' colours turn around a colour wheel |
| `0x62` | `[ToggleOffRainbow]` | none | Back to the colour from before `[ToggleOnRainbow]`. Rainbow letters already printed keep changing colour until the dialogue ends |

**ASMC:** `EnableScreenGlitch`, `DisableScreenGlitch`, `EnableScreenStatic`, `DisableScreenStatic`, `EnableScreenEarthquake`, `DisableScreenEarthquake` and `StartScreenImpactFlash` can also be called from events with `ASMC`, when the events are assembled together with this installer so the labels resolve.

The integrated [Text Engine Rework](../../Documentation/Features/TextEngineRework.md) documentation goes into each feature in more detail.

## Settings

`Installer.event` has two settings that you can change without compiling:

| Setting | Default | Meaning |
|---------|---------|---------|
| `#define TEXT_ENGINE_CHATLOG` (top of the file) | commented out | Remove the `//` in front of it to install the [conversation log](#conversation-log). |
| `gTextEngineConfig` byte 0 | `1` | Dim the portraits of characters who aren't speaking. `0` turns it off. |

Fonts, text colours, box palettes and shapes, and popups are tables in [`Data/`](Data/):

- `Data/Tables.event`: `FontGlyphsPointerTable`, `TextPaletteTable`, `TextBoxBgPaletteTable`, `TextBoxTypePointerTable` and the boop pitch table.
- `Data/Popups.event`: `TextEnginePopupTable`. Each popup needs `*.img.bin`, `*.map.bin` and `*.pal.bin` from grit (`grit Name.png -gB 4 -gzl -m -mLf -mR4 -mzl -pn 16 -ftb -fh! -o Name.img.bin`). `Data/Popups/generate_popups.py` draws the shipped style.
- Box shapes are LZ77 `.dmp` files made from the PNGs in `Data/BoxGfx` by `make`.

## Conversation log

The C Skill System's `chatlog_enabled`. It is off by default; to install it, open `Installer.event` and change `// #define TEXT_ENGINE_CHATLOG` near the top to `#define TEXT_ENGINE_CHATLOG` before you insert it. (When assembling from your own event file, `#define TEXT_ENGINE_CHATLOG` before `#include "Installer.event"` works too.)

- Press **SELECT** while a conversation is on screen to open the log. It lists the chapter's dialogue so far, newest at the bottom: each speaker's mini portrait and name, then their lines, rewrapped to the log's width. The line being printed shows as far as it has got.
- **UP** / **DOWN** scroll (a scroll bar shows when there's more than fits), **SELECT** closes it. The dialogue waits while the log is open (A and B do nothing) and carries on where it was when you close it.
- The log keeps the last 17 lines of the chapter. It starts again at each chapter start (including restarting a chapter).
- The log draws on BG2 with tiles and palette slots the scene isn't using, so it shows fewer lines (or doesn't open) when a scene leaves little VRAM free. It doesn't open over world map narration, which prints its text as sprites.

With the setting on, the installer adds two hooks (`TalkSkipListener_OnIdle` and `TalkWaitForInput_OnIdle`, so the dialogue doesn't skip or move on under the log), `Source/Chatlog.lyn.event` (about 5 KB) and uses another 0x600 bytes of EWRAM (see [RAM](#ram)). With it off, `Source/ChatlogStubs.lyn.event` gives the engine empty stand-ins and the engine behaves as it did without the log.

[`Standalone/chatlog`](../chatlog/) is the same log for vanilla dialogue, without this text engine. Don't install both.

## RAM

This patch uses **0x1A0 bytes** of EWRAM, set in `Source/RamAlloc.s`:

| Symbol | Address range | Size | Use |
|--------|---------------|------|-----|
| `sTextEngineWaveOffsets` | `0x0203AB00` .. `0x0203ABEF` | `0xF0` | Per-scanline offsets for the wave, tear and static |
| `sTextEngineNameplateState` | `0x0203ABF0` .. `0x0203AC8F` | `0xA0` | Speaker nameplate text |
| `sTextEngineEarthquakeOffset` | `0x0203AC90` .. `0x0203AC93` | 4 | Earthquake offset |
| `sTextEngineSceneLayoutState` | `0x0203AC94` .. `0x0203AC97` | 4 | Scene layout flags |
| `sTextEngineBigTextState` | `0x0203AC98` .. `0x0203AC9B` | 4 | Big text flags |
| `sTextEngineAutoAdvanceState` | `0x0203AC9C` .. `0x0203AC9F` | 4 | Auto-advance frames |

The block sits in FreeRamSpace2 (`0x0203AAA4` .. `0x0203DDE0`), after the slices used by [alpha_blend_movement_sprites](../alpha_blend_movement_sprites/) and [text_box_extension_layout](../text_box_extension_layout/). To move it, change `TextEngineRamBase` in `Source/RamAlloc.s` and run `make`.

With `TEXT_ENGINE_CHATLOG`, the conversation log uses another **0x600 bytes**, set in `Source/ChatlogRamAlloc.s`:

| Symbol | Address range | Size | Use |
|--------|---------------|------|-----|
| `sChatLogState` | `0x0203B000` .. `0x0203B443` | `0x444` | The logged lines (17 × 64 bytes) and scroll position |
| `sChatLogStamp` | `0x0203B444` .. `0x0203B44B` | 8 | Which chapter the log belongs to: a marker and `gPlaySt`'s chapter start time |
| `sChatlogUiState` | `0x0203B44C` .. `0x0203B5FF` | `0x1AC` used | Drawing state while the log is open |

To move it, change `ChatlogRamBase` in `Source/ChatlogRamAlloc.s` and run `make`. Nothing is added to the save file: the log is cleared when the chapter's start time changes, and it isn't kept in a suspend save.

## Target ROM

- **FE8U (USA)** clean ROM
- Free space: `$1000000`. The code and data take about 114 KB (`$1000000` .. `$101CA24`), or about 120 KB (`$1000000` .. `$101DF6C`) with `TEXT_ENGINE_CHATLOG`.

## Build

No build step is required to install the patch. `Source/TextEngine.lyn.event` and the box shape `.dmp` files are checked in.

`Source/Chatlog.lyn.event` and `Source/ChatlogStubs.lyn.event` are checked in too.

Run `make` if you edit `Source/`, `Source/RamAlloc.s`, `Source/ChatlogRamAlloc.s` or the PNGs in `Data/BoxGfx`:

```bash
make -C Standalone/text_engine
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`; it uses `jumpToHack` and `callHack_r3`, which FEBuilder's Event Assembler already defines.

### FEBuilderGBA

1. Copy the `text_engine` folder (or download this standalone package).
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
"$EA_DIR/ColorzCore" A FE8 -input:_install.event -I:"$EA_DIR" -output:/path/to/fe8-text-engine.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: settings, hooks and free-space placement |
| `Source/TextEngine.c` | The text engine (interpreter, codes, effects) |
| `Source/TalkOnIdle.c` | Vanilla `Talk_OnIdle` with the per-letter hooks |
| `Source/TextEngine.h` | Stand-ins for the C Skill System kernel pieces the engine was written against |
| `Source/RamAlloc.s` | EWRAM addresses |
| `Source/TextEngine.lyn.event` | Checked-in lyn output |
| `Source/Chatlog.c`, `Source/Chatlog.h` | The conversation log (`TEXT_ENGINE_CHATLOG`) |
| `Source/ChatlogRamAlloc.s` | The conversation log's EWRAM addresses |
| `Source/Chatlog.lyn.event` | Checked-in lyn output of the log, installed with `TEXT_ENGINE_CHATLOG` |
| `Source/ChatlogStubs.c`, `Source/ChatlogStubs.lyn.event` | Empty stand-ins for the log, installed without `TEXT_ENGINE_CHATLOG` |
| `Data/Tables.event`, `Data/Fonts`, `Data/BoxGfx`, `Data/Text_Boop_Table.event` | Fonts, colours, box shapes and boop pitches |
| `Data/Popups.event`, `Data/Popups` | Popup table and art |
| `textdefs.txt` | Control code aliases for text tools |
| `makefile` | Compiles the C and RAM objects to `.lyn.event` and the box PNGs to `.dmp` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `sub_8005FE0` (face sprite) | `$5FE0` | 8 bytes |
| `sub_8006134` (face eyes) | `$6134` | 8 bytes |
| `InitTalk` | `$680C` | 8 bytes |
| `Talk_OnInit` | `$6C00` | 8 bytes |
| `Talk_OnIdle` | `$6C34` | 8 bytes |
| `TalkPrepNextChar` (box width) | `$6DC4` | 14 bytes (`callHack_r3`) |
| `sub_8006F00` (`ChangeTextColorID`) | `$6F00` | 8 bytes |
| `TalkInterpret` | `$6FD0` | 8 bytes |
| `StartTalkFaceMove` | `$7A58` | 8 bytes |
| `TalkFaceMove_OnInit` | `$7AB0` | 8 bytes |
| `Talk_OnEnd` | `$7C0C` | 8 bytes |
| `TalkShiftClearAll_OnInit` | `$7D38` | 8 bytes |
| `TalkShiftClear_OnInit` | `$7F9C` | 8 bytes |
| `StartOpenTalkBubble` (talk bubble proc pointer) | `$83F4` | 4 bytes |
| `StartTalkOpen` | `$88A8` | 8 bytes |
| `GetTalkFaceHPos` | `$8934` | 8 bytes |
| `GetStrTalkLen` | `$8B44` | 8 bytes |
| `ClassChgLoadUI` | `$CCBF4` | 8 bytes |
| `TalkSkipListener_OnIdle` (only with `TEXT_ENGINE_CHATLOG`) | `$6B4C` | 8 bytes |
| `TalkWaitForInput_OnIdle` (only with `TEXT_ENGINE_CHATLOG`) | `$7C4C` | 8 bytes |

**Free space:** `$1000000`.

## Conflicts

- Any other dialogue text engine, including Tequila's original assembly text engine and Zeta's AutoNewLine, and any patch that hooks the functions above.
- **EWRAM** `0x0203AB00` .. `0x0203AC9F` (and `0x0203B000` .. `0x0203B5FF` with `TEXT_ENGINE_CHATLOG`) overlaps with other hacks using FreeRamSpace2. Change `Source/RamAlloc.s` / `Source/ChatlogRamAlloc.s` if needed.
- [`Standalone/chatlog`](../chatlog/) hooks `Talk_OnIdle` too; use `TEXT_ENGINE_CHATLOG` instead.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.
- Do not install it on top of the full C Skill System, which already has this engine.

`Installer.event` uses `PROTECT` on every hook site and the free-space body, so overlapping ROM writes fail assembly with a clear EA error.

## Differences from the C Skill System

- Text is vanilla FE8U's one byte per character. The C Skill System version decodes UTF-8.
- The C Skill System's half-body portraits and 32-colour portraits aren't included, so the engine's hooks into them do nothing here. Its conversation log is optional (`TEXT_ENGINE_CHATLOG`).
- Conversation log: it isn't kept in the suspend save, and it is cleared when `gPlaySt`'s chapter start time changes instead of by a chapter-start hook. Map sprites aren't hidden while it is open (they are dimmed with the map behind it). It doesn't open over world map narration, where its text came out garbled. Each redraw copies its tilemap to VRAM straight away, which cuts down the half-drawn frames the integrated log shows while scrolling.
- The screen tear, static and earthquake move backgrounds and portraits. In the C Skill System, map sprites move with them too; here they don't.

## Limitations

- Meant for standard cutscene text. World map text, tutorial boxes and other alternate text paths are untested with the new codes.
- The [Text Engine Rework](../../Documentation/Features/TextEngineRework.md#limitations--bugs) limitations apply too.
- Conversation log: redrawing it takes most of a frame, so while you scroll quickly the frame being redrawn can show a few letters out of place (as in the C Skill System). START still skips the whole scene while the log is open (the event skip isn't blocked, as in the C Skill System); the log closes cleanly. The speaker of each line is guessed from the active portrait, so generic portraits show as `?` and may take the name of a character who shares the portrait.

## Credits

Text engine by Tequila, with help from Zahlman; C port and extensions from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester). See [TextEngineRework.md](../../Documentation/Features/TextEngineRework.md) for integrated documentation.
