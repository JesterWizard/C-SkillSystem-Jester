#!/usr/bin/env python3
"""
hexshop_building.py: build the Hex shop building of each district in a copy of the district's tileset.

The building in each district's Hex shop slot (a vanilla vendor house or hut) gets a slate roof and a small gold-rimmed
Hex card sign. The changed 8x8 tiles go into tile slots no metatile of the tileset uses; the changed metatiles go into
metatile slots that no vanilla map of the tileset and no district layout uses. The door metatile gets terrain 0x03
(village), so free movement runs the door's VISIT location event when the player stands on it.

Outputs, per tileset copy: <key>_obj.bin (raw 4bpp tiles), <key>_cfg.bin (raw tile config, 0x2400 bytes), and for each
district the layout JSON with the building cells replaced.
"""
import argparse
import json
import os
import struct
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "tilegen"))
from fe8map import tileset, load_map  # noqa: E402

SIGN = ["..OOOO..",
        ".OGGGGO.",
        "OGRRWWGO",
        "OGRRWWGO",
        "OGRRWWGO",
        ".OGGGGO.",
        "..OOOO.."]
SIGN_SMALL = [".OOOO.",
              "OGRWGO",
              "OGRWGO",
              ".OOOO."]

# One entry per tileset copy. block: metatiles of the building (rows); region: roof pixels (x0, y0, x1, y1) in block
# pixels; ramp: slate colours (dark, mid, light) taken from the building's own bank; keep: colours left as they are in
# the roof region; sign: top-left pixel of the sign and its colours; door: (col, row) of the door cell in the block.
SETS = {
    "town": dict(tsid=(0x0E, 0x0F, 0x10), maps=[0x11, 0x1F, 0x2B, 0x73, 0xD2, 0xEB],
                 districts={"Serafew": (18, 9), "PortKiris": (18, 11)},
                 block=[[672, 609, 674], [704, 641, 706]], door=(1, 1),
                 region=(12, 1, 35, 16), ramp=[(56, 64, 128), (56, 120, 176), (104, 144, 192)],
                 keep={(96, 88, 64), (248, 248, 248), (208, 224, 232), (160, 168, 160)},
                 sign=(20, 17), small=False,
                 signc={"O": (96, 88, 64), "G": (200, 208, 80), "R": (200, 24, 0), "W": (248, 248, 248)}),
    "caer": dict(tsid=(0x2E, 0x34, 0x30), maps=[0x31, 0x35, 0xD8],
                 districts={"CaerPelyn": (18, 8)},
                 block=[[836]], door=(0, 0),
                 region=(0, 0, 16, 9), ramp=[(88, 88, 56), (160, 168, 160), (208, 224, 232)],
                 keep={(248, 248, 248), (208, 224, 232), (160, 168, 160), (88, 88, 56)},
                 sign=(5, 2), small=True,
                 signc={"O": (88, 88, 56), "G": (208, 200, 80), "R": (200, 0, 0), "W": (248, 248, 248)}),
    "jeh": dict(tsid=(0x42, 0x43, 0x44), maps=[0x45],
                districts={"Jehanna": (15, 10)},
                block=[[904]], door=(0, 0),
                region=(0, 0, 16, 9), ramp=[(88, 88, 56), (136, 144, 136), (184, 200, 208)],
                keep={(240, 248, 232), (184, 200, 208), (136, 144, 136), (112, 104, 64), (120, 56, 24)},
                sign=(5, 2), small=True,
                signc={"O": (112, 104, 64), "G": (216, 184, 64), "R": (184, 88, 48), "W": (240, 248, 232)}),
}


def lum(c):
    return 0.3 * c[0] + 0.59 * c[1] + 0.11 * c[2]


def is_ground(c):
    r, g, b = c
    return g > r + 20 and g > b


def tile_rgb(gfx, pals, e):
    tno, hf, vf, pl = e & 0x3FF, (e >> 10) & 1, (e >> 11) & 1, (e >> 12) & 15
    out = np.zeros((8, 8, 3), dtype=np.int32)
    for y in range(8):
        for x in range(8):
            sx = 7 - x if hf else x
            sy = 7 - y if vf else y
            o = tno * 32 + sy * 4 + sx // 2
            c = (gfx[o] >> 4) if sx & 1 else gfx[o] & 15
            out[y, x] = pals[pl][c]
    return out


def meta_entries(cfg, m):
    return list(struct.unpack("<4H", cfg[8 * m:8 * m + 8]))


def meta_rgb(gfx, pals, cfg, m):
    img = np.zeros((16, 16, 3), dtype=np.int32)
    for q, e in enumerate(meta_entries(cfg, m)):
        img[(q >> 1) * 8:(q >> 1) * 8 + 8, (q & 1) * 8:(q & 1) * 8 + 8] = tile_rgb(gfx, pals, e)
    return img


def pack_tile(ix):
    b = bytearray(32)
    for y in range(8):
        for x in range(0, 8, 2):
            b[y * 4 + x // 2] = (int(ix[y, x]) & 15) | ((int(ix[y, x + 1]) & 15) << 4)
    return bytes(b)


def quantize(target, orig, pals):
    """Bank and indices (1..15) for an 8x8 target; unchanged pixels weigh 25x so the rest of the tile survives."""
    changed = (target != orig).any(-1)
    best = None
    for p in range(5):
        P = np.array(pals[p][1:], dtype=np.int32)
        d = ((target[:, :, None, :] - P[None, None, :, :]) ** 2).sum(-1)
        k = d.argmin(-1)
        err = (d.min(-1) * np.where(changed, 1, 25)).sum()
        if best is None or err < best[2]:
            best = (p, k + 1, err)
    return best[0], best[1]


def build(st, layouts):
    gfx, pals, cfg = tileset(*st["tsid"])
    gfx, cfg = bytearray(gfx), bytearray(cfg)
    block = st["block"]
    bh, bw = len(block), len(block[0])
    img = np.zeros((bh * 16, bw * 16, 3), dtype=np.int32)
    for j in range(bh):
        for i in range(bw):
            img[j * 16:j * 16 + 16, i * 16:i * 16 + 16] = meta_rgb(gfx, pals, cfg, block[j][i])
    new = img.copy()
    x0, y0, x1, y1 = st["region"]
    ramp = st["ramp"]
    for y in range(y0, y1):
        for x in range(x0, x1):
            c = tuple(int(v) for v in img[y, x])
            if c in st["keep"] or is_ground(c):
                continue
            l = lum(c)
            new[y, x] = ramp[0 if l < 90 else (1 if l < 150 else 2)]
    shape = SIGN_SMALL if st["small"] else SIGN
    sx, sy = st["sign"]
    for j, row in enumerate(shape):
        for i, ch in enumerate(row):
            if ch != ".":
                new[sy + j, sx + i] = st["signc"][ch]

    referenced = {e & 0x3FF for m in range(1024) for e in meta_entries(cfg, m)}
    free_tiles = [t for t in range(1023, -1, -1) if t not in referenced]
    used_meta = set()
    for mid in st["maps"]:
        used_meta |= set(load_map(mid)[2])
    for lay in layouts.values():
        used_meta |= set(lay["tiles"])
    free_meta = [m for m in range(1023, -1, -1) if m not in used_meta]

    mapping = {}
    for j in range(bh):
        for i in range(bw):
            cell_old = img[j * 16:j * 16 + 16, i * 16:i * 16 + 16]
            cell_new = new[j * 16:j * 16 + 16, i * 16:i * 16 + 16]
            if (cell_old == cell_new).all() and (i, j) != st["door"]:
                continue
            m_old = block[j][i]
            m_new = free_meta.pop(0)
            entries = []
            for q, e_old in enumerate(meta_entries(cfg, m_old)):
                ty, tx = (q >> 1) * 8, (q & 1) * 8
                t_old, t_new = cell_old[ty:ty + 8, tx:tx + 8], cell_new[ty:ty + 8, tx:tx + 8]
                if (t_old == t_new).all():
                    entries.append(e_old)
                    continue
                bank, ix = quantize(t_new, t_old, pals)
                t = free_tiles.pop(0)
                if (t + 1) * 32 > len(gfx):
                    gfx.extend(b"\0" * ((t + 1) * 32 - len(gfx)))
                gfx[t * 32:t * 32 + 32] = pack_tile(ix)
                entries.append(t | (bank << 12))
            cfg[8 * m_new:8 * m_new + 8] = struct.pack("<4H", *entries)
            cfg[0x2000 + m_new] = 0x03 if (i, j) == st["door"] else cfg[0x2000 + m_old]
            mapping[(i, j)] = (m_old, m_new)
    return bytes(gfx), bytes(cfg), mapping


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--chapters", required=True, help="CustomCampaign/Chapters directory (reads <Name>/map/<Name>_layout.json)")
    ap.add_argument("--out-dir", required=True)
    ap.add_argument("--write-layouts", action="store_true", help="also rewrite the district layout JSON files in place")
    args = ap.parse_args()
    os.makedirs(args.out_dir, exist_ok=True)
    for key, st in SETS.items():
        layouts = {}
        for name in st["districts"]:
            with open(os.path.join(args.chapters, name, "map", name + "_layout.json")) as f:
                layouts[name] = json.load(f)
        gfx, cfg, mapping = build(st, layouts)
        open(os.path.join(args.out_dir, key + "_obj.bin"), "wb").write(gfx)
        open(os.path.join(args.out_dir, key + "_cfg.bin"), "wb").write(cfg)
        print(key, "tiles", len(gfx) // 32, "changed cells", {f"{k}": v for k, v in mapping.items()})
        for name, (bx, by) in st["districts"].items():
            lay = layouts[name]
            w = lay["w"]
            for (i, j), (m_old, m_new) in mapping.items():
                k = (by + j) * w + bx + i
                if lay["tiles"][k] not in (m_old, m_new):
                    sys.exit(f"{name}: cell ({bx + i},{by + j}) holds {lay['tiles'][k]}, expected {m_old}")
                lay["tiles"][k] = m_new
            out = os.path.join(args.chapters, name, "map", name + "_layout.json") if args.write_layouts \
                else os.path.join(args.out_dir, name + "_layout.json")
            with open(out, "w") as f:
                json.dump(lay, f)
            print(" ", name, "->", out)


if __name__ == "__main__":
    main()
