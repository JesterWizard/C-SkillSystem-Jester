#!/usr/bin/env python3
"""Draw the Tutorials screen's titles in the Extras labels' letters.

    python3 Tools/tutorial-titles/titles.py fe8.gba [preview.png]

Each tutorial's title (the `## MSG_TUT_<KEY>_TITLE` messages at the end of Contents/Texts/Source/texts/Districts.txt)
is drawn as the Extras labels are (Tools/extras-labels/labels.py): vanilla's own letters from the clean ROM, a shadow
column apart, the outline drawn round the word. Vanilla's menu words lack some letters (U, W, O, V, F, I, P, K, X, b,
w, q, g, -): those are drawn below in the same strokes (palette 1 the stroke, 3 its shaded right side, 2 and 4 its serifs
and inner shading, as vanilla's letters have them).

Writes Kernel/Wizardry/Tutorials/Source/TutorialTitles.c and .h: per title a 128x16 image (two rows of 16 tiles, 4bpp,
the label palette's colours), centred, as Img_TutTitle_<KEY>. Run it again after adding or renaming a tutorial.
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.join(ROOT, "Tools", "extras-labels"))
import labels as L  # noqa: E402

TEXTS = os.path.join(ROOT, "Contents/Texts/Source/texts/Districts.txt")
OUT = os.path.join(ROOT, "Kernel/Wizardry/Tutorials/Source/TutorialTitles")
WIDTH = 128

# Vanilla's letters: (sheet, band's y, first column, column after)
CUTS = {
    "T": ("main", 0, 19, 29), "B": ("main", 0, 138, 146), "H": ("main", 0, 174, 183),
    "M": ("main", 16, 44, 55), "S": ("main", 32, 23, 29), "A": ("main", 32, 174, 183),
    "R": ("save", 32, 16, 24), "E": ("save", 32, 140, 147), "D": ("save", 32, 173, 181),
    "G": ("save", 80, 40, 47), "C": ("save", 80, 101, 108),
    "a": ("main", 0, 146, 151), "t": ("main", 0, 152, 156), "e": ("main", 0, 164, 169),
    "p": ("main", 16, 133, 139), "v": ("main", 16, 186, 191), "k": ("main", 32, 165, 173),
    "i": ("main", 32, 155, 158), "n": ("main", 32, 194, 199), "s": ("diff", 32, 40, 44),
    "y": ("diff", 32, 44, 50), "c": ("diff", 0, 165, 170), "d": ("diff", 0, 193, 198),
    "u": ("save", 32, 34, 39), "r": ("save", 48, 44, 48), "h": ("save", 32, 67, 72),
    "m": ("save", 80, 54, 61), "o": ("save", 80, 109, 114), "x": ("save", 16, 183, 188),
    "l": ("sheet", 16, 49, 52), "f": ("diff", 16, 168, 172),
}

# The letters vanilla's menu words lack: rows from the top of the 16-pixel band (row 2 is the capitals' top, row 11
# the baseline, rows 12-13 the descenders), "." empty
DRAWN = {
    "I": (2, ["2123", ".13.", ".13.", ".13.", ".13.", ".13.", ".13.", ".13.", ".13.", "2123"]),
    "F": (2, ["2111113", "3133313", ".13..33", ".13.13.", ".11113.", ".13313.", ".13.33.", ".13....", ".13....",
              "2123..."]),
    "P": (2, ["211113.", ".133313", ".13..13", ".13..13", ".13..13", ".11113.", ".13....", ".13....", ".13....",
              "2123..."]),
    "O": (2, [".11113.", "1233213", "13...13", "13...13", "13...13", "13...13", "13...13", "13...13", "124.413",
              ".11113."]),
    "U": (2, ["2123.2123", ".13...13.", ".13...13.", ".13...13.", ".13...13.", ".13...13.", ".13...13.", ".13...13.",
              ".313.313.", "..31113.."]),
    "V": (2, ["2123..2123", ".13....13.", ".13....13.", ".313..313.", "..13..13..", "..313313..", "...1313...",
              "...1313...", "....13....", "....13...."]),
    "W": (2, ["2123.2123.2123", ".13...13...13.", ".13...13...13.", ".13...13...13.", ".13...13...13.",
              ".13..1313..13.", ".313.1313.313.", "..13131313.13.", "..3113.3113...", "...13...13...."]),
    "K": (2, ["2123.2123", ".13...13.", ".13..13..", ".13.13...", ".1113....", ".13313...", ".13.313..", ".13..313.",
              ".13...13.", "2123.2123"]),
    "b": (2, ["213...", ".13...", ".13...", ".13...", ".1113.", ".13313", ".13.13", ".13.13", ".13.13", "21113."]),
    "q": (5, [".31113", "31331.", "13.13.", "13.13.", "13.13.", "31413.", ".1113.", "...13.", "..2123"]),
    "g": (5, [".31113", "31331.", "13.13.", "13.13.", "13.13.", "31413.", ".1113.", "...13.", "3113.."]),
    "X": (2, ["2123.2123", ".313.313.", "..13.13..", "..31313..", "...313...", "...313...", "..31313..", "..13.13..",
              ".313.313.", "2123.2123"]),
    "-": (7, ["11113", "33333"]),
    "w": (5, ["1..1..1", "1..1..1", "13.1.31", "3131313", ".13.13.", ".13.13.", ".1..1.."]),
}
SPACE = 3


def drawn(top, rows):
    core = {}
    for y, row in enumerate(rows):
        for x, ch in enumerate(row):
            if ch in "1234":
                core[(x, top + y)] = int(ch)
    left = min(x for x, _ in core)
    return {(x - left, y): c for (x, y), c in core.items()}, {}


def titles():
    out, lines = [], open(TEXTS).read().split("\n")
    for i, line in enumerate(lines):
        m = re.match(r"## MSG_TUT_(\w+)_TITLE$", line)
        if m:
            out.append((m.group(1), lines[i + 1].replace("[X]", "").strip()))
    return out


def main():
    if len(sys.argv) not in (2, 3):
        sys.exit(__doc__)
    rom = open(sys.argv[1], "rb").read()
    pixels = {k: L.tiles(L.lz77(rom, v)) for k, v in L.VANILLA.items()}
    sheet = L.Image.open(L.SHEET)
    spx = sheet.load()
    pixels["sheet"] = lambda x, y: spx[x, y]

    glyphs = {c: L.letter(pixels[k], y0, x0, x1) for c, (k, y0, x0, x1) in CUTS.items()}
    glyphs.update({c: drawn(top, rows) for c, (top, rows) in DRAWN.items()})

    images = []
    for key, text in titles():
        missing = sorted({c for c in text if c != " " and c not in glyphs})
        if missing:
            sys.exit("%s: no letter for %s" % (text, " ".join(missing)))
        rows, w = L.render([SPACE if c == " " else glyphs[c] for c in text])
        if w > WIDTH:
            sys.exit("%s: %d pixels, a title holds %d" % (text, w, WIDTH))
        left = (WIDTH - w) // 2
        img = [[0] * WIDTH for _ in range(16)]
        for y in range(16):
            img[y][left:left + w] = rows[y]
        images.append((key, text, img))
        print("%-20s %3d pixels" % (text, w))

    c = ["/* Generated by Tools/tutorial-titles/titles.py: run it again after changing a title. */",
         '#include "common-chax.h"', '#include "TutorialTitles.h"', ""]
    h = ["/* Generated by Tools/tutorial-titles/titles.py */", "#pragma once", "",
         "/* A tutorial's title in the Extras labels' letters: 128x16, two rows of 16 tiles (4bpp) */"]
    for key, text, img in images:
        data = []
        for ty in range(2):
            for tx in range(16):
                for y in range(8):
                    row = img[ty * 8 + y]
                    for x in range(0, 8, 2):
                        data.append(row[tx * 8 + x] | row[tx * 8 + x + 1] << 4)
        c.append("/* %s */" % text)
        c.append("const u8 Img_TutTitle_%s[%d] __attribute__((aligned(4))) = {" % (key, len(data)))
        for i in range(0, len(data), 32):
            c.append("\t" + ", ".join("0x%02X" % b for b in data[i:i + 32]) + ",")
        c += ["};", ""]
        h.append("extern const u8 Img_TutTitle_%s[];" % key)
    open(OUT + ".c", "w").write("\n".join(c))
    open(OUT + ".h", "w").write("\n".join(h) + "\n")

    if len(sys.argv) == 3:
        pal = [(40, 120, 40), (248, 248, 248), (200, 200, 248), (144, 144, 216), (96, 96, 176), (64, 64, 64),
               (24, 24, 24)] + [(255, 0, 255)] * 9
        im = L.Image.new("RGB", (WIDTH * 2, 18 * ((len(images) + 1) // 2)), (40, 120, 40))
        for n, (_, _, img) in enumerate(images):
            for y in range(16):
                for x in range(WIDTH):
                    im.putpixel(((n & 1) * WIDTH + x, (n >> 1) * 18 + y), pal[img[y][x]])
        im.resize((im.width * 3, im.height * 3), L.Image.NEAREST).save(sys.argv[2])


if __name__ == "__main__":
    main()
