#!/usr/bin/env python3
"""
Build the Wood VeslyUI style from the vanilla UI graphics in ../Vanilla.

Each vanilla image keeps its tile layout and palette indices, so the game's
tilemaps still line up. Two things change:
  * the palettes: oak / mahogany / moss-stained / weathered wood instead of
    vanilla's blue / red / green / sepia, and brass instead of gold trim;
  * a wood-grain pattern on the fill colours. The pattern repeats every 8x8
    tile, so tiles the game repeats side by side join into continuous grain.

Outputs (next to this script):
  Wood_*.png            indexed images, read by Png2Dmp from the installer
  Wood_MenuPal.pal      8 menu palettes (4 window colours + 4 guide rows)
  Wood_BattlePal.pal    4 battle UI palettes (one per faction)
  Wood_PrepItemsPal.pal prep-screen funds/item OBJ palette

Png2Dmp maps pixels by colour, so every PNG palette here has 16 distinct
colours. Rerun this script after changing a colour, then rebuild.
"""
import os
import struct

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
VANILLA = os.path.join(HERE, "..", "Vanilla")


def rgb(text):
    return tuple(int(text[i:i + 2], 16) for i in (0, 2, 4))


# Menu palette index roles (vanilla gUiFramePaletteA):
#   0 backdrop, 1 white, 2 light trim, 3 mid trim, 4 dark trim,
#   5-9 light fill ramp (light -> dark), 10-15 frame ramp (light -> dark).
TRIM = ["7080A0", "F8F0D8", "E8C878", "A87838", "402010"]

WOODS = [
    # Oak: window colour 0, player faction.
    ["F4DCA8", "ECCC94", "E2BC80", "D6A86A", "C89456",
     "AC7038", "9C6030", "8C522A", "7C4624", "6C3A1C", "5C3016"],
    # Mahogany: window colour 1, enemy faction.
    ["F2D2BC", "EAC2A6", "E0B092", "D49A7C", "C68466",
     "A44C2C", "944026", "843620", "742E1C", "642616", "541E12"],
    # Moss-stained: window colour 2, allied (green) faction.
    ["E6E2B4", "DCD8A2", "CECC90", "C0BE7E", "B0AE6C",
     "727C3C", "646E34", "56602C", "4A5226", "3E4420", "323618"],
    # Weathered: window colour 3, other (purple) faction.
    ["EAE2D2", "DFD6C4", "D2C8B6", "C4BAA8", "B6AC9A",
     "8E8272", "7E7264", "6E6456", "60564A", "52483E", "443C34"],
]

# Battle palettes hold the same colours in another order (vanilla gUnknown_08802558).
BATTLE_ORDER = [0, 3, 1, 4, 2] + list(range(5, 16))

# Prep funds/items OBJ palette (vanilla gUnknown_08A1B638): 2-9 dark -> light ramp.
PREP_ITEMS = ["7080A0", "3A2614", "4A2E16", "5C3A1C", "6E4824", "80582E",
              "94683A", "A87C48", "BC9058", "CEA46A", "382818", "F8F0D8",
              "EAD8B4", "C0A47C", "8C7454", "000000"]

GRAIN_RAMPS_UI = [list(range(5, 10)), list(range(10, 16))]
GRAIN_RAMPS_PREP = [list(range(9, 1, -1))]

# Tile-local grain: two wavy lines per 8x8 tile.
GRAIN_WAVE = [0, 0, 1, 1, 1, 1, 0, 0]


def ui_row(wood, dim=1.0):
    row = [rgb(c) for c in TRIM] + [rgb(c) for c in wood]
    if dim != 1.0:
        row = [row[0]] + [tuple(int(v * dim) for v in c) for c in row[1:]]
    return row


def menu_palette():
    rows = [ui_row(w) for w in WOODS]
    rows += [ui_row(w, 0.78) for w in WOODS]
    return rows


def to_gba(rows):
    out = bytearray()
    for row in rows:
        for r, g, b in row:
            out += struct.pack("<H", (r >> 3) | (g >> 3) << 5 | (b >> 3) << 10)
    return bytes(out)


def is_grain_line(x, y):
    return (y - GRAIN_WAVE[x % 8]) % 4 == 1


def add_grain(image, ramps):
    px = image.load()
    lookup = {}
    for ramp in ramps:
        for i, index in enumerate(ramp):
            lookup[index] = (ramp, i)

    for y in range(image.height):
        for x in range(image.width):
            index = px[x, y]
            if index not in lookup or not is_grain_line(x, y):
                continue
            ramp, i = lookup[index]
            # One step darker; the darkest shade steps lighter instead.
            px[x, y] = ramp[i + 1] if i + 1 < len(ramp) else ramp[i - 1]


def save(image, palette, name):
    assert len(set(palette)) == 16, f"{name}: palette colours must be distinct"
    image.putpalette([c for color in palette for c in color])
    image.save(os.path.join(HERE, name), transparency=0)


def main():
    menu = menu_palette()
    battle = [[row[i] for i in BATTLE_ORDER] for row in menu[:4]]
    prep_items = [rgb(c) for c in PREP_ITEMS]

    open(os.path.join(HERE, "Wood_MenuPal.pal"), "wb").write(to_gba(menu))
    open(os.path.join(HERE, "Wood_BattlePal.pal"), "wb").write(to_gba(battle))
    open(os.path.join(HERE, "Wood_PrepItemsPal.pal"), "wb").write(to_gba([prep_items]))

    jobs = [
        # vanilla name, palette for the PNG, grain ramps
        ("Main", menu[0], GRAIN_RAMPS_UI),
        ("Preps", menu[0], GRAIN_RAMPS_UI),
        ("PrepsUseGfx", menu[0], GRAIN_RAMPS_UI),
        ("PrepPopupWindow", menu[0], GRAIN_RAMPS_UI),
        ("ShopGoldDisplay", menu[0], GRAIN_RAMPS_UI),
        ("PrepItems", prep_items, GRAIN_RAMPS_PREP),
        ("Battle", battle[0], GRAIN_RAMPS_UI),
        ("Left", battle[0], GRAIN_RAMPS_UI),
        ("LeftItem", battle[0], GRAIN_RAMPS_UI),
        ("Right", battle[0], GRAIN_RAMPS_UI),
        ("RightItem", battle[0], GRAIN_RAMPS_UI),
    ]
    for name, palette, ramps in jobs:
        image = Image.open(os.path.join(VANILLA, f"Vanilla_{name}.png")).copy()
        add_grain(image, ramps)
        save(image, palette, f"Wood_{name}.png")
        print(f"Wood_{name}.png")


if __name__ == "__main__":
    main()
