#!/usr/bin/env python3
"""Turn an audio file into a sequenced FE8 song (GBA m4a / "Sappy" note data).

The song is transcribed, not sampled: the output holds notes only and plays them on
instrument samples that are already in the vanilla FE8U ROM. See README.md.

usage: audio2m4a.py AUDIO OUTDIR --name LordOfTheDance [--work DIR] [--no-loop]

Writes OUTDIR/<name>.event (for Event Assembler) and OUTDIR/<name>.mid (the same
arrangement, to look at or edit), and prints the size and the length in frames.
With --bin-base ADDR it also writes song.bin linked at ADDR and song.hdr (the
header's address), to patch a built ROM for a quick listen.

Needs ffmpeg and: pip install numpy scipy librosa soundfile pretty_midi mir_eval
resampy onnxruntime torch torchaudio demucs; pip install --no-deps basic-pitch
"""
import sys, os, pickle, struct, argparse, subprocess
import numpy as np
import librosa
import soundfile as sf

ap = argparse.ArgumentParser()
ap.add_argument('audio'); ap.add_argument('out')
ap.add_argument('--name', required=True, help='label of the song in the .event file')
ap.add_argument('--work', help='folder for the intermediate files (default OUTDIR/work)')
ap.add_argument('--bin-base', help='also write song.bin linked at this address, e.g. 0x09700000')
ap.add_argument('--only', help='keep only these tracks (comma separated names), to listen to them alone')
ap.add_argument('--gain', type=float, default=1.5,
                help='scale every track volume: 1.0 is about as loud as vanilla BGM; the default is louder, to hide the GBA output noise')
ap.add_argument('--no-stems', action='store_true', help='do not use demucs stem separation')
ap.add_argument('--no-loop', action='store_true', help='end the song instead of looping it')
A = ap.parse_args()
wd = A.work or os.path.join(A.out, 'work')
os.makedirs(wd, exist_ok=True); os.makedirs(A.out, exist_ok=True)
TPB = 48            # ticks per beat (the tempo byte is then about the bpm itself)
GRID = 8            # onset grid in ticks (a sixth of a beat)
FPS = 59.7275       # GBA frames a second

# ---------------------------------------------------------------- transcription (cached in the work folder)
if not os.path.exists(wd + '/song.wav'):
    subprocess.run(['ffmpeg', '-loglevel', 'error', '-y', '-i', A.audio, '-vn', '-ac', '2', '-ar', '44100',
                    wd + '/song.wav'], check=True)
def run_demucs():
    """Split the mix into drums, bass, vocals and other (needs the demucs package and its model)."""
    try:
        subprocess.run([sys.executable, '-m', 'demucs', '-n', 'htdemucs', '-d', 'cpu', '-o', wd + '/stems', wd + '/song.wav'],
                       check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except Exception as e:
        print('no stem separation (%s): using the harmonic / percussive split' % type(e).__name__)
SD = wd + '/stems/htdemucs/song/'
if not A.no_stems and not os.path.exists(SD + 'other.wav'): run_demucs()
STEMS = not A.no_stems and os.path.exists(SD + 'other.wav')
if not os.path.exists(wd + '/mel.wav'):
    if STEMS:
        ld = lambda k: librosa.load(SD + k + '.wav', sr=22050, mono=True)[0]
        sf.write(wd + '/mel.wav', ld('other') + ld('vocals'), 22050)
        sf.write(wd + '/bass.wav', ld('bass'), 22050)
        sf.write(wd + '/drum.wav', ld('drums'), 22050)
    else:
        y0, sr0 = librosa.load(wd + '/song.wav', sr=22050, mono=True)
        H, P = librosa.decompose.hpss(librosa.stft(y0, n_fft=2048, hop_length=512), margin=(2.0, 2.0))
        sf.write(wd + '/mel.wav', librosa.istft(H, hop_length=512, length=len(y0)), sr0)
        sf.write(wd + '/drum.wav', librosa.istft(P, hop_length=512, length=len(y0)), sr0)
for part in (('mel', 'bass') if STEMS else ('mel',)):
    if os.path.exists(wd + '/bp_%s.pkl' % part): continue
    from basic_pitch.inference import predict, Model
    from basic_pitch import ICASSP_2022_MODEL_PATH
    model = Model(os.path.join(os.path.dirname(str(ICASSP_2022_MODEL_PATH)), 'nmp.onnx'))
    out, _, bp_notes = predict(wd + '/%s.wav' % part, model, onset_threshold=0.5, frame_threshold=0.3,
                               minimum_note_length=80, melodia_trick=True, multiple_pitch_bends=False)
    pickle.dump({'out': out, 'notes': [(n[0], n[1], n[2], n[3]) for n in bp_notes]}, open(wd + '/bp_%s.pkl' % part, 'wb'))

# ---------------------------------------------------------------- inputs
y, sr = librosa.load(wd + '/song.wav', sr=22050, mono=True)
dur = len(y) / sr
notes = sorted(pickle.load(open(wd + '/bp_mel.pkl', 'rb'))['notes'])
hop = 512
oenv = librosa.onset.onset_strength(y=y, sr=sr, hop_length=hop)
_, beats = librosa.beat.beat_track(onset_envelope=oenv, sr=sr, hop_length=hop, start_bpm=104,
                                   tightness=300, units='time', trim=False)
beats = list(beats)
# extend the grid to cover the whole file
ibi0 = float(np.median(np.diff(beats[:8]))); ibi1 = float(np.median(np.diff(beats[-8:])))
while beats[0] > 0: beats.insert(0, beats[0] - ibi0)
while beats[-1] < dur + 2: beats.append(beats[-1] + ibi1)
beats = np.array(beats)

def phase_score(shift):
    s = 0
    for st, en, p, a in notes:
        t = st - shift
        i = np.searchsorted(beats, t) - 1
        if 0 <= i < len(beats) - 1:
            ph = (t - beats[i]) / (beats[i + 1] - beats[i]) * 3
            s += a * np.cos(2 * np.pi * ph)
    return s
shifts = np.arange(-0.10, 0.101, 0.005)
best = max(shifts, key=phase_score)
beats = beats + best
print('beats %d, first %.3f, grid shift %.3f s, median bpm %.2f' % (len(beats), beats[0], best, 60 / np.median(np.diff(beats))))

def t2tick(t):
    i = int(np.clip(np.searchsorted(beats, t) - 1, 0, len(beats) - 2))
    return TPB * (i + (t - beats[i]) / (beats[i + 1] - beats[i]))
tick0 = t2tick(0.0)
def q(t, g=GRID):
    return int(round((t2tick(t) - tick0) / g)) * g

# loudness of the original per beat, for dynamics
rms = librosa.feature.rms(y=y, frame_length=2048, hop_length=hop)[0]
rt = librosa.frames_to_time(np.arange(len(rms)), sr=sr, hop_length=hop)
def loud(t, w=1.2):
    m = (rt >= t - w) & (rt < t + w)
    return float(rms[m].mean()) if m.any() else 0.0
LMAX = np.percentile(rms, 97)
def dyn(t):
    # flatter than the recording: the GBA's output noise is fixed, so quiet parts would hiss
    return float(np.clip((loud(t) / LMAX) ** 0.4, 0.55, 1.0))

# ---------------------------------------------------------------- pitched voices
# drop overtone ghosts: a weaker note an octave / twelfth / two octaves above a stronger one
def ghosts(ns):
    keep = []
    for n in ns:
        st, en, p, a = n
        g = False
        for m in ns:
            if m is n: continue
            if p - m[2] in (12, 19, 24, 28) and m[0] - 0.06 <= st and st < m[1] and m[3] > a * 1.25:
                g = True; break
        if not g: keep.append(n)
    return keep
n0 = len(notes)
notes = ghosts(notes)
print('notes %d -> %d after overtone filter' % (n0, len(notes)))

BASS_TOP = 44 if STEMS else 50     # with stems the bass is not in these notes
qn = []
for st, en, p, a in notes:
    s = q(st); e = max(s + GRID, q(en, 4))
    qn.append([s, e, int(p), float(a), st])

hi_c = [n for n in qn if n[2] > BASS_TOP]

# melody and bass: one line each, tracked through the note posteriorgram (Viterbi), then cut into notes
def load_pg(part):
    pg = pickle.load(open(wd + '/bp_%s.pkl' % part, 'rb'))['out']
    return pg['note'], pg['onset']
PG_MEL = load_pg('mel')
PG_BASS = load_pg('bass') if STEMS else PG_MEL
PFPS = PG_MEL[0].shape[0] / dur
def track_line(pg, lo, hi, prior, unv, jump, sw, on_thr, min_fr, legato=0.3, gap=GRID):
    S = pg[0][:, lo - 21:hi - 21 + 1].astype(np.float64)
    O = pg[1][:, lo - 21:hi - 21 + 1]
    T, P = S.shape
    em = np.log(S + 0.02) + prior(np.arange(lo, hi + 1))[None, :]
    em = np.concatenate([em, np.full((T, 1), np.log(unv))], axis=1)
    d = np.abs(np.arange(P)[:, None] - np.arange(P)[None, :])
    tr = np.zeros((P + 1, P + 1))
    tr[:P, :P] = -jump * np.minimum(d, 7) - 0.4 * (d > 0)
    tr[P, :P] = -sw; tr[:P, P] = -sw
    sc = em[0].copy(); bp = np.zeros((T, P + 1), dtype=np.int16)
    for t in range(1, T):
        m = sc[:, None] + tr
        bp[t] = m.argmax(0); sc = m.max(0) + em[t]
    path = np.zeros(T, dtype=np.int16); path[-1] = sc.argmax()
    for t in range(T - 1, 0, -1): path[t - 1] = bp[t, path[t]]
    out = []; t = 0
    while t < T:
        p = path[t]
        if p == P: t += 1; continue
        u = t + 1
        while u < T and path[u] == p:
            # a new attack on the same pitch starts a new note
            if u - t >= 5 and O[u, p] >= on_thr and O[u, p] >= O[u - 1, p] and (u + 1 >= T or O[u, p] >= O[u + 1, p]): break
            u += 1
        if u - t >= min_fr:
            st, en = t / PFPS, u / PFPS
            s0 = q(st); e0 = max(s0 + GRID, q(en, 4))
            out.append([s0, e0, int(p + lo), float(S[t:u, p].mean()), st, float(O[max(0, t - 2):t + 3, p].max())])
        t = u
    # one note per slot (the stronger), no overlaps
    by = {}
    for n in out:
        if n[0] not in by or by[n[0]][3] < n[3]: by[n[0]] = n
    line = [by[k] for k in sorted(by)]
    for i in range(len(line) - 1): line[i][1] = min(line[i][1], line[i + 1][0])
    # a same-pitch continuation with no real attack is one held note
    merged = []
    for n in line:
        if merged and merged[-1][2] == n[2] and n[0] - merged[-1][1] <= gap and n[5] < legato:
            merged[-1][1] = n[1]
        else: merged.append(n)
    return merged

melody = track_line(PG_MEL, 64, 96, lambda p: 0.25 * np.clip((p - 64) / 8.0, 0, 1), unv=0.30 if STEMS else 0.20, jump=0.22, sw=1.6, on_thr=0.45, min_fr=4)
melset_iv = [(n[0], n[1], n[2]) for n in melody]
def in_melody(n):
    for s0, e0, p in melset_iv:
        if p == n[2] and s0 - GRID <= n[0] < e0: return True
    return False
bass = track_line(PG_BASS, 26, 52, lambda p: -0.02 * (p - 26), unv=0.17, jump=0.30 if STEMS else 0.10, sw=1.2, on_thr=0.55,
                  min_fr=8 if STEMS else 6, legato=0.35, gap=TPB // 2)
if STEMS:
    # only where the bass stem really sounds
    yb, _ = librosa.load(wd + '/bass.wav', sr=22050, mono=True)
    brms = librosa.feature.rms(y=yb, frame_length=2048, hop_length=hop)[0]
    bref = np.percentile(brms, 90)
    def bass_on(n):
        t0 = n[4]; t1 = t0 + 0.25
        m = (rt >= t0) & (rt < t1)
        return m.any() and brms[:len(rt)][m[:len(brms)]].mean() >= 0.12 * bref
    bass = [n for n in bass if bass_on(n)]
for n in bass:
    while n[2] < 28: n[2] += 12

# accompaniment: what is left, at most 3 voices
acc = [n for n in hi_c if not in_melody(n) and n[3] >= (0.28 if STEMS else 0.33) and n[2] <= 84]
acc.sort(key=lambda n: (n[0], -n[3]))
ACC_POLY = 4 if STEMS else 3
active = []; acc2 = []
for n in acc:
    active = [m for m in active if m[1] > n[0]]
    if any(m[2] == n[2] for m in active): continue
    if len(active) >= ACC_POLY: continue
    # not above the melody sounding now
    top = [m[2] for m in melody if m[0] <= n[0] < m[1]]
    # (a clean stem's notes above the lead are real; a full mix's are mostly overtones)
    if top and n[2] >= top[0] and not STEMS: continue
    active.append(n); acc2.append(n)
pad = [n for n in acc2 if n[1] - n[0] >= TPB]
pluck = [n for n in acc2 if n[1] - n[0] < TPB]

def vel(n, lo=48, hi=127):
    a = np.clip((n[3] - 0.25) / 0.5, 0, 1)
    return int(np.clip((lo + (hi - lo) * a) * dyn(n[4]), 20, 127))

# ---------------------------------------------------------------- drums
yp, _ = librosa.load(wd + '/drum.wav', sr=22050, mono=True)
fr = librosa.fft_frequencies(sr=sr, n_fft=1024)
drums = {}
if STEMS:
    # a clean drum stem: every clear hit, named by where its energy is
    denv = librosa.onset.onset_strength(y=yp, sr=sr, hop_length=256)
    denv = denv / (np.percentile(denv, 99.5) + 1e-9)
    Sd = np.abs(librosa.stft(yp, n_fft=2048, hop_length=256)) ** 2
    fd = librosa.fft_frequencies(sr=sr, n_fft=2048)
    BANDS = ((0, 90, 36), (90, 200, 41), (200, 600, 45), (600, 3000, 37), (3000, 11025, 42))   # kick, toms, rim, hat
    hits = []
    for i in librosa.util.peak_pick(denv, pre_max=3, post_max=3, pre_avg=12, post_avg=12, delta=0.08, wait=6):
        w = Sd[:, i:i + 6].sum(1)
        e = [w[(fd >= lo) & (fd < hi)].sum() for lo, hi, _ in BANDS]
        hits.append((i * 256 / sr, np.sqrt(sum(e)), BANDS[int(np.argmax(e))][2]))
    ref = np.percentile([h[1] for h in hits], 95) if hits else 1.0
    for t, e, key in hits:
        if e < 0.15 * ref: continue
        k = (q(t), key)
        drums[k] = max(drums.get(k, 0), float(np.clip((e / ref) ** 0.5, 0, 1)))
else:
    Sp = np.abs(librosa.stft(yp, n_fft=1024, hop_length=256))
    def band_onsets(lo, hi, delta, wait):
        b = np.log1p(30 * Sp[(fr >= lo) & (fr < hi)].sum(axis=0))
        d = np.maximum(0, np.diff(b, prepend=b[0]))
        d = d / (np.percentile(d, 99.5) + 1e-9)
        pk = librosa.util.peak_pick(d, pre_max=3, post_max=3, pre_avg=10, post_avg=10, delta=delta, wait=wait)
        return [(librosa.frames_to_time(i, sr=sr, hop_length=256), float(min(d[i], 1.0))) for i in pk]
    prms = librosa.feature.rms(y=yp, frame_length=2048, hop_length=hop)[0]
    def perc_share(t, w=2.0):
        m = (rt >= t - w) & (rt < t + w)
        return float(prms[:len(rt)][m[:len(prms)]].mean() / (rms[m].mean() + 1e-9)) if m.any() else 0.0
    # no hi-hat here: in a harmonic / percussive split that band is mostly noise
    for key, lo, hi, delta, wait in ((36, 30, 140, 0.30, 8), (38, 180, 2500, 0.40, 8)):
        for t, s_ in band_onsets(lo, hi, delta, wait):
            if perc_share(t) < 0.09: continue
            k = (q(t), key)
            drums[k] = max(drums.get(k, 0), s_ * dyn(t))
drum_notes = [[s, s + 4, key, v, 0] for (s, key), v in sorted(drums.items())]
print('stems %s  melody %d  pad %d  pluck %d  bass %d  drums %d' % (STEMS, len(melody), len(pad), len(pluck), len(bass), len(drum_notes)))

# ---------------------------------------------------------------- tempo map
end_tick = int(np.ceil((t2tick(dur) - tick0) / TPB)) * TPB
nbeat = end_tick // TPB
tempo_ev = []; err = 0.0; cur = None
for b in range(nbeat):
    t_a = np.interp((b * TPB + tick0) / TPB, np.arange(len(beats)), beats)
    t_b = np.interp(((b + 1) * TPB + tick0) / TPB, np.arange(len(beats)), beats)
    want = (t_b - t_a) + err                       # seconds this beat should take
    td = int(np.clip(round(TPB * 75 / (FPS * want)), 20, 255))   # the engine adds 2 * byte a frame, a tick per 150
    got = TPB * 75 / (FPS * td)
    err = want - got
    if td != cur:
        tempo_ev.append((b * TPB, td)); cur = td
print('tempo events %d, final drift %.1f ms' % (len(tempo_ev), err * 1000))

# ---------------------------------------------------------------- encoder
LEN = list(range(0, 25)) + [28, 30, 32, 36, 40, 42, 44, 48, 52, 54, 56, 60, 64, 66, 68, 72, 76, 78, 80, 84, 88, 90, 92, 96]
LIDX = {v: i for i, v in enumerate(LEN)}

class Track:
    def __init__(self, name, voice, vol, pan, notes, velf, tempo=None, keysh=0, extra=b''):
        self.name, self.voice, self.vol, self.pan = name, voice, vol, pan
        self.notes = notes; self.velf = velf; self.tempo = tempo or []; self.keysh = keysh; self.extra = extra
    def encode(self, loop):
        ev = []   # (tick, order, kind, payload)
        for n in self.notes:
            s, e, key = n[0], n[1], n[2]
            d = e - s
            v = self.velf(n)
            if d in LIDX and d > 0: ev.append((s, 2, 'N', (d, key, v)))
            else:
                ev.append((s, 2, 'T', (key, v))); ev.append((e, 0, 'E', key))
        for t, td in self.tempo: ev.append((t, 1, 'tempo', td))
        ev.sort(key=lambda x: (x[0], x[1]))
        out = bytearray()
        out += bytes([0xBC, self.keysh & 0xFF])
        hdr_tempo = [e for e in ev if e[2] == 'tempo' and e[0] == 0]
        for e in hdr_tempo: out += bytes([0xBB, e[3]])
        out += bytes([0xBD, self.voice, 0xBE, self.vol, 0xBF, self.pan]) + self.extra
        loop_off = len(out)
        cur = 0; run = None; lastv = None; fresh = False  # fresh: previous byte was a command/wait
        def wait(to):
            nonlocal cur, fresh
            d = to - cur
            while d > 0:
                w = max(x for x in LEN if x <= d)
                out.append(0x80 + LIDX[w]); d -= w
            cur = to; fresh = True
        evs = [e for e in ev if not (e[2] == 'tempo' and e[0] == 0)]
        for i, (t, o, kind, pl) in enumerate(evs):
            if t > cur: wait(t)
            nxt_same = i + 1 < len(evs) and evs[i + 1][0] == t
            if kind == 'tempo':
                out += bytes([0xBB, pl]); run = None; fresh = True
            elif kind == 'E':
                out += bytes([0xCE, pl]); run = None; fresh = False
            else:
                if kind == 'N': cmd = 0xCF + LIDX[pl[0]]; key, v = pl[1], pl[2]
                else: cmd = 0xCF; key, v = pl
                if not (run == cmd and fresh): out.append(cmd)
                run = cmd
                out.append(key)
                # velocity may be dropped only if the next byte is a command or a wait
                if v != lastv or nxt_same: out.append(v); lastv = v
                fresh = False
        if end_tick + (0 if A.no_loop else TPB) > cur: wait(end_tick + (0 if A.no_loop else TPB))
        self.loop_off = loop_off
        self.body = out
        return out

mel_v = lambda n: vel(n, 70, 127)
# the lead again on strings where the recording is loud: the flute alone is too pure for a fiddle
lead2 = [n for n in melody if dyn(n[4]) >= 0.72 and n[2] <= 88]
# track volumes are near vanilla's (its BGM tracks sit around 35 to 50)
tracks = [
    Track('lead', 0, 48, 0x40, melody, mel_v, tempo=tempo_ev),
    Track('fiddle', 1, 36, 0x40 + 8, lead2, lambda n: vel(n, 60, 115)),
    Track('strings', 1, 46, 0x40 - 14, pad, lambda n: vel(n, 50, 105)),
    Track('harp', 2, 58, 0x40 + 16, pluck, lambda n: vel(n, 55, 115)),
    # the bass sample sounds an octave below its key
    Track('bass', 3, 56, 0x40, bass, lambda n: vel(n, 75, 127), keysh=12),
    Track('drums', 4, 66, 0x40, drum_notes, lambda n: int(np.clip(40 + 87 * n[3], 30, 127))),
]
for tr in tracks: tr.vol = int(np.clip(round(tr.vol * A.gain), 1, 127))
if A.only:
    tracks[0].notes = tracks[0].notes if 'lead' in A.only.split(',') else []
    tracks = [tracks[0]] + [t for t in tracks[1:] if t.name in A.only.split(',')]
# voicegroup built from samples that are already in the vanilla ROM
def ds(smp, adsr): return struct.pack('<BBBBI', 0, 60, 0, 0, 0x08000000 + smp) + bytes.fromhex(adsr)
VOICES = [
    ('flute', ds(0x27E640, 'ff00ffa5')),
    ('strings', ds(0x22B4DC, 'ff00ffa5')),
    ('harp', ds(0x22807C, 'fff200a5')),
    ('bass', ds(0x227988, 'ff00ffa5')),
    ('drum kit', struct.pack('<BBBBII', 0x80, 0, 0, 0, 0x082228F0, 0)),
]
base = int(A.bin_base, 16) if A.bin_base else 0
blob = bytearray(); labels = {}; fix = []   # fix: (offset, label, add)
labels['vg'] = 0
for nm, v in VOICES: blob += v
for i, tr in enumerate(tracks):
    b = tr.encode(not A.no_loop)
    labels['t%d' % i] = len(blob)
    blob += b
    if A.no_loop: blob += b'\xB1'
    else:
        blob += b'\xB2'; fix.append((len(blob), 't%d' % i, tr.loop_off)); blob += b'\0\0\0\0'; blob += b'\xB1'
while len(blob) % 4: blob.append(0)
labels['hdr'] = len(blob)
blob += bytes([len(tracks), 0, 10, 0x94])   # priority and reverb as vanilla's BGM
fix.append((len(blob), 'vg', 0)); blob += b'\0\0\0\0'
for i in range(len(tracks)):
    fix.append((len(blob), 't%d' % i, 0)); blob += b'\0\0\0\0'
fixset = {o: (l, a) for o, l, a in fix}
bin_ = bytearray(blob)
for o, (l, a) in fixset.items(): struct.pack_into('<I', bin_, o, base + labels[l] + a)
if A.bin_base:
    open(A.out + '/song.bin', 'wb').write(bin_)
    open(A.out + '/song.hdr', 'w').write('%#x\n' % (base + labels['hdr']))

# event file
nm = A.name
L = ['// %s: generated by Tools/audio2m4a/audio2m4a.py from %s. Do not edit by hand.' % (nm, os.path.basename(A.audio)),
     '// %d bytes, %d frames: a %d-voice bank of vanilla FE8 samples (%s) and %d tracks (%s).' % (len(blob), int(dur * FPS), len(VOICES), ', '.join(v[0] for v in VOICES), len(tracks), ', '.join(t.name for t in tracks)), '',
     'ALIGN 4', '%s_voicegroup:' % nm]
inv = {v: k for k, v in labels.items()}
names = {'vg': '%s_voicegroup' % nm, 'hdr': nm}
for i, tr in enumerate(tracks): names['t%d' % i] = '%s_track%d_%s' % (nm, i, tr.name)
o = 0; n = len(blob)
bounds = sorted(set(labels.values()) | {n})
def emit_bytes(a, b):
    while a < b:
        c = min(b, a + 24)
        L.append('BYTE ' + ' '.join('0x%02X' % x for x in blob[a:c])); a = c
for bi in range(len(bounds) - 1):
    a, b = bounds[bi], bounds[bi + 1]
    lab = inv[a]
    if lab != 'vg':
        L.append(''); L.append(names[lab] + ':')
    p = a
    while p < b:
        nxt = min([f for f in fixset if p <= f < b] + [b])
        emit_bytes(p, nxt); p = nxt
        if p < b:
            l, add = fixset[p]
            L.append('POIN2 %s%s' % (names[l], ('+%d' % add) if add else '')); p += 4
open(A.out + '/%s.event' % nm, 'w').write('\n'.join(x for x in L if x is not None) + '\n')

# midi for reference / hand editing
import pretty_midi
pm = pretty_midi.PrettyMIDI(initial_tempo=104)
tick2t = lambda k: float(np.interp((k + tick0) / TPB, np.arange(len(beats)), beats))
GM = {'lead': 73, 'fiddle': 40, 'strings': 48, 'harp': 46, 'bass': 33, 'drums': 0}
for tr in tracks:
    prog = GM[tr.name]
    ins = pretty_midi.Instrument(program=prog, is_drum=(tr.name == 'drums'), name=tr.name)
    for nn in tr.notes:
        ins.notes.append(pretty_midi.Note(velocity=int(tr.velf(nn)), pitch=int(nn[2]), start=max(0, tick2t(nn[0])), end=max(0.01, tick2t(nn[1]))))
    pm.instruments.append(ins)
pm.write(A.out + '/%s.mid' % nm)
evs = []
for tr in tracks:
    for nn in tr.notes: evs += [(nn[0], 1), (nn[1], -1)]
evs.sort(); c = m = 0
for _, dlt in evs: c += dlt; m = max(m, c)
print('max simultaneous notes %d' % m)
frames_total = int(dur * FPS)
print('SIZE bytes=%d  tracks=%s  song frames=%d' % (len(blob), [len(t.body) for t in tracks], frames_total))
