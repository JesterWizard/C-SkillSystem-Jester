# Tiled-model WFC with bitsets: H/V adjacency learned from vanilla maps of one tileset; 2x2 windows scored afterwards.
import random
from fe8map import load_map

DIRS = ((1, 0), (-1, 0), (0, 1), (0, -1))


def train(mapids):
    freq = {}
    maps = [load_map(m) for m in mapids]
    for W, H, T in maps:
        for v in T:
            freq[v] = freq.get(v, 0) + 1
    order = sorted(freq)
    idx = {v: i for i, v in enumerate(order)}
    comp = [[0] * len(order) for _ in range(4)]
    pats = set()
    for W, H, T in maps:
        for y in range(H):
            for x in range(W):
                a = idx[T[y * W + x]]
                for d, (dx, dy) in enumerate(DIRS):
                    nx, ny = x + dx, y + dy
                    if 0 <= nx < W and 0 <= ny < H:
                        comp[d][a] |= 1 << idx[T[ny * W + nx]]
                if x < W - 1 and y < H - 1:
                    pats.add((T[y * W + x], T[y * W + x + 1], T[(y + 1) * W + x], T[(y + 1) * W + x + 1]))
    return order, idx, comp, pats, freq


def bits(m):
    while m:
        b = m & -m
        yield b.bit_length() - 1
        m ^= b


def seams(w, h, t, pats):
    return [(x, y) for y in range(h - 1) for x in range(w - 1)
            if (t[y * w + x], t[y * w + x + 1], t[(y + 1) * w + x], t[(y + 1) * w + x + 1]) not in pats]


class WFC:
    def __init__(s, w, h, model, fixed=None, allowed=None, seed=0, weights=None):
        s.order, s.idx, s.comp, s.pats, s.freq = model
        s.w, s.h = w, h
        s.rnd = random.Random(seed)
        full = (1 << len(s.order)) - 1
        s.dom = [full] * (w * h)
        if allowed:
            for i, a in enumerate(allowed):
                if a is not None:
                    s.dom[i] &= sum(1 << s.idx[v] for v in a if v in s.idx)
        s.fixed = fixed or {}
        for i, v in s.fixed.items():
            s.dom[i] = 1 << s.idx[v]
        wsrc = weights or s.freq
        s.wt = [wsrc.get(v, 1) for v in s.order]

    def prop(s, start):
        st = list(start)
        while st:
            i = st.pop()
            x, y = i % s.w, i // s.w
            for d, (dx, dy) in enumerate(DIRS):
                nx, ny = x + dx, y + dy
                if not (0 <= nx < s.w and 0 <= ny < s.h):
                    continue
                j = ny * s.w + nx
                if j in s.fixed:
                    continue
                sup = 0
                for a in bits(s.dom[i]):
                    sup |= s.comp[d][a]
                nd = s.dom[j] & sup
                if nd != s.dom[j]:
                    if not nd:
                        raise ValueError((nx, ny))
                    s.dom[j] = nd
                    st.append(j)

    def run(s, max_backtracks=3000):
        s.prop(range(s.w * s.h))
        stack = []
        backtracks = 0
        while True:
            best = None
            for i, dm in enumerate(s.dom):
                if dm & (dm - 1):
                    c = bin(dm).count("1")
                    if best is None or c < best[0]:
                        best = (c, [i])
                    elif c == best[0]:
                        best[1].append(i)
            if not best:
                return [s.order[d.bit_length() - 1] for d in s.dom]
            i = s.rnd.choice(best[1])
            vs = list(bits(s.dom[i]))
            v = s.rnd.choices(vs, [s.wt[k] for k in vs])[0]
            while True:
                snap = list(s.dom)
                s.dom[i] = 1 << v
                try:
                    s.prop([i])
                    stack.append((snap, i, v))
                    break
                except ValueError:
                    backtracks += 1
                    if backtracks > max_backtracks:
                        raise
                    s.dom = snap
                    s.dom[i] &= ~(1 << v)
                    while not s.dom[i]:
                        if not stack:
                            raise ValueError("exhausted")
                        snap, i, v = stack.pop()
                        s.dom = snap
                        s.dom[i] &= ~(1 << v)
                    try:
                        s.prop([i])
                    except ValueError:
                        s.dom[i] = 0
                        continue
                    vs = list(bits(s.dom[i]))
                    if len(vs) == 1:
                        stack.append((list(s.dom), i, vs[0]))
                        break
                    v = s.rnd.choices(vs, [s.wt[k] for k in vs])[0]
