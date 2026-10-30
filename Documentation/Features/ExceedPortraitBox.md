# Exceed Portrait Box

---

## 📑 Index
- [Introduction](#introduction)
- [Portrait Data](#portrait-data)
- [Text Engine](#text-engine)
- [Config](#config)
- [Code Locations](#code-locations)
- [Limitations](#limitations)

---

## 🧩 Introduction

A portrait can draw two extra 16×16 pieces outside its normal box, for hair, hats or weapons that would otherwise be cut off. This is a C port of tiki's FE7 "Exceed the Portrait Hackbox" (FE8 port by aera, side-column fix by laqieer), which FEBuilder ships as the `MugExceed` patch.

The pieces are drawn in both places a portrait appears:

- **80×72 box** (stat screen, unit and item menus): as BG tiles, by `PutFace80x72_Standard`.
- **Face sprites** (dialogue and other `StartFace` users): as two extra 16×16 sprites, by `Face_OnIdle`.

Portraits that don't opt in are drawn as before.

For a clean FE8U ROM without the C Skill System, use [`Standalone/exceed_portrait_box`](../../Standalone/exceed_portrait_box/), or the `TEXT_ENGINE_EXCEED_PORTRAITS` setting of [`Standalone/text_engine`](../../Standalone/text_engine/) if you use that package.

---

## 🖼️ Portrait Data

A portrait opts in through its portrait table entry (`struct FaceData`, 0x1C bytes):

| Offset | Field | Meaning |
|--------|-------|---------|
| `0x10` | `xLeft` | Left piece X, in tiles |
| `0x11` | `yLeft` | Left piece Y, in tiles |
| `0x12` | `xRight` | Right piece X, in tiles |
| `0x13` | `yRight` | Right piece Y, in tiles |
| `0x1A` | `exceed` | `1` = this portrait has exceed pieces |

Bytes `0x10`-`0x13` are the `imgCard` pointer in vanilla. A portrait with a face image (`img`) never reads it. These are the bytes FEBuilder's `MugExceed` patch edits (it numbers them 17-20 and 27, counting from 1), so portrait data made for that patch works unchanged.

The graphics come from the spare tiles of the portrait sheet (32×4 tiles once decompressed):

| Piece | Top tiles | Bottom tiles | Pixels in the 256×32 sheet |
|-------|-----------|--------------|----------------------------|
| Left | `0x5C`-`0x5D` | `0x7C`-`0x7D` | x 224-239, y 16-31 |
| Right | `0x5E`-`0x5F` | `0x7E`-`0x7F` | x 240-255, y 16-31 |

Positions:

- **80×72 box:** the piece's top-left tile goes to column `x - 1`, row `y` of the 10×9-tile box. Tiles that fall outside the box are skipped.
- **Face sprites:** the piece's left edge is at `face X + (x - 4) * 8 - 16` and its top at `face Y + y * 8`. On a flipped face the piece is mirrored: left edge at `face X + (4 - x) * 8`, drawn with H-flip.

Vanilla blanks the side columns (0 and 9) of the top five rows of the 80×72 box. For an exceed portrait, the top six rows are checked and only the filler tile is blanked, so a piece placed in a side column stays visible.

---

## 💬 Text Engine

The pieces work with [TextEngineRework](./TextEngineRework.md) dialogue:

- Portrait moves, jumping, vibrating and shimmying change the face's position, and the pieces follow it.
- `[ClearFaceAsh]` breaks the pieces into chips along with the rest of the portrait.
- The screen tear and the earthquake shift each piece by its face's offset, so the pieces stay attached to the face.
- Fades, listener dimming and the conversation log's dimming act on the face's palette or OBJ blend, which the pieces share.
- Sweat and anger marks use their own OBJ tiles, not the face's.

---

## ⚙️ Config

`exceed_portrait_box` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c). Default `true`. It does nothing until a portrait sets byte `0x1A` to `1`. When it is `false`, no pieces are drawn and the side columns are blanked as in vanilla.

---

## 📂 Code Locations

| Feature | Location | Description |
|---------|----------|-------------|
| Installer | [`ExceedPortraitBox_Installer.event`](../../Kernel/Wizardry/ExceedPortraitBox/ExceedPortraitBox_Installer.event) | Included from the `WIZARDRY_CUSTOM` block of `wizardry.event` |
| 80×72 box | [`ExceedPortraitBox.c`](../../Kernel/Wizardry/ExceedPortraitBox/ExceedPortraitBox.c) | Replaces `PutFace80x72_Standard` (`0x08005B78`) and `PutFace80x72_Core` (`0x08005CA4`) |
| Face sprites | [`ExceedPortraitBox.c`](../../Kernel/Wizardry/ExceedPortraitBox/ExceedPortraitBox.c) | `ExceedPortrait_PutFaceSprites`, called from `Face_OnIdle` in [`Portrait32Color.c`](../../Kernel/Wizardry/Portrait32Color/Source/Portrait32Color.c) |
| Detection | [`ExceedPortraitBox.c`](../../Kernel/Wizardry/ExceedPortraitBox/ExceedPortraitBox.c) | `ExceedPortrait_IsExceedFace` |
| Piece list | [`ExceedPortraitBox.c`](../../Kernel/Wizardry/ExceedPortraitBox/ExceedPortraitBox.c) | `ExceedPortrait_GetFacePieces`: each piece's offset from the face, tile and flip, for the face sprites and the text engine |
| Ash dissolve | [`TextEngineRework.c`](../../Kernel/Wizardry/TextEngineRework/Source/TextEngineRework.c) | `TextEngineAsh_DrawChips` and `TextEngineAsh_CountFaceChips` add the pieces to `[ClearFaceAsh]` |

---

## ⚠️ Limitations

- Raised portraits (the `PortraitHeight` table, drawn by `PutFace80x72_Raised`) get no pieces in the 80×72 box, as in the original patch. Their face sprites still get them.
- Half-body portraits never get pieces: they use tiles `0x5C`-`0x5F` for their own body.
- A 32-color portrait uses `imgCard` for its second palette, so it can't also be an exceed portrait. With `portrait_32_color` on, the pieces are drawn from the base layer only.
