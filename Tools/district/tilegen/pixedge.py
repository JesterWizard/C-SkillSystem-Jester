# Pixel-edge compatibility: two metatiles may touch when their shared 16-pixel edge is as smooth as a vanilla join.
import struct
import numpy as np


def meta_pixels(ts, m):
    gfx, pals, cfg = ts
    out = np.zeros((16, 16, 3), dtype=np.int16)
    for q in range(4):
        e = struct.unpack("<H", cfg[8 * m + 2 * q:8 * m + 2 * q + 2])[0]
        tno = e & 0x3FF
        hf = (e >> 10) & 1
        vf = (e >> 11) & 1
        pl = (e >> 12) & 15
        base = tno * 32
        for y in range(8):
            for x in range(8):
                sx = 7 - x if hf else x
                sy = 7 - y if vf else y
                o = base + sy * 4 + sx // 2
                c = 0
                if o < len(gfx):
                    b = gfx[o]
                    c = (b >> 4) if sx & 1 else b & 15
                col = pals[pl][c] if pl < len(pals) else (255, 0, 255)
                out[(q >> 1) * 8 + y, (q & 1) * 8 + x] = col
    return out


def edge_cost(A, B, d):
    """d: 0 = B right of A, 2 = B below A. Mean colour jump across the edge minus the tiles' own texture."""
    if d == 0:
        a1, a0, b0, b1 = A[:, 15], A[:, 14], B[:, 0], B[:, 1]
    else:
        a1, a0, b0, b1 = A[15, :], A[14, :], B[0, :], B[1, :]
    jump = np.abs(a1 - b0).sum(axis=1).mean()
    tex = (np.abs(a1 - a0).sum(axis=1).mean() + np.abs(b0 - b1).sum(axis=1).mean()) / 2
    return jump - tex


def extend_model(model, ts, thresh=12.0, same_terrain_only=True, terrain=None, exclude=()):
    order, idx, comp, pats, freq = model
    px = [meta_pixels(ts, v) for v in order]
    n = len(order)
    added = 0
    for a in range(n):
        for b in range(n):
            for d, opp in ((0, 1), (2, 3)):
                if (comp[d][a] >> b) & 1:
                    continue
                if (order[a] in exclude) != (order[b] in exclude):
                    continue
                if same_terrain_only and terrain and terrain(order[a]) != terrain(order[b]):
                    continue
                if edge_cost(px[a], px[b], d) <= thresh:
                    comp[d][a] |= 1 << b
                    comp[opp][b] |= 1 << a
                    added += 1
    return added
