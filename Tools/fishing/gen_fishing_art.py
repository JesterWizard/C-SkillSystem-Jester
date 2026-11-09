#!/usr/bin/env python3
"""gen_fishing_art.py [--check] [--preview PATH.png]

Draws the art of the district fishing screen and writes it as C data to
Kernel/Wizardry/Minigames/Fishing/Source/FishingArt.c and include/kernel/fishing-art.h. Nothing is read from image files:
every pixel is drawn here, so change the drawing code and run the script again instead of editing the output by hand.

- The scene: one 240 x 448 background (30 x 56 tiles) in one 16-colour palette. The screen shows 160 rows of it and
  scrolls down with the hook. From the top: sky (under the text window), clouds, the far shore, the grassy bank on the
  right (the unit stands on it), the water in five depth bands, and a sandy bed with weeds and rocks at the bottom.
  Tiles are de-duplicated (no flips).
- The sprites, laid out for 1D OBJ mapping, in three OBJ palettes:
  FX   the fish shadows (small 16x8, medium 32x16, large and huge 64x32; two tail frames each, facing right), the rod
       (64x32), the bobber (two frames), the hook, an 8x8 line piece, the "!", a splash and the three bait icons.
  UI   the button icons (A, B, START, D-pad up/down and left/right) and the meter icons.
  GOLD the golden fish (32x16, two frames).
- The map bobber: one 8x8 tile in the item icon palette 0 (used on the district map at the fishing spots).
- The species of the fish journal (fish_species.py): a 64x32 portrait (the catch card) and a 32x16 icon (the journal)
  for each, with its own palette, copied into OBJ VRAM when shown; and the silhouette palette of an uncaught species.

--check: exit 1 if the written files differ from what the script makes now.
--preview PATH: write a 2x preview of the whole scene with sprites drawn on it.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fish_species  # noqa: E402

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT_C = os.path.join(ROOT, "Kernel/Wizardry/Minigames/Fishing/Source/FishingArt.c")
OUT_H = os.path.join(ROOT, "include/kernel/fishing-art.h")

W, H = 240, 448

# ---- Geometry (also written to fishing-art.h) ----
GEO = dict(
    FISHING_WORLD_H=H,          # rows of the scene
    FISHING_WINDOW_H=32,        # the text window covers the top 32 rows of the screen
    FISHING_SURFACE_Y=96,       # first water row
    FISHING_WATER_RIGHT=176,    # first column of the bank
    FISHING_BANK_TOP=88,        # first grass row of the bank (the unit's feet)
    FISHING_BED_Y=424,          # first sand row
    FISHING_ROD_TIP_X=148,      # the line hangs from here
    FISHING_ROD_TIP_Y=62,
    FISHING_UNIT_X=188,         # left edge of the unit's 32x32 sprite when it stands on the bank
    FISHING_HAND_X=192,         # the rod turns about the unit's hand
    FISHING_HAND_Y=74,
    FISHING_ROD_LEN=44,         # hand to tip, in pixels
    FISHING_ROD_ART_ANGLE=32,   # the rod's angle in its art (256 to a turn, up and to the left)
)
G = type("G", (), {k.replace("FISHING_", ""): v for k, v in GEO.items()})

# ---- Scene palette (BG) ----
SCENE_PAL = [
    (0, 0, 0),          # 0 unused
    (112, 176, 248),    # 1 sky high
    (168, 216, 248),    # 2 sky low
    (248, 248, 248),    # 3 cloud
    (128, 208, 80),     # 4 grass light
    (64, 144, 56),      # 5 grass dark
    (192, 144, 88),     # 6 dirt light
    (144, 96, 56),      # 7 dirt mid
    (96, 64, 40),       # 8 dirt dark
    (176, 232, 248),    # 9 water surface
    (72, 152, 216),     # 10 water 1
    (48, 112, 184),     # 11 water 2
    (32, 80, 144),      # 12 water 3
    (24, 56, 104),      # 13 water 4
    (208, 184, 128),    # 14 sand
    (16, 32, 72),       # 15 water 5 (deepest)
]
SKY_HI, SKY_LO, CLOUD, GR_L, GR_D, DI_L, DI_M, DI_D, W_S, W1, W2, W3, W4, SAND, W5 = range(1, 16)

# ---- OBJ palettes ----
PAL_FX = [
    (0, 0, 0),          # 0 transparent
    (8, 16, 40),        # 1 shadow
    (24, 40, 72),       # 2 shadow edge
    (248, 248, 248),    # 3 white
    (224, 48, 40),      # 4 red
    (232, 232, 224),    # 5 line
    (168, 168, 176),    # 6 hook
    (40, 40, 40),       # 7 dark outline
    (248, 216, 64),     # 8 yellow
    (112, 72, 32),      # 9 rod
    (176, 128, 72),     # 10 rod light
    (184, 232, 248),    # 11 splash light
    (120, 192, 240),    # 12 splash
    (240, 136, 168),    # 13 worm pink
    (248, 152, 64),     # 14 shrimp orange
    (232, 184, 48),     # 15 gold
]
PAL_UI = [
    (0, 0, 0),          # 0 transparent
    (32, 32, 40),       # 1 outline
    (232, 232, 232),    # 2 button face
    (168, 168, 176),    # 3 button shade
    (48, 48, 56),       # 4 letter
    (248, 248, 248),    # 5 white
    (216, 64, 56),      # 6 A red
    (232, 192, 48),     # 7 B yellow
    (96, 104, 120),     # 8 start grey
    (72, 200, 88),      # 9 green
    (80, 136, 232),     # 10 blue
    (232, 88, 64),      # 11 red
    (40, 40, 64),       # 12 meter back
    (200, 200, 216),    # 13 meter frame
    (248, 216, 64),     # 14 yellow
    (24, 112, 48),      # 15 green dark
]
PAL_GOLD = [
    (0, 0, 0),          # 0 transparent
    (152, 96, 16),      # 1 gold dark
    (216, 160, 32),     # 2 gold
    (248, 216, 72),     # 3 gold light
    (255, 248, 200),    # 4 shine
    (96, 56, 8),        # 5 outline
    (40, 24, 8),        # 6 eye
] + [(0, 0, 0)] * 9


def new(w, h, c=0):
    return [[c] * w for _ in range(h)]


def put(img, x, y, c):
    if 0 <= y < len(img) and 0 <= x < len(img[0]):
        img[y][x] = c


def line(img, x0, y0, x1, y1, c, thick=1):
    dx, dy = abs(x1 - x0), -abs(y1 - y0)
    sx, sy = (1 if x0 < x1 else -1), (1 if y0 < y1 else -1)
    err = dx + dy
    while True:
        for t in range(thick):
            put(img, x0, y0 + t, c)
        if x0 == x1 and y0 == y1:
            break
        e2 = 2 * err
        if e2 >= dy:
            err += dy
            x0 += sx
        if e2 <= dx:
            err += dx
            y0 += sy


def ellipse(img, cx, cy, rx, ry, c):
    for y in range(int(cy - ry) - 1, int(cy + ry) + 2):
        for x in range(int(cx - rx) - 1, int(cx + rx) + 2):
            if ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2 <= 1.0:
                put(img, x, y, c)


def hash2(x, y):
    v = (x * 374761393 + y * 668265263) & 0xFFFFFFFF
    v = ((v ^ (v >> 13)) * 1274126177) & 0xFFFFFFFF
    return (v ^ (v >> 16)) & 0xFF


# ---- The scene ----
def draw_scene():
    img = new(W, H)
    for y in range(G.SURFACE_Y):
        for x in range(W):
            if y < 64:
                c = SKY_HI
            elif y < 68:
                c = SKY_LO if (x + y) % 2 == 0 else SKY_HI
            else:
                c = SKY_LO
            img[y][x] = c
    for cx, cy, s in ((36, 58, 1.0), (100, 52, 0.8), (150, 70, 0.6)):
        for dx, dy, rx, ry in ((0, 0, 14, 5), (-10, 2, 9, 4), (10, 2, 10, 4), (3, -4, 8, 4)):
            ellipse(img, cx + dx * s, cy + dy * s, rx * s, ry * s, CLOUD)
    for x in range(G.WATER_RIGHT):
        top = 84 + int(4 * ((x * 7 % 37) / 37.0)) - (6 if 30 < x < 90 else 0) + (2 if x > 120 else 0)
        for y in range(top, G.SURFACE_Y):
            img[y][x] = GR_D if (y > top + 1 or hash2(x, y) & 1) else GR_L
    # Water: surface row with ripples, then five bands with dithered seams.
    bands = [(G.SURFACE_Y + 1, W1), (140, W2), (210, W3), (290, W4), (360, W5)]
    for y in range(G.SURFACE_Y, G.BED_Y):
        for x in range(G.WATER_RIGHT):
            if y == G.SURFACE_Y:
                c = W_S if (x // 3) % 4 != 3 else W1
            else:
                c = W1
                for i, (y0, bc) in enumerate(bands):
                    if y >= y0:
                        c = bc
                    elif y >= y0 - 4 and (x + y) % 2 == 0 and i > 0 and (y >= y0 - 2 or (x // 2 + y) % 2 == 0):
                        c = bc
            img[y][x] = c
    for gx, gy in ((20, 102), (64, 106), (110, 100), (40, 116), (130, 110), (80, 124)):
        for i in range(4):
            put(img, gx + i, gy, W_S)
    # A few rising bubbles in the deep.
    for bx, by in ((30, 250), (32, 238), (120, 330), (118, 318), (70, 390), (72, 376)):
        put(img, bx, by, W1)
        put(img, bx + 1, by, W1)
    for y in range(G.BED_Y, H):
        for x in range(G.WATER_RIGHT):
            img[y][x] = SAND
            if hash2(x, y) < 18:
                img[y][x] = DI_L
            elif hash2(x + 3, y) < 8:
                img[y][x] = DI_M
    for x in range(G.WATER_RIGHT):
        if (x // 5) % 3 == 0:
            img[G.BED_Y][x] = W5
    # Rocks on the bed.
    for rx, rw, rh in ((40, 14, 8), (96, 10, 6), (150, 16, 10)):
        ellipse(img, rx, G.BED_Y + 1, rw / 2.0, rh / 2.0, DI_D)
        ellipse(img, rx - 1, G.BED_Y, rw / 2.0 - 2, rh / 2.0 - 2, DI_M)
    for wx, hgt in ((14, 30), (19, 20), (58, 40), (63, 24), (104, 28), (140, 36), (146, 18)):
        for y in range(G.BED_Y - hgt, G.BED_Y + 1):
            sway = 1 if ((y // 5) % 2) else 0
            put(img, wx + sway, y, GR_D)
            if y % 3 == 0:
                put(img, wx + sway + 1, y, GR_D)
    # The bank: grass top, dirt body with strata, a darker cliff face that wobbles.
    for y in range(G.BANK_TOP, H):
        for x in range(G.WATER_RIGHT - 4, W):
            edge = G.WATER_RIGHT + (1 if (y // 6) % 3 == 1 else 0) - (1 if (y // 9) % 4 == 2 else 0)
            if x < edge:
                continue
            d = y - G.BANK_TOP
            if d < 2:
                c = GR_L
            elif d < 4:
                c = GR_D if hash2(x, y) & 3 else GR_L
            elif x < edge + 2:
                c = DI_D
            else:
                c = DI_L if (y // 10) % 2 == 0 else DI_M
                if hash2(x, y) < 20:
                    c = DI_D
            img[y][x] = c
    for x in range(G.WATER_RIGHT, W, 5):
        put(img, x + 1, G.BANK_TOP - 1, GR_D)
        put(img, x + 2, G.BANK_TOP - 2, GR_L)
        put(img, x + 3, G.BANK_TOP - 1, GR_D)
    for y in range(G.SURFACE_Y, H):
        for x in range(G.WATER_RIGHT - 1, G.WATER_RIGHT + 2):
            if img[y][x] in (DI_L, DI_M):
                img[y][x] = DI_D
    return img


# ---- Sprites ----
def fish(w, h, body_w, body_h, tail, frame, body=1, edge=2, eye=None):
    """A fish facing right: an oval body, a tail on the left, a dorsal fin. frame 1 bends the tail."""
    img = new(w, h)
    cx = tail + body_w / 2.0 - 0.5
    cy = h / 2.0 - 0.5
    ellipse(img, cx, cy, body_w / 2.0, body_h / 2.0, body)
    bend = -1 if frame else 0
    for i in range(tail + 1):
        spread = int((tail - i) * body_h / (2.0 * tail)) + 1
        notch = max(0, (tail // 3 - i) * body_h // (2 * tail)) if tail >= 6 else 0
        for s in range(-spread, spread + 1):
            if abs(s) >= notch:
                put(img, i, int(cy) + s + (bend if i < tail // 2 else 0), body)
    for i in range(max(2, body_w // 6)):
        put(img, int(cx) - i // 2, int(cy - body_h / 2.0) - (1 if i < body_w // 10 + 1 else 0), body)
    out = [row[:] for row in img]
    for y in range(h):
        for x in range(w):
            if img[y][x] == body:
                n = sum(1 for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))
                        if 0 <= x + dx < w and 0 <= y + dy < h and img[y + dy][x + dx] == body)
                if n < 4:
                    out[y][x] = edge
    if eye is not None:
        ex = int(cx + body_w / 2.0 * 0.6)
        put(out, ex, int(cy) - max(1, body_h // 6), eye)
    return out


def golden(frame):
    img = fish(32, 16, 24, 10, 6, frame, body=2, edge=5, eye=6)
    for y in range(16):
        for x in range(32):
            if img[y][x] == 2 and y < 7:
                img[y][x] = 3
            elif img[y][x] == 2 and y > 9:
                img[y][x] = 1
    put(img, 18 + frame * 2, 6, 4)
    put(img, 19 + frame * 2, 5, 4)
    put(img, 12, 7, 4)
    return img


def rod():
    """The rod in a 64x64 sprite: the hand at the centre (32, 32), the tip FISHING_ROD_LEN away at 45 degrees up and to
    the left. The screen turns it about its centre (an affine sprite), so the hand stays put and the tip moves."""
    img = new(64, 64)
    n = 0
    for i in range(0, 32):
        x = y = 32 - i
        put(img, x, y, 9)
        if i < 18:
            put(img, x + 1, y, 9)
            put(img, x, y + 1, 10)
        n += 1
    put(img, 1, 1, 3)
    put(img, 32, 33, 7)
    put(img, 33, 33, 7)
    return img


def bobber(dip):
    img = new(8, 8)
    for y in range(8):
        for x in range(8):
            if (x - 3.5) ** 2 + (y - 3.5 - dip) ** 2 <= 9:
                img[y][x] = 4 if y - dip < 4 else 3
    put(img, 3, dip - 1 if dip else 0, 7)
    put(img, 4, dip - 1 if dip else 0, 7)
    return img


def hook():
    img = new(8, 8)
    for y in range(0, 6):
        put(img, 0, y, 6)
    for x in range(1, 4):
        put(img, x, 6, 6)
    put(img, 4, 5, 6)
    put(img, 4, 4, 6)
    put(img, 3, 3, 7)
    return img


def line_piece():
    img = new(8, 8)
    for y in range(8):
        img[y][0] = 5
    return img


def bang():
    img = new(8, 16)
    for y in range(1, 11):
        for x in range(2, 6):
            img[y][x] = 7 if x in (2, 5) or y in (1, 10) else 8
    for y in range(12, 16):
        for x in range(2, 6):
            img[y][x] = 7 if x in (2, 5) or y in (12, 15) else 8
    return img


def splash(frame):
    img = new(16, 8)
    pts = [(2, 6), (4, 4), (6, 2), (8, 1), (10, 3), (12, 5), (13, 7)] if frame == 0 else \
          [(1, 7), (3, 5), (5, 4), (8, 3), (11, 4), (13, 6), (14, 7)]
    for x, y in pts:
        put(img, x, y, 11)
        put(img, x, min(7, y + 1), 12)
    return img


def bait(kind):
    img = new(8, 8)
    if kind == 0:  # worm: a pink wiggle
        for x, y in ((1, 5), (2, 4), (3, 4), (4, 5), (5, 5), (6, 4), (7, 3)):
            put(img, x, y, 13)
            put(img, x, y + 1, 13)
        put(img, 7, 3, 7)
    elif kind == 1:  # shrimp: an orange curl
        for x, y in ((2, 2), (3, 2), (4, 2), (5, 3), (6, 4), (6, 5), (5, 6), (4, 6), (3, 5)):
            put(img, x, y, 14)
        put(img, 2, 3, 14)
        put(img, 1, 1, 7)
        put(img, 0, 1, 14)
    else:  # golden lure: a gold spoon with a shine
        ellipse(img, 3.5, 3.5, 2.5, 3.2, 15)
        put(img, 3, 2, 3)
        put(img, 3, 7, 6)
    return img


def spark(frame):
    img = new(8, 8)
    if frame == 0:
        for i in range(1, 7):
            put(img, 3, i, 8)
            put(img, i, 3, 8)
        put(img, 3, 3, 3)
        for d in (2, 4):
            put(img, d, d, 3)
            put(img, d, 6 - d, 3)
    else:
        put(img, 3, 2, 8)
        put(img, 3, 4, 8)
        put(img, 2, 3, 8)
        put(img, 4, 3, 8)
        put(img, 3, 3, 3)
    return img


def coin(frame):
    img = new(8, 8)
    rx = (3.2, 1.6)[frame]
    ellipse(img, 3.5, 3.5, rx, 3.2, 1)
    ellipse(img, 3.5, 3.5, max(0.6, rx - 1), 2.2, 2 if frame else 3)
    put(img, 3, 2, 4)
    return img


# Pop-up words: a bold 5x7 font drawn at twice the size, with a gradient fill, an outline and a drop shadow.
BIG = {
    "A": ["01110", "10001", "10001", "11111", "10001", "10001", "10001"],
    "C": ["01111", "10000", "10000", "10000", "10000", "10000", "01111"],
    "D": ["11110", "10001", "10001", "10001", "10001", "10001", "11110"],
    "E": ["11111", "10000", "10000", "11110", "10000", "10000", "11111"],
    "F": ["11111", "10000", "10000", "11110", "10000", "10000", "10000"],
    "G": ["01111", "10000", "10000", "10111", "10001", "10001", "01111"],
    "H": ["10001", "10001", "10001", "11111", "10001", "10001", "10001"],
    "J": ["00111", "00010", "00010", "00010", "00010", "10010", "01100"],
    "K": ["10001", "10010", "10100", "11000", "10100", "10010", "10001"],
    "O": ["01110", "10001", "10001", "10001", "10001", "10001", "01110"],
    "P": ["11110", "10001", "10001", "11110", "10000", "10000", "10000"],
    "R": ["11110", "10001", "10001", "11110", "10100", "10010", "10001"],
    "T": ["11111", "00100", "00100", "00100", "00100", "00100", "00100"],
    "U": ["10001", "10001", "10001", "10001", "10001", "10001", "01110"],
    "!": ["1", "1", "1", "1", "1", "0", "1"],
    "L": ["10000", "10000", "10000", "10000", "10000", "10000", "11111"],
    "M": ["10001", "11011", "10101", "10101", "10001", "10001", "10001"],
    "S": ["01111", "10000", "10000", "01110", "00001", "00001", "11110"],
    "V": ["10001", "10001", "10001", "10001", "10001", "01010", "00100"],
    " ": ["000", "000", "000", "000", "000", "000", "000"],
    "I": ["111", "010", "010", "010", "010", "010", "111"],
    "N": ["10001", "11001", "10101", "10011", "10001", "10001", "10001"],
    "B": ["11110", "10001", "10001", "11110", "10001", "10001", "11110"],
    "W": ["10001", "10001", "10001", "10101", "10101", "11011", "10001"],
    "0": ["01110", "10001", "10011", "10101", "11001", "10001", "01110"],
    "1": ["00100", "01100", "00100", "00100", "00100", "00100", "01110"],
    "2": ["01110", "10001", "00001", "00110", "01000", "10000", "11111"],
    "3": ["11110", "00001", "00001", "01110", "00001", "00001", "11110"],
    "4": ["00010", "00110", "01010", "10010", "11111", "00010", "00010"],
    "5": ["11111", "10000", "11110", "00001", "00001", "10001", "01110"],
    "6": ["00110", "01000", "10000", "11110", "10001", "10001", "01110"],
    "7": ["11111", "00001", "00010", "00100", "01000", "01000", "01000"],
    "8": ["01110", "10001", "10001", "01110", "10001", "10001", "01110"],
    "9": ["01110", "10001", "10001", "01111", "00001", "00010", "01100"],
    "+": ["00000", "00100", "00100", "11111", "00100", "00100", "00000"],
    "x": ["00000", "00000", "10001", "01010", "00100", "01010", "10001"],
}
POP_WORDS = ["PERFECT!", "GREAT!", "HOOKED!", "CAUGHT!", "JACKPOT!", "NEW!", "RECORD!"]


def pop_word(word):
    """128x32: the word centred, colours 1 outline, 2 shadow, 3..5 fill top to bottom, 6 shine."""
    glyphs = [BIG[c] for c in word]
    width = sum(len(g[0]) * 2 + 2 for g in glyphs) - 2
    img = new(128, 32)
    fill = new(128, 32)
    x0 = (128 - width) // 2
    y0 = 9
    x = x0
    for g in glyphs:
        for gy, row in enumerate(g):
            for gx, b in enumerate(row):
                if b == "1":
                    for dy in range(2):
                        for dx in range(2):
                            fill[y0 + gy * 2 + dy][x + gx * 2 + dx] = 1
        x += len(g[0]) * 2 + 2
    for y in range(32):
        for x in range(128):
            if fill[y][x]:
                img[y][x] = 3 if y < y0 + 5 else (4 if y < y0 + 10 else 5)
    for y in range(32):
        for x in range(128):
            if fill[y][x] and y in (y0, y0 + 1) and (x // 2) % 3 == 0:
                img[y][x] = 6
    out = [row[:] for row in img]
    for y in range(32):
        for x in range(128):
            if fill[y][x]:
                continue
            near = any(0 <= y + dy < 32 and 0 <= x + dx < 128 and fill[y + dy][x + dx]
                       for dy in (-1, 0, 1) for dx in (-1, 0, 1))
            if near:
                out[y][x] = 1
            elif 0 <= y - 2 < 32 and 0 <= x - 2 < 128 and fill[y - 2][x - 2]:
                out[y][x] = 2
    return out


def pop_halves(word):
    img = pop_word(word)
    return [[row[:64] for row in img], [row[64:] for row in img]]


POP_PALS = {  # outline, shadow, fill top, mid, bottom, shine
    "GOLD": [(64, 32, 0), (0, 0, 24), (255, 240, 120), (248, 200, 40), (216, 136, 16), (255, 255, 255)],
    "ORANGE": [(72, 24, 0), (0, 0, 24), (255, 208, 128), (248, 144, 48), (224, 80, 24), (255, 255, 255)],
    "BLUE": [(8, 24, 72), (0, 0, 24), (200, 240, 255), (112, 184, 248), (48, 112, 224), (255, 255, 255)],
    "GREEN": [(8, 56, 16), (0, 0, 24), (200, 255, 176), (112, 224, 96), (40, 168, 56), (255, 255, 255)],
}
POP_WORD_PAL = ["GOLD", "ORANGE", "BLUE", "GREEN", "GOLD", "GREEN", "GOLD"]


# Tiny 3x5 font for the START button.
FONT = {
    "S": ["111", "100", "111", "001", "111"], "T": ["111", "010", "010", "010", "010"],
    "A": ["010", "101", "111", "101", "101"], "R": ["110", "101", "110", "101", "101"],
}


def button_round(letter, face):
    img = new(16, 16)
    ellipse(img, 7.5, 7.5, 7.2, 7.2, 1)
    ellipse(img, 7.5, 7.0, 6.0, 6.0, face)
    ellipse(img, 7.5, 9.0, 5.0, 3.0, face)
    shapes = {
        "A": ["0110", "1001", "1001", "1111", "1001", "1001"],
        "B": ["1110", "1001", "1110", "1001", "1001", "1110"],
    }[letter]
    for y, row in enumerate(shapes):
        for x, ch in enumerate(row):
            if ch == "1":
                put(img, 6 + x, 5 + y, 5)
    return img


def button_start():
    img = new(32, 16)
    for y in range(3, 13):
        for x in range(1, 31):
            r = 5
            dx = 0 if 6 <= x <= 25 else (6 - x if x < 6 else x - 25)
            if dx * dx + (y - 7.5) ** 2 * 1.5 <= r * r + 4:
                img[y][x] = 8
    out = [row[:] for row in img]
    for y in range(16):
        for x in range(32):
            if img[y][x] == 8 and any(not (0 <= x + dx < 32 and 0 <= y + dy < 16) or img[y + dy][x + dx] == 0
                                      for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                out[y][x] = 1
    for i, ch in enumerate("START"):
        for y, row in enumerate(FONT[ch]):
            for x, b in enumerate(row):
                if b == "1":
                    put(out, 6 + i * 4 + x, 5 + y, 5)
    return out


def dpad(vertical):
    img = new(16, 16)
    for y in range(16):
        for x in range(16):
            if (5 <= x <= 10 and 1 <= y <= 14) or (1 <= x <= 14 and 5 <= y <= 10):
                img[y][x] = 1
            if (6 <= x <= 9 and 2 <= y <= 13) or (2 <= x <= 13 and 6 <= y <= 9):
                img[y][x] = 3
    # light the two arms in use
    arms = [(6, 2, 9, 5), (6, 10, 9, 13)] if vertical else [(2, 6, 5, 9), (10, 6, 13, 9)]
    for x0, y0, x1, y1 in arms:
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                img[y][x] = 2
    tri = [(7, 3), (8, 3), (6, 4), (9, 4)] if vertical else [(3, 7), (3, 8), (4, 6), (4, 9)]
    for x, y in tri:
        put(img, x, y, 4)
        put(img, (15 - x) if not vertical else x, (15 - y) if vertical else y, 4)
    return img


def meter_icon(kind):
    img = new(8, 8)
    if kind == 0:  # tension: a taut line
        for i in range(8):
            put(img, i, 7 - i, 13)
        put(img, 0, 7, 11)
    elif kind == 1:  # progress: a small fish
        ellipse(img, 4.5, 4, 2.6, 1.8, 10)
        put(img, 1, 3, 10)
        put(img, 1, 5, 10)
        put(img, 0, 4, 10)
    else:  # time: a clock
        ellipse(img, 3.5, 3.5, 3.4, 3.4, 13)
        ellipse(img, 3.5, 3.5, 2.4, 2.4, 12)
        put(img, 3, 2, 5)
        put(img, 3, 3, 5)
        put(img, 4, 3, 5)
    return img


def to_tiles(img):
    h, w = len(img), len(img[0])
    out = []
    for ty in range(h // 8):
        for tx in range(w // 8):
            b = bytearray()
            for y in range(8):
                for x in range(0, 8, 2):
                    b.append(img[ty * 8 + y][tx * 8 + x] | (img[ty * 8 + y][tx * 8 + x + 1] << 4))
            out.append(bytes(b))
    return out


def map_bobber():
    img = new(8, 8)
    for y in range(8):
        for x in range(8):
            if (x - 3.5) ** 2 + (y - 4) ** 2 <= 7:
                img[y][x] = 0x4 if y < 4 else 0x1
    for x in range(1, 7):
        if (x - 3.5) ** 2 <= 7:
            put(img, x, 7, 0xF)
    put(img, 3, 0, 0xF)
    put(img, 4, 0, 0xF)
    return img


SPRITES = [  # name, palette, frames; tiles are packed in this order
    ("FISH_SMALL", "FX", [fish(16, 8, 12, 5, 3, f) for f in (0, 1)]),
    ("FISH_MEDIUM", "FX", [fish(32, 16, 24, 9, 6, f) for f in (0, 1)]),
    ("FISH_LARGE", "FX", [fish(64, 32, 42, 14, 12, f) for f in (0, 1)]),
    ("FISH_HUGE", "FX", [fish(64, 32, 52, 24, 12, f) for f in (0, 1)]),
    ("FISH_GOLDEN", "GOLD", [golden(f) for f in (0, 1)]),
    ("ROD", "FX", [rod()]),
    ("BOBBER", "FX", [bobber(0), bobber(2)]),
    ("HOOK", "FX", [hook()]),
    ("LINE", "FX", [line_piece()]),
    ("BANG", "FX", [bang()]),
    ("SPLASH", "FX", [splash(0), splash(1)]),
    ("BAIT", "FX", [bait(k) for k in (0, 1, 2)]),
    ("SPARK", "FX", [spark(f) for f in (0, 1)]),
    ("COIN", "GOLD", [coin(f) for f in (0, 1)]),
    ("BTN_A", "UI", [button_round("A", 6)]),
    ("BTN_B", "UI", [button_round("B", 7)]),
    ("BTN_START", "UI", [button_start()]),
    ("BTN_UPDOWN", "UI", [dpad(True)]),
    ("BTN_LEFTRIGHT", "UI", [dpad(False)]),
    ("METER_ICON", "UI", [meter_icon(k) for k in (0, 1, 2)]),
]
PALS = {"FX": PAL_FX, "UI": PAL_UI, "GOLD": PAL_GOLD}


def pal_words(pal):
    return [((r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)) for r, g, b in pal]


def c_bytes(name, data, comment):
    s = "/* %s */\nconst u8 %s[%d] = {\n" % (comment, name, len(data))
    for i in range(0, len(data), 16):
        s += "\t" + ", ".join("0x%02X" % v for v in data[i:i + 16]) + ",\n"
    return s + "};\n\n"


def c_u16(name, data, comment, per=16):
    s = "/* %s */\nconst u16 %s[%d] = {\n" % (comment, name, len(data))
    for i in range(0, len(data), per):
        s += "\t" + ", ".join("0x%04X" % v for v in data[i:i + per]) + ",\n"
    return s + "};\n\n"


def build():
    scene = draw_scene()
    tiles, index, tmap = [], {}, []
    for t in to_tiles(scene):
        if t not in index:
            index[t] = len(tiles)
            tiles.append(t)
        tmap.append(index[t])
    obj, offsets = [], {}
    for name, _, frames in SPRITES:
        offsets[name] = len(obj)
        for f in frames:
            obj.extend(to_tiles(f))
        offsets[name + "_FRAME"] = len(to_tiles(frames[0]))
    mb = to_tiles(map_bobber())[0]
    rows = H // 8
    pops = b"".join(b"".join(to_tiles(half)) for w in POP_WORDS for half in pop_halves(w))
    pop_pals = []
    for name in POP_PALS:
        pop_pals += pal_words([(0, 0, 0)] + POP_PALS[name] + [(0, 0, 0)] * 9)
    species = fish_species.SPECIES
    pics = b"".join(b"".join(to_tiles(fish_species.portrait(sp))) for sp in species)
    icons = b"".join(b"".join(to_tiles(fish_species.icon(sp))) for sp in species)
    sp_pals = []
    for sp in species:
        sp_pals += pal_words(sp["pal"])

    h = ["/* Generated by Tools/fishing/gen_fishing_art.py. Do not edit. */", "#pragma once", "",
         "/* Scene geometry in pixels (world rows: the screen scrolls down the scene) */"]
    for k, v in GEO.items():
        h.append("#define %s %d" % (k, v))
    h += ["", "#define FISHING_SCENE_ROWS %d" % rows, "#define FISHING_SCENE_TILES %d" % len(tiles),
          "#define FISHING_OBJ_TILES %d" % len(obj), "",
          "/* OBJ tile offsets (1D mapping), tiles per frame, and the palette each sprite uses */"]
    for name, pal, _ in SPRITES:
        h.append("#define FISHING_CHR_%s %d" % (name, offsets[name]))
        h.append("#define FISHING_CHR_%s_FRAME %d" % (name, offsets[name + "_FRAME"]))
    h += ["", "extern const u8 gFishingSceneTiles[FISHING_SCENE_TILES * 32];",
          "extern const u16 gFishingSceneMap[30 * FISHING_SCENE_ROWS];", "extern const u16 gFishingScenePal[16];",
          "extern const u8 gFishingObjTiles[FISHING_OBJ_TILES * 32];", "extern const u16 gFishingObjPalFx[16];",
          "extern const u16 gFishingObjPalUi[16];", "extern const u16 gFishingObjPalGold[16];",
          "extern const u8 gFishingMapBobberTile[32];", "",
          "/* Pop-up words: two 64x32 halves (64 tiles) each, copied into OBJ VRAM when shown */",
          "enum { %s, FISHING_POP_WORDS };" % ", ".join("FISHING_POP_" + w.strip("!") for w in POP_WORDS),
          "enum { %s };" % ", ".join("FISHING_POP_PAL_%s" % n for n in POP_PALS),
          "#define FISHING_POP_TILES 64",
          "extern const u8 gFishingPopTiles[FISHING_POP_WORDS * FISHING_POP_TILES * 32];",
          "extern const u16 gFishingPopPals[%d * 16];" % len(POP_PALS),
          "extern const u8 gFishingPopWordPal[FISHING_POP_WORDS];", "",
          "/* The journal's species (water * 5 + kind): a 64x32 portrait, a 32x16 icon and a palette each */",
          "#define FISHING_SPECIES %d" % len(species), "#define FISHING_PIC_TILES 32", "#define FISHING_ICON_TILES 8",
          "extern const u8 gFishingSpeciesPics[FISHING_SPECIES * FISHING_PIC_TILES * 32];",
          "extern const u8 gFishingSpeciesIcons[FISHING_SPECIES * FISHING_ICON_TILES * 32];",
          "extern const u16 gFishingSpeciesPals[FISHING_SPECIES * 16];",
          "extern const u16 gFishingSilhouettePal[16];",
          "extern const u8 gFishingSpeciesCm[FISHING_SPECIES * 2]; /* smallest and biggest, centimetres */", ""]

    c = ["/* Generated by Tools/fishing/gen_fishing_art.py. Do not edit: change the script and run it again. */",
         '#include "common-chax.h"', '#include "fishing-art.h"', ""]
    c.append(c_bytes("gFishingSceneTiles", b"".join(tiles), "Scene tiles (4bpp, %d unique)" % len(tiles)))
    c.append(c_u16("gFishingSceneMap", tmap, "Scene map, 30 x %d tile indices (no palette bits)" % rows, 30))
    c.append(c_u16("gFishingScenePal", pal_words(SCENE_PAL), "Scene palette"))
    c.append(c_bytes("gFishingObjTiles", b"".join(obj), "Sprite tiles (4bpp, 1D order)"))
    c.append(c_u16("gFishingObjPalFx", pal_words(PAL_FX), "Sprite palette: fish shadows, rod, line, bobber, bait"))
    c.append(c_u16("gFishingObjPalUi", pal_words(PAL_UI), "Sprite palette: buttons and meters"))
    c.append(c_u16("gFishingObjPalGold", pal_words(PAL_GOLD), "Sprite palette: the golden fish"))
    c.append(c_bytes("gFishingMapBobberTile", mb, "District map bobber (item icon palette 0)"))
    c.append(c_bytes("gFishingPopTiles", pops, "Pop-up words %s, two 64x32 halves each" % ", ".join(POP_WORDS)))
    c.append(c_u16("gFishingPopPals", pop_pals, "Pop-up palettes: %s" % ", ".join(POP_PALS)))
    c.append(c_bytes("gFishingPopWordPal", bytes(list(POP_PALS).index(n) for n in POP_WORD_PAL), "Each word's palette"))
    names = ", ".join(sp["name"] for sp in species)
    c.append(c_bytes("gFishingSpeciesPics", pics, "Species portraits, 64x32 (1D order): %s" % names))
    c.append(c_bytes("gFishingSpeciesIcons", icons, "Species icons, 32x16 (1D order)"))
    c.append(c_u16("gFishingSpeciesPals", sp_pals, "Species palettes"))
    c.append(c_u16("gFishingSilhouettePal", pal_words(fish_species.SILHOUETTE), "An uncaught species' silhouette"))
    c.append(c_bytes("gFishingSpeciesCm", bytes(v for sp in species for v in sp["cm"]),
                     "Species sizes: smallest and biggest, centimetres"))
    return "\n".join(c).rstrip() + "\n", "\n".join(h) + "\n", scene


def preview(path, scene):
    from PIL import Image
    im = Image.new("RGB", (W, H))
    px = im.load()
    for y in range(H):
        for x in range(W):
            px[x, y] = SCENE_PAL[scene[y][x]]

    def blit(img, x0, y0, pal, flip=False):
        for y, row in enumerate(img):
            for x, v in enumerate(row):
                if v and 0 <= x0 + x < W and 0 <= y0 + y < H:
                    xx = x0 + (len(row) - 1 - x if flip else x)
                    px[xx, y0 + y] = pal[v]
    sp = {n: f for n, _, f in SPRITES}
    blit(sp["FISH_SMALL"][0], 30, 120, PAL_FX)
    blit(sp["FISH_MEDIUM"][1], 70, 180, PAL_FX, True)
    blit(sp["FISH_LARGE"][0], 10, 280, PAL_FX)
    blit(sp["FISH_HUGE"][0], 80, 360, PAL_FX, True)
    blit(sp["FISH_GOLDEN"][0], 40, 230, PAL_GOLD)
    blit(sp["ROD"][0], G.HAND_X - 32, G.HAND_Y - 32, PAL_FX)
    for i, w in enumerate(POP_WORDS):
        pal = [(0, 0, 0)] + POP_PALS[POP_WORD_PAL[i]] + [(0, 0, 0)] * 9
        blit(pop_word(w), 8, 120 + i * 30, pal)
    for y in range(G.ROD_TIP_Y, 200, 8):
        blit(sp["LINE"][0], G.ROD_TIP_X, y, PAL_FX)
    blit(sp["BOBBER"][0], G.ROD_TIP_X - 3, G.SURFACE_Y - 5, PAL_FX)
    blit(sp["HOOK"][0], G.ROD_TIP_X, 200, PAL_FX)
    blit(sp["BAIT"][1], G.ROD_TIP_X + 2, 204, PAL_FX)
    for i, n in enumerate(("BTN_A", "BTN_B", "BTN_UPDOWN", "BTN_LEFTRIGHT")):
        blit(sp[n][0], 8 + i * 20, 24, PAL_UI)
    blit(sp["BTN_START"][0], 90, 24, PAL_UI)
    im.resize((W * 2, H * 2), Image.NEAREST).save(path)


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
