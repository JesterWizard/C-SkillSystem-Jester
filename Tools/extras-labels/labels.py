#!/usr/bin/env python3
"""Draw the Extras labels made from vanilla's letters into the Extras label sheet.

    python3 Tools/extras-labels/labels.py fe8.gba

The Extras list's labels are sprites in a hand-drawn font, in Kernel/Wizardry/NewGamePlus/Gfx/Img_NgpMenuObjs.png.
Vanilla has no label for the rows this hack adds, so their words are built from the letters of vanilla's own labels
(Img_GameMainMenuObjs, Img_SaveScreenSprits, Img_DifficultyMenuObjs, read from the clean ROM):

- A letter is its core: the palette 1-4 pixels (2-pixel strokes) between two columns of a vanilla word. Pieces of a
  neighbour cut off with it (under 3 pixels) are dropped.
- A word is its letters' cores a column apart, rendered in the font's style: a core pixel is white (1), tan (3) where
  it continues a stroke from the left and from above; the drop shadow (5) is right of and below the core; the outline
  (6) is round the lot.
- Two letters are drawn by hand in the same strokes: the i (vanilla's is a stem with a notch, which reads as an l)
  and the x (Extras' runs into its t).

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
    "d": ("Select Mode", 192, 198),
}
DRAWN = {
    "i": (2, [".##", ".##", "...", "###", ".##", ".##", ".##", ".##", ".##", "###"]),
    "x": (5, ["##...##", ".##.##.", "..###..", "..###..", ".##.##.", "##...##", "##...##"]),
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
    return [(x - left, y) for x, y in out]


def render(glyphs):
    core, cur = set(), 1
    for g in glyphs:
        if isinstance(g, int):
            cur += g
            continue
        core |= {(cur + x, y) for x, y in g}
        cur += max(x for x, _ in g) + 2
    w = cur + 1
    rows = [[0] * w for _ in range(16)]
    for x, y in core:
        rows[y][x] = 3 if (x - 1, y) in core and (x, y - 1) in core else 1
    shadow = {(x, y) for y in range(16) for x in range(w) if (x, y) not in core and
              ((x - 1, y) in core or (x, y - 1) in core or (x - 1, y - 1) in core)}
    for x, y in shadow:
        rows[y][x] = 5
    body = core | shadow
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
    glyphs.update({c: [(x, top + y) for y, r in enumerate(rows) for x, ch in enumerate(r) if ch == "#"]
                   for c, (top, rows) in DRAWN.items()})

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
