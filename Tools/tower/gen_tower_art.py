#!/usr/bin/env python3
"""gen_tower_art.py [--check] [--preview PATH.png]

Draws the art of the district Tower Stack minigame and writes it as C data to
Kernel/Wizardry/Minigames/Tower/Source/TowerArt.c and include/kernel/tower-art.h. Nothing is read from image files: every
pixel is drawn here, so change the drawing code and run the script again instead of editing the output.

- The scene: one 240 x 256 background (30 x 32 tiles) in one 16-colour palette: sky (darker high up), clouds, a far town
  and hills, and the ground with a stone plinth for the tower. The screen shows 160 rows of it and scrolls up as the
  tower grows; above the sky's top the background stays put.
- The sprites, laid out for 1D OBJ mapping:
  BLOCK  five block styles (granite, sandstone, brick, mossy, gold), each 48x16 as a 32x16 and a 16x16 sprite.
  FX     sparks, hearts.
  UI     the button icons (A, B, START) and the timer meter colours (the fishing art's).
  DIGIT  the big digits 0-9, "+" and "x" (16x16 each) for the score pop-ups, drawn like the pop-up words.
- The pop-up words (PERFECT!, FEVER!, MISS!, GAME OVER), two 64x32 halves each.
- The chain palettes: ten palettes for the pop-ups that run from green (a chain of 1) to orange to gold (a chain of 10),
  and a white one for a plain landing.
- The map tower: one 8x8 tile in the item icon palette 0, drawn on the district map at each Tower Stack spot.

--check: exit 1 if the written files differ from what the script makes now.
--preview PATH: write a 3x preview of the scene with sprites drawn on it.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "Tools", "fishing"))

import gen_fishing_art as fa  # noqa: E402

OUT_C = os.path.join(ROOT, "Kernel/Wizardry/Minigames/Tower/Source/TowerArt.c")
OUT_H = os.path.join(ROOT, "include/kernel/tower-art.h")

W, H = 240, 256
GROUND_Y = 192          # first ground row of the scene
PLINTH_X0, PLINTH_X1 = 88, 152

GEO = dict(
    TOWER_SCENE_H=H,
    TOWER_GROUND_Y=GROUND_Y,    # the tower's base stands on this row of the scene
    TOWER_BLOCK_W=48,
    TOWER_BLOCK_H=16,
    TOWER_BLOCK_STYLES=4,       # plus the gold style, used during a fever
)

SCENE_PAL = [
    (0, 0, 0),          # 0 unused
    (40, 72, 168),      # 1 sky top
    (64, 112, 208),     # 2 sky high
    (104, 160, 240),    # 3 sky mid
    (160, 208, 248),    # 4 sky low
    (248, 248, 248),    # 5 cloud
    (200, 216, 240),    # 6 cloud shade
    (96, 120, 168),     # 7 far hills
    (72, 96, 136),      # 8 far town
    (120, 200, 80),     # 9 grass light
    (64, 144, 56),      # 10 grass dark
    (152, 112, 72),     # 11 earth
    (104, 72, 48),      # 12 earth dark
    (176, 176, 184),    # 13 stone light
    (120, 120, 136),    # 14 stone dark
    (232, 200, 120),    # 15 window light
]

PAL_BLOCK = [
    (0, 0, 0),          # 0 transparent
    (40, 40, 48),       # 1 outline
    (176, 176, 184),    # 2 granite light
    (128, 128, 144),    # 3 granite
    (88, 88, 104),      # 4 granite dark
    (232, 208, 144),    # 5 sandstone light
    (200, 168, 104),    # 6 sandstone
    (200, 88, 64),      # 7 brick
    (144, 56, 40),      # 8 brick dark
    (232, 216, 200),    # 9 mortar
    (96, 160, 72),      # 10 moss
    (255, 240, 136),    # 11 gold light
    (240, 192, 48),     # 12 gold
    (184, 128, 24),     # 13 gold dark
    (255, 255, 255),    # 14 shine
    (152, 128, 80),     # 15 sandstone dark
]

PAL_FX = [
    (0, 0, 0), (40, 40, 40), (0, 0, 0), (248, 248, 248), (224, 48, 40), (0, 0, 0), (0, 0, 0), (40, 40, 40),
    (248, 216, 64), (0, 0, 0), (0, 0, 0), (0, 0, 0), (232, 56, 88), (144, 24, 48), (255, 168, 184), (0, 0, 0),
]


def new(w, h, c=0):
    return fa.new(w, h, c)


def put(img, x, y, c):
    fa.put(img, x, y, c)


def draw_scene():
    img = new(W, H)
    h = fa.hash2
    bands = [(0, 1), (56, 2), (112, 3), (168, 4)]
    for y in range(H):
        for x in range(W):
            c = 1
            for y0, bc in bands:
                if y >= y0:
                    c = bc
                elif y >= y0 - 4 and (x + y) % 2 == 0:
                    c = bc
            img[y][x] = c
    for cx, cy, s in ((40, 40, 1.0), (180, 70, 0.8), (90, 120, 0.7), (200, 150, 0.6), (30, 160, 0.5)):
        for dx, dy, rx, ry in ((0, 0, 16, 6), (-11, 2, 10, 5), (11, 2, 11, 5), (3, -5, 9, 5)):
            fa.ellipse(img, cx + dx * s, cy + dy * s, rx * s, ry * s, 5)
        fa.ellipse(img, cx, cy + 4 * s, 14 * s, 2 * s, 6)
    # Far hills and a far town
    for x in range(W):
        top = 172 + int(6 * ((x * 5 % 41) / 41.0)) - (8 if 150 < x < 220 else 0)
        for y in range(top, GROUND_Y):
            img[y][x] = 7
    for bx, bw, bh in ((14, 14, 14), (30, 10, 20), (44, 18, 10), (176, 12, 22), (190, 16, 12), (208, 10, 26)):
        for y in range(GROUND_Y - bh, GROUND_Y):
            for x in range(bx, bx + bw):
                img[y][x] = 8
        for x in range(bx, bx + bw):
            if (x - bx) % 4 == 1:
                put(img, x, GROUND_Y - bh + 4, 15)
    # The ground
    for y in range(GROUND_Y, H):
        for x in range(W):
            d = y - GROUND_Y
            if d < 3:
                c = 9
            elif d < 6:
                c = 10 if h(x, y) & 1 else 9
            else:
                c = 11 if h(x, y) > 40 else 12
            img[y][x] = c
    # The stone plinth the tower stands on
    for y in range(GROUND_Y - 2, GROUND_Y + 10):
        for x in range(PLINTH_X0, PLINTH_X1):
            img[y][x] = 13 if (y - GROUND_Y + 2) < 3 else (14 if (x + y // 4) % 12 == 0 else 13)
            if x in (PLINTH_X0, PLINTH_X1 - 1) or y == GROUND_Y + 9:
                img[y][x] = 14
    return img


def block(style):
    """A 48x16 block: an outline, a top highlight, and a face per style."""
    img = new(48, 16)
    face = {0: (3, 2, 4), 1: (6, 5, 15), 2: (7, 9, 8), 3: (3, 2, 4), 4: (12, 11, 13)}[style]
    for y in range(16):
        for x in range(48):
            c = face[0]
            if y <= 2:
                c = face[1]
            elif y >= 13:
                c = face[2]
            if style == 2:  # bricks with mortar
                course = (y - 1) // 5
                if (y - 1) % 5 == 4 or ((x + (course % 2) * 8) % 16 == 0 and y > 1):
                    c = 9
            elif style in (0, 3):  # granite: speckles and a seam
                if fa.hash2(x, y + style * 31) < 24:
                    c = face[2]
                if x in (16, 32) and y > 2:
                    c = face[2]
            elif style == 1:  # sandstone: horizontal strata
                if y in (6, 10) and x % 7 != 3:
                    c = face[2]
            elif style == 4:  # gold: a bevel and shines
                if (x + y) % 13 == 0 and 3 < y < 12:
                    c = 14
            if x == 0 or x == 47 or y == 0 or y == 15:
                c = 1
            img[y][x] = c
    if style == 3:  # moss on top
        for x in range(1, 47):
            if fa.hash2(x, 7) < 140:
                put(img, x, 1, 10)
                if fa.hash2(x, 9) < 70:
                    put(img, x, 2, 10)
    return img


def halves(img):
    """48x16 -> a 32x16 and a 16x16"""
    return [[row[:32] for row in img], [row[32:] for row in img]]


def big_glyph(ch):
    """A 16x16 glyph of the pop-up font: colours 1 outline, 2 shadow, 3..5 fill, 6 shine (as fa.pop_word)."""
    g = fa.BIG[ch]
    fill = new(16, 16)
    w = len(g[0]) * 2
    x0 = (16 - w) // 2
    y0 = 1
    for gy, row in enumerate(g):
        for gx, b in enumerate(row):
            if b == "1":
                for dy in range(2):
                    for dx in range(2):
                        fill[y0 + gy * 2 + dy][x0 + gx * 2 + dx] = 1
    img = new(16, 16)
    for y in range(16):
        for x in range(16):
            if fill[y][x]:
                img[y][x] = 3 if y < y0 + 5 else (4 if y < y0 + 10 else 5)
                if y in (y0, y0 + 1) and x % 3 == 0:
                    img[y][x] = 6
    out = [row[:] for row in img]
    for y in range(16):
        for x in range(16):
            if fill[y][x]:
                continue
            if any(0 <= y + dy < 16 and 0 <= x + dx < 16 and fill[y + dy][x + dx]
                   for dy in (-1, 0, 1) for dx in (-1, 0, 1)):
                out[y][x] = 1
    return out


def pip(kind):
    """An 8x8 chain-meter pip in the BLOCK palette: 0 unlit, 1 green, 2 orange, 3 gold."""
    img = new(8, 8)
    fill = {0: 4, 1: 10, 2: 7, 3: 12}[kind]
    light = {0: 3, 1: 10, 2: 9, 3: 11}[kind]
    fa.ellipse(img, 3.5, 3.5, 3.2, 3.2, 1)
    fa.ellipse(img, 3.5, 3.5, 2.2, 2.2, fill)
    put(img, 2, 2, light if kind else 3)
    if kind:
        put(img, 3, 2, 14)
    return img


def map_tower():
    """8x8 for the district map, in item icon palette 0: 1 white, 4 red, 15 dark."""
    img = new(8, 8)
    rows = ["00044000", "00444400", "0FFFFFF0", "0F1111F0", "0F1F11F0", "0F1111F0", "0F11F1F0", "0FFFFFF0"]
    for y, r in enumerate(rows):
        for x, ch in enumerate(r):
            if ch != "0":
                img[y][x] = int(ch, 16)
    return img


def lerp(a, b, t):
    return tuple(int(round(a[i] + (b[i] - a[i]) * t)) for i in range(3))


def chain_pals():
    """Palettes for chains of 1..10: green, through orange (at 5-6), to gold; then a white one for a plain landing."""
    keys = [fa.POP_PALS["GREEN"], fa.POP_PALS["ORANGE"], fa.POP_PALS["GOLD"]]
    pals = []
    for n in range(10):
        t = n / 9.0 * 2
        k = min(1, int(t))
        f = t - k
        cols = [lerp(keys[k][i], keys[k + 1][i], f) for i in range(6)]
        pals.append([(0, 0, 0)] + cols + [(0, 0, 0)] * 9)
    white = [(48, 48, 64), (0, 0, 24), (255, 255, 255), (232, 232, 240), (200, 200, 216), (255, 255, 255)]
    pals.append([(0, 0, 0)] + white + [(0, 0, 0)] * 9)
    return pals


DIGITS = "0123456789+x"
POP_WORDS = ["PERFECT!", "FEVER!", "MISS!", "GAME OVER", "NEW BEST!", "FLOORS!"]
POP_WORD_PAL = [-1, 9, 10, 10]   # -1: the chain's palette; 9: gold; 10: white (MISS! and GAME OVER use the red one below)

SPRITES = [  # name, palette, frames (each frame one sprite)
    ("BLOCK", "BLOCK", [half for s in range(5) for half in halves(block(s))]),
    ("SPARK", "FX", [fa.spark(0), fa.spark(1)]),
    ("HEART", "FX", [fa.heart()] if hasattr(fa, "heart") else []),
    ("BTN_A", "UI", [fa.button_round("A", 6)]),
    ("BTN_B", "UI", [fa.button_round("B", 7)]),
    ("BTN_START", "UI", [fa.button_start()]),
    ("DIGIT", "POP", [big_glyph(c) for c in DIGITS]),
    ("PIP", "BLOCK", [pip(k) for k in range(4)]),
]


def heart():
    img = new(8, 8)
    for y, row in enumerate(["01100110", "11111111", "11111111", "11111111", "01111110", "00111100", "00011000",
                             "00000000"]):
        for x, b in enumerate(row):
            if b == "1":
                img[y][x] = 12
    put(img, 1, 1, 14)
    put(img, 2, 1, 14)
    for x, y in ((0, 1), (7, 1), (1, 0), (2, 0), (5, 0), (6, 0), (0, 2), (7, 2), (0, 3), (7, 3), (1, 4), (6, 4),
                 (2, 5), (5, 5), (3, 6), (4, 6)):
        if img[y][x]:
            img[y][x] = 13
    return img


SPRITES[2] = ("HEART", "FX", [heart()])


def build():
    scene = draw_scene()
    tiles, index, tmap = [], {}, []
    for t in fa.to_tiles(scene):
        if t not in index:
            index[t] = len(tiles)
            tiles.append(t)
        tmap.append(index[t])
    obj, offsets = [], {}
    for name, _, frames in SPRITES:
        offsets[name] = len(obj)
        for f in frames:
            obj.extend(fa.to_tiles(f))
    # Block sprites: style s is a 32x16 (8 tiles) then a 16x16 (4 tiles)
    pops = b"".join(b"".join(fa.to_tiles(half)) for w in POP_WORDS for half in fa.pop_halves(w))
    pals = chain_pals()
    red = [(0, 0, 0)] + [(64, 8, 8), (0, 0, 24), (255, 176, 160), (240, 88, 72), (192, 32, 32),
                         (255, 255, 255)] + [(0, 0, 0)] * 9
    pals.append(red)
    pals.append([(0, 0, 0)] + fa.POP_PALS["BLUE"] + [(0, 0, 0)] * 9)
    pop_pals = []
    for p in pals:
        pop_pals += fa.pal_words(p)
    mt = fa.to_tiles(map_tower())[0]

    h = ["/* Generated by Tools/tower/gen_tower_art.py. Do not edit. */", "#pragma once", ""]
    for k, v in GEO.items():
        h.append("#define %s %d" % (k, v))
    h += ["", "#define TOWER_SCENE_TILES %d" % len(tiles), "#define TOWER_OBJ_TILES %d" % len(obj), "",
          "/* OBJ tile offsets (1D mapping). A block style is 12 tiles: a 32x16 (8 tiles) then a 16x16 (4 tiles). */"]
    for name, _, _ in SPRITES:
        h.append("#define TOWER_CHR_%s %d" % (name, offsets[name]))
    h += ["#define TOWER_CHR_BLOCK_STYLE 12", "#define TOWER_CHR_DIGIT_PLUS (TOWER_CHR_DIGIT + 10 * 4)",
          "#define TOWER_CHR_DIGIT_X (TOWER_CHR_DIGIT + 11 * 4)", "",
          "/* Pop-up palettes: chains of 1..10 (0..9), then white (10), red (11) and blue (12) */",
          "#define TOWER_PAL_CHAIN(n) ((n) - 1)", "#define TOWER_PAL_WHITE 10", "#define TOWER_PAL_RED 11",
          "#define TOWER_PAL_BLUE 12",
          "#define TOWER_POP_PALS %d" % len(pals), "",
          "extern const u8 gTowerSceneTiles[TOWER_SCENE_TILES * 32];",
          "extern const u16 gTowerSceneMap[30 * 32];", "extern const u16 gTowerScenePal[16];",
          "extern const u8 gTowerObjTiles[TOWER_OBJ_TILES * 32];", "extern const u16 gTowerObjPalBlock[16];",
          "extern const u16 gTowerObjPalFx[16];", "extern const u16 gTowerObjPalUi[16];",
          "extern const u8 gTowerMapTile[32];", "",
          "/* Pop-up words: two 64x32 halves (64 tiles) each, copied into OBJ VRAM when shown */",
          "enum { %s, TOWER_POP_WORDS };" % ", ".join("TOWER_POP_" + w.strip("!").replace(" ", "_") for w in POP_WORDS),
          "#define TOWER_POP_TILES 64",
          "extern const u8 gTowerPopTiles[TOWER_POP_WORDS * TOWER_POP_TILES * 32];",
          "extern const u16 gTowerPopPals[TOWER_POP_PALS * 16];", ""]

    c = ["/* Generated by Tools/tower/gen_tower_art.py. Do not edit: change the script and run it again. */",
         '#include "common-chax.h"', '#include "tower-art.h"', ""]
    c.append(fa.c_bytes("gTowerSceneTiles", b"".join(tiles), "Scene tiles (4bpp, %d unique)" % len(tiles)))
    c.append(fa.c_u16("gTowerSceneMap", tmap, "Scene map, 30 x 32 tile indices (no palette bits)", 30))
    c.append(fa.c_u16("gTowerScenePal", fa.pal_words(SCENE_PAL), "Scene palette"))
    c.append(fa.c_bytes("gTowerObjTiles", b"".join(obj), "Sprite tiles (4bpp, 1D order)"))
    c.append(fa.c_u16("gTowerObjPalBlock", fa.pal_words(PAL_BLOCK), "Sprite palette: the blocks"))
    c.append(fa.c_u16("gTowerObjPalFx", fa.pal_words(PAL_FX), "Sprite palette: sparks and hearts"))
    c.append(fa.c_u16("gTowerObjPalUi", fa.pal_words(fa.PAL_UI), "Sprite palette: buttons and the timer meter"))
    c.append(fa.c_bytes("gTowerMapTile", mt, "District map tower (item icon palette 0)"))
    c.append(fa.c_bytes("gTowerPopTiles", pops, "Pop-up words %s, two 64x32 halves each" % ", ".join(POP_WORDS)))
    c.append(fa.c_u16("gTowerPopPals", pop_pals, "Pop-up palettes: chains 1..10 (green to orange to gold), white, red, blue"))
    return "\n".join(c).rstrip() + "\n", "\n".join(h) + "\n", scene, pals


def preview(path, scene, pals):
    from PIL import Image
    im = Image.new("RGB", (W, H))
    px = im.load()
    for y in range(H):
        for x in range(W):
            px[x, y] = SCENE_PAL[scene[y][x]]

    def blit(img, x0, y0, pal):
        for y, row in enumerate(img):
            for x, v in enumerate(row):
                if v and 0 <= x0 + x < W and 0 <= y0 + y < H:
                    px[x0 + x, y0 + y] = pal[v]
    for i in range(6):
        blit(block(i % 4 if i < 5 else 4), 96 + (i % 2) * 4 - 2, GROUND_Y - 16 * (i + 1), PAL_BLOCK)
    for n, ch in enumerate("x10"):
        blit(big_glyph(ch), 150 + n * 12, 60, pals[9])
    for n in range(10):
        blit(big_glyph(str(n)), 4 + n * 16, 210, pals[n])
    blit(fa.pop_word("PERFECT!"), 56, 20, pals[4])
    im.resize((W * 3, H * 3), Image.NEAREST).save(path)


def main():
    c, h, scene, pals = build()
    if "--check" in sys.argv:
        ok = os.path.exists(OUT_C) and open(OUT_C).read() == c and os.path.exists(OUT_H) and open(OUT_H).read() == h
        print("up to date" if ok else "OUT OF DATE")
        sys.exit(0 if ok else 1)
    os.makedirs(os.path.dirname(OUT_C), exist_ok=True)
    open(OUT_C, "w").write(c)
    open(OUT_H, "w").write(h)
    print("wrote", os.path.relpath(OUT_C, ROOT), "and", os.path.relpath(OUT_H, ROOT))
    if "--preview" in sys.argv:
        p = sys.argv[sys.argv.index("--preview") + 1]
        preview(p, scene, pals)
        print("preview", p)


if __name__ == "__main__":
    main()
