#!/usr/bin/env python3
"""
Draw the text engine's popup graphics: a jagged white burst with big slanted
lettering, saved as 16-colour indexed PNGs next to this script.

The PNGs are committed, so this only needs rerunning to change the art.
Then convert each PNG with the makefile's grit rule, for example:
    grit Objection.png -gB 4 -gzl -m -mLf -mR4 -mzl -pn 16 -ftb -fh! -o Objection.img.bin

Every shape is drawn as a smooth mask at SUPERSAMPLE times the final size,
tilted there, scaled down and thresholded, so edges come out as clean pixel
steps instead of the noise a 1x rotate leaves.

The whole popup must stay within the dialogue text tiles it borrows at
runtime (240 unique 8x8 tiles after flip reduction); this script reports the
count for each popup.

Usage: python3 generate_popups.py [path/to/bold-italic-font.ttf]
"""
import math
import os
import sys

from PIL import Image, ImageChops, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
WIDTH, HEIGHT = 240, 128
SUPERSAMPLE = 4
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

# Pixels at the final size.
OUTLINE = 3
SHADOW_OFFSET = 3
HIGHLIGHT = 2

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


def big_canvas():
    return Image.new("L", (WIDTH * SUPERSAMPLE, HEIGHT * SUPERSAMPLE), 0)


def to_pixels(mask):
    """Scale a supersampled mask down to the final size and threshold it."""
    small = mask.resize((WIDTH, HEIGHT), Image.BOX)
    return small.point(lambda v: 255 if v >= 128 else 0)


def polygon_mask(points):
    mask = big_canvas()
    ImageDraw.Draw(mask).polygon(
        [(x * SUPERSAMPLE, y * SUPERSAMPLE) for x, y in points], fill=255)
    return to_pixels(mask)


def text_masks(text, size, tilt, font_path):
    """Fill, outline and shadow masks for the tilted lettering."""
    font = ImageFont.truetype(font_path, size * SUPERSAMPLE)
    stroke = OUTLINE * SUPERSAMPLE
    probe = ImageDraw.Draw(big_canvas())
    left, top, right, bottom = probe.textbbox((0, 0), text, font=font, stroke_width=stroke)
    x = (WIDTH * SUPERSAMPLE - (right - left)) // 2 - left
    y = (HEIGHT * SUPERSAMPLE - (bottom - top)) // 2 - top

    def render(stroke_width, offset):
        mask = big_canvas()
        ImageDraw.Draw(mask).text(
            (x + offset, y + offset), text, font=font, fill=255,
            stroke_width=stroke_width, stroke_fill=255)
        mask = mask.rotate(tilt, resample=Image.BICUBIC)
        return to_pixels(mask)

    fill = render(0, 0)
    outline = render(stroke, 0)
    shadow = render(stroke, SHADOW_OFFSET * SUPERSAMPLE)
    return fill, outline, shadow


def shifted(mask, dx, dy):
    return ImageChops.offset(mask, dx, dy)


def despeckle(image):
    """Recolor pixels whose four neighbours all disagree with them."""
    data = image.load()
    source = image.copy().load()
    for py in range(1, HEIGHT - 1):
        for px in range(1, WIDTH - 1):
            color = source[px, py]
            neighbours = [source[px - 1, py], source[px + 1, py],
                          source[px, py - 1], source[px, py + 1]]
            if color not in neighbours:
                data[px, py] = max(set(neighbours), key=neighbours.count)


def draw_popup(text, size, tilt, seed, font_path):
    image = Image.new("P", (WIDTH, HEIGHT), 0)
    image.putpalette([c for rgb in PALETTE for c in rgb] + [0] * (768 - 3 * len(PALETTE)))

    outer = polygon_mask(burst_points(seed, WIDTH / 2, HEIGHT / 2, 112, 58))
    inner = polygon_mask(burst_points(seed, WIDTH / 2, HEIGHT / 2, 104, 52))
    # Two-pixel burst outline: the outer shape minus itself shrunk by a pixel.
    outer_core = ImageChops.multiply(
        ImageChops.multiply(shifted(outer, 1, 0), shifted(outer, -1, 0)),
        ImageChops.multiply(shifted(outer, 0, 1), shifted(outer, 0, -1)))
    outer_core = ImageChops.multiply(
        outer_core,
        ImageChops.multiply(
            ImageChops.multiply(shifted(outer_core, 1, 0), shifted(outer_core, -1, 0)),
            ImageChops.multiply(shifted(outer_core, 0, 1), shifted(outer_core, 0, -1))))

    fill, outline, shadow = text_masks(text, size, tilt, font_path)
    # Highlight: a band along the top edge of each letter.
    highlight = ImageChops.subtract(fill, shifted(fill, 0, HIGHLIGHT))

    layers = [
        (outer, 1),
        (outer_core, 3),
        (inner, 2),
        (shadow, 5),
        (outline, 1),
        (fill, 4),
        (highlight, 6),
    ]
    out = image.load()
    for mask, color in layers:
        pixels = mask.load()
        for py in range(HEIGHT):
            for px in range(WIDTH):
                if pixels[px, py]:
                    out[px, py] = color

    despeckle(image)
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
