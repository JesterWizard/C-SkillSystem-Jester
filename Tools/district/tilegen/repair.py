# Min-conflicts repair and patch re-solving: fix H/V neighbour pairs (and 2x2 windows) that never appear in vanilla.
import random
from wfc import DIRS, bits, WFC


def conflicts_at(t, w, h, i, v, model):
    order, idx, comp, pats, freq = model
    x, y = i % w, i // w
    a = idx.get(v)
    n = 0
    for d, (dx, dy) in enumerate(DIRS):
        nx, ny = x + dx, y + dy
        if 0 <= nx < w and 0 <= ny < h:
            b = idx.get(t[ny * w + nx])
            if a is None or b is None or not (comp[d][a] >> b) & 1:
                n += 1
    return n


def window_bad(t, w, h, i, model):
    pats = model[3]
    x, y = i % w, i // w
    n = 0
    for wy in (y - 1, y):
        for wx in (x - 1, x):
            if 0 <= wx < w - 1 and 0 <= wy < h - 1:
                p = (t[wy * w + wx], t[wy * w + wx + 1], t[(wy + 1) * w + wx], t[(wy + 1) * w + wx + 1])
                if p not in pats:
                    n += 1
    return n


def total_conflicts(t, w, h, model):
    return sum(conflicts_at(t, w, h, i, t[i], model) for i in range(w * h)) // 2


def repair(t, w, h, model, fixed, allowed=None, iters=20000, seed=0, use_windows=True):
    order, idx, comp, pats, freq = model
    rnd = random.Random(seed)
    t = list(t)
    free = [i for i in range(w * h) if i not in fixed]
    cand_all = list(range(len(order)))
    for it in range(iters):
        bad = [i for i in free if conflicts_at(t, w, h, i, t[i], model)]
        if not bad:
            if not use_windows:
                break
            bad = [i for i in free if window_bad(t, w, h, i, model)]
            if not bad or it > iters // 2:
                break
        i = rnd.choice(bad)
        x, y = i % w, i // w
        mask = (1 << len(order)) - 1
        for d, (dx, dy) in enumerate(DIRS):
            nx, ny = x + dx, y + dy
            if 0 <= nx < w and 0 <= ny < h:
                b = idx.get(t[ny * w + nx])
                opp = d ^ 1
                if b is not None:
                    mask &= comp[opp][b]
        if allowed and allowed[i] is not None:
            am = sum(1 << idx[v] for v in allowed[i] if v in idx)
            if mask & am:
                mask &= am
        if mask:
            cands = list(bits(mask))
        else:
            cands = cand_all
        scored = []
        old = t[i]
        for k in cands:
            v = order[k]
            t[i] = v
            c = conflicts_at(t, w, h, i, v, model) * 10 + (window_bad(t, w, h, i, model) if use_windows else 0)
            scored.append((c, k))
        t[i] = old
        best = min(c for c, _ in scored)
        pool = [k for c, k in scored if c == best]
        k = rnd.choices(pool, [freq.get(order[q], 1) for q in pool])[0]
        if rnd.random() < 0.05:
            k = rnd.choice(cands)
        t[i] = order[k]
    return t


def patch_repair(t, w, h, model, fixed, allowed=None, seed=0, rmax=5, rounds=400):
    """Re-solve small square patches around adjacency conflicts with backtracking WFC."""
    rnd = random.Random(seed)
    t = list(t)
    for rnd_i in range(rounds):
        bad = [i for i in range(w * h) if i not in fixed and conflicts_at(t, w, h, i, t[i], model)]
        if not bad:
            break
        i = rnd.choice(bad)
        x, y = i % w, i // w
        done = False
        for r in range(1, rmax + 1):
            region = {yy * w + xx for yy in range(y - r, y + r + 1) for xx in range(x - r, x + r + 1)
                      if 0 <= xx < w and 0 <= yy < h and (yy * w + xx) not in fixed}
            fx = {j: t[j] for j in range(w * h) if j not in region}
            fx = {j: v for j, v in fx.items() if v in model[1]}
            try:
                res = WFC(w, h, model, fixed=fx, allowed=allowed, seed=rnd.randrange(1 << 30)).run(max_backtracks=400)
            except ValueError:
                continue
            for j in region:
                t[j] = res[j]
            done = True
            break
    return t
