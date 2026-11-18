#!/usr/bin/env python3
"""Write Documentation/Features/CustomBattleAnimationSizes.md: every custom battle animation's size in the ROM, against
the same animation uncompressed and inserted the usual way.

Usage:
  python3 Tools/banim/sizes.py [OUT.md]

Every animation installed by Contents/BattleAnims/BattleAnims.event is read back from its committed files (the
converter's output, Tools/banim/feditor_dump.py or febuilder_txt.py) and decoded to its raw parts: the sheets (8 KB
each), the script, the right and left OAMs, the palettes and the modes. Then:

- Uncompressed: the raw parts.
- Usual: what FEditor Advance and FEBuilderGBA insert: every part but the modes in the BIOS's LZ77 (a greedy
  compressor, Tools/gallery/png2cg.py's), both OAMs stored.
- Now: the bytes the build puts in the ROM: the Huffman sheets with their 8-byte headers and the shared trees, the
  optimal-parse LZ77 parts, no left OAM when AutoGenLeftOAM makes it (Documentation/Features/CustomBattleAnimations.md,
  "Compression").

ALIGN padding (at most 3 bytes a part) is left out of every column.
"""
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'gallery'))

from banimpack import huff_unpack  # noqa: E402
from feditor_dump import mirror_oam  # noqa: E402
from png2cg import lz77_compress, lz77_decompress  # noqa: E402

ROOT = os.path.normpath(os.path.join(HERE, '..', '..'))
ANIM_DIR = os.path.join(ROOT, 'Contents', 'BattleAnims')
OUT = os.path.join(ROOT, 'Documentation', 'Features', 'CustomBattleAnimationSizes.md')
FAKE_SHEET = 0x08A00000  # sheet pointers in the script, for the usual-way column: sheet k at FAKE_SHEET + 0x2000 * k


def fail(msg):
    sys.exit('sizes: error: ' + msg)


def installed():
    """[(id, prefix, path of the .event)] in BattleAnims.event's order."""
    text = open(os.path.join(ANIM_DIR, 'BattleAnims.event')).read()
    defines = {m.group(1): int(m.group(2), 0) for m in re.finditer(r'#define\s+(\w+)\s+(0x[0-9A-Fa-f]+)\b', text)}
    paths = {}
    for m in re.finditer(r'#include\s+"([^"]+\.event)"', text):
        paths[os.path.splitext(os.path.basename(m.group(1)))[0]] = os.path.join(ANIM_DIR, m.group(1))
    out = []
    for m in re.finditer(r'BanimEntry\((\w+)\)\s*\n\s*(\w+)_BANIM_ENTRY', text):
        name, prefix = m.groups()
        if name not in defines or prefix not in paths:
            fail('cannot resolve %s / %s' % (name, prefix))
        out.append((defines[name], prefix, paths[prefix]))
    return out


def read_anim(prefix, path):
    """Decode an animation's committed files. Returns (raw parts, ROM bytes now, sheet count)."""
    d = os.path.dirname(path)
    lines = open(path).read().splitlines()

    def blob(name):
        return open(os.path.join(d, name), 'rb').read()

    labels, cur = {}, None
    for ln in lines:
        m = re.match(r'^(%s_\w+):' % re.escape(prefix), ln)
        if m:
            cur = m.group(1)[len(prefix) + 1:]
            labels.setdefault(cur, [])
            continue
        if cur is not None:
            labels[cur].append(ln)
    now = 0
    trees = None
    if 'Trees' in labels:
        tn = re.search(r'"(.+)"', labels['Trees'][0]).group(1)
        trees = blob(tn)
        now += len(trees)
    sheets = []
    k = 1
    while 'Sheet%d' % k in labels:
        body = labels['Sheet%d' % k]
        fn = re.search(r'#incbin "(.+)"', '\n'.join(body)).group(1)
        data = blob(fn)
        if fn.endswith('.huff'):
            sheets.append(huff_unpack(trees, data))
            now += 8 + len(data)
        else:
            sheets.append(lz77_decompress(data))
            now += len(data)
        k += 1
    modes = blob(re.search(r'"(.+)"', labels['Modes'][0]).group(1))
    now += len(modes)
    oam_r_lz = blob(re.search(r'"(.+)"', '\n'.join(labels['OamR'] + labels.get('OamL', []))).group(1))
    oam_r = lz77_decompress(oam_r_lz)
    now += len(oam_r_lz)
    # OamR and OamL are one pointer (OamL's label right after OamR's) when AutoGenLeftOAM makes the left OAM
    left_body = [ln for ln in labels.get('OamL', []) if '#incbin' in ln]
    if any('#incbin' in ln for ln in labels['OamR']) and left_body:
        oam_l_lz = blob(re.search(r'"(.+)"', left_body[0]).group(1))
        oam_l = lz77_decompress(oam_l_lz)
        now += len(oam_l_lz)
    else:
        oam_l = mirror_oam(oam_r)
    pal_lz = blob(re.search(r'"(.+)"', labels['Pal'][0]).group(1))
    pal = lz77_decompress(pal_lz)
    now += len(pal_lz)
    script = bytearray()
    for ln in labels['Script']:
        m = re.match(r'^BANIM_PTR_BYTE\(%s_Sheet(\d+), (\d)\)' % re.escape(prefix), ln)
        if m:
            script.append((FAKE_SHEET + 0x2000 * (int(m.group(1)) - 1)) >> (8 * int(m.group(2))) & 0xFF)
        elif ln.startswith('BYTE '):
            script += bytes(int(t, 0) for t in ln[5:].split())
    now += len(script)
    script = lz77_decompress(bytes(script))
    parts = {'sheets': sheets, 'script': script, 'oam_r': oam_r, 'oam_l': oam_l, 'pal': pal, 'modes': modes}
    return parts, now


def kb(n):
    return '%.1f KB' % (n / 1024)


def pct(a, b):
    return '%.1f%%' % (100 * (1 - b / a))


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else OUT
    rows = []
    for anim_id, prefix, path in installed():
        p, now = read_anim(prefix, path)
        raw_parts = [*p['sheets'], p['script'], p['oam_r'], p['oam_l'], p['pal']]
        raw = sum(len(x) for x in raw_parts) + len(p['modes'])
        usual_sheets = sum(len(lz77_compress(s)) for s in p['sheets'])
        usual = usual_sheets + sum(len(lz77_compress(x)) for x in raw_parts[len(p['sheets']):]) + len(p['modes'])
        rows.append((anim_id, os.path.relpath(os.path.dirname(path), ANIM_DIR), prefix, len(p['sheets']), raw, usual,
                     now))
        print('%#x %-28s %2d sheets  %8d raw  %8d usual  %8d now' % (anim_id, prefix, len(p['sheets']), raw, usual,
                                                                          now))
    rows.sort()
    t = ['# Custom battle animation sizes', '',
         'Generated by `python3 Tools/banim/sizes.py`. Sizes are what each animation in',
         '`Contents/BattleAnims/BattleAnims.event` takes in the ROM:', '',
         '- **Uncompressed**: the raw parts: the sheets (8 KB each), the script, the right and left OAMs, the palettes'
         ' and the modes.',
         '- **Usual**: inserted as FEditor Advance and FEBuilderGBA do it: every part but the modes in the BIOS\'s LZ77'
         ' (a greedy compressor), both OAMs stored.',
         '- **Now**: what the build inserts: Huffman-coded sheets, optimal-parse LZ77 for the rest, the left OAM left to'
         ' AutoGenLeftOAM ([CustomBattleAnimations.md](CustomBattleAnimations.md#compression)).', '',
         '"Saved" is how much smaller Now is. ALIGN padding (at most 3 bytes a part) is left out.', '',
         '| ID | Animation | Sheets | Uncompressed | Usual | Now | Saved vs uncompressed | vs usual |',
         '|---|---|---|---|---|---|---|---|']
    tr = tu = tn = ts = 0
    for anim_id, where, prefix, ns, raw, usual, now in rows:
        t.append('| 0x%02X | %s (`%s`) | %d | %s | %s | %s | %s | %s |' % (
            anim_id, where, prefix, ns, kb(raw), kb(usual), kb(now), pct(raw, now), pct(usual, now)))
        tr += raw
        tu += usual
        tn += now
        ts += ns
    t.append('| | **All %d** | %d | %s | %s | %s | %s | %s |' % (len(rows), ts, kb(tr), kb(tu), kb(tn), pct(tr, tn),
                                                                  pct(tu, tn)))
    t += ['', 'In all, the custom animations take %s instead of %s inserted the usual way: %s less.' % (
        kb(tn), kb(tu), kb(tu - tn)), '']
    worse = [r for r in rows if r[6] > r[5]]
    if worse:
        t += ['Bigger than the usual way: %s. Each has one sheet, and the Huffman trees (about 2 KB an animation) cost'
              ' more than they save; `feditor_dump.py --lz77` would write its sheet in LZ77 instead.' % ', '.join(
                  '0x%02X (`%s`, %s more)' % (r[0], r[2], kb(r[6] - r[5])) for r in worse), '']
    with open(out, 'w') as f:
        f.write('\n'.join(t))
    print('wrote %s: %d animations, %d usual, %d now' % (out, len(rows), tu, tn))


if __name__ == '__main__':
    main()
