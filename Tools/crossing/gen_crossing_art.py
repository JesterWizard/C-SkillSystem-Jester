#!/usr/bin/env python3
"""gen_crossing_art.py [--check] [--preview PATH.png]

Draws the art of the district River Crossing minigame and writes it as C data to
Kernel/Wizardry/Minigames/Crossing/Source/CrossingArt.c and include/kernel/crossing-art.h. Nothing is read from image
files: every pixel is drawn here, so change the drawing code and run the script again instead of editing the output.

- The scene: one 240x160 background (15 x 10 cells of 16x16) in one 16-colour palette. From the top: a hedge (the status
  bar), the far bank with five docks, three river lanes, a grass median, three road lanes, and the start grass.
- The sprites, laid out for 1D OBJ mapping, in two OBJ palettes:
  FX  the raft pieces (left end, middle, right end), a splash (two frames), a spark, the dock flag and the heart.
  UI  the button icons (D-pad, B, START) and the timer meter colours (shared with the fishing art).
- The pop-up words (HOME!, CLEAR!, OUCH!, SPLASH!, GAME OVER), two 64x32 halves each, with their palettes. They are
  drawn with the fishing art's font (Tools/fishing/gen_fishing_art.py).
- The map flag: one 8x8 tile in the item icon palette 0, drawn on the district map at each crossing spot.

--check: exit 1 if the written files differ from what the script makes now.
--preview PATH: write a 3x preview of the scene with sprites drawn on it.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "Tools", "fishing"))

import gen_fishing_art as fa  # noqa: E402

OUT_C = os.path.join(ROOT, "Kernel/Wizardry/Minigames/Crossing/Source/CrossingArt.c")
OUT_H = os.path.join(ROOT, "include/kernel/crossing-art.h")

W, H = 240, 160
CELL = 16
DOCK_COLS = [1, 4, 7, 10, 13]

GEO = dict(
    CROSSING_COLS=15,
    CROSSING_ROWS=10,
    CROSSING_ROW_BANK=1,        # the docks
    CROSSING_ROW_RIVER_TOP=2,   # river rows 2..4
    CROSSING_ROW_RIVER_BOTTOM=4,
    CROSSING_ROW_MEDIAN=5,
    CROSSING_ROW_ROAD_TOP=6,    # road rows 6..8
    CROSSING_ROW_ROAD_BOTTOM=8,
    CROSSING_ROW_START=9,
)

SCENE_PAL = [
    (0, 0, 0),          # 0 unused
    (24, 64, 40),       # 1 hedge dark
    (48, 112, 56),      # 2 hedge light
    (88, 88, 104),      # 3 stone dark
    (152, 152, 168),    # 4 stone light
    (32, 88, 168),      # 5 water dark
    (56, 128, 208),     # 6 water mid
    (144, 208, 248),    # 7 water light
    (64, 144, 56),      # 8 grass dark
    (120, 200, 80),     # 9 grass light
    (208, 184, 128),    # 10 sand
    (104, 88, 80),      # 11 road dark
    (152, 136, 120),    # 12 road light
    (240, 240, 232),    # 13 white
    (168, 112, 64),     # 14 wood
    (96, 56, 32),       # 15 wood dark
]
HD, HL, SD, SL, WD, WM, WL, GD, GL, SA, RD, RL, WH, WO, WK = range(1, 16)

PAL_FX = [
    (0, 0, 0),          # 0 transparent
    (88, 48, 24),       # 1 wood dark
    (152, 96, 48),      # 2 wood mid
    (200, 144, 80),     # 3 wood light
    (224, 200, 136),    # 4 rope
    (248, 248, 248),    # 5 white
    (184, 232, 248),    # 6 splash light
    (96, 168, 232),     # 7 splash
    (248, 216, 64),     # 8 yellow
    (40, 40, 40),       # 9 outline
    (224, 48, 48),      # 10 red
    (120, 120, 128),    # 11 pole
    (232, 56, 88),      # 12 heart
    (144, 24, 48),      # 13 heart dark
    (255, 168, 184),    # 14 heart shine
    (32, 64, 120),      # 15 shadow
]


def new(w, h, c=0):
    return fa.new(w, h, c)


def put(img, x, y, c):
    fa.put(img, x, y, c)


def draw_scene():
    img = new(W, H)
    h = fa.hash2
    for y in range(H):
        row = y // CELL
        for x in range(W):
            if row == 0:
                c = HL if h(x, y) < 70 else HD
                if y >= 14:
                    c = HD
            elif row == 1:
                c = SL if (y - 16) % 8 != 0 and (x + (8 if (y - 16) // 8 else 0)) % 16 != 0 else SD
                if y >= 30:
                    c = SD
            elif 2 <= row <= 4:
                c = WM
                if y % 5 == 0 and (x + (y // 5) * 11) % 28 < 5:
                    c = WL
                elif (y + 2) % 5 == 0 and (x + (y // 5) * 11 + 14) % 28 < 3:
                    c = WD
            elif row == 5:
                c = GL if h(x, y) > 40 else GD
                if y <= 81:
                    c = SA
                if y >= 94:
                    c = SL
            elif 6 <= row <= 8:
                c = RL if h(x // 4, y // 4) & 1 else RD
                if (x % 8 == 0) or (y % 8 == 0):
                    c = RD
                if y in (111, 112, 127, 128) and (x // 8) % 2 == 0:
                    c = WH
                if y >= 142:
                    c = SL
            else:
                c = GL if h(x, y) > 50 else GD
                if 104 <= x < 136:
                    c = SA if h(x, y) > 30 else GL
            img[y][x] = c
    # Flowers on the median and the start grass
    for fx, fy in ((20, 87), (60, 90), (130, 86), (190, 89), (222, 85), (40, 150), (180, 152), (220, 148)):
        put(img, fx, fy, WH)
        put(img, fx + 1, fy, SA)
        put(img, fx, fy + 1, SA)
    # The docks: water alcoves cut into the bank, with posts
    for col in DOCK_COLS:
        x0 = col * CELL
        for y in range(18, 32):
            for x in range(x0 + 1, x0 + 15):
                img[y][x] = WM if (y + x // 5) % 6 else WL
        for y in range(16, 32):
            img[y][x0] = WK
            img[y][x0 + 15] = WK
        for x in range(x0, x0 + 16):
            img[16][x] = WO
            img[17][x] = WK
    # Shore lines on both sides of the river
    for x in range(W):
        img[32][x] = WD
        img[79][x] = WD
    return img


def raft(piece):
    img = new(16, 16)
    for y in range(3, 14):
        for x in range(16):
            if piece == 0 and x < 2:
                continue
            if piece == 2 and x > 13:
                continue
            c = 2
            if y in (3, 13):
                c = 1
            elif (y - 3) % 4 == 0:
                c = 1
            elif y in (4, 8):
                c = 3
            img[y][x] = c
    if piece == 0:
        for y in range(3, 14):
            img[y][2] = 1
    if piece == 2:
        for y in range(3, 14):
            img[y][13] = 1
    rope_x = {0: (5,), 1: (8,), 2: (10,)}[piece]
    for rx in rope_x:
        for y in range(3, 14):
            img[y][rx] = 4
    for x in range(16):
        if img[14 - 0][x] == 0 and img[13][x]:
            img[14][x] = 15
    return img


def splash(frame):
    img = new(16, 16)
    if frame == 0:
        fa.ellipse(img, 7.5, 10, 6, 3, 7)
        fa.ellipse(img, 7.5, 10, 4, 2, 6)
        for x, y in ((3, 5), (7, 3), (12, 5), (5, 7), (10, 7)):
            put(img, x, y, 5)
    else:
        fa.ellipse(img, 7.5, 11, 7, 3, 6)
        fa.ellipse(img, 7.5, 11, 5, 1.6, 7)
        for x, y in ((1, 8), (14, 8), (4, 6), (11, 6)):
            put(img, x, y, 5)
    return img


def flag():
    img = new(16, 16)
    for y in range(2, 15):
        put(img, 5, y, 11)
    for y in range(2, 8):
        for x in range(6, 6 + (8 - abs(y - 5) * 2 if y != 5 else 8)):
            put(img, x, y, 10)
    put(img, 5, 1, 8)
    for x in range(3, 8):
        put(img, x, 15, 1)
    return img


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


def dpad_all():
    img = fa.dpad(True)
    side = fa.dpad(False)
    for y in range(16):
        for x in range(16):
            if side[y][x] == 2 or side[y][x] == 4:
                img[y][x] = side[y][x]
    return img


def map_flag():
    """8x8 for the district map, in item icon palette 0 (as the fishing bobber): 4 red, 1 white, 15 dark."""
    img = new(8, 8)
    for y in range(0, 8):
        put(img, 1, y, 0xF)
    for y in range(0, 4):
        for x in range(2, 7 - abs(y - 1) if y != 1 else 8):
            put(img, x, y, 0x4)
    put(img, 3, 1, 0x1)
    return img


SPRITES = [
    ("RAFT", "FX", [raft(0), raft(1), raft(2)]),
    ("SPLASH", "FX", [splash(0), splash(1)]),
    ("SPARK", "FX", [fa.spark(0), fa.spark(1)]),
    ("FLAG", "FX", [flag()]),
    ("HEART", "FX", [heart()]),
    ("BTN_DPAD", "UI", [dpad_all()]),
    ("BTN_A", "UI", [fa.button_round("A", 6)]),
    ("BTN_B", "UI", [fa.button_round("B", 7)]),
    ("BTN_START", "UI", [fa.button_start()]),
]
POP_WORDS = ["HOME!", "CLEAR!", "OUCH!", "SPLASH!", "GAME OVER"]
POP_PALS = dict(fa.POP_PALS)
POP_PALS["RED"] = [(64, 8, 8), (0, 0, 24), (255, 176, 160), (240, 88, 72), (192, 32, 32), (255, 255, 255)]
POP_WORD_PAL = ["GREEN", "GOLD", "ORANGE", "BLUE", "RED"]


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
        offsets[name + "_FRAME"] = len(fa.to_tiles(frames[0]))
    pops = b"".join(b"".join(fa.to_tiles(half)) for w in POP_WORDS for half in fa.pop_halves(w))
    pal_names = list(POP_PALS)
    pop_pals = []
    for name in pal_names:
        pop_pals += fa.pal_words([(0, 0, 0)] + POP_PALS[name] + [(0, 0, 0)] * 9)
    mf = fa.to_tiles(map_flag())[0]

    h = ["/* Generated by Tools/crossing/gen_crossing_art.py. Do not edit. */", "#pragma once", "",
         "/* The field: 16x16 cells */"]
    for k, v in GEO.items():
        h.append("#define %s %d" % (k, v))
    h += ["", "/* The dock columns, left to right */",
          "#define CROSSING_DOCKS %d" % len(DOCK_COLS),
          "#define CROSSING_DOCK_COLS { %s }" % ", ".join(str(c) for c in DOCK_COLS), "",
          "#define CROSSING_SCENE_TILES %d" % len(tiles), "#define CROSSING_OBJ_TILES %d" % len(obj), "",
          "/* OBJ tile offsets (1D mapping) and tiles per frame */"]
    for name, _, _ in SPRITES:
        h.append("#define CROSSING_CHR_%s %d" % (name, offsets[name]))
        h.append("#define CROSSING_CHR_%s_FRAME %d" % (name, offsets[name + "_FRAME"]))
    h += ["", "extern const u8 gCrossingSceneTiles[CROSSING_SCENE_TILES * 32];",
          "extern const u16 gCrossingSceneMap[30 * 20];", "extern const u16 gCrossingScenePal[16];",
          "extern const u8 gCrossingObjTiles[CROSSING_OBJ_TILES * 32];", "extern const u16 gCrossingObjPalFx[16];",
          "extern const u16 gCrossingObjPalUi[16];", "extern const u8 gCrossingMapFlagTile[32];", "",
          "/* Pop-up words: two 64x32 halves (64 tiles) each, copied into OBJ VRAM when shown */",
          "enum { %s, CROSSING_POP_WORDS };" % ", ".join("CROSSING_POP_" + w.strip("!").replace(" ", "_")
                                                         for w in POP_WORDS),
          "#define CROSSING_POP_TILES 64",
          "extern const u8 gCrossingPopTiles[CROSSING_POP_WORDS * CROSSING_POP_TILES * 32];",
          "extern const u16 gCrossingPopPals[%d * 16];" % len(pal_names),
          "extern const u8 gCrossingPopWordPal[CROSSING_POP_WORDS];", ""]

    c = ["/* Generated by Tools/crossing/gen_crossing_art.py. Do not edit: change the script and run it again. */",
         '#include "common-chax.h"', '#include "crossing-art.h"', ""]
    c.append(fa.c_bytes("gCrossingSceneTiles", b"".join(tiles), "Scene tiles (4bpp, %d unique)" % len(tiles)))
    c.append(fa.c_u16("gCrossingSceneMap", tmap, "Scene map, 30 x 20 tile indices (no palette bits)", 30))
    c.append(fa.c_u16("gCrossingScenePal", fa.pal_words(SCENE_PAL), "Scene palette"))
    c.append(fa.c_bytes("gCrossingObjTiles", b"".join(obj), "Sprite tiles (4bpp, 1D order)"))
    c.append(fa.c_u16("gCrossingObjPalFx", fa.pal_words(PAL_FX), "Sprite palette: rafts, splash, spark, flag, heart"))
    c.append(fa.c_u16("gCrossingObjPalUi", fa.pal_words(fa.PAL_UI), "Sprite palette: buttons and the timer meter"))
    c.append(fa.c_bytes("gCrossingMapFlagTile", mf, "District map flag (item icon palette 0)"))
    c.append(fa.c_bytes("gCrossingPopTiles", pops, "Pop-up words %s, two 64x32 halves each" % ", ".join(POP_WORDS)))
    c.append(fa.c_u16("gCrossingPopPals", pop_pals, "Pop-up palettes: %s" % ", ".join(pal_names)))
    c.append(fa.c_bytes("gCrossingPopWordPal", bytes(pal_names.index(n) for n in POP_WORD_PAL), "Each word's palette"))
    return "\n".join(c).rstrip() + "\n", "\n".join(h) + "\n", scene


def preview(path, scene):
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
    sp = {n: f for n, _, f in SPRITES}
    for i, piece in enumerate((0, 1, 1, 2)):
        blit(sp["RAFT"][piece], 32 + i * 16, 48, PAL_FX)
    for i, piece in enumerate((0, 2)):
        blit(sp["RAFT"][piece], 150 + i * 16, 64, PAL_FX)
    blit(sp["FLAG"][0], 4 * 16, 16, PAL_FX)
    blit(sp["SPLASH"][0], 120, 32, PAL_FX)
    for i in range(3):
        blit(sp["HEART"][0], 160 + i * 10, 4, PAL_FX)
    blit(sp["BTN_DPAD"][0], 8, 144, fa.PAL_UI)
    blit(sp["BTN_B"][0], 60, 144, fa.PAL_UI)
    blit(sp["BTN_START"][0], 100, 144, fa.PAL_UI)
    im.resize((W * 3, H * 3), Image.NEAREST).save(path)


def main():
    c, h, scene = build()
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
        preview(p, scene)
        print("preview", p)


if __name__ == "__main__":
    main()
