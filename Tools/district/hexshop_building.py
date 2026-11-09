#!/usr/bin/env python3
"""
hexshop_building.py: draw the Hex shop building of each district into a copy of the district's tileset.

The building in each district's Hex shop slot is replaced by a market stall from a supplied illustration: a blue hipped
roof, a cream stone wall with two green windows, a red-and-white striped awning and a wooden door. The art is
hexshop_art/stall.txt, a 48x32 block (3x2 metatiles) used by every tileset copy. '.' in the art is the ground under the
stall, copied from the block's own left strip: the town yard, the Caer Pelyn plaza floor or the Jehanna sand.

The changed 8x8 tiles go into tile slots no metatile of the tileset uses; the changed metatiles go into metatile slots
that no vanilla map of the tileset and no district layout uses. The door metatile gets terrain 0x03 (village), so free
movement runs the door's VISIT location event when the player stands on it.

Each set also has a palette copy (pal_copy). The stall's colours (outline, four roof blues, three wall creams, awning
red and orange, window green, door brown) are not in the vanilla palettes, so each set gets its own palette: its
vanilla palette (base) with bank 5 redefined (bank 5 is unused by these tilesets) to the stall's 12 colours plus 3
ground colours. The copies are town_pal.bin (palette 0xEE: Serafew, Port Kiris), caer_pal.bin (0xEF: Caer Pelyn) and
jeh_pal.bin (0xF0: Jehanna). Every 8x8 tile that holds stall pixels uses bank 5; its ground pixels go to the nearest
bank 5 colour. Cells of the block other than the door get terrain body_terrain when the set gives one (the caer and jeh
blocks replace plain ground, so they need a building terrain; the town block keeps the vanilla house's terrain).

Outputs, per tileset copy: <key>_obj.bin (raw 4bpp tiles), <key>_cfg.bin (raw tile config, 0x2400 bytes), and for each
district the layout JSON with the building cells replaced. A set with a palette copy also writes its raw palette
(<pal_copy out>, 320 bytes). With --png DIR, also previews of each district with the new tiles and a close-up of the
building block.
"""
import argparse
import json
import os
import struct
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "tilegen"))
from fe8map import ROM, binptr, rom_at, tileset, load_map, render, render_grid  # noqa: E402

ART_DIR = os.path.join(HERE, "hexshop_art")

# The stall art's characters and colours, shared by every set. STALL_BANK is bank 5 of each palette copy without its
# 3 ground colours.
STALL_COLORS = {"K": (72, 64, 32), "b": (48, 48, 112), "q": (56, 80, 168), "l": (80, 136, 216),
                "w": (152, 200, 248), "C": (248, 248, 184), "L": (240, 224, 136), "s": (184, 160, 80),
                "R": (184, 56, 16), "y": (248, 88, 24), "G": (136, 200, 96), "n": (112, 104, 56)}
STALL_BANK = [(72, 64, 32), (48, 48, 112), (56, 80, 168), (80, 136, 216), (152, 200, 248), (248, 248, 184),
              (240, 224, 136), (184, 160, 80), (184, 56, 16), (248, 88, 24), (136, 200, 96), (112, 104, 56)]

# One entry per tileset copy. block: metatiles of the building (rows); art: file in hexshop_art/; colors: art char ->
# RGB; yard: '.' inside the block (x 8..39) is the ground copied from the block's own left strip, '.' outside it keeps
# the vanilla pixel; door: (col, row) of the door cell in the block; body_terrain: terrain of the other block cells
# (optional); districts: name -> (col, row) of the block's top-left cell in that district's layout; pal_copy: the
# set's palette copy (id, output file, bank, base palette, the bank's 15 colours).
SETS = {
    "town": dict(tsid=(0x0E, 0x0F, 0x10), maps=[0x11, 0x1F, 0x2B, 0x73, 0xD2, 0xEB],
                 districts={"Serafew": (4, 18), "PortKiris": (10, 14)},
                 block=[[672, 609, 674], [704, 641, 706]], door=(1, 1),
                 art="stall.txt", yard=True,
                 pal_copy=dict(id=0xEE, out="town_pal.bin", bank=5, base=0x0F,
                               colors=STALL_BANK + [(104, 144, 192), (120, 192, 128), (96, 160, 96)]),
                 colors=STALL_COLORS),
    "caer": dict(tsid=(0x2E, 0x34, 0x30), maps=[0x31, 0x35, 0xD8],
                 districts={"CaerPelyn": (10, 7)},
                 block=[[723, 723, 723], [723, 723, 723]], door=(1, 1), body_terrain=0x2E,
                 art="stall.txt", yard=True,
                 pal_copy=dict(id=0xEF, out="caer_pal.bin", bank=5, base=0x34,
                               colors=STALL_BANK + [(248, 248, 216), (240, 240, 168), (224, 216, 136)]),
                 colors=STALL_COLORS),
    "jeh": dict(tsid=(0x42, 0x43, 0x44), maps=[0x45],
                districts={"Jehanna": (15, 5)},
                block=[[433, 433, 433], [433, 433, 433]], door=(1, 1), body_terrain=0x2E,
                art="stall.txt", yard=True,
                pal_copy=dict(id=0xF0, out="jeh_pal.bin", bank=5, base=0x43,
                              colors=STALL_BANK + [(248, 248, 152), (232, 232, 144), (248, 248, 208)]),
                colors=STALL_COLORS),
}


def load_art(name, w, h, colors):
    """Art rows of a file (lines starting with '#' are comments). Exits with a message on a bad size or colour."""
    rows = []
    with open(os.path.join(ART_DIR, name)) as f:
        for line in f:
            line = line.rstrip("\r\n")
            if line.startswith("#") or not line.strip():
                continue
            rows.append(line)
    if len(rows) != h:
        sys.exit(f"{name}: {len(rows)} art rows, expected {h}")
    for y, row in enumerate(rows):
        if len(row) != w:
            sys.exit(f"{name}: row {y} has {len(row)} chars, expected {w}")
        for ch in row:
            if ch != "." and ch not in colors:
                sys.exit(f"{name}: art char {ch!r} at row {y} has no colour in the set")
    return rows


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


def to555(rgb):
    """8-bit RGB to GBA BGR555 (0..0x7FFF)."""
    r, g, b = (v >> 3 for v in rgb)
    return r | (g << 5) | (b << 10)


def from555(c):
    """GBA BGR555 to the 8-bit RGB that fe8map shows for it."""
    return ((c & 31) * 8, ((c >> 5) & 31) * 8, ((c >> 10) & 31) * 8)


def district_pals(st, pals):
    """The palettes this set's tiles are quantized and previewed with: the vanilla ones, with bank 5 replaced by the
    palette copy (index 0 kept, indices 1..15 the copy's colours, as they are written to the file)."""
    if "pal_copy" not in st:
        return pals
    pc = st["pal_copy"]
    out = [list(b) for b in pals]
    out[pc["bank"]] = [pals[pc["bank"]][0]] + [from555(to555(c)) for c in pc["colors"]]
    return out


def pal_copy_bytes(st):
    """The raw 320-byte palette of the copy: the set's vanilla palette (base), bank indices 1..15 replaced."""
    pc = st["pal_copy"]
    palp = rom_at(binptr("obj-palette.bin", pc["base"]))
    raw = bytearray(ROM[palp:palp + 320])
    assert len(raw) == 320
    base = pc["bank"] * 32
    for i, c in enumerate(pc["colors"]):
        raw[base + 2 * (i + 1):base + 2 * (i + 1) + 2] = struct.pack("<H", to555(c))
    return bytes(raw)


def quantize(target, orig, pals, nbanks=5):
    """Bank and indices (1..15) for an 8x8 target; unchanged pixels weigh 25x so the rest of the tile survives.
    Banks 0..nbanks-1 are tried."""
    changed = (target != orig).any(-1)
    best = None
    for p in range(nbanks):
        P = np.array(pals[p][1:], dtype=np.int32)
        d = ((target[:, :, None, :] - P[None, None, :, :]) ** 2).sum(-1)
        k = d.argmin(-1)
        err = (d.min(-1) * np.where(changed, 1, 25)).sum()
        if best is None or err < best[2]:
            best = (p, k + 1, err)
    return best[0], best[1]


def paint(st, img):
    """The new building block: the art composited over the vanilla block image."""
    h, w = img.shape[:2]
    rows = load_art(st["art"], w, h, st["colors"])
    new = img.copy()
    for y, row in enumerate(rows):
        for x, ch in enumerate(row):
            if ch == ".":
                if st.get("yard"):
                    if 8 <= x < 40:
                        sx = x % 8 if (x // 8) % 2 == 0 else 7 - (x % 8)
                        new[y, x] = img[y, sx]
                    # outside the yard range the vanilla pixel stays
            else:
                new[y, x] = st["colors"][ch]
    return new


def build(st, layouts):
    gfx, pals, cfg = tileset(*st["tsid"])
    gfx, cfg = bytearray(gfx), bytearray(cfg)
    qpals = district_pals(st, pals)
    nbanks = st["pal_copy"]["bank"] + 1 if "pal_copy" in st else 5
    block = st["block"]
    bh, bw = len(block), len(block[0])
    img = np.zeros((bh * 16, bw * 16, 3), dtype=np.int32)
    for j in range(bh):
        for i in range(bw):
            img[j * 16:j * 16 + 16, i * 16:i * 16 + 16] = meta_rgb(gfx, pals, cfg, block[j][i])
    new = paint(st, img)
    rows = load_art(st["art"], bw * 16, bh * 16, st["colors"])

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
                ay, ax = j * 16 + ty, i * 16 + tx
                if "pal_copy" in st and any(rows[ay + yy][ax + xx] != "." for yy in range(8) for xx in range(8)):
                    _, ix = quantize(t_new, t_old, [qpals[st["pal_copy"]["bank"]]], 1)
                    bank = st["pal_copy"]["bank"]
                else:
                    bank, ix = quantize(t_new, t_old, qpals, nbanks)
                t = free_tiles.pop(0)
                if (t + 1) * 32 > len(gfx):
                    gfx.extend(b"\0" * ((t + 1) * 32 - len(gfx)))
                gfx[t * 32:t * 32 + 32] = pack_tile(ix)
                entries.append(t | (bank << 12))
            cfg[8 * m_new:8 * m_new + 8] = struct.pack("<4H", *entries)
            cfg[0x2000 + m_new] = 0x03 if (i, j) == st["door"] else st.get("body_terrain", cfg[0x2000 + m_old])
            mapping[(i, j)] = (m_old, m_new)
    return bytes(gfx), bytes(cfg), mapping


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--chapters", required=True, help="CustomCampaign/Chapters directory (reads <Name>/map/<Name>_layout.json)")
    ap.add_argument("--out-dir", required=True)
    ap.add_argument("--write-layouts", action="store_true", help="also rewrite the district layout JSON files in place")
    ap.add_argument("--png", metavar="DIR", help="also save previews: each district with the new tiles, and the block at 8x")
    args = ap.parse_args()
    os.makedirs(args.out_dir, exist_ok=True)
    if args.png:
        os.makedirs(args.png, exist_ok=True)
    for key, st in SETS.items():
        layouts = {}
        for name in st["districts"]:
            with open(os.path.join(args.chapters, name, "map", name + "_layout.json")) as f:
                layouts[name] = json.load(f)
        gfx, cfg, mapping = build(st, layouts)
        open(os.path.join(args.out_dir, key + "_obj.bin"), "wb").write(gfx)
        open(os.path.join(args.out_dir, key + "_cfg.bin"), "wb").write(cfg)
        if "pal_copy" in st:
            pal_path = os.path.join(args.out_dir, st["pal_copy"]["out"])
            open(pal_path, "wb").write(pal_copy_bytes(st))
            print(key, "palette copy", pal_path)
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
        if args.png:
            pals = district_pals(st, tileset(*st["tsid"])[1])
            ts = (gfx, pals, cfg)
            for name in st["districts"]:
                lay = layouts[name]
                render_grid(lay["w"], lay["h"], lay["tiles"], ts, os.path.join(args.png, name + ".png"), 2)
                print("  preview", os.path.join(args.png, name + ".png"))
            block = st["block"]
            bh, bw = len(block), len(block[0])
            btiles = []
            for j in range(bh):
                for i in range(bw):
                    btiles.append(mapping[(i, j)][1] if (i, j) in mapping else block[j][i])
            bpath = os.path.join(args.png, key + "_block.png")
            render(bw, bh, btiles, ts, bpath, 8)
            print("  preview", bpath)


if __name__ == "__main__":
    main()
