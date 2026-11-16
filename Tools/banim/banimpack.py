"""Compression for battle animations (Tools/banim/feditor_dump.py, Documentation/Features/CustomBattleAnimations.md).

- lz77_parse / lz77_stream: the GBA BIOS's LZ77 (tag 0x10) with an optimal parse. The input is a list of tokens: ints
  (bytes) or other hashable values (the script's sheet-pointer bytes, which EA fills in). Matches only copy equal
  tokens, so a pointer byte is only ever copied from the same byte of the same sheet's pointer.
- huff_pack: the sheets of one animation in BanimCodec's format (Kernel/Wizardry/BanimCodec): deflate's symbols, with
  one pair of canonical Huffman trees for all the sheets, stored once (struct BanimHuffTrees).
- huff_unpack: the decoder, as the C one does it, for the self-checks.
"""
import bisect
import collections
import struct

HUFF_TAG = 0xD1

LEN_BASE = [3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195,
            227, 258]
LEN_EXTRA = [0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0]
DIST_BASE = [1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073,
             4097, 6145, 8193, 12289, 16385, 24577]
DIST_EXTRA = [0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13]
MAX_BITS = 15
LIT_FAST_BITS = 9   # BANIM_HUFF_LIT_BITS (include/kernel/banim-codec.h)
DIST_FAST_BITS = 7  # BANIM_HUFF_DIST_BITS


def _len_code(n):
    i = bisect.bisect_right(LEN_BASE, n) - 1
    return 257 + i, LEN_EXTRA[i], n - LEN_BASE[i]


def _dist_code(d):
    i = bisect.bisect_right(DIST_BASE, d) - 1
    return i, DIST_EXTRA[i], d - DIST_BASE[i]


def find_matches(seq, min_len, max_len, max_dist, chain=64):
    """For each position, the matches that are longer than every closer one: [(length, distance)]."""
    n = len(seq)
    heads = {}
    out = [()] * n
    for p in range(n):
        if p >= min_len:
            heads.setdefault(tuple(seq[p - min_len:p]), []).append(p - min_len)
        found = []
        best = min_len - 1
        tries = 0
        for q in reversed(heads.get(tuple(seq[p:p + min_len]), ())):
            d = p - q
            if d > max_dist or tries >= chain:
                break
            tries += 1
            length = 0
            while length < max_len and p + length < n and seq[p + length] == seq[q + length]:
                length += 1
            if length > best:
                best = length
                found.append((length, d))
                if length == max_len:
                    break
        out[p] = tuple(found)
    return out


# ---- LZ77 (BIOS) ----

def lz77_parse(seq):
    """Optimal parse for the BIOS's LZ77: a literal costs 9 bits, a match (3-18 bytes, distance 1-4096) 17."""
    n = len(seq)
    matches = find_matches(seq, 3, 18, 4096)
    cost = [0] * (n + 1)
    step = [1] * (n + 1)
    dist = [0] * (n + 1)
    for p in range(n - 1, -1, -1):
        best, bl, bd = 9 + cost[p + 1], 1, 0
        lo = 3
        for length, d in matches[p]:
            for k in range(lo, length + 1):
                v = 17 + cost[p + k]
                if v < best:
                    best, bl, bd = v, k, d
            lo = length + 1
        cost[p], step[p], dist[p] = best, bl, bd
    ops = []
    p = 0
    while p < n:
        if step[p] == 1:
            ops.append(('lit', seq[p]))
        else:
            ops.append(('match', step[p], dist[p]))
        p += step[p]
    return ops


def lz77_stream(ops, length):
    """The stream as a list of items: ints (bytes) and the literal tokens themselves, flag bytes included."""
    out = list(struct.pack('<I', length << 8 | 0x10))
    for g in range(0, len(ops), 8):
        group = ops[g:g + 8]
        flags = 0
        body = []
        for bit, op in enumerate(group):
            if op[0] == 'lit':
                body.append(op[1])
            else:
                flags |= 0x80 >> bit
                x = (op[1] - 3) << 12 | (op[2] - 1)
                body += [x >> 8, x & 0xFF]
        out.append(flags)
        out += body
    while len(out) % 4:
        out.append(0)
    return out


def lz77_compress_optimal(data):
    return bytes(lz77_stream(lz77_parse(list(data)), len(data)))


# ---- Shared-Huffman deflate symbols (BanimCodec) ----

def _code_lengths(freq, nsyms):
    """Length-limited (MAX_BITS) Huffman code lengths by package-merge. freq: {symbol: count}."""
    syms = sorted(s for s in range(nsyms) if freq.get(s))
    lengths = [0] * nsyms
    if not syms:
        return lengths
    if len(syms) == 1:
        lengths[syms[0]] = 1
        return lengths
    leaves = sorted((freq[s], (s,)) for s in syms)
    packages = list(leaves)
    for _ in range(MAX_BITS - 1):
        merged = [(packages[i][0] + packages[i + 1][0], packages[i][1] + packages[i + 1][1])
                  for i in range(0, len(packages) - 1, 2)]
        packages = sorted(leaves + merged, key=lambda x: x[0])
    for _, members in packages[:2 * len(syms) - 2]:
        for s in members:
            lengths[s] += 1
    return lengths


def _canonical(lengths):
    """Deflate's canonical codes: {symbol: (code, length)}; and the decoder's count/symbol tables."""
    count = [0] * 16
    for n in lengths:
        if n:
            count[n] += 1
    code, nxt = 0, [0] * 16
    for bits in range(1, 16):
        code = (code + count[bits - 1]) << 1 if bits > 1 else 0
        nxt[bits] = code
    codes = {}
    for s, n in enumerate(lengths):
        if n:
            codes[s] = (nxt[n], n)
            nxt[n] += 1
    order = sorted((n, s) for s, n in enumerate(lengths) if n)
    return codes, count, [s for _, s in order]


class _BitWriter:
    def __init__(self):
        self.out = bytearray()
        self.acc = 0
        self.n = 0

    def bits(self, value, n):  # low bit first
        for i in range(n):
            self.acc |= ((value >> i) & 1) << self.n
            self.n += 1
            if self.n == 8:
                self.out.append(self.acc)
                self.acc, self.n = 0, 0

    def code(self, code, n):  # a Huffman code: high bit first
        for i in range(n - 1, -1, -1):
            self.bits((code >> i) & 1, 1)

    def done(self):
        if self.n:
            self.out.append(self.acc)
        while len(self.out) % 4:
            self.out.append(0)
        return bytes(self.out)


def _huff_parse(data, matches, lcost, dcost):
    n = len(data)
    cost = [0] * (n + 1)
    choice = [None] * (n + 1)
    for p in range(n - 1, -1, -1):
        best = lcost[data[p]] + cost[p + 1]
        bc = None
        lo = 3
        for length, d in matches[p]:
            ds, de, _ = _dist_code(d)
            dc = dcost[ds] + de
            for k in range(lo, length + 1):
                s, e, _ = _len_code(k)
                v = lcost[s] + e + dc + cost[p + k]
                if v < best:
                    best, bc = v, (k, d)
            lo = length + 1
        cost[p], choice[p] = best, bc
    ops = []
    p = 0
    while p < n:
        if choice[p] is None:
            ops.append(data[p])
            p += 1
        else:
            ops.append(choice[p])
            p += choice[p][0]
    return ops


def huff_pack(blobs, iterations=4):
    """Pack several blobs with one shared pair of trees. Returns (trees_bytes, [stream_bytes]) where each stream is
    the bits only: the 8-byte header (size and tag, then the trees' pointer) is written by the EA file."""
    matches = [find_matches(list(b), 3, 258, 32768) for b in blobs]
    lcost = [9] * 286
    dcost = [5] * 30
    for _ in range(iterations):
        lf, df = collections.Counter(), collections.Counter()
        parses = []
        for b, m in zip(blobs, matches):
            ops = _huff_parse(b, m, lcost, dcost)
            parses.append(ops)
            for op in ops:
                if isinstance(op, int):
                    lf[op] += 1
                else:
                    lf[_len_code(op[0])[0]] += 1
                    df[_dist_code(op[1])[0]] += 1
            lf[256] += 1
        llen = _code_lengths(lf, 286)
        dlen = _code_lengths(df, 30)
        lcost = [n if n else 20 for n in llen]
        dcost = [n if n else 20 for n in dlen]
    lcodes, lcount, lsym = _canonical(llen)
    dcodes, dcount, dsym = _canonical(dlen)
    streams = []
    for ops in parses:
        w = _BitWriter()
        for op in ops:
            if isinstance(op, int):
                w.code(*lcodes[op])
            else:
                s, e, v = _len_code(op[0])
                w.code(*lcodes[s])
                w.bits(v, e)
                s, e, v = _dist_code(op[1])
                w.code(*dcodes[s])
                w.bits(v, e)
        w.code(*lcodes[256])
        streams.append(w.done())
    trees = struct.pack('<16H', *lcount) + struct.pack('<288H', *(lsym + [0] * (288 - len(lsym))))
    trees += struct.pack('<16H', *dcount) + struct.pack('<32H', *(dsym + [0] * (32 - len(dsym))))
    trees += struct.pack('<%dH' % (1 << LIT_FAST_BITS), *_fast_table(lcodes, LIT_FAST_BITS))
    trees += struct.pack('<%dH' % (1 << DIST_FAST_BITS), *_fast_table(dcodes, DIST_FAST_BITS))
    return trees, streams


def _fast_table(codes, bits):
    """The decoder's lookup table: for every value of the next `bits` stream bits, (length << 9) | symbol of the code
    they start with, or 0 if that code is longer. Codes go high bit first into a stream read low bit first, so a code
    is found at its bit-reversed value, and at every value that adds more bits above it."""
    table = [0] * (1 << bits)
    for sym, (code, n) in codes.items():
        if n > bits:
            continue
        rev = int(format(code, '0%db' % n)[::-1], 2)
        for i in range(rev, 1 << bits, 1 << n):
            table[i] = n << 9 | sym
    return table


def huff_unpack(trees, stream):
    """BanimHuffUnComp (Kernel/Wizardry/BanimCodec/Source/BanimCodec.c) in Python: the same bit buffer and the same
    lookup tables, so the self-check covers them."""
    lcount = struct.unpack_from('<16H', trees, 0)
    lsym = struct.unpack_from('<288H', trees, 32)
    dcount = struct.unpack_from('<16H', trees, 32 + 576)
    dsym = struct.unpack_from('<32H', trees, 32 + 576 + 32)
    off = 32 + 576 + 32 + 64
    lfast = struct.unpack_from('<%dH' % (1 << LIT_FAST_BITS), trees, off)
    dfast = struct.unpack_from('<%dH' % (1 << DIST_FAST_BITS), trees, off + 2 * (1 << LIT_FAST_BITS))
    st = {'src': 0, 'buf': 0, 'count': 0}
    data = stream + bytes(4)

    def refill():
        while st['count'] <= 24:
            st['buf'] |= data[st['src']] << st['count']
            st['src'] += 1
            st['count'] += 8

    def bits(n):
        refill()
        v = st['buf'] & ((1 << n) - 1)
        st['buf'] >>= n
        st['count'] -= n
        return v

    def decode(fast, fbits, count, symbol):
        refill()
        e = fast[st['buf'] & ((1 << fbits) - 1)]
        if e:
            st['buf'] >>= e >> 9
            st['count'] -= e >> 9
            return e & 0x1FF
        s = cur = n = 0
        while True:
            cur = 2 * cur + (st['buf'] & 1)
            st['buf'] >>= 1
            st['count'] -= 1
            n += 1
            s += count[n]
            cur -= count[n]
            if cur < 0:
                return symbol[s + cur]

    out = bytearray()
    while True:
        sym = decode(lfast, LIT_FAST_BITS, lcount, lsym)
        if sym < 256:
            out.append(sym)
            continue
        if sym == 256:
            return bytes(out)
        sym -= 257
        length = LEN_BASE[sym] + bits(LEN_EXTRA[sym])
        sym = decode(dfast, DIST_FAST_BITS, dcount, dsym)
        d = DIST_BASE[sym] + bits(DIST_EXTRA[sym])
        for _ in range(length):
            out.append(out[-d])
