# Exceed Portrait Box

Lets a portrait draw two extra 16x16 pieces outside its normal box, for hair, hats or weapons that would otherwise be cut off. This is the C Skill System's [Exceed Portrait Box](../../Documentation/Features/ExceedPortraitBox.md) for vanilla FE8U: a C port of tiki's FE7 "Exceed the Portrait Hackbox" (FE8 port by aera, side-column fix by laqieer), which FEBuilder ships as `MugExceed`.

The pieces are drawn wherever the portrait appears:

- **80x72 box** (stat screen, unit and item menus): as background tiles.
- **Face sprites** (dialogue and other scenes that show a face): as two extra 16x16 sprites. A flipped face gets mirrored pieces.

Portraits that don't opt in are drawn exactly as in vanilla.

If you use [`Standalone/text_engine`](../text_engine/), don't install this package. Turn on that package's `TEXT_ENGINE_EXCEED_PORTRAITS` setting instead. It installs the same code (built from this folder's `Source/ExceedPortraitBox.c`), and the text engine's portrait motion, screen effects and ash dissolve then move the pieces with their face.

## Portrait data

A portrait opts in through its portrait table entry (0x1C bytes, `GetPortraitData` at `0x08005514`):

| Offset | Meaning |
|--------|---------|
| `0x10` | Left piece X, in tiles |
| `0x11` | Left piece Y, in tiles |
| `0x12` | Right piece X, in tiles |
| `0x13` | Right piece Y, in tiles |
| `0x1A` | `1` = this portrait has exceed pieces |

Bytes `0x10`-`0x13` are the `imgCard` pointer in vanilla, which a portrait with a face image never reads. These are the bytes FEBuilder's `MugExceed` patch edits (it numbers them 17-20 and 27, counting from 1), so portrait data made for that patch works unchanged.

The graphics come from the spare tiles of the portrait sheet (32x4 tiles once decompressed):

| Piece | Top tiles | Bottom tiles | Pixels in the 256x32 sheet |
|-------|-----------|--------------|----------------------------|
| Left | `0x5C`-`0x5D` | `0x7C`-`0x7D` | x 224-239, y 16-31 |
| Right | `0x5E`-`0x5F` | `0x7E`-`0x7F` | x 240-255, y 16-31 |

Positions:

- **80x72 box:** the piece's top-left tile goes to column `x - 1`, row `y` of the 10x9-tile box. Tiles that fall outside the box are skipped.
- **Face sprites:** the piece's left edge is at `face X + (x - 4) * 8 - 16` and its top at `face Y + y * 8`. On a flipped face it is mirrored: left edge at `face X + (4 - x) * 8`.

Vanilla blanks the side columns (0 and 9) of the top five rows of the 80x72 box. For an exceed portrait, the top six rows are checked and only the filler tile is blanked, so a piece in a side column stays visible.

## Target ROM

Fire Emblem: The Sacred Stones (USA), FE8U.

## Installation

### FEBuilderGBA

1. Open a clean FE8U ROM in FEBuilder.
2. Go to **Advanced Editors → Insert EA**, click **Select File**, choose `Installer.event`, then click **Load Script**.
3. Set bytes `0x10`-`0x13` and `0x1A` of each portrait that should use pieces, in the portrait editor or a hex editor.

### Event Assembler

```bash
EA_DIR=<repo>/Tools/EventAssembler
{
  echo '#include "Extensions/Hack Installation.txt"'
  echo '#include "Installer.event"'
} > _install.event
"$EA_DIR/ColorzCore" A FE8 -input:_install.event -I:"$EA_DIR" -output:/path/to/fe8.gba
```

## Build

You only need to build if you change `Source/`. `Source/ExceedPortraitBox.lyn.event` is checked in.

```bash
make -C Standalone/exceed_portrait_box
```

This needs [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's tools (see [Documentation/Setup.md](../../Documentation/Setup.md)).

## Files

| File | Role |
|------|------|
| `Installer.event` | Hooks and free-space placement |
| `Source/ExceedPortraitBox.c` | The code (also built into `Standalone/text_engine`) |
| `Source/ExceedPortraitBox.h` | Its declarations, shared with `Standalone/text_engine` |
| `Source/ExceedPortraitBox.lyn.event` | Compiled code, checked in |
| `makefile` | Builds the `.lyn.event` |

## Hooks and free space

| Address | Function | Bytes | Replaced by |
|---------|----------|-------|-------------|
| `0x080055BC` | `Face_OnIdle` | 8 | `Face_OnIdle_Exceed`: vanilla, then the pieces |
| `0x08005B78` | `PutFace80x72_Standard` | 8 | `PutFace80x72_Standard_Exceed`: vanilla, then the pieces |
| `0x08005CA4` | `PutFace80x72_Core` | 16 | `PutFace80x72_Core_Exceed`: vanilla, with the side-column rule above |

The code goes at `$1000000` and takes about 1 KB.

## Limitations

- Raised portraits (drawn by `PutFace80x72_Raised`) get no pieces in the 80x72 box, as in the original patch. Their face sprites still get them.

## Conflicts

- FEBuilder's `MugExceed` patch (the assembly original). Remove it first; the portrait data carries over.
- [`Standalone/text_engine`](../text_engine/): use its `TEXT_ENGINE_EXCEED_PORTRAITS` setting instead.
- [`Standalone/portrait_formats`](../portrait_formats/): both hook `Face_OnIdle` and `PutFace80x72_Core`. Use `Standalone/text_engine` with both `TEXT_ENGINE_HALF_BODY_PORTRAITS` and `TEXT_ENGINE_EXCEED_PORTRAITS` instead.
- Other standalone patches that also use `$1000000`. Install only one there, or move this patch's `ORG`.
- The full C Skill System, which already has this feature.

## Credits

Exceed the Portrait Hackbox by tiki, FE8 port by aera, side-column fix by laqieer. C port from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
