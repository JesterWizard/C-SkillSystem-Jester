#!/usr/bin/env python3
"""Draw the Extras labels made from vanilla's letters into the Extras label sheet.

    python3 Tools/extras-labels/labels.py fe8.gba

The Extras list's labels are sprites in a hand-drawn font, in Kernel/Wizardry/NewGamePlus/Gfx/Img_NgpMenuObjs.png.
Vanilla has no label for the rows this hack adds, so their words are built from the letters of vanilla's own labels
(Img_GameMainMenuObjs, Img_SaveScreenSprits, Img_DifficultyMenuObjs, read from the clean ROM):

- A letter is vanilla's own pixels for it: its core (the palette 1-4 pixels: its strokes, serifs, highlights and
  shading) between two columns of a vanilla word, less pieces of a neighbour cut off with it (under 3 pixels), and the
  shadow colours vanilla gives round that core.
- A word is its letters a shadow column apart, as in vanilla's words, with the outline (6) drawn round the lot:
  where a letter was cut from its neighbour, vanilla's outline is gone, so it is drawn again.

Every letter is vanilla's shape; none is drawn by hand.

Each word is centred in the sheet's free tiles given below (label row, first tile column, tiles); the sprites that
show them are gExtrasLabelSprites (Kernel/Wizardry/NewGamePlus/Source/NgpSaveMenu.c). Run it again after changing a
word: it clears those tiles first. See Documentation/FeatureTypes/Menus.md, "Extras enum is fixed".
"""
import os
import sys

from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SHEET = os.path.join(ROOT, "Kernel/Wizardry/NewGamePlus/Gfx/Img_NgpMenuObjs.png")

# The vanilla sheets (LZ77, 4bpp, 32 tiles a row) and the 16-pixel bands of the words used
VANILLA = {"main": 0xA2812C, "save": 0xA26A74, "diff": 0xA28A0C}
BANDS = {
    "Battle History": ("main", 0), "Link Arena": ("main", 32), "Sound Room": ("main", 32),
    "Restart Chapter": ("save", 48), "Copy Data": ("save", 80), "Extras": ("save", 16),
    "Select Mode": ("diff", 0), "Easy": ("diff", 32), "Gallery": ("sheet", 16),
}
# Each letter: the word it comes from and its columns in that sheet
CUTS = {
    "B": ("Battle History", 138, 146), "a": ("Battle History", 146, 151), "t": ("Battle History", 152, 156),
    "e": ("Battle History", 164, 169), "s": ("Easy", 40, 44), "y": ("Easy", 44, 50),
    "k": ("Link Arena", 165, 173), "r": ("Restart Chapter", 44, 48), "S": ("Sound Room", 23, 29),
    "l": ("Gallery", 49, 52), "C": ("Copy Data", 101, 108), "o": ("Copy Data", 109, 114),
    "d": ("Select Mode", 192, 198), "i": ("Link Arena", 155, 158), "x": ("Extras", 183, 188),
}
SPACE = 3
# The words and where they go: (label row, first tile column, tiles), in the order the canvas is cut
LABELS = [
    ("Bestiary", [(1, 0, 4), (1, 10, 2)]),
    ("Skill Codex", [(2, 14, 4), (2, 28, 4), (2, 26, 2)]),
]


def lz77(rom, off):
    size = rom[off + 1] | rom[off + 2] << 8 | rom[off + 3] << 16
    out, i = bytearray(), off + 4
    while len(out) < size:
        flags = rom[i]
        i += 1
        for b in range(8):
            if len(out) >= size:
                break
            if flags & (0x80 >> b):
                v = rom[i] << 8 | rom[i + 1]
                i += 2
                for _ in range((v >> 12) + 3):
                    out.append(out[-((v & 0xFFF) + 1)])
            else:
                out.append(rom[i])
                i += 1
    return bytes(out)


def tiles(data):
    """4bpp tiles, 32 a row, as a function (x, y) -> palette index"""
    def px(x, y):
        t = (y >> 3) * 32 + (x >> 3)
        b = data[t * 32 + (y & 7) * 4 + ((x & 7) >> 1)]
        return (b >> ((x & 1) * 4)) & 0xF
    return px


def letter(pixel, y0, x0, x1):
    """A letter as vanilla draws it: {(x, y): palette index} of its core (palette 1-4, the strokes with their own
    highlights and shading) between columns x0 and x1, less pieces of fewer than 3 pixels (a neighbour's corner);
    and the colours vanilla gives the shadow pixels round it, right of and below the core"""
    pts = {(x, y) for y in range(16) for x in range(x1 - x0) if 1 <= pixel(x0 + x, y0 + y) <= 4}
    out, seen = set(), set()
    for p in sorted(pts):
        if p in seen:
            continue
        comp, stack = {p}, [p]
        seen.add(p)
        while stack:
            cx, cy = stack.pop()
            for q in ((cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)):
                if q in pts and q not in seen:
                    seen.add(q)
                    comp.add(q)
                    stack.append(q)
        if len(comp) >= 3:
            out |= comp
    left = min(x for x, _ in out)
    core = {(x - left, y): pixel(x0 + x, y0 + y) for x, y in out}
    shade = {}
    for x, y in out:
        for q in ((x + 1, y), (x, y + 1), (x + 1, y + 1)):
            if q not in out and 0 <= q[1] < 16 and pixel(x0 + q[0], y0 + q[1]) in (4, 5):
                shade[(q[0] - left, q[1])] = pixel(x0 + q[0], y0 + q[1])
    return core, shade


def render(glyphs):
    """The letters side by side, a shadow column apart: vanilla's pixels for the cores and the shadows, the outline
    drawn round the lot (where letters were cut from their neighbours, vanilla's outline is gone)"""
    core, colour, cur = {}, {}, 1
    for g in glyphs:
        if isinstance(g, int):
            cur += g
            continue
        pts, shade = g
        for (x, y), c in pts.items():
            core[(cur + x, y)] = c
        for (x, y), c in shade.items():
            colour[(cur + x, y)] = c
        cur += max(x for x, _ in pts) + 2
    w = cur + 1
    rows = [[0] * w for _ in range(16)]
    for (x, y), c in core.items():
        rows[y][x] = c
    shadow = {(x, y) for y in range(16) for x in range(w) if (x, y) not in core and
              ((x - 1, y) in core or (x, y - 1) in core or (x - 1, y - 1) in core)}
    for x, y in shadow:
        rows[y][x] = colour.get((x, y), 5)
    body = set(core) | shadow
    for y in range(16):
        for x in range(w):
            if (x, y) not in body and any((x + dx, y + dy) in body for dx in (-1, 0, 1) for dy in (-1, 0, 1)):
                rows[y][x] = 6
    return rows, w


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    rom = open(sys.argv[1], "rb").read()
    sheet = Image.open(SHEET)
    px = sheet.load()
    pixels = {k: tiles(lz77(rom, v)) for k, v in VANILLA.items()}
    pixels["sheet"] = lambda x, y: px[x, y]

    glyphs = {c: letter(pixels[BANDS[w][0]], BANDS[w][1], x0, x1) for c, (w, x0, x1) in CUTS.items()}

    for text, places in LABELS:
        rows, w = render([SPACE if c == " " else glyphs[c] for c in text])
        canvas = sum(n for _, _, n in places) * 8
        if w > canvas:
            sys.exit("%s: %d pixels, the tiles hold %d" % (text, w, canvas))
        left = (canvas - w) // 2
        flat = [[0] * canvas for _ in range(16)]
        for y in range(16):
            flat[y][left:left + w] = rows[y]
        cx = 0
        for row, col, n in places:
            for y in range(16):
                for x in range(n * 8):
                    px[col * 8 + x, row * 16 + y] = flat[y][cx + x]
            cx += n * 8
        print("%s: %d pixels, %d in a %d-pixel canvas" % (text, w, left, canvas))
    sheet.save(SHEET)


if __name__ == "__main__":
    main()
