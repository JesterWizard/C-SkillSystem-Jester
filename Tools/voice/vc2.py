#!/usr/bin/env python3
"""VC2: transform codec for voice lines (Documentation/Features/VoiceStream.md).

About half the size of the m4a mixer's DPCM at 13379 Hz. The game decodes it while it plays
(Kernel/Wizardry/VoiceStream).

    vc2.py encode IN.wav OUT.vc2 [--ratio 0.5]   WAV (any rate, 8/16-bit) -> VC2
    vc2.py dmp IN.dmp OUT.vc2 [--ratio 0.5]      a DPCM .dmp the game already has -> VC2
    vc2.py decode IN.vc2 OUT.wav                 reference decoder (what the game plays, to about 50 dB)
    vc2.py gen-unit [--keep]                     unit selection quotes: wav/<Character>/*.wav -> vc2/ and
                                                 UnitSelectionSFX_Installer.event (new WAVs get free song IDs)
    vc2.py gen-wm [--keep]                       world map lines: each chapter's music/*.event becomes a stream song,
                                                 its audio (music/wav/<name>.wav, else its .dmp) -> music/vc2/
    vc2.py report                                Documentation/Features/VoiceStreamSizes.md (sizes, original vs VC2)
    vc2.py gen-tables                            the decoder's tables (VoiceStreamTables.h, vsynth_tables.h)

--keep skips lines whose .vc2 already exists. See Documentation/Features/VoiceActedIntros.md for the whole process.

Format: 'VC2\\0', u16 frames, u16 samples_lo, u16 samples_hi, u16 0, 14 x 7 bytes of level thresholds, then the bit
stream (MSB first). 13379 Hz, MDCT 256/128 with a sine window, 14 bands. Per frame: band 0's scale factor (6 bits),
13 Rice(k=1) zigzag deltas, then every coded band's coefficients as Rice(k from the band's level count) zigzag
values. A band's level count comes from its scale factor and the line's thresholds (no side info); a band with none
is noise-filled. Frame 0 only primes the overlap: output starts with frame 1.

Needs numpy only.
"""
import argparse, math, os, re, struct, sys, wave
import numpy as np

FS, M = 13379, 128
N = 2 * M
WIDTHS = [4, 4, 4, 4, 6, 6, 8, 8, 10, 12, 14, 16, 16, 16]
EDGES = np.cumsum([0] + WIDTHS)
NB = len(WIDTHS)
WIN = np.sin(np.pi * (np.arange(N) + 0.5) / N)
_k = np.arange(M)[:, None]
_n = np.arange(N)[None, :]
MDCT = np.cos(np.pi / M * (_n + 0.5 + M / 2) * (_k + 0.5))
SF_DB, SF_BASE = 2.0, 10
LEVELS = [3, 5, 7, 9, 15, 31, 63]
RICE_K = [0, 0, 1, 1, 2, 3, 4]
BIAS = [0.3, 0.3, 0.2, 0.1, 0, -0.1, -0.2, -0.3, -0.4, -0.5, -0.6, -0.7, -0.8, -0.9]   # bits: lows favoured
DPCM_BYTES_PER_SAMPLE = 33 / 64

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


# ---------------------------------------------------------------- audio in

def read_wav(path):
    w = wave.open(path, 'rb')
    ch, sw, fs, n = w.getnchannels(), w.getsampwidth(), w.getframerate(), w.getnframes()
    raw = w.readframes(n)
    w.close()
    if sw == 1:
        x = (np.frombuffer(raw, dtype=np.uint8).astype(float) - 128) * 256
    elif sw == 2:
        x = np.frombuffer(raw, dtype='<i2').astype(float)
    elif sw == 3:
        b = np.frombuffer(raw, dtype=np.uint8).reshape(-1, 3).astype(np.int32)
        x = ((b[:, 0] << 8) | (b[:, 1] << 16) | (b[:, 2] << 24)).astype(float) / 65536
    else:
        x = np.frombuffer(raw, dtype='<i4').astype(float) / 65536
    return x.reshape(-1, ch).mean(axis=1), fs


def resample(x, fs_in, fs_out=FS, taps=32):
    """Windowed-sinc resampler (Kaiser, cut-off at 0.45 of the lower rate)."""
    if fs_in == fs_out:
        return x
    ratio = fs_out / fs_in
    cut = 0.45 * min(1.0, ratio) * 2           # of the input Nyquist
    nout = int(len(x) * ratio)
    t = np.arange(nout) / ratio                 # input positions
    base = np.floor(t).astype(int)
    out = np.zeros(nout)
    half = int(taps / min(1.0, ratio))
    xp = np.concatenate([np.zeros(half + 1), x, np.zeros(half + 2)])
    for j in range(-half, half + 1):
        idx = base + j
        d = t - idx
        r = np.clip(d / (half + 1), -1, 1)
        w = cut * np.sinc(cut * d) * np.i0(8.0 * np.sqrt(1 - r * r)) / np.i0(8.0)
        out += xp[idx + half + 1] * w
    return out


def prepare(x):
    """What the old SoX chain did (silence 1 0.1 1% -1 0.1 1%, gain -n), kept at 16 bits."""
    peak = np.max(np.abs(x)) if len(x) else 0
    if peak <= 0:
        return x
    thr = 0.01 * peak
    win = int(0.1 * FS)
    loud = np.abs(x) > thr
    start = 0
    for i in np.flatnonzero(loud):       # first point that starts 0.1 s of sound
        if loud[i:i + win].mean() > 0.05:
            start = i
            break
    end = len(x)
    rev = np.flatnonzero(loud)
    if len(rev):
        end = rev[-1] + 1
    x = x[start:end]
    return x * (32000 / np.max(np.abs(x)))


def read_dmp(path):
    """A DPCM wave as the mixer plays it (16-byte header, then 64-sample blocks of 33 bytes)."""
    d = open(path, 'rb').read()
    kind, freq, loop, n = struct.unpack_from('<IIII', d, 0)
    t = [0, 1, 4, 9, 16, 25, 36, 49, -64, -49, -36, -25, -16, -9, -4, -1]
    out = []
    if kind & 0xFF:
        for p in range(16, len(d), 33):
            blk = d[p:p + 33]
            s = blk[0]
            out.append(s)
            s = (s + t[blk[1] & 15]) & 255
            out.append(s)
            for b in blk[2:]:
                s = (s + t[b >> 4]) & 255
                out.append(s)
                s = (s + t[b & 15]) & 255
                out.append(s)
    else:
        out = list(d[16:16 + n])
    pcm = np.array([v - 256 if v > 127 else v for v in out[:n]], dtype=float) * 256
    return pcm, freq / 1024


# ---------------------------------------------------------------- encoder

def sf_q(p):
    with np.errstate(divide='ignore'):
        v = np.round(20 * np.log10(np.maximum(p, 1e-9)) / SF_DB - SF_BASE)
    return np.where(p <= 1e-9, 0, np.clip(v, 0, 63)).astype(int)


def sf_v(i):
    i = np.asarray(i)
    return np.where(i == 0, 0.0, 10 ** ((i + SF_BASE) * SF_DB / 20))


def analyse(x):
    nfr = (len(x) + M - 1) // M + 1
    pad = np.concatenate([np.zeros(M), x, np.zeros(nfr * M + M - len(x))])
    frames = np.stack([pad[f * M: f * M + N] * WIN for f in range(nfr)])
    C = frames @ MDCT.T                                  # nfr x M
    raw = np.stack([sf_q(np.max(np.abs(C[:, EDGES[b]:EDGES[b + 1]]), axis=1)) for b in range(NB)], axis=1)
    sfi = np.zeros_like(raw)
    sfi[:, 0] = raw[:, 0]
    for b in range(1, NB):
        sfi[:, b] = np.clip(sfi[:, b - 1] + np.clip(raw[:, b] - sfi[:, b - 1], -15, 15), 0, 63)
    return C, sfi


def levels_for(sfi, b, offset):
    p = sfi * SF_DB / 6.02 + BIAS[b] - offset
    if sfi == 0 or p < 0.9:
        return 0
    want = max(2 ** p, 1)
    return min(LEVELS, key=lambda L: abs(math.log2(L) - math.log2(want)))


def thresholds(offset):
    thr = np.full((NB, len(LEVELS)), 255, dtype=int)
    for b in range(NB):
        for j, L in enumerate(LEVELS):
            for s in range(1, 64):
                Lb = levels_for(s, b, offset)
                if Lb and Lb >= L:
                    thr[b, j] = s
                    break
    return thr


def level_index(thr, sfi):
    """Per frame and band: index into LEVELS, or -1 for a noise-filled band (as the decoder works it out)."""
    lj = np.full(sfi.shape, -1)
    for j in range(len(LEVELS)):
        lj = np.where(sfi >= thr[:, j][None, :], j, lj)
    return lj


def zz(v):
    return np.where(v >= 0, 2 * v, -2 * v - 1)


def quantise(C, sfi, lj):
    """Per coefficient: zigzag value and Rice k (k = -1 for noise-filled bands)."""
    U = np.zeros(C.shape, dtype=int)
    K = np.full(C.shape, -1, dtype=int)
    lv = np.array(LEVELS)
    rk = np.array(RICE_K)
    for b in range(NB):
        s, e = EDGES[b], EDGES[b + 1]
        j = lj[:, b]
        coded = j >= 0
        half = np.where(coded, (lv[np.maximum(j, 0)] - 1) // 2, 1)
        step = np.where(coded, sf_v(sfi[:, b]) / half, 1.0)
        q = np.clip(np.round(C[:, s:e] / step[:, None]), -half[:, None], half[:, None]).astype(int)
        U[:, s:e] = zz(q)
        K[:, s:e] = np.where(coded, rk[np.maximum(j, 0)], -1)[:, None]
    return U, K


def count_bits(sfi, U, K):
    d = zz(np.diff(sfi, axis=1))
    bits = 6 * len(sfi) + np.sum((d >> 1) + 2)
    m = K >= 0
    bits += np.sum(((U >> np.maximum(K, 0)) + 1 + K)[m])
    return int(bits)


def encode(x, ratio=0.5):
    C, sfi = analyse(x)
    target = ratio * DPCM_BYTES_PER_SAMPLE * len(x) * 8
    lo, hi = -6.0, 12.0                                     # bigger offset -> fewer bits
    for _ in range(24):
        mid = (lo + hi) / 2
        thr = thresholds(mid)
        U, K = quantise(C, sfi, level_index(thr, sfi))
        if count_bits(sfi, U, K) > target:
            lo = mid
        else:
            hi = mid
    thr = thresholds(hi)
    U, K = quantise(C, sfi, level_index(thr, sfi))
    out = bytearray()
    acc, nacc = 0, 0

    def put(v, n):
        nonlocal acc, nacc
        acc = (acc << n) | v
        nacc += n
        while nacc >= 8:
            nacc -= 8
            out.append((acc >> nacc) & 0xFF)
        acc &= (1 << nacc) - 1

    def rice(u, k):
        q = u >> k
        while q >= 16:
            put(0xFFFF, 16)
            q -= 16
        put(((1 << q) - 1) << 1, q + 1)
        if k:
            put(u & ((1 << k) - 1), k)

    for f in range(len(sfi)):
        put(int(sfi[f, 0]), 6)
        for b in range(1, NB):
            rice(int(zz(sfi[f, b] - sfi[f, b - 1])), 1)
        for i in range(M):
            if K[f, i] >= 0:
                rice(int(U[f, i]), int(K[f, i]))
    if nacc:
        put(0, 8 - nacc)
    n = len(x)
    hdr = b'VC2\0' + struct.pack('<HHHH', len(sfi), n & 0xFFFF, n >> 16, 0) + bytes(int(v) for v in thr.flatten())
    return hdr + bytes(out)


# ---------------------------------------------------------------- reference decoder (integer, as the game)

AMP = [0] + [int(round(sf_v(i) * 2 / M * 16)) for i in range(1, 64)]
STEPQ4 = [[0 if i == 0 else int(round(sf_v(i) * 2 / M / ((L - 1) // 2) * 16)) for L in LEVELS] for i in range(64)]


def decode(data):
    assert data[:4] == b'VC2\0', 'not a VC2 file'
    nfr, lo, hi, _ = struct.unpack_from('<HHHH', data, 4)
    n = lo | (hi << 16)
    thr = [list(data[12 + 7 * b: 19 + 7 * b]) for b in range(NB)]
    pos = 8 * (12 + 7 * NB)
    bits = np.unpackbits(np.frombuffer(data, dtype=np.uint8))

    def get(k):
        nonlocal pos
        v = 0
        for _ in range(k):
            v = (v << 1) | int(bits[pos])
            pos += 1
        return v

    def rice(k):
        nonlocal pos
        q = 0
        while bits[pos]:
            q += 1
            pos += 1
        pos += 1
        return (q << k) | get(k)

    def unzz(u):
        return (u >> 1) if not (u & 1) else -((u + 1) >> 1)

    rng = 1

    def rnd():
        nonlocal rng
        rng = (rng * 1664525 + 1013904223) & 0xFFFFFFFF
        v = rng >> 16
        return v - 65536 if v >= 32768 else v

    y = np.zeros((nfr + 2) * M + N)
    for f in range(nfr):
        sfi = [get(6)]
        for b in range(1, NB):
            sfi.append(sfi[-1] + unzz(rice(1)))
        X = [0] * M
        for b in range(NB):
            s, e = EDGES[b], EDGES[b + 1]
            sf = max(0, min(63, sfi[b]))
            lj = -1
            for j, t in enumerate(thr[b]):
                if sf >= t:
                    lj = j
            if lj < 0:
                a = AMP[sf] >> 4
                for i in range(s, e):
                    X[i] = ((((rnd() >> 1) * a) >> 14) * 45) >> 6
                continue
            st = STEPQ4[sf][lj]
            for i in range(s, e):
                q = unzz(rice(RICE_K[lj]))
                X[i] = (q * st) >> 4 if q else (((rnd() >> 1) * (st >> 4)) >> 16)
        y[f * M: f * M + N] += (MDCT.T @ np.array(X, dtype=float)) * WIN
    return np.clip(np.round(y[M: M + n]), -32768, 32767).astype(int)


def write_wav(path, pcm, fs=FS):
    w = wave.open(path, 'wb')
    w.setnchannels(1)
    w.setsampwidth(2)
    w.setframerate(fs)
    w.writeframes(np.asarray(pcm, dtype='<i2').tobytes())
    w.close()


# ---------------------------------------------------------------- song events

VSC2_MAGIC = '$32435356'
FREE_UNIT_IDS = list(range(0x26D, 0x2BC)) + list(range(0x385, 0x3AC)) + list(range(0x3D7, 0x3E5))


def stream_song(label, vc2_rel, prio=0, rev=0):
    """A stream song: a one-track song whose instrument plays the decoder's ring (VoiceStream_Tone), followed by
    the magic and the VC2 data's address (VoiceStream.c reads them)."""
    return ('ALIGN 4\n%s_vc2:\n    #incbin "%s"\n' % (label, vc2_rel) +
            'ALIGN 4\n%s:\n    BYTE 1 0 %s %s\n    POIN VoiceStream_Tone\n    POIN VoiceStream_Track\n'
            '    WORD %s // "VSC2"\n    POIN %s_vc2\n' % (label, prio, rev, VSC2_MAGIC, label))


def _load(src):
    """A line's samples at 13379 Hz: a WAV (resampled, trimmed, normalised like the old SoX step) or a DPCM .dmp."""
    if src.lower().endswith('.wav'):
        x, fs = read_wav(src)
        return prepare(resample(x, fs))
    x, fs = read_dmp(src)
    return resample(x, int(round(fs)))


def _encode_job(args):
    src, dst, ratio = args
    x = _load(src)
    data = encode(x, ratio)
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    open(dst, 'wb').write(data)
    return dst, len(x), len(data)


def _run_jobs(jobs, nproc):
    from multiprocessing import Pool
    tot_n = tot_b = 0
    if not jobs:
        print('nothing to encode')
        return
    with Pool(nproc) as p:
        for dst, n, b in p.imap_unordered(_encode_job, jobs):
            tot_n += n
            tot_b += b
            print('%-60s %6d samples %6d bytes (%.0f%% of DPCM)' % (os.path.relpath(dst, ROOT), n, b,
                  100 * b / (n * DPCM_BYTES_PER_SAMPLE)))
    print('total: %d bytes, %.1f%% of DPCM' % (tot_b, 100 * tot_b / (tot_n * DPCM_BYTES_PER_SAMPLE)))


UNIT_DIR = os.path.join(ROOT, 'Kernel/Wizardry/UnitSelectionSFX')
UNIT_EVENT = os.path.join(UNIT_DIR, 'UnitSelectionSFX_Installer.event')


def _unit_wavs():
    """{folder: [wav file names]} under UnitSelectionSFX/wav, sorted. Names ending in _Compressed are the old SoX
    step's leftovers, not quotes: skipped."""
    out = {}
    base = os.path.join(UNIT_DIR, 'wav')
    for folder in sorted(os.listdir(base)):
        d = os.path.join(base, folder)
        if os.path.isdir(d):
            fs = sorted(f for f in os.listdir(d)
                        if f.lower().endswith('.wav') and not f.lower().endswith('_compressed.wav'))
            if fs:
                out[folder] = fs
    return out


def unit_quotes():
    """The quotes in UnitSelectionSFX_Installer.event, in order: (label, song id, group, folder, wav name).
    Reads the stream format this tool writes, and the old DPCM one (incbin "dmp/Folder/Name_Compressed.dmp")."""
    if not os.path.exists(UNIT_EVENT):
        return []
    ev = open(UNIT_EVENT).read()
    incs = dict(re.findall(r'^(\w+?)_(?:wav|vc2):\s*\n\s*#incbin "([^"]+)"', ev, re.M))
    wavs = {}
    for folder, fs in _unit_wavs().items():
        for f in fs:
            wavs[os.path.splitext(f)[0].lower()] = (folder, f)
    out = []
    for sid, label, grp in re.findall(r'^SongTable\(\s*([^,]+?)\s*,\s*(\w+?)(?:_Sound)?\s*,\s*([^)]+?)\s*\)', ev, re.M):
        stem = os.path.splitext(os.path.basename(incs[label]))[0]
        if stem.endswith('_Compressed'):
            stem = stem[:-len('_Compressed')]
        if stem.lower() not in wavs:
            raise SystemExit('%s: no WAV named %s.wav under %s/wav' % (label, stem, UNIT_DIR))
        folder, wav = wavs[stem.lower()]
        out.append((label, int(sid, 0), grp, folder, wav))
    return out


def gen_unit(ratio, nproc, skip_existing):
    """UnitSelectionSFX_Installer.event from the WAVs in UnitSelectionSFX/wav/<Character>/: quotes already in it
    keep their song IDs; new WAVs get free ones. Prints BeginActionHook.c's table."""
    quotes = unit_quotes()
    known = {(q[3], q[4]) for q in quotes}
    used = {q[1] for q in quotes}
    free = [i for i in FREE_UNIT_IDS if i not in used]
    count = {}
    for q in quotes:
        m = re.match(r'(.*)_(\d+)$', q[0])
        if m:
            count[m.group(1)] = max(count.get(m.group(1), 0), int(m.group(2)))
    for folder, fs in _unit_wavs().items():
        for f in fs:
            if (folder, f) in known:
                continue
            if not free:
                raise SystemExit('no free song IDs left for %s/%s' % (folder, f))
            count[folder] = count.get(folder, 0) + 1
            quotes.append(('%s_%d' % (folder, count[folder]), free.pop(0), '7', folder, f))
            print('new quote %s/%s: song 0x%X' % (folder, f, quotes[-1][1]))

    out = ['// Generated by Tools/voice/vc2.py gen-unit from wav/<Character>/*.wav. Do not edit by hand.',
           '// Unit selection quotes as VC2 stream songs (Documentation/Features/UnitSelectionQuotes.md).',
           '#ifndef SongTableOffset',
           '#define SongTableOffset  0x224470',
           '#define SongTable(index,SongPointer,Group) "PUSH; ORG SongTableOffset+(8*index); POIN SongPointer; '
           'SHORT Group Group; POP"',
           '#endif', '']
    jobs = []
    for label, sid, grp, folder, wav in quotes:
        stem = os.path.splitext(wav)[0]
        rel = 'vc2/%s/%s.vc2' % (folder, stem)
        dst = os.path.join(UNIT_DIR, rel)
        if not (skip_existing and os.path.exists(dst)):
            jobs.append((os.path.join(UNIT_DIR, 'wav', folder, wav), dst, ratio))
        out.append('// %s' % wav)
        out.append(stream_song(label + '_Sound', rel).rstrip('\n').replace('%s_Sound_vc2' % label, '%s_vc2' % label))
        out.append('SongTable(0x%X, %s_Sound, %s)\n' % (sid, label, grp))
    _run_jobs(jobs, nproc)
    open(UNIT_EVENT, 'w').write('\n'.join(out))

    print('\nBeginActionHook.c\'s character_voice_ids (the first three quotes of each character play):')
    by = {}
    for label, sid, grp, folder, wav in quotes:
        by.setdefault(label.rsplit('_', 1)[0], []).append(sid)
    for ch, ids in by.items():
        print('    { CHARACTER_%s,    {%s} },' % (ch.upper(), ', '.join('0x%X' % i for i in ids[:3])))


WM_BASE = os.path.join(ROOT, 'CustomCampaign/Chapters')


def wm_lines():
    """Every voice line a chapter's music/installer.event includes: (event path, label, song arg, group, source)."""
    out = []
    for ch in sorted(os.listdir(WM_BASE)):
        md = os.path.join(WM_BASE, ch, 'music')
        inst = os.path.join(md, 'installer.event')
        if not os.path.exists(inst):
            continue
        for evf in re.findall(r'^#include "([^"]+)"', open(inst).read(), re.M):
            path = os.path.join(md, evf)
            ev = open(path).read()
            m = re.search(r'^SongTable\(\s*([^,]+?)\s*,\s*(\w+)\s*,\s*([^)]+?)\s*\)', ev, re.M)
            if not m:
                continue
            sid, label, grp = m.groups()
            src = re.search(r'^// Source: (\S+)', ev, re.M)
            if src:
                src = src.group(1)
            else:
                inc = re.search(r'#incbin "([^"]+\.dmp)"', ev)
                src = inc.group(1) if inc else 'wav/%s.wav' % label
            # a WAV master beats a DPCM one
            if src.endswith('.dmp') and os.path.exists(os.path.join(md, 'wav', label + '.wav')):
                src = 'wav/%s.wav' % label
            out.append((path, label, sid, grp, src))
    return out


def gen_wm(ratio, nproc, skip_existing):
    """Each world map voice line's .event becomes a stream song; its audio is encoded to music/vc2/<label>.vc2 from
    music/wav/<label>.wav, or from its DPCM .dmp when that is all there is."""
    jobs = []
    for path, label, sid, grp, src in wm_lines():
        md = os.path.dirname(path)
        if not os.path.exists(os.path.join(md, src)):
            raise SystemExit('%s: no %s' % (path, src))
        rel = 'vc2/%s.vc2' % label
        dst = os.path.join(md, rel)
        if not (skip_existing and os.path.exists(dst)):
            jobs.append((os.path.join(md, src), dst, ratio))
        open(path, 'w').write(
            '// Voice line, a VC2 stream song (Documentation/Features/VoiceActedIntros.md). Written by\n'
            '// Tools/voice/vc2.py gen-wm from the source below; run it again after changing the source.\n'
            '// Source: %s\n\n%sSongTable(%s, %s, %s)\n' % (src, stream_song(label, rel), sid, label, grp))
    _run_jobs(jobs, nproc)


def _fmt_kb(n):
    return '%.1f KB' % (n / 1024)


def report():
    """Documentation/Features/VoiceStreamSizes.md: every line's sizes, original against VC2."""
    def vc2_info(path):
        d = open(path, 'rb').read()
        nfr, lo, hi, _ = struct.unpack_from('<HHHH', d, 4)
        return lo | (hi << 16), len(d)

    def dpcm_size(n):
        return 16 + -(-n // 64) * 33

    rows_u, rows_w = [], []
    for label, sid, grp, folder, wav in unit_quotes():
        stem = os.path.splitext(wav)[0]
        n, v = vc2_info(os.path.join(UNIT_DIR, 'vc2', folder, stem + '.vc2'))
        rows_u.append(('%s/%s' % (folder, stem), os.path.getsize(os.path.join(UNIT_DIR, 'wav', folder, wav)), n, v))
    for path, label, sid, grp, src in wm_lines():
        md = os.path.dirname(path)
        n, v = vc2_info(os.path.join(md, 'vc2', label + '.vc2'))
        w = os.path.getsize(os.path.join(md, src)) if src.endswith('.wav') else None
        rows_w.append(('%s/%s' % (os.path.basename(os.path.dirname(md)), label), w, n, v))

    def pct(a, b):
        return '%.1f%%' % (100 * (1 - b / a))

    def table(rows, title, what):
        t = ['## %s' % title, '', what, '',
             '| Line | Length | Original WAV | 8-bit PCM | DPCM | VC2 | Saved vs WAV | vs PCM | vs DPCM |',
             '|---|---|---|---|---|---|---|---|---|']
        tw = tp = td = tv = 0
        for name, w, n, v in rows:
            p, dp = n, dpcm_size(n)
            t.append('| %s | %.2f s | %s | %s | %s | %s | %s | %s | %s |' % (
                name, n / FS, _fmt_kb(w) if w else '-', _fmt_kb(p), _fmt_kb(dp), _fmt_kb(v),
                pct(w, v) if w else '-', pct(p, v), pct(dp, v)))
            tw += w or 0
            tp += p
            td += dp
            tv += v
        t.append('| **All %d** | %.1f s | %s | %s | %s | %s | %s | %s | %s |' % (
            len(rows), tp / FS, _fmt_kb(tw) if tw else '-', _fmt_kb(tp), _fmt_kb(td), _fmt_kb(tv),
            pct(tw, tv) if tw else '-', pct(tp, tv), pct(td, tv)))
        return t, (tw, tp, td, tv)

    tu, su = table(rows_u, 'Unit selection quotes', 'Original WAV: the source in `Kernel/Wizardry/UnitSelectionSFX/wav` '
                   '(44.1 kHz, 16-bit, untrimmed).')
    tm, sm = table(rows_w, 'World map lines', 'These lines only exist as DPCM `.dmp` files (no WAV), so the VC2 '
                   'versions are made from those.')
    head = ['# Voice line sizes', '',
            'Generated by `python3 Tools/voice/vc2.py report`. Sizes are what goes in the ROM, per line:', '',
            '- **8-bit PCM**: the line as an uncompressed GBA sample at 13379 Hz (1 byte a sample).',
            '- **DPCM**: the m4a mixer\'s compressed format the lines used before (33 bytes per 64 samples).',
            '- **VC2**: the format they use now ([VoiceStream.md](VoiceStream.md)).', '',
            '"Saved" is how much smaller VC2 is.', '',
            '## Totals', '',
            '| Set | Lines | 8-bit PCM | DPCM | VC2 | Saved vs PCM | vs DPCM |', '|---|---|---|---|---|---|---|']
    for name, rows, (tw, tp, td, tv) in (('Unit selection quotes', rows_u, su), ('World map lines', rows_w, sm)):
        head.append('| %s | %d | %s | %s | %s | %s | %s |' % (name, len(rows), _fmt_kb(tp), _fmt_kb(td), _fmt_kb(tv),
                                                            pct(tp, tv), pct(td, tv)))
    tp, td, tv = su[1] + sm[1], su[2] + sm[2], su[3] + sm[3]
    head.append('| **All** | %d | %s | %s | %s | %s | %s |' % (len(rows_u) + len(rows_w), _fmt_kb(tp), _fmt_kb(td),
                                                              _fmt_kb(tv), pct(tp, tv), pct(td, tv)))
    if su[0]:
        head += ['', 'The unit selection quotes\' source WAVs total %s: VC2 is %s smaller.' % (
            _fmt_kb(su[0]), pct(su[0], su[3]))]
    open(os.path.join(ROOT, 'Documentation/Features/VoiceStreamSizes.md'), 'w').write(
        '\n'.join(head + [''] + tu + [''] + tm) + '\n')
    print('wrote Documentation/Features/VoiceStreamSizes.md')


def _c_array(ctype, name, vals, per=16):
    rows = [', '.join(str(v) for v in vals[i:i + per]) for i in range(0, len(vals), per)]
    return 'static const %s %s[%d] = {\n\t%s,\n};\n' % (ctype, name, len(vals), ',\n\t'.join(rows))


def gen_tables():
    """The decoder's fixed-point tables: the parser's (VoiceStreamTables.h) and the ARM synthesis's
    (Tools/voice/vsynth_tables.h, built into VoiceSynthArm.s by Tools/voice/gen_synth_asm.sh)."""
    q = 1 << 14

    def pack(c, s):
        v = ((s & 0xFFFF) << 16) | (c & 0xFFFF)
        return v - (1 << 32) if v >= 1 << 31 else v

    def rot(a):
        return pack(round(q * math.cos(a)), round(-q * math.sin(a)))

    head = '/* Generated by Tools/voice/vc2.py gen-tables. Do not edit. */\n#pragma once\n\n'
    lead1 = [next((i for i in range(8) if not (v >> (7 - i)) & 1), 8) for v in range(256)]
    p = head + '#define VD_NB %d\n#define VD_NLEVELS %d\n\n' % (NB, len(LEVELS))
    p += _c_array('u8', 'vd_edges', list(EDGES))
    p += _c_array('u8', 'vd_ricek', RICE_K)
    p += _c_array('s32', 'vd_amp', AMP, 8)
    p += 'static const s32 vd_step[64][%d] = {\n' % len(LEVELS)
    p += ''.join('\t{%s},\n' % ', '.join(str(v) for v in row) for row in STEPQ4) + '};\n'
    p += _c_array('u8', 'vd_lead1', lead1)
    open(os.path.join(ROOT, 'Kernel/Wizardry/VoiceStream/Source/VoiceStreamTables.h'), 'w').write(p)

    s = head + '#define VD_Q 14\n\n'
    s += '/* (cos | sin << 16), Q14: DCT-IV pre and post twiddles, FFT twiddles */\n'
    s += _c_array('s32', 'vd_pre', [rot(math.pi * i / 128) for i in range(64)], 8)
    s += _c_array('s32', 'vd_post', [rot(math.pi * (i + .25) / 128) for i in range(64)], 8)
    s += _c_array('s32', 'vd_tw', [rot(2 * math.pi * k / 64) for k in range(64)], 8)
    s += '/* sine window, first half */\n'
    s += _c_array('s16', 'vd_win128', [round(q * math.sin(math.pi * (i + .5) / 256)) for i in range(128)])
    s += '/* base-4 digit reversal of 0..63 */\n'
    s += _c_array('u8', 'vd_digrev4', [((i & 3) << 4) | (((i >> 2) & 3) << 2) | (i >> 4) for i in range(64)])
    open(os.path.join(ROOT, 'Tools/voice/vsynth_tables.h'), 'w').write(s)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest='cmd', required=True)
    for c in ('encode', 'dmp'):
        p = sub.add_parser(c)
        p.add_argument('src')
        p.add_argument('dst')
        p.add_argument('--ratio', type=float, default=0.5, help='size as a fraction of DPCM (default 0.5)')
    p = sub.add_parser('decode')
    p.add_argument('src')
    p.add_argument('dst')
    sub.add_parser('gen-tables')
    sub.add_parser('report')
    for c in ('gen-unit', 'gen-wm'):
        p = sub.add_parser(c)
        p.add_argument('--ratio', type=float, default=0.5)
        p.add_argument('--jobs', type=int, default=os.cpu_count() or 1)
        p.add_argument('--keep', action='store_true', help='skip lines whose .vc2 already exists')
    a = ap.parse_args()
    if a.cmd in ('encode', 'dmp'):
        _, n, b = _encode_job((a.src, a.dst, a.ratio))
        print('%d samples -> %d bytes (%.0f%% of DPCM)' % (n, b, 100 * b / (n * DPCM_BYTES_PER_SAMPLE)))
    elif a.cmd == 'decode':
        write_wav(a.dst, decode(open(a.src, 'rb').read()))
    elif a.cmd == 'report':
        report()
    elif a.cmd == 'gen-tables':
        gen_tables()
    elif a.cmd == 'gen-unit':
        gen_unit(a.ratio, a.jobs, a.keep)
    else:
        gen_wm(a.ratio, a.jobs, a.keep)


if __name__ == '__main__':
    main()
