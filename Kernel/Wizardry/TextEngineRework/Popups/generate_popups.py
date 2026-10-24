#!/usr/bin/env python3
"""
Draw the text engine's popup graphics: a jagged white burst with big slanted
lettering, saved as 16-colour indexed PNGs next to this script.

The PNGs are committed, so this only needs rerunning to change the art.
Then convert each PNG with the makefile's grit rule, for example:
    grit Objection.png -gB 4 -gzl -m -mLf -mR4 -mzl -pn 16 -ftb -fh! -o Objection.img.bin

The whole popup must stay within the dialogue text tiles it borrows at
runtime (240 unique 8x8 tiles after flip reduction); this script reports the
count for each popup.

Usage: python3 generate_popups.py [path/to/bold-italic-font.ttf]
"""
import math
import os
import sys

from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
WIDTH, HEIGHT = 240, 128
MAX_TILES = 240

# Index 0 is transparent on the GBA.
PALETTE = [
    (248, 0, 248),   # 0 transparent
    (24, 16, 24),    # 1 outline
    (248, 248, 240), # 2 burst white
    (216, 216, 208), # 3 burst shade
    (232, 32, 24),   # 4 letter red
    (128, 16, 16),   # 5 letter shadow
    (248, 120, 64),  # 6 letter highlight
]

POPUPS = [
    # name, text, font size, tilt in degrees, burst seed
    ("Objection", "Objection!", 40, -10, 3),
    ("HoldIt", "Hold it!", 50, -9, 5),
    ("TakeThat", "Take that!", 42, -10, 7),
]

FONT_CANDIDATES = [
    "/usr/share/fonts/opentype/inter/InterDisplay-BlackItalic.otf",
    "/usr/share/fonts/opentype/inter/Inter-BlackItalic.otf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-BoldOblique.ttf",
]


def find_font(arg):
    for path in ([arg] if arg else []) + FONT_CANDIDATES:
        if path and os.path.exists(path):
            return path
    sys.exit("No bold italic font found; pass one as the first argument.")


def burst_points(seed, cx, cy, rx, ry, spikes=22):
    """Star outline around an ellipse; spike lengths vary a little per point."""
    points = []
    for i in range(spikes * 2):
        angle = math.pi * 2 * i / (spikes * 2)
        wobble = ((seed * 37 + i * 53) % 11) / 10.0
        if i % 2 == 0:
            scale = 1.0 + 0.10 * wobble
        else:
            scale = 0.80 + 0.06 * wobble
        points.append((cx + math.cos(angle) * rx * scale,
                       cy + math.sin(angle) * ry * scale))
    return points


def draw_popup(text, size, tilt, seed, font_path):
    image = Image.new("P", (WIDTH, HEIGHT), 0)
    image.putpalette([c for rgb in PALETTE for c in rgb] + [0] * (768 - 3 * len(PALETTE)))
    draw = ImageDraw.Draw(image)
    draw.fontmode = "1"

    # Burst: shade ring, then white face, then outline.
    outer = burst_points(seed, WIDTH / 2, HEIGHT / 2, 112, 58)
    inner = burst_points(seed, WIDTH / 2, HEIGHT / 2, 104, 52)
    draw.polygon(outer, fill=3, outline=1)
    draw.polygon(inner, fill=2)
    draw.line(outer + [outer[0]], fill=1, width=2)

    # Lettering on its own layer so it can be tilted.
    font = ImageFont.truetype(font_path, size)
    layer = Image.new("P", (WIDTH * 2, HEIGHT * 2), 0)
    layer_draw = ImageDraw.Draw(layer)
    layer_draw.fontmode = "1"
    left, top, right, bottom = layer_draw.textbbox((0, 0), text, font=font, stroke_width=3)
    x = (layer.width - (right - left)) // 2 - left
    y = (layer.height - (bottom - top)) // 2 - top

    layer_draw.text((x + 4, y + 4), text, font=font, fill=5, stroke_width=3, stroke_fill=5)
    layer_draw.text((x, y), text, font=font, fill=4, stroke_width=3, stroke_fill=1)
    # Thin highlight along the top of each letter.
    highlight = Image.new("1", layer.size, 0)
    ImageDraw.Draw(highlight).text((x, y), text, font=font, fill=1)
    pixels = layer.load()
    mask = highlight.load()
    for py in range(1, layer.height):
        for px in range(layer.width):
            if mask[px, py] and not mask[px, py - 1] and pixels[px, py] == 4:
                pixels[px, py] = 6

    layer = layer.rotate(tilt, resample=Image.NEAREST, center=(layer.width / 2, layer.height / 2))
    layer = layer.crop((WIDTH // 2, HEIGHT // 2, WIDTH // 2 + WIDTH, HEIGHT // 2 + HEIGHT))

    out = image.load()
    lettering = layer.load()
    for py in range(HEIGHT):
        for px in range(WIDTH):
            if lettering[px, py]:
                out[px, py] = lettering[px, py]

    return image


def count_tiles(image):
    """Unique 8x8 tiles once horizontal/vertical flips are merged (grit -mR4)."""
    seen = set()
    data = image.load()
    for ty in range(0, HEIGHT, 8):
        for tx in range(0, WIDTH, 8):
            tile = tuple(tuple(data[tx + x, ty + y] for x in range(8)) for y in range(8))
            if not any(any(row) for row in tile):
                continue
            variants = {
                tile,
                tuple(row[::-1] for row in tile),
                tuple(tile[::-1]),
                tuple(row[::-1] for row in tile[::-1]),
            }
            if not variants & seen:
                seen.add(tile)
    # Plus the blank tile grit keeps at index 0.
    return len(seen) + 1


def main():
    font_path = find_font(sys.argv[1] if len(sys.argv) > 1 else None)
    failed = False

    for name, text, size, tilt, seed in POPUPS:
        image = draw_popup(text, size, tilt, seed, font_path)
        tiles = count_tiles(image)
        image.save(os.path.join(HERE, name + ".png"), transparency=0)
        status = "ok" if tiles <= MAX_TILES else "TOO MANY"
        failed |= tiles > MAX_TILES
        print(f"{name}.png: {tiles} tiles ({status}, limit {MAX_TILES})")

    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
