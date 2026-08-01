# FE8 Text Engine Rework

<p align="center">
  <img src="../Gifs/Text_Engine_Rework.gif" alt="Text Engine Rework" width="600"/>
</p>

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Supported Control Codes](#supported-control-codes)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

This rework expands the FE8 dialogue interpreter while keeping existing text functional. It is a C port of Tequila's text-engine hack (with help from Zahlman), wired through LynJump hooks so the game always runs the extended interpreter.

Player-facing impact:

- Dialogue can switch fonts, colors, box styles, and print speed from script commands.
- Portraits remember per-slot attributes (font, color group, box palette, box type, boop pitch) and reuse them when the speaker changes.
- Portraits can load with flip / eyes-closed options, move at custom speeds, and use remapped screen positions.
- Existing vanilla text remains readable and compatible with the new parser.

This feature is intended for **standard cutscene text**. It has not been validated against every alternate dialogue path the game can use.

---

## 🛠️ Plan

The rework is a small set of C hooks plus shared tables.

| Layer | Behavior | Result |
|-------|----------|--------|
| Interpreter dispatch | Replaces the vanilla talk interpreter with `TalkInterpret` | Extended `0x80` commands parse safely |
| Text measurement | Recalculates box width with `GetStringTextWidthWithDialogueCodes` | Control codes and font changes no longer break width math |
| Presentation state | Copies and updates face attributes before the box appears | Palette, box, and font settings follow the active speaker |
| Face / box hooks | C replacements for load, move, open, clear, and promotion UI | Variable-speed moves, 1–3 line boxes, and fancy face load work on the main dialogue path |
| Script-facing aliases | Friendly names in `Contents/Texts/textdefs.txt` plus table data in `_Text_Engine_Tables.txt` | Text writers can use macros instead of raw bytes |

### Attribute model

When a portrait is loaded (`LoadFace` / `LoadFaceFancy`), the engine assigns default attributes to that face slot:

| Attribute | Default index | Meaning |
|-----------|---------------|---------|
| Font | `0x01` | Normal FE8 font |
| Color group | `0x02` | Default conversation text colors |
| Box background palette | `0x01` | Default box palette |
| Box type | `0x01` | Default speech bubble |
| Boop pitch | `0x0D` | Default pitch |

Updates stick to the face until you change them again, clear the face, or end the dialogue. Moving a portrait copies attributes to the new slot, so speaker switches do not require re-setting every style command.

### Build wiring

1. `TextEngineRework_Installer.event` includes `Source/LynJump.event`, generated `Source/TextEngineRework.lyn.event`, and `_Text_Engine_Tables.txt`.
2. `Source/LynJump.event` installs whole-function trampolines and a few call-sites.
3. Tables define fonts, text palettes, box palettes, box graphics, and boop timing data.
4. `Contents/Texts/textdefs.txt` exposes author-facing aliases such as `[Font]`, `[LoadFaceFancy]`, and `[MML2]`.

---

## 🎛️ Supported Control Codes

All extended commands use the `[0x80][XX]...` form. **Arguments must be non-zero**: `0x00` terminates copied text. Portrait IDs are written as little-endian shorts with `0x100` added (portrait `0x02` → `[0x02][0x01]`).

| Code | Alias | Arguments | Summary |
|------|-------|-----------|---------|
| `0x26` | `[Font]` | `XX` font index (`0x01`–`0xFF`) | Switches the active font from `FontGlyphsPointerTable` (1 normal, 2 bold, 3 italics by default) |
| `0x27` | `[TextPalette]` | `XX` color group, `YY` palette index | Updates one 3-color text group in the active palette |
| `0x28` | `[ColorGroup]` | `XX` color group (`0x01`–`0x05`) | Selects which color group draws new text (`0x05` ≈ `[ToggleRed]`) |
| `0x29` | `[BoxBgPalette]` | `XX` palette index | Swaps the text-box background palette |
| `0x2A` | `[BoxType]` | `XX` type index; bit `0x80` controls tails | Changes box graphics / speech-tail behavior before the next bubble opens |
| `0x2B` | `[BoxHeight]` | `XX` lines (`0x01`–`0x03`) | Sets 1-, 2-, or 3-line box height |
| `0x2C` | `[BoopPitch_*]` | `XX` pitch (`0x01`–`0x19`, default `0x0D`) | Stores boop pitch for the active face (see limitations) |
| `0x2D` | `[PlaySound]` | four nybble bytes | Plays sound `0xABCD` encoded little-endian with each nybble OR'd with `0x80` |
| `0x2E` | `[MugLoc]` | `XX` slot (`0x01`–`0x08`), `YY` signed X tiles | Remaps a face-slot X position (`YY = 0x80` means tile 0) |
| `0x2F` | `[LoadFaceFancy]` | options + attrs + portrait id | Loads a face with flip / eyes-closed options and explicit attributes |
| `0x30`–`0x37` | `[MFL2]` … `[MFFR2]` | `YY` frames | Moves a portrait to that slot over `YY` frames |
| `0x38` | `[TextSpeed]` | `XX` frames, or `0xFF` | Overrides print delay; `0xFF` restores the Options menu speed |

### Fancy LoadFace options

`[LoadFaceFancy]` expects options with bit `0x80` always set (avoids an accidental terminator), then font / color group / bg palette / box type / pitch, then the portrait id:

- bit `0x01`: flip the portrait to face right; if unset, the portrait always faces left
- bit `0x02`: load with eyes closed

Helpers like `[FlipRight]` / `[FlipLeft]` and portrait-specific `[LoadFaceFancy_*]` aliases live in `textdefs.txt`.

### Color groups

A text palette is 16 colors. Dialogue glyphs are 2bpp (4 colors, first transparent). With a shared transparent color there are five usable groups:

| Group | Colors used | Vanilla role |
|-------|-------------|--------------|
| 1 | 1–4 | World map text |
| 2 | 1, 5–7 | Default conversation text |
| 3 | 1, 8–10 | Unused by default (blue, hard to read) |
| 4 | 1, 11–13 | Unused by default (yellow, hard to read) |
| 5 | 1, 14–16 | `[ToggleRed]` |

`[TextPalette]` rewrites one group's three opaque colors. Any on-screen text already using that group updates immediately.

### Examples

```text
[TextSpeed][0xFF]
[LoadFaceFancy][0x04][0x01][0x02][0x01][0x02][0x01][0x01][0x0D]
[Font][0x02]Eirika: I can change the text style mid-sentence.
[ColorGroup][0x05][TextPalette][0x02][0x05]Another color is now active.
[BoxType][0x03][BoxHeight][0x03]A different textbox shape can appear on the next line.
[PlaySound][0x8D][0x8C][0x8B][0x8A]That sound effect fires before the next message.
[MML2][0x10]Move me more slowly than vanilla.
```

### Custom fonts

1. Create a PNG using `Fonts/FE8_Text_Norm.png` as a reference.
2. Edit and run `Fonts/generate_font.py` (Python 3) to emit an installer.
3. Include the installer from `_Text_Engine_Tables.txt`.

Glyph width lives in the 6th header byte of each glyph entry; adjust kerning there if bold/italic spacing looks too wide.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Dialogue interpreter | `TalkInterpret` in [`Source/TextEngineRework.c`](../../Kernel/Wizardry/Misc/TextEngineRework/Source/TextEngineRework.c) | Parses vanilla and extended control codes; LynJump at `$6FD0` |
| Width calculation | `GetStringTextWidthWithDialogueCodes` in [`Source/TextEngineRework.c`](../../Kernel/Wizardry/Misc/TextEngineRework/Source/TextEngineRework.c) | Measures dialogue including rework codes and font switches; LynJump at `$8B44` |
| Font / color / box helpers | `UpdateFontGlyphSet`, `ChangeTextColorID`, `UpdateTextBoxBgPalette`, `Copy_Text_Attributes`, `DecompressTextBoxGraphics` in [`Source/TextEngineRework.c`](../../Kernel/Wizardry/Misc/TextEngineRework/Source/TextEngineRework.c) | Apply presentation state and drive `gProc_DialogueBoxAppearingAnimation` |
| Face position table | `WriteFaceXPosTableToRAM`, `GetTalkFaceHPos` in [`Source/TextEngineRework.c`](../../Kernel/Wizardry/Misc/TextEngineRework/Source/TextEngineRework.c) | Seeds and reads the remappable face X table |
| Talk init / open / clear | `InitTalk_C`, `StartTalkOpen_C`, `TalkShiftClearAll_OnInit_C`, `TalkShiftClear_OnInit_C` in [`Source/TextEngineRework.c`](../../Kernel/Wizardry/Misc/TextEngineRework/Source/TextEngineRework.c) | Supports 1–3 line boxes and clear origins |
| Variable-speed face move | `StartTalkFaceMoveC`, `TalkFaceMove_OnInitOverride` in [`Source/TextEngineRework.c`](../../Kernel/Wizardry/Misc/TextEngineRework/Source/TextEngineRework.c) | Preserves move-proc return ABI and custom move duration |
| Fancy / normal face load | `TextEngine_LoadFace` in [`Source/TextEngineRework.c`](../../Kernel/Wizardry/Misc/TextEngineRework/Source/TextEngineRework.c) | Shared loader used by `LoadFace` and `LoadFaceFancy` |
| Promotion UI box fix | `ClassChgLoadUI_C` in [`Source/TextEngineRework.c`](../../Kernel/Wizardry/Misc/TextEngineRework/Source/TextEngineRework.c) | Keeps class-change UI box graphics compatible |
| Explicit hooks | [`Source/LynJump.event`](../../Kernel/Wizardry/Misc/TextEngineRework/Source/LynJump.event) | Whole-function trampolines and callHack sites |
| Generated Lyn output | [`Source/TextEngineRework.lyn.event`](../../Kernel/Wizardry/Misc/TextEngineRework/Source/TextEngineRework.lyn.event) | Auto-generated from the C object; do not edit by hand |
| Installer | [`TextEngineRework_Installer.event`](../../Kernel/Wizardry/Misc/TextEngineRework/TextEngineRework_Installer.event) | Pulls LynJump, lyn output, and tables into the build |
| Tables | [`_Text_Engine_Tables.txt`](../../Kernel/Wizardry/Misc/TextEngineRework/_Text_Engine_Tables.txt), [`text_boop_table.txt`](../../Kernel/Wizardry/Misc/TextEngineRework/text_boop_table.txt) | Fonts, palettes, box graphics, boop timing |
| Font assets / tools | [`Fonts/`](../../Kernel/Wizardry/Misc/TextEngineRework/Fonts/), [`Text_Box_Graphics/`](../../Kernel/Wizardry/Misc/TextEngineRework/Text_Box_Graphics/) | Editable font/box assets and generation scripts |
| Text aliases | [`Contents/Texts/textdefs.txt`](../../Contents/Texts/textdefs.txt) | `[Font]`, `[LoadFaceFancy]`, move-speed macros, and related helpers |

---

## 📝 TODO

- Re-hook pitched text boops so `[BoopPitch_*]` / attribute pitch actually affect letter sounds during print.
- Add more author-facing example scripts beyond the control-code matrix above.
- Consider portrait-shake / other presentation effects now that the engine lives in C.

---

## 🐛 Limitations & Bugs

- Meant for **standard cutscene text** only. World map text, tutorial boxes, scroll/parchment boxes, brown location boxes, and other alternate interpreters are untested and may no-op or crash.
- **Not compatible** with Zeta's AutoNewLine hack.
- All script arguments must be **non-zero**; `0x00` terminates text copies.
- Custom box types have limited tile variety; new shapes do not include the vanilla multi-frame expand animation.
- Boop pitch is stored on face/current attributes and accepted by `0x2C`, but the old `PlayTextBoop` idle hook is not currently installed, so pitch changes may not audibly apply until that path is restored in C.
- Some features only apply when dialogue uses the hooked vanilla talk path.

Please report issues in the repository’s **Issues** tab.

---
