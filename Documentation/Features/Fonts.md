# Viable Fonts

Pick **one** `CONFIG_FONT_*` in [`include/configs/configs.h`](../../include/configs/configs.h). Rebuild fonts after changing it.

---

## Index

- [Introduction](#introduction)
- [Viable](#viable)
- [Rejected](#rejected)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations](#limitations)

---

## Introduction

Menus, stat screens, and item names are laid out for a **narrow** GBA font. Vanilla `A` advances **5** pixels. Anything much wider clips or wraps in those boxes.

This file is the viability list. `configs.h` should only offer fonts marked viable here.

---

## Viable

| Font | Define | Cap `A` width | Notes |
|------|--------|---------------|--------|
| Vanilla FE8 | `CONFIG_FONT_VANILLA` | 5 | Stock glyphs. No extra `FontList.txt` entry; `Glyph/fe8u/` is always the fallback. |
| Pokémon Emerald | `CONFIG_FONT_POKE_EMERALD` | 5 | Same Latin advance as vanilla. |
| Advance Wars 2 | `CONFIG_FONT_ADVANCE_WARS_2` | 6 | Slightly wider than vanilla; fits current UI. Extract: `Fonts/Scripts/extract-aw2-font.py`. |
| Super Star Saga | `CONFIG_FONT_SUPER_STAR_SAGA` | 8 | Outlined Mario & Luigi battle font. Lowercase is 6; caps match Fusion's 8 and may clip in tight menus. Extract: `Fonts/Scripts/extract-superstar-saga-font.py`. Sheet omits `q` (mirrored from `p`) and digits other than `1` (vanilla fills those). |

Enable exactly one of those defines (comment the others out). Then:

```
rm -f Fonts/GlyphInstaller.event
make -C Fonts
make -j8
```

If the font has an extract script, run that first so `font.fontall.txt` and the PNGs match.

---

## Rejected

| Font | Define | Cap `A` width | Why |
|------|--------|---------------|-----|
| Metroid Fusion | `CONFIG_FONT_METROID_FUSION` | 8 | Too wide for menus and stat text. Glyphs stay under `Fonts/Glyph/MetroidFusion/` for reference; do not ship it. |

Do not add a rejected define to `configs.h`. `Fonts/FontList.txt` may still wrap the list so a local retest only needs the define uncommented.

---

## Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Selectable defines | `include/configs/configs.h` | Exactly one `CONFIG_FONT_*`. EA understands `#ifdef` only, not `#if`/`#error`. |
| Glyph install order | `Fonts/FontList.txt` | First matching generic font wins; `fe8u` then `NarrowFonts` fill missing characters. |
| AW2 extract | `Fonts/Scripts/extract-aw2-font.py` | Native-size sheet → 16x16 `FontText_*` / `FontItem_*`. |
| Super Star Saga extract | `Fonts/Scripts/extract-superstar-saga-font.py` | Native-size outlined sheet → 16x16 `FontText_*` / `FontItem_*`. |
| Fusion extract | `Fonts/Scripts/extract-metroid-fusion-font.py` | Same pipeline; kept for the rejected sheet only. |

---

## TODO

- Record a new font here as viable or rejected **before** leaving it enabled in `configs.h`.

---

## Limitations

- One font per ROM. There is no runtime toggle.
- Width is cursor advance in `font.fontall.txt`, not the 16x16 PNG size.
- Missing punctuation on a custom sheet is filled by vanilla `fe8u`.
