"""The fish species of the fishing journal: their looks, drawn pixel by pixel (imported by gen_fishing_art.py).

Each district's water has five species, one for each kind of fish in the game (small, medium, large, huge, golden), in
that order, and the waters go in the order of FISHING_WATER_* (include/kernel/fishing.h): Serafew Pond, Port Kiris
Harbour, Caer Pelyn Pond, Jehanna Oasis. The names are texts (MSG_FISHING_SP_*); `name` here is only a label. A species is drawn twice from the same description: a 64x32 portrait for the catch card and a 32x16 icon
for the journal. Each has its own 16-colour palette:

  0 transparent  1 outline  2 back dark  3 back  4 side  5 belly  6 fin  7 fin dark  8 pattern  9 eye ring
  10 pupil  11 shine  12 pattern 2  13 gill and mouth  14 accent  15 unused
"""
import math

def hsh(x, y, s=0):
    v = (x * 374761393 + y * 668265263 + s * 2246822519) & 0xFFFFFFFF
    v = ((v ^ (v >> 13)) * 1274126177) & 0xFFFFFFFF
    return (v ^ (v >> 16)) & 0xFF


def pal(outline, back_dark, back, side, belly, fin, fin_dark, pattern, pattern2=(0, 0, 0), accent=(0, 0, 0),
        eye=(232, 224, 200), shine=(255, 255, 255)):
    return [(0, 0, 0), outline, back_dark, back, side, belly, fin, fin_dark, pattern, eye, (16, 16, 24), shine,
            pattern2, tuple(max(0, c - 48) for c in back_dark), accent, (0, 0, 0)]


# Shape: len (body length, fraction of the width), depth (fraction of the height), peak (where the body is deepest,
# 0 tail .. 1 snout), ped (the tail stalk's depth, fraction of the body's), nose (2 round .. 1 pointed), belly (share of
# the depth below the centre line), tail (fork, square, round, lunate, eel), tailw (tail length, fraction of the width),
# dorsal / anal: lists of (u0, u1, height as a fraction of the depth, style tri|round|spiny|sail),
# pattern: a list of (kind, ...), whiskers, jaw (hooked lower jaw).
SPECIES = [
    # ---- Serafew Pond ----
    dict(name="Minnow", cm=(4, 9),
         pal=pal((24, 40, 32), (72, 96, 72), (104, 128, 88), (184, 192, 168), (232, 232, 216), (152, 160, 128),
                 (96, 104, 80), (56, 64, 56)),
         len=0.78, depth=0.42, peak=0.55, ped=0.38, nose=1.6, belly=0.5, tail="fork", tailw=0.2,
         dorsal=[(0.42, 0.58, 0.45, "tri")], anal=[(0.25, 0.4, 0.3, "tri")],
         pattern=[("stripe", 0.5, 1)]),
    dict(name="Perch", cm=(15, 35),
         pal=pal((32, 40, 16), (72, 96, 32), (120, 152, 48), (184, 200, 88), (232, 232, 176), (232, 120, 48),
                 (176, 72, 32), (48, 64, 24)),
         len=0.74, depth=0.66, peak=0.6, ped=0.32, nose=1.5, belly=0.5, tail="fork", tailw=0.2,
         dorsal=[(0.55, 0.84, 0.55, "spiny"), (0.3, 0.52, 0.45, "round")], anal=[(0.25, 0.4, 0.35, "tri")],
         pattern=[("bars", 5, 0.75)]),
    dict(name="Carp", cm=(35, 80),
         pal=pal((48, 32, 16), (112, 80, 32), (152, 112, 48), (200, 160, 72), (232, 208, 136), (168, 104, 48),
                 (112, 64, 24), (120, 88, 40), (232, 200, 112)),
         len=0.74, depth=0.72, peak=0.55, ped=0.4, nose=1.9, belly=0.48, tail="fork", tailw=0.2,
         dorsal=[(0.3, 0.68, 0.32, "round")], anal=[(0.25, 0.36, 0.3, "tri")],
         pattern=[("scales", 4)], whiskers=1),
    dict(name="Pike", cm=(70, 130),
         pal=pal((24, 40, 24), (48, 80, 40), (80, 120, 56), (128, 160, 80), (216, 224, 176), (120, 136, 64),
                 (72, 88, 40), (200, 216, 136)),
         len=0.82, depth=0.36, peak=0.45, ped=0.5, nose=1.15, belly=0.5, tail="fork", tailw=0.17,
         dorsal=[(0.12, 0.26, 0.5, "round")], anal=[(0.12, 0.24, 0.45, "round")],
         pattern=[("spots", 40, 8, 0.0, 0.7)], jaw=1),
    dict(name="Gold Koi", cm=(30, 70), golden=True,
         pal=pal((96, 48, 8), (216, 136, 24), (248, 184, 48), (255, 216, 96), (255, 240, 184), (255, 208, 120),
                 (224, 144, 48), (232, 72, 32), (255, 248, 216)),
         len=0.74, depth=0.6, peak=0.55, ped=0.36, nose=1.9, belly=0.5, tail="fork", tailw=0.2,
         dorsal=[(0.3, 0.68, 0.3, "round")], anal=[(0.25, 0.36, 0.28, "tri")],
         pattern=[("patches", 9), ("scales", 4)], whiskers=1),
    # ---- Port Kiris Harbour ----
    dict(name="Sardine", cm=(10, 20),
         pal=pal((16, 32, 56), (40, 88, 136), (72, 128, 184), (192, 208, 224), (240, 244, 248), (152, 176, 200),
                 (88, 112, 144), (24, 40, 72)),
         len=0.8, depth=0.4, peak=0.55, ped=0.35, nose=1.5, belly=0.55, tail="fork", tailw=0.18,
         dorsal=[(0.45, 0.6, 0.4, "tri")], anal=[(0.22, 0.32, 0.25, "tri")],
         pattern=[("dotrow", 0.42, 6)]),
    dict(name="Mackerel", cm=(25, 45),
         pal=pal((16, 32, 40), (32, 88, 96), (56, 136, 136), (184, 208, 208), (240, 244, 240), (112, 144, 144),
                 (64, 96, 96), (16, 32, 40)),
         len=0.8, depth=0.44, peak=0.55, ped=0.22, nose=1.4, belly=0.5, tail="fork", tailw=0.18,
         dorsal=[(0.55, 0.7, 0.45, "tri"), (0.3, 0.4, 0.3, "tri")], anal=[(0.28, 0.38, 0.28, "tri")],
         pattern=[("wavy", 0.42)], finlets=1),
    dict(name="Sea Bass", cm=(40, 90),
         pal=pal((32, 40, 48), (80, 96, 112), (120, 136, 152), (184, 192, 200), (232, 236, 240), (128, 136, 144),
                 (80, 88, 96), (96, 104, 120)),
         len=0.76, depth=0.56, peak=0.55, ped=0.4, nose=1.4, belly=0.5, tail="square", tailw=0.18,
         dorsal=[(0.55, 0.8, 0.55, "spiny"), (0.3, 0.52, 0.4, "round")], anal=[(0.25, 0.4, 0.35, "tri")],
         pattern=[("lines", 3)]),
    dict(name="Tuna", cm=(100, 250),
         pal=pal((8, 16, 40), (24, 40, 88), (40, 64, 128), (152, 168, 200), (232, 236, 244), (64, 80, 128),
                 (32, 40, 80), (248, 216, 56), accent=(248, 216, 56)),
         len=0.78, depth=0.62, peak=0.55, ped=0.14, nose=1.6, belly=0.52, tail="lunate", tailw=0.2,
         dorsal=[(0.55, 0.68, 0.45, "tri"), (0.36, 0.44, 0.6, "sail")], anal=[(0.34, 0.42, 0.55, "sail")],
         pattern=[("band", 0.48)], finlets=2),
    dict(name="Gold Bream", cm=(25, 55), golden=True,
         pal=pal((96, 40, 24), (216, 128, 56), (248, 176, 88), (255, 208, 128), (255, 236, 200), (248, 152, 136),
                 (208, 96, 88), (232, 112, 104), (255, 248, 216)),
         len=0.72, depth=0.76, peak=0.55, ped=0.3, nose=1.6, belly=0.48, tail="fork", tailw=0.22,
         dorsal=[(0.28, 0.75, 0.32, "spiny")], anal=[(0.22, 0.4, 0.28, "tri")],
         pattern=[("lines", 4), ("shine", 0)]),
    # ---- Caer Pelyn Pond ----
    dict(name="Loach", cm=(8, 15),
         pal=pal((40, 24, 16), (104, 72, 40), (144, 104, 64), (192, 160, 112), (232, 216, 176), (168, 136, 96),
                 (112, 80, 48), (64, 40, 24)),
         len=0.86, depth=0.3, peak=0.5, ped=0.65, nose=2.2, belly=0.5, tail="round", tailw=0.12,
         dorsal=[(0.38, 0.5, 0.35, "round")], anal=[(0.2, 0.28, 0.25, "round")],
         pattern=[("spots", 60, 6, 0.0, 0.9)], whiskers=2),
    dict(name="Trout", cm=(25, 50),
         pal=pal((40, 32, 16), (104, 88, 40), (152, 128, 64), (208, 184, 112), (240, 232, 192), (168, 144, 88),
                 (112, 96, 56), (40, 32, 24), (216, 56, 40)),
         len=0.78, depth=0.48, peak=0.55, ped=0.42, nose=1.7, belly=0.5, tail="square", tailw=0.17,
         dorsal=[(0.42, 0.6, 0.45, "tri")], anal=[(0.22, 0.34, 0.3, "tri")],
         pattern=[("spots", 36, 8, 0.0, 0.75), ("spots2", 16, 12, 0.3, 0.75)]),
    dict(name="Char", cm=(35, 70),
         pal=pal((24, 32, 24), (48, 72, 56), (72, 104, 80), (120, 144, 104), (232, 112, 48), (224, 104, 56),
                 (160, 64, 40), (224, 224, 192)),
         len=0.78, depth=0.48, peak=0.55, ped=0.42, nose=1.7, belly=0.5, tail="square", tailw=0.17,
         dorsal=[(0.42, 0.6, 0.45, "tri")], anal=[(0.22, 0.34, 0.3, "tri")],
         pattern=[("spots", 40, 8, 0.0, 0.62), ("finedge", 0)]),
    dict(name="Salmon", cm=(70, 120),
         pal=pal((40, 32, 48), (88, 96, 120), (128, 136, 160), (216, 184, 192), (244, 236, 236), (136, 136, 152),
                 (88, 88, 104), (40, 40, 56), (232, 136, 136)),
         len=0.8, depth=0.5, peak=0.55, ped=0.36, nose=1.5, belly=0.5, tail="square", tailw=0.17,
         dorsal=[(0.45, 0.62, 0.45, "tri")], anal=[(0.22, 0.34, 0.3, "tri")],
         pattern=[("spots", 30, 8, 0.0, 0.4), ("blush", 0.5)], jaw=1),
    dict(name="Golden Trout", cm=(25, 55), golden=True,
         pal=pal((96, 48, 16), (208, 144, 40), (248, 192, 72), (255, 224, 120), (255, 244, 200), (248, 168, 72),
                 (200, 112, 40), (120, 64, 24), (232, 64, 48)),
         len=0.78, depth=0.48, peak=0.55, ped=0.42, nose=1.7, belly=0.5, tail="square", tailw=0.17,
         dorsal=[(0.42, 0.6, 0.45, "tri")], anal=[(0.22, 0.34, 0.3, "tri")],
         pattern=[("stripe2", 0.55, 1), ("parr", 6), ("shine", 0)]),
    # ---- Jehanna Oasis ----
    dict(name="Pupfish", cm=(3, 7),
         pal=pal((16, 24, 64), (48, 88, 184), (80, 136, 224), (136, 184, 240), (216, 232, 248), (96, 152, 224),
                 (24, 40, 96), (16, 24, 64)),
         len=0.72, depth=0.6, peak=0.55, ped=0.5, nose=1.9, belly=0.5, tail="square", tailw=0.2,
         dorsal=[(0.4, 0.6, 0.45, "round")], anal=[(0.3, 0.45, 0.35, "round")],
         pattern=[("tailband", 0)]),
    dict(name="Tilapia", cm=(15, 40),
         pal=pal((32, 40, 32), (88, 104, 88), (128, 144, 120), (176, 184, 160), (224, 224, 208), (200, 96, 80),
                 (136, 64, 56), (96, 112, 96)),
         len=0.72, depth=0.72, peak=0.55, ped=0.36, nose=1.8, belly=0.5, tail="square", tailw=0.2,
         dorsal=[(0.25, 0.8, 0.3, "spiny")], anal=[(0.2, 0.4, 0.3, "tri")],
         pattern=[("bars", 6, 0.6)]),
    dict(name="Catfish", cm=(60, 140),
         pal=pal((32, 24, 24), (72, 64, 56), (112, 104, 88), (152, 144, 128), (216, 208, 192), (104, 96, 80),
                 (64, 56, 48), (64, 56, 48)),
         len=0.84, depth=0.42, peak=0.72, ped=0.45, nose=2.4, belly=0.42, tail="round", tailw=0.14,
         dorsal=[(0.62, 0.72, 0.5, "tri")], anal=[(0.05, 0.5, 0.3, "round")],
         pattern=[("mottle", 0)], whiskers=3, flathead=1),
    dict(name="Arapaima", cm=(150, 250),
         pal=pal((24, 32, 24), (48, 72, 56), (80, 104, 80), (144, 152, 120), (208, 200, 168), (128, 56, 48),
                 (88, 32, 32), (88, 112, 80), (200, 56, 48)),
         len=0.84, depth=0.42, peak=0.6, ped=0.55, nose=1.8, belly=0.5, tail="round", tailw=0.14,
         dorsal=[(0.03, 0.26, 0.4, "round")], anal=[(0.03, 0.24, 0.4, "round")],
         pattern=[("redrear", 0.42), ("scales", 5)]),
    dict(name="Gold Arowana", cm=(60, 100), golden=True,
         pal=pal((96, 56, 8), (200, 136, 24), (240, 184, 48), (255, 216, 96), (255, 240, 176), (248, 192, 72),
                 (200, 128, 24), (176, 112, 16), (255, 248, 216)),
         len=0.84, depth=0.42, peak=0.5, ped=0.5, nose=1.4, belly=0.5, tail="round", tailw=0.14,
         dorsal=[(0.04, 0.24, 0.4, "round")], anal=[(0.04, 0.34, 0.4, "round")],
         pattern=[("scales", 5), ("shine", 0)], whiskers=4, upmouth=1),
]

# A species not caught yet: its shape in the journal, dark against the window
SILHOUETTE = [(0, 0, 0)] + [(32, 40, 64)] + [(56, 64, 96)] * 13 + [(0, 0, 0)]


def paint(sp, w, h):
    """Draws a species facing right in a w x h image (palette indices as in the module docstring)."""
    img = [[0] * w for _ in range(h)]
    small = w < 48
    tailw = max(3, int(round(w * sp["tailw"]))) if sp["tail"] != "eel" else 0
    body_len = int(round(w * sp["len"]))
    x0 = max(tailw, (w - body_len - tailw) // 2 + tailw)    # the tail stalk
    x1 = min(w - 1, x0 + body_len - 1)                      # the snout
    D = h * sp["depth"]
    cy = h / 2.0 + (h * 0.04 if sp.get("dorsal") else 0)
    peak, ped, nose, belly = sp["peak"], sp["ped"], sp["nose"], sp["belly"]

    def prof(u):
        if u < peak:
            t = u / peak
            return ped + (1 - ped) * math.sin(math.pi / 2 * t) ** 0.9
        t = (u - peak) / (1 - peak)
        return max(0.0, 1 - t ** nose) ** (1 / nose)

    def u_of(x):
        return (x - x0) / float(max(1, x1 - x0))

    tops, bots = {}, {}
    for x in range(x0, x1 + 1):
        d = prof(u_of(x)) * D
        up, dn = d * (1 - belly), d * belly
        if sp.get("flathead") and u_of(x) > 0.7:
            dn *= 0.8
        tops[x] = int(round(cy - up))
        bots[x] = int(round(cy + dn)) - 1
        if bots[x] < tops[x]:
            bots[x] = tops[x]

    def body_px(x, y):
        return x in tops and tops[x] <= y <= bots[x]

    # Fins first (the body is drawn over their roots)
    def fin(u0, u1, hf, style, above):
        xa, xb = x0 + int(u0 * (x1 - x0)), x0 + int(u1 * (x1 - x0))
        if xb <= xa:
            xb = xa + 1
        H = hf * D
        for x in range(xa, xb + 1):
            t = (x - xa) / float(max(1, xb - xa))     # 0 back .. 1 front
            if style == "tri":
                f = t ** 0.8 if t < 0.85 else (1 - t) / 0.15 * 0.85 + 0.15 * t
                f = min(1.0, f)
            elif style == "round":
                f = math.sin(math.pi * t) ** 0.6
            elif style == "sail":
                f = (1 - t) ** 0.5 if t > 0.3 else t / 0.3
            else:  # spiny: a raked fan of spines
                f = 0.3 + 0.7 * math.sin(math.pi * (0.15 + 0.85 * t)) ** 0.7
            n = int(round(H * f))
            base = tops[x] if above else bots[x]
            for k in range(1, n + 1):
                y = base - k if above else base + k
                if 0 <= y < h:
                    c = 6
                    if style == "spiny" and not small and (x - xa) % 3 == 0:
                        c = 7
                    img[y][x] = c
            if style == "spiny" and not small and n > 0 and (x - xa) % 3 == 0:
                y = base - n - 1 if above else base + n + 1
                if 0 <= y < h:
                    img[y][x] = 7

    for f in sp.get("dorsal", []):
        fin(f[0], f[1], f[2], f[3], True)
    for f in sp.get("anal", []):
        fin(f[0], f[1], f[2], f[3], False)
    # Finlets (mackerel, tuna): a row of tiny fins before the tail
    for side in range(sp.get("finlets", 0) and 2):
        for k in range(3 if not small else 2):
            x = x0 + 2 + k * (3 if not small else 2)
            if x in tops:
                y = tops[x] - 1 if side == 0 else bots[x] + 1
                if 0 <= y < h:
                    img[y][x] = 14 if sp.get("finlets") == 2 else 6

    # The tail, from the stalk back to the left edge
    tail = sp["tail"]
    sh = (bots[x0] - tops[x0] + 1)
    tcy = (tops[x0] + bots[x0]) / 2.0
    spread_max = {"fork": 0.62, "square": 0.5, "round": 0.36, "lunate": 0.8}.get(tail, 0) * D
    for x in range(x0 - 1, x0 - tailw - 1, -1):
        if tail == "eel" or x < 0:
            break
        t = (x0 - x) / float(tailw)                  # 0 at the stalk .. 1 at the tip
        if tail == "round":
            # A fan: it widens from the stalk, then its end is rounded off
            half = sh / 2.0 + (spread_max - sh / 2.0) * min(1.0, t / 0.6)
            if t > 0.6:
                half *= math.sqrt(max(0.0, 1 - ((t - 0.6) / 0.42) ** 2))
        else:
            half = sh / 2.0 + (spread_max - sh / 2.0) * t ** (0.7 if tail == "lunate" else 1.0)
        notch = 0
        if tail == "fork":
            notch = half * max(0.0, (t - 0.35) / 0.65) * 0.75
        elif tail == "lunate":
            notch = half * max(0.0, (t - 0.3) / 0.7) * 0.6
        for y in range(int(round(tcy - half)), int(round(tcy + half)) + 1):
            if 0 <= y < h and abs(y - tcy) >= notch:
                img[y][x] = 6 if not (not small and abs(y - tcy) > half - 1.5 and tail in ("fork", "lunate")) else 7

    # The body, shaded from the back to the belly
    for x in range(x0, x1 + 1):
        for y in range(tops[x], bots[x] + 1):
            v = (y - tops[x]) / float(max(1, bots[x] - tops[x]))
            c = 2 if v < 0.14 else (3 if v < 0.36 else (4 if v < 0.66 else 5))
            img[y][x] = c

    def on_body(x, y, lo=0.0, hi=1.0):
        if not body_px(x, y):
            return False
        v = (y - tops[x]) / float(max(1, bots[x] - tops[x]))
        return lo <= v <= hi

    # Patterns
    for pat in sp.get("pattern", []):
        kind = pat[0]
        if kind == "stripe" or kind == "stripe2":
            c = 8 if kind == "stripe" else 12
            for x in range(x0 + 1, x1 - (x1 - x0) // 6):
                y = int(round(tops[x] + (bots[x] - tops[x]) * pat[1]))
                for k in range(pat[2] + (0 if small else 1)):
                    if on_body(x, y + k):
                        img[y + k][x] = c
        elif kind == "bars":
            n, depth_v = pat[1], pat[2]
            for b in range(n):
                xc = x0 + int((b + 0.6) * (x1 - x0) * 0.82 / n)
                bw = 1 if small else 2
                for x in range(xc, xc + bw):
                    for y in range(h):
                        if on_body(x, y, 0.08, depth_v):
                            img[y][x] = 8
        elif kind == "spots" or kind == "spots2":
            dens, seed, lo, hi = pat[1], pat[2], pat[3], pat[4]
            c = 8 if kind == "spots" else 12
            step = 2 if small else 3
            for x in range(x0 + 2, x1 - (x1 - x0) // 5, step):
                for y in range(0, h, step):
                    jx = x + (hsh(x, y, seed) & 1)
                    jy = y + ((hsh(x, y, seed) >> 1) & 1)
                    if hsh(x, y, seed + 7) < dens * 255 // 100 and on_body(jx, jy, lo + 0.05, hi):
                        img[jy][jx] = c
        elif kind == "dotrow":
            for k in range(pat[2]):
                x = x0 + int((k + 1) * (x1 - x0) * 0.7 / pat[2]) + (x1 - x0) // 5
                y = int(round(tops[x] + (bots[x] - tops[x]) * pat[1])) if x in tops else 0
                if on_body(x, y):
                    img[y][x] = 8
        elif kind == "wavy":
            for x in range(x0 + 1, x1 - (x1 - x0) // 5):
                for k in range(3):
                    y = tops[x] + 1 + k * (2 if small else 3) + int(round(math.sin(x * 0.9 + k) * (0.6 if small else 1.2)))
                    if on_body(x, y, 0.0, pat[1]):
                        img[y][x] = 8
        elif kind == "lines":
            for k in range(pat[1]):
                v = 0.3 + 0.45 * k / max(1, pat[1] - 1)
                for x in range(x0 + 1, x1 - (x1 - x0) // 5, 1 if not small else 2):
                    y = int(round(tops[x] + (bots[x] - tops[x]) * v))
                    if on_body(x, y) and not small:
                        img[y][x] = 8
        elif kind == "band":
            for x in range(x0 + 1, x1 - (x1 - x0) // 4):
                y = int(round(tops[x] + (bots[x] - tops[x]) * pat[1]))
                if on_body(x, y):
                    img[y][x] = 14
        elif kind == "scales":
            s = pat[1] if not small else 3
            for x in range(x0 + 1, x1 - (x1 - x0) // 4):
                for y in range(h):
                    if not on_body(x, y, 0.1, 0.8):
                        continue
                    row = (y + 100) // s
                    xx = x + (s // 2 if row & 1 else 0)
                    if (y + 100) % s == s - 1 and (xx % s) not in (0,):
                        if img[y][x] in (3, 4, 5):
                            img[y][x] = 8 if img[y][x] != 5 else 4
                    elif (xx % s) == 0 and (y + 100) % s >= s - 2 and img[y][x] in (3, 4):
                        img[y][x] = 12 if sp.get("golden") or img[y][x] == 4 else img[y][x]
        elif kind == "patches":
            for x in range(x0, x1 - (x1 - x0) // 6):
                for y in range(h):
                    n = (hsh(x // 4, y // 3, pat[1]) + hsh((x + 2) // 4, (y + 1) // 3, pat[1])) // 2
                    if on_body(x, y, 0.0, 0.7) and n > 150:
                        img[y][x] = 8
        elif kind == "mottle":
            for x in range(x0, x1 + 1):
                for y in range(h):
                    n = hsh(x // 2, y // 2, 3)
                    if on_body(x, y, 0.0, 0.7) and n > 170:
                        img[y][x] = 8 if img[y][x] != 2 else 1
        elif kind == "redrear":
            for x in range(x0, x0 + int((x1 - x0) * pat[1])):
                for y in range(h):
                    if on_body(x, y, 0.15, 0.95):
                        img[y][x] = 12
        elif kind == "blush":
            for x in range(x0 + 2, x1 - (x1 - x0) // 4):
                for y in range(h):
                    if on_body(x, y, pat[1] - 0.12, pat[1] + 0.06):
                        img[y][x] = 12
        elif kind == "parr":
            for b in range(pat[1]):
                xc = x0 + int((b + 0.8) * (x1 - x0) * 0.8 / pat[1])
                for y in range(h):
                    if on_body(xc, y, 0.42, 0.68) and not small:
                        img[y][xc] = 7
        elif kind == "tailband":
            for x in range(0, x0):
                for y in range(h):
                    if img[y][x] == 6 and x < x0 - tailw // 2:
                        img[y][x] = 8
        elif kind == "finedge":
            pass  # drawn with the outline below
        elif kind == "shine":
            for x in range(x0 + (x1 - x0) // 3, x0 + (x1 - x0) * 3 // 4):
                y = tops[x] + max(1, (bots[x] - tops[x]) // 4)
                if on_body(x, y) and (x & 1 or small):
                    img[y][x] = 11

    # The head: the gill line, the eye and the mouth
    ug = 0.76 if not sp.get("flathead") else 0.8
    xg = x0 + int(ug * (x1 - x0))
    if not small:
        for y in range(tops[xg] + 1, bots[xg]):
            dx = int(round(1.2 * math.sin(math.pi * (y - tops[xg]) / float(max(1, bots[xg] - tops[xg])))))
            if body_px(xg + dx, y):
                img[y][xg + dx] = 13
        # The pectoral fin, just behind the gill
        py = int(round(cy + (bots[xg] - cy) * 0.25))
        for k in range(max(2, w // 12)):
            for j in range(max(1, (w // 12 - k) // 2)):
                if body_px(xg - 2 - k, py + j):
                    img[py + j][xg - 2 - k] = 6
    ue = 0.9 if not sp.get("upmouth") else 0.88
    xe = x0 + int(ue * (x1 - x0))
    if xe in tops:
        ye = int(round(tops[xe] + (bots[xe] - tops[xe]) * (0.34 if not sp.get("flathead") else 0.4)))
        if small:
            img[ye][xe] = 10
        else:
            for dx, dy in ((0, 0), (1, 0), (0, 1), (1, 1)):
                img[ye + dy][xe + dx] = 10
            for dx, dy in ((-1, 0), (-1, 1), (2, 0), (2, 1), (0, -1), (1, -1), (0, 2), (1, 2)):
                if body_px(xe + dx, ye + dy):
                    img[ye + dy][xe + dx] = 9
            img[ye][xe + 1] = 11
    # The mouth: a dark notch at the snout
    my = int(round(cy + (0 if not sp.get("upmouth") else -D * 0.15)))
    for k in range(1 if small else (3 if not sp.get("jaw") else 4)):
        x = x1 - k
        if body_px(x, my):
            img[my][x] = 13
    if sp.get("jaw") and not small:
        if body_px(x1, my + 1):
            img[my + 1][x1] = 4
    # Whiskers
    wk = sp.get("whiskers", 0)
    if wk:
        n = (1 if wk < 3 else 2) if not small else 1
        ln = (2 if wk < 3 else 6) if not small else (1 if wk < 3 else 3)
        for i in range(n):
            for k in range(1, ln + 1):
                x = x1 - 1 + (k if wk == 4 else -k // 2 + k // 3)
                y = my + i * 2 + k * (1 if wk != 4 else -1) // (1 if wk == 3 else 2) + 1
                if 0 <= x < w and 0 <= y < h and img[y][x] == 0:
                    img[y][x] = 13

    # Outlines: the body's in colour 1, the fins' in their dark colour
    out = [row[:] for row in img]
    for y in range(h):
        for x in range(w):
            c = img[y][x]
            if c == 0:
                continue
            edge = any(not (0 <= x + dx < w and 0 <= y + dy < h) or img[y + dy][x + dx] == 0
                       for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
            if not edge:
                continue
            if c in (6, 7, 14):
                fin_edge = any(p[0] == "finedge" for p in sp.get("pattern", []))
                out[y][x] = 11 if fin_edge and not small else 7
            elif c != 13:
                out[y][x] = 1
    return out


def portrait(sp):
    return paint(sp, 64, 32)


def icon(sp):
    return paint(sp, 32, 16)
