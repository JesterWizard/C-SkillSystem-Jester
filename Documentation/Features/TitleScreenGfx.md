# Title screen graphics

The title screen uses a new background, a dragon emblem and a logo (Fire Emblem™, The Sacred Stones banner, EXCEED).
The vanilla title screen code is kept. `TitleScreenGfx_Installer.event` only repoints its graphics, sprite layouts and
positions; there is no C code.

The build date (top left, `BuildVersionTitleScreen`), Press START and the copyright line are vanilla.

## Layers

| Layer | What | Format | Limit |
|---|---|---|---|
| BG1 background | 240x160 picture | 16-colour palette in bank 14, 4bpp tiles at 0x06000000, loaded as two LZ77 blobs (first 384 tiles, then the rest) | 640 tiles (now 597) |
| BG0 emblem | dragon emblem | palette bank 15, tiles at 0x06005000. The game adds 0xF280 to every map entry (palette 15, tile +0x280) | 192 tiles (now 127) |
| OBJ sprites | logo (palette 2, tile 0x000), banner (palette 3, tile 0x080), EXCEED (palette 2, tile 0x100) | 2D mapping, 256-px-wide sheet, 10 tile rows | tiles 0x000-0x13F |
| Intro zoom | BG2 image shown during the logo zoom | 8bpp affine, 256x256 texture. At the end of the zoom it is shown 1:1, so the logo and EXCEED sit at their final screen positions in it. Palette entry 0 is 0x4290 (it sets the backdrop) | 160 tiles (now 93) |

OBJ palettes 0 and 1 (Press START, copyright) are copied from the vanilla ROM (`ROM_OBJ_PAL_OFF`, 0xAADB68). The OBJ
tiles at 0x06013000 (copyright, Press START) are vanilla.

## Screen positions

| Element | Size | Position (x, y) |
|---|---|---|
| Emblem (BG0) | 148x61 | (46, 1) |
| Logo | 156x24 | (42, 63) |
| Banner | 144x22 | (48, 88) |
| EXCEED | 118x14 | (61, 112) |
| Press START | vanilla sprite | y 129 (vanilla 124) |

Sprite slots are reused. The vanilla TM sprite call now draws EXCEED, because the TM is part of the new logo image.
The vanilla logo's translucent shine call is moved off screen (y 160).

## How the images are made

Tool: `Tools/titlescreen/title_gfx.py`. Source images are in `Kernel/Wizardry/TitleScreenGfx/src/`:
`title_logo.png` (RGBA, with the four elements) and `title_background.png` (RGB).

- Elements are cropped by fixed boxes and scaled to their width with Lanczos. Alpha >= 128 counts as opaque.
- Background: resized to 240x160 and reduced to 16 colours with Floyd-Steinberg dithering. The least used colour gives
  its pixels to the nearest remaining colour and becomes index 0 (transparent, so nothing shows through). The other 15
  are sorted dark to light.
- Emblem: 15 colours, index 0 transparent.
- Logo and EXCEED share one 15-colour palette (bank 2, also used by the zoom). The banner has its own (bank 3).
- Tiles are deduplicated. Text BGs may use flipped tiles; the zoom uses exact matches only.

## Changing the art

1. Replace a source image in `src/`, or change the crop boxes, widths or positions at the top of `title_gfx.py`.
2. Run `python3 Tools/titlescreen/title_gfx.py`.
3. Rebuild with `Tools/cloud/build.sh` (in a cloud session; see `Tools/cloud/README.md`).

The tool writes:

- `dmp/*`: the binaries the game loads.
- `gfx/*`: previews, not used by the game. `gfx/preview.png` is the whole screen. In single-layer previews, index 0
  shows as magenta.
- `TitleScreenGfx_Data.event`: generated, do not edit.

The tool asserts the tile limits above. Positions are set in the tool (they reach the installer through the
`TITLE_*` defines), except Press START's y, which is in the installer.

## Patched addresses

All in `TitleScreenGfx_Installer.event`.

| ROM offset | Function | What |
|---|---|---|
| 0x080C56F4 (`Title_SetupMainGraphics`) | BG1 background | Tiles A/B, map, palette (0xC5730, 0xC5780, 0xC5788, 0xC5790) |
| | BG0 emblem | Tiles, map, palette (0xC57E4, 0xC57EC, 0xC57F4) |
| | OBJ | Tiles and palettes 0-3 (0xC5808, 0xC5838) |
| 0x080C5440 (`DrawTitleSprites_Loop`) | Sprites | Logo position and layout (0xC5450, 0xC5452, 0xC54F4); EXCEED position, layout and attr2 (0xC5462, 0xC5464, 0xC54F8, 0xC54FC); shine moved off screen (0xC5500); banner (0xC5484, 0xC5486, 0xC5504, 0xC5508); Press START y (0xC5498) |
| 0x080C6204 (`Title_Loop_LightExplosionFx`) | Banner during the intro | Position, layout and attr2 (0xC6290, 0xC6292, 0xC62C4, 0xC62C8) |
| 0x080C5FE8 (`Title_PrepareMainLogoZoom`) | Intro zoom | Tiles, map, palette (0xC6070, 0xC6078, 0xC6080) |
| (intro effects) | Palette reloads | Background palette at 0xC5CC0, 0xC5F1C, 0xC62B4; emblem palette at 0xC62C0 |

## Testing

- Checked in the emulator on the idle title screen: date, emblem, logo, banner, EXCEED, the Press START blink and the
  copyright line all shown.
- The quick smoke test passed (`python3 Tools/cloud/smoketest.py --quick`).
- The animated intro (logo zoom) only plays after the opening sequence. It was checked from `gfx/zoom.png`, not in game.
