# FE8 Text Interpreter Rework

<p align="center">
  <em>By Tequila</em>
</p>

---

## 📑 Index

* [Introduction](#introduction)
* [Compatibility & Usage Notes](#compatibility--usage-notes)
* [Core Behavior](#core-behavior)
* [Command Reference](#command-reference)

  * [Fonts](#fonts)
  * [Change Text Palette](#change-text-palette)
  * [Change Color Group](#change-color-group)
  * [Change Box Background Palette](#change-box-background-palette)
  * [Change Box Type](#change-box-type)
  * [Change Box Height](#change-box-height)
  * [Change Text Boop Pitch](#change-text-boop-pitch)
  * [Play Sound](#play-sound)
  * [Change Portrait Location](#change-portrait-location)
  * [Fancy LoadFace](#fancy-loadface)
  * [Move Portrait with Variable Speed](#move-portrait-with-variable-speed)
  * [Set Text Speed](#set-text-speed)
* [Buildfile Definitions](#buildfile-definitions)
* [Example Script](#example-script)
* [Credits](#credits)

---

## 🧩 Introduction

This hack adds **new functionality to the FE8 text engine** while remaining **fully backwards compatible** with existing text.

It provides extended control over:

* Fonts
* Text palettes and color groups
* Text box appearance
* Portrait behavior and positioning
* Text speed and sound effects

---

## ⚠️ Compatibility & Usage Notes

* ✅ **Backwards compatible** with vanilla FE8 text
* ❌ **NOT compatible** with **Zeta’s AutoNewLine hack**
* ⚠️ **All arguments MUST be non-zero**

  * `0x00` is interpreted as a **terminator** when copying text from ROM
  * This is why portrait IDs (shorts) have `0x100` added
    (e.g. portrait `0x02` → `0x0102` or `[0x02][0x01]`)
* ⚠️ **Intended for standard cutscene text only**

### Untested / Unsupported Contexts

This hack has **not been tested** with:

* World map text
* Tutorial-style text boxes
* Scroll/parchment boxes
* Brown location text boxes (e.g. “Renais Castle”)

Some features may work, some may crash, and some may do nothing due to different text interpreters being used.

---

## 🔧 Core Behavior

When a portrait is loaded using `LoadFace`, the game assigns default values to that portrait’s **location**:

* Font
* Color group
* Background palette
* Box type
* Text boop pitch

If these values are changed, they are **remembered** for future use.

### Example

* If Eirika speaks using the **bold font**, all her future dialogue will remain bold until:

  * The font is changed again
  * The portrait is cleared
  * The text ends
* If Eirika (bold) and Seth (normal) converse, you do **not** need to manually swap fonts every time
* Moving a portrait to a new position copies all its attributes

All referenced tables are located in:

```
_Text_Engine_Tables.txt
```

---

## 🛠️ Command Reference

---

### Fonts

**Syntax**

```
[0x80][0x26][XX]
```

* `XX` → Index in `FontGlyphsPointerTable` (`0x01–0xFF`)
* Defaults:

  * `0x01` = Normal
  * `0x02` = Bold
  * `0x03` = Italics

#### Creating a Custom Font

1. Create a `.png` of your font
   (see `Fonts/FE8_Text_Norm.png`)
2. Edit and run:

   ```
   Fonts/generate_font.py
   ```

   * Requires **Python 3**
   * Script by **Zahlman**
3. Include the generated installer in `_Text_Engine_Tables.txt`

⚠️ **Kerning Note**
Glyph spacing may need manual adjustment.
The **6th byte** of each glyph header controls width.

Example (`A` in italic font):

```
ital_text65:
BYTE 0x00 0x00 0x00 0x00 0x00 0x09 0x00 0x00
```

* Width = `0x09` pixels
* Too small → overlap
* Too large → excess whitespace

Making good-looking fonts is **hard**.

---

### Change Text Palette

**Syntax**

```
[0x80][0x27][XX][YY]
```

* `XX` → Color group (`0x01–0x05`)
* `YY` → Index in `TextPaletteTable`

**Color Groups Explained**

* Palette = 16 colors
* Text = 2bpp (4 colors, 1 transparent)
* Allows **5 color groups** sharing transparency

| Group | Colors Used |
| ----- | ----------- |
| 1     | 1,2,3,4     |
| 2     | 1,5,6,7     |
| 3     | 1,8,9,10    |
| 4     | 1,11,12,13  |
| 5     | 1,14,15,16  |

**Vanilla Usage**

* Group 1 → World map
* Group 2 → Conversations
* Group 5 → `[ToggleRed]`

Updating a color group updates **all text using that group**.

---

### Change Color Group

**Syntax**

```
[0x80][0x28][XX]
```

* `XX` → Color group (`0x01–0x05`)

Allows multiple colors on-screen simultaneously.

Equivalent to `[ToggleRed]` when using group `0x05`.

⚠️ Groups 3 and 4 require `Change Text Palette` first.

---

### Change Box Background Palette

**Syntax**

```
[0x80][0x29][XX]
```

* `XX` → Index in `TextBoxBgPaletteTable`

Changes the text bubble palette.

---

### Change Box Type

**Syntax**

```
[0x80][0x2A][XX]
```

* `XX` → Index in `TextBoxTypePointerTable`
* Bit `0x80` enables **speech tails**

Use **before opening** a new text bubble.

Includes:

* Default box (animated expansion)
* Quasi-spiky box
* Thought bubble box

---

### Change Box Height

**Syntax**

```
[0x80][0x2B][XX]
```

* `XX` → Number of lines (`0x01–0x03`)

---

### Change Text Boop Pitch

**Syntax**

```
[0x80][0x2C][XX]
```

* Range: `0x01–0x19`
* Default: `0x0D`
* Each increment = one semitone

---

### Play Sound

**Syntax**

```
[0x80][0x2D][0x8D][0x8C][0x8B][0x8A]
```

Plays sound `0xABCD` (little endian).

Designed to avoid accidental `0x00` terminators.

---

### Change Portrait Location

**Syntax**

```
[0x80][0x2E][XX][YY]
```

* `XX` → Position ID (`0x01–0x08`)
* `YY` → Signed X offset (tiles)

| ID | Position    |
| -- | ----------- |
| 01 | FarLeft     |
| 02 | MidLeft     |
| 03 | Left        |
| 04 | Right       |
| 05 | MidRight    |
| 06 | FarRight    |
| 07 | FarFarLeft  |
| 08 | FarFarRight |

⚠️ Use `YY = 0x80` to target tile `0x00`.

---

### Fancy LoadFace

**Syntax**

```
[0x80][0x2F][Options|0x80][Font][ColorGroup][BgPalette][BoxType][Pitch]
```

**Options Bitfield**

* `0x80` → Always set
* `0x01` → Flip portrait right, by default always set to left
* `0x02` → Eyes closed

Defaults (normal LoadFace):

```
0x01 0x02 0x01 0x01 0x0D
```

---

### Move Portrait with Variable Speed

**Syntax**

```
[0x80][0x3X][YY]
```

* `X` → Position ID (`0–7`)
* `YY` → Frames to move

Same behavior as vanilla move commands, but speed is configurable.

---

### Set Text Speed

**Syntax**

```
[0x80][0x38][XX]
```

| Speed  | Frames |
| ------ | ------ |
| Slow   | 8      |
| Normal | 4      |
| Fast   | 1      |
| Max    | 0      |

* Use `0xFF` to revert to option-selected speed.

---

## 🧱 Buildfile Definitions

For use with **TextProcess** buildfiles:

```
[Font] = [0x80][0x26]
[TextPalette] = [0x80][0x27]
[ColorGroup] = [0x80][0x28]
[BoxBgPalette] = [0x80][0x29]
[BoxType] = [0x80][0x2A]
[BoxHeight] = [0x80][0x2B]
[BoopPitch] = [0x80][0x2C]
[PlaySound] = [0x80][0x2D]
[MugLoc] = [0x80][0x2E]
[LoadFaceFancy] = [0x80][0x2F]
[MFL2] = [0x80][0x30]
[MML2] = [0x80][0x31]
[ML2] = [0x80][0x32]
[MR2] = [0x80][0x33]
[MMR2] = [0x80][0x34]
[MFR2] = [0x80][0x35]
[MFFL2] = [0x80][0x36]
[MFFR2] = [0x80][0x37]
[TextSpeed] = [0x80][0x38]
[DefaultAttrs] = [0x01][0x02][0x01][0x01][0x0D]
```

⚠️ FEBuilder uses `@` instead of `[]`.

---

## 🧪 Example Script

```
[TextSpeed][0x1]
[oml][LoadFace][0x14][0x1]
[omr][LoadFace][0x2][0x1]
[oml]
Hi! This is a demonstration[N]
of the new text engine hack![A]
...
```

*(Example unchanged from original for accuracy.)*

---

## 🎖️ Credits

* **Zahlman** — FE7 text engine overhaul & font generator
* **Stan** — Troubleshooting
* **Black Mage & Eliwan** — Text box graphics
* **The Awful Emblem Team** — Beta testing

---