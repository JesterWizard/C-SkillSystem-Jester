---
name: font-edits
description: "Add or replace a build-time UI/dialogue font in this repo (16x16 2bpp glyphs, FontItem vs FontText, CONFIG_FONT_*). Use when extracting a pixel font sheet, adding a font config, fixing menu/item glyph color, tightening letter spacing, or switching vanilla / PokeEmerald / Advance Wars 2 fonts. Never use RAM or a runtime toggle."
---

# Font Edits

Install an extra glyph set at **compile time**. The working example is Advance Wars 2: `Fonts/Glyph/AdvanceWars2/` plus `Fonts/Scripts/extract-aw2-font.py`.

## Use This Skill When

- The user wants a new font, a replacement sheet, or a `CONFIG_FONT_*` option.
- Menu/stat text should be white-outlined (item) while dialogue stays dark (text).
- Letter spacing / tracking needs a tweak.
- A font sheet must be sliced into this repo's glyph PNGs.

## Guardrails

- **Build-time only.** One font per ROM. Do not add designer-config toggles, `SetTextFontGlyphs` hooks, or memmap copies of glyph tables.
- **No RAM.** If switching fonts would need EWRAM, use a `CONFIG_FONT_*` define instead.
- **Native pixels only.** If the sheet is already GBA-scale, blit 1:1 into 16x16. Do not downscale a hi-res dump (nearest-neighbor punches holes; box downsample still looks wrong).
- **Event Assembler cannot parse `#if` or `#error`.** `include/configs/configs.h` is included by EA. Use `#define` / `// #define` and `#ifdef` only. A C-style `#if defined(A) + defined(B) != 1` will fail at `[GEN] fe8-kernel-dev.gba`.
- Missing characters are fine: later lists in `Fonts/FontList.txt` fill gaps (vanilla `fe8u`, then `NarrowFonts`).

## Glyph Format

Each character is two 16x16 mode-`P` PNGs:

| File | Where it shows | Pixels |
|------|----------------|--------|
| `FontText_XX.png` | Dialogue / talk | Dark body, index **3** |
| `FontItem_XX.png` | Stats, items, menus | White fill index **2**, 4-connected dark outline index **3** |

Shared 4-color palette (index 0 is preview bg only; grit drops it):

| Index | RGB | Role |
|-------|-----|------|
| 0 | text `(224,224,224)` / item `(104,136,168)` | Background |
| 1 | `(168,168,167)` | Unused |
| 2 | `(248,248,248)` | White / highlight |
| 3 | `(40,40,40)` | Dark body or outline |

Vanilla reference: `Fonts/Glyph/fe8u/FontText_41.png` vs `FontItem_41.png`. Item outline matches `Fonts/Scripts/glyph-fix.py` (N/E/S/W neighbors of white become 3).

`font.fontall.txt` lines (generator regex is one character in column 1):

```
//char	type	Width	Filename
A	item	6	FontItem_41.png
A	text	6	FontText_41.png
```

Width is cursor advance, not clip. Typical: `LEFT_PAD = 1`, body in x=1…N, `width = last_ink_x + 1`. Reducing `+1` to `+0` makes letters touch; `+2` is too loose for AW2.

Place caps on dest baseline **y=11** (9px caps occupy y=3–11). Map each sheet row by its own baseline so x-height and descenders line up.

## Adding a Font

1. Confirm the source sheet is native GBA pixels (or accept vanilla fallback for missing glyphs). Dump it as ASCII if row packing is irregular; do not assume a uniform grid.
2. Put assets in `Fonts/Glyph/<Name>/` (`sheet.png`, `FontItem_*.png`, `FontText_*.png`, `font.fontall.txt`).
3. Prefer a one-shot extract script next to `Fonts/Scripts/extract-aw2-font.py`: explicit `(char, x0, y0, x1, y1, sheet_baseline)` boxes, 1:1 blit, then `to_item()` for the white+outline variant. Do not reuse the same dark bitmap for item and text.
4. Add **exactly one** selectable define in `include/configs/configs.h`:

```c
// #define CONFIG_FONT_VANILLA
// #define CONFIG_FONT_POKE_EMERALD
#define CONFIG_FONT_YOUR_FONT
```

   Comment out the others. Do not add an `#if defined(...) + ...` uniqueness check.
5. Wrap the new list in `Fonts/FontList.txt` **above** `Glyph/fe8u/`. First matching generic font wins; later lists only fill missing characters:

```
#ifdef CONFIG_FONT_YOUR_FONT
Glyph/YourFont/font.fontall.txt
#endif
```

6. Force a font rebuild, then the ROM (see below).

## Rebuild (required)

`Fonts/makefile` depends only on `FontList.txt` and `configs.h`, **not** the PNGs or `font.fontall.txt`. After changing glyphs or widths:

```
python3 Fonts/Scripts/extract-aw2-font.py   # or the font's extract script
rm -f Fonts/GlyphInstaller.event
make -C Fonts
make -j8
```

`make rebuild_font_indexes` also deletes the installer and remakes. A plain `make` will not pick up PNG-only edits.

EA success line is `No errors. Please continue being awesome.` If you see `Directive not recognized: if` in `configs.h`, remove the `#if`/`#error`.

## Spacing and Color Tweaks

- **Too much gap between letters:** lower advance in the extract script (`max_x + 1` → `max_x + 0` is the next step) and regenerate. Keep item and text advances the same unless a specific glyph clips.
- **Menu text still dark:** item PNGs are still the dialogue bitmap. Rebuild item glyphs as white+outline; dialogue `FontText_*` stays index-3 only.
- **Jagged / hollow strokes:** the sheet was downscaled. Replace with a native-size sheet and re-extract; do not try to repair the old PNGs.

## Do Not Reintroduce

- Runtime `SetTextFontGlyphs` overlay or designer-config `text_font`
- Static BSS copies of 256-pointer font tables / `_kernel_malloc` for glyph overlays
- Hand-painted substitutes when the sheet already has the character
- `#if` / `#error` in `configs.h`

## Good Default Output

When asked to add or fix a font, do the extract/config/FontList work, force-rebuild fonts, then `make` the ROM so the change is actually in `fe8-kernel-dev.gba`.
