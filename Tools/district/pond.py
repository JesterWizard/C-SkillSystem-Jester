#!/usr/bin/env python3
"""pond.py LAYOUT.json OBJ_BIN PAL_BIN CFG_BIN X Y W H [--png OUT.png] [--avoid-bank N]

Picks the metatiles of a small closed pond: the W x H cells at (X, Y) of a district layout. Vanilla maps never put water
inside a closed shape, so the tile generator can't learn pond joins (tilegen/README.md, "Small closed ponds"). This
script picks, from the tileset's water metatiles, the arrangement whose 16-pixel edges join best with each other and with
the ground around the pond (tilegen/pixedge.py's edge_cost, beam search). It prints the cells as compose-spec edits,
[x, y, metatile], ready for the "tiles" list of Tools/district/compose/<Name>.json.

OBJ_BIN, PAL_BIN and CFG_BIN are the district's own tileset copies (CustomCampaign/Chapters/_hexshop/*_obj.bin,
*_pal.bin, *_cfg.bin), so the pond is judged with the tiles and colours the game shows. --avoid-bank N skips metatiles that use palette bank N (bank 5 of the
district palettes holds the Hex shop stall's colours).
"""
import json
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "tilegen"))

import fe8map  # noqa: E402
import pixedge  # noqa: E402

WATER = {0x10, 0x15, 0x16, 0x36, 0x3C}


def load_pals(path):
    d = open(path, "rb").read()
    out = []
    for p in range(len(d) // 32):
        row = []
        for i in range(16):
            c = struct.unpack_from("<H", d, p * 32 + i * 2)[0]
            row.append(((c & 31) * 8, ((c >> 5) & 31) * 8, ((c >> 10) & 31) * 8))
        out.append(row)
    return out


def banks(cfg, m):
    return {(struct.unpack_from("<H", cfg, 8 * m + 2 * q)[0] >> 12) & 15 for q in range(4)}


def main():
    a = sys.argv[1:]
    layout, objp, palp, cfgp = a[0], a[1], a[2], a[3]
    X, Y, W, H = (int(v) for v in a[4:8])
    png = a[a.index("--png") + 1] if "--png" in a else None
    avoid = int(a[a.index("--avoid-bank") + 1]) if "--avoid-bank" in a else None

    L = json.load(open(layout))
    w, h, tiles = L["w"], L["h"], list(L["tiles"])
    gfx = open(objp, "rb").read()
    cfg = open(cfgp, "rb").read()
    ts = (gfx, load_pals(palp), cfg)

    cands = [m for m in range(1024) if cfg[0x2000 + m] in WATER and (avoid is None or avoid not in banks(cfg, m))]
    px = {}

    def P(m):
        if m not in px:
            px[m] = pixedge.meta_pixels(ts, m)
        return px[m]

    cells = [(x, y) for y in range(Y, Y + H) for x in range(X, X + W)]
    inside = set(cells)

    def cost_at(assign, x, y, m):
        c = 0.0
        for dx, dy, d, mine_first in ((-1, 0, 0, False), (1, 0, 0, True), (0, -1, 2, False), (0, 1, 2, True)):
            nx, ny = x + dx, y + dy
            if not (0 <= nx < w and 0 <= ny < h):
                continue
            if (nx, ny) in inside:
                if (nx, ny) not in assign:
                    continue
                n = assign[(nx, ny)]
            else:
                n = tiles[ny * w + nx]
            c += max(0.0, pixedge.edge_cost(P(m), P(n), d) if mine_first else pixedge.edge_cost(P(n), P(m), d))
        return c

    beam = [(0.0, {})]
    for (x, y) in cells:
        nxt = []
        for score, assign in beam:
            for m in cands:
                nxt.append((score + cost_at(assign, x, y, m), {**assign, (x, y): m}))
        nxt.sort(key=lambda t: t[0])
        beam = nxt[:400]
    score, best = beam[0]
    print("cost %.1f" % score)
    print(json.dumps([[x, y, best[(x, y)]] for (x, y) in cells]))
    if png:
        for (x, y), m in best.items():
            tiles[y * w + x] = m
        x0, y0 = max(0, X - 3), max(0, Y - 3)
        x1, y1 = min(w, X + W + 3), min(h, Y + H + 3)
        sub = [tiles[yy * w + xx] for yy in range(y0, y1) for xx in range(x0, x1)]
        fe8map.render(x1 - x0, y1 - y0, sub, ts, png, scale=4)
        print("wrote", png)


if __name__ == "__main__":
    main()
