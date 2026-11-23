#!/usr/bin/env python3
# Lists the committed build outputs a build should leave as they are (make -o), one per line.
#
# Committed generated files: every tracked .lyn.event file, every path in Tools/cloud/protected-outputs.txt,
# and every tracked grit output of a tracked .png. A file is not listed if it is modified or if its .c, .s or
# .png source is modified, so a file you edit still regenerates its own outputs.
#
# Why: the committed .lyn.event files come from one compiler, and another one (devkitARM vs the Ubuntu
# arm-none-eabi in cloud sessions) makes different output, so a full rebuild would rewrite hundreds of committed
# files and block git pull. Grit outputs need it because gritshim.py output can differ from real grit, and a
# fresh clone's file times can make any .png look newer than its outputs.
#
# ea-dep names some targets through "Dir/../" paths, so every spelling it uses is listed too.
#
# --make-args prints make arguments instead, one per line: "-o FILE" for each file. With --rom ROM --rom-args FILE,
# it also writes "-W main.event" to FILE (else leaves FILE empty) when a listed file is newer than ROM: make -o also
# stops make from rebuilding the ROM because of a changed file, e.g. a .lyn.event that git pull brought in. Pass
# those to the make that builds the ROM only: any later make given them would assemble it again.
# Run from the repo root.
import argparse
import os
import subprocess

ap = argparse.ArgumentParser()
ap.add_argument('list', nargs='?', default='Tools/cloud/protected-outputs.txt')
ap.add_argument('--make-args', action='store_true')
ap.add_argument('--rom')
ap.add_argument('--rom-args')
args = ap.parse_args()
P = args.list


def git(*args):
    return subprocess.run(['git'] + list(args), capture_output=True, text=True).stdout


changed = set(l[3:].strip() for l in git('status', '--porcelain', '--untracked-files=all').splitlines())
tracked = set(git('ls-files', '*.lyn.event').split())
tracked |= set(l.strip() for l in open(P) if l.strip() and not l.strip().endswith('.lyn.event'))
pngs = set(git('ls-files', '*.png').split())
GRIT = ('.img.bin', '.map.bin', '.pal.bin', '.4bpp', '.4bpp.lz', '.gbapal', '.lz77')


def png_of(p):
    for e in GRIT:
        if p.endswith(e) and p[:-len(e)] + '.png' in pngs:
            return p[:-len(e)] + '.png'
    return None


tracked |= set(p for p in git('ls-files', *['*' + e for e in GRIT]).split() if png_of(p))


def source(p):
    if p.endswith('.lyn.event'):
        b = p[:-len('.lyn.event')]
        return [b + '.c', b + '.s']
    return [png_of(p)] if png_of(p) else []


def protected(p):
    if p not in tracked or p in changed:
        return False
    return not any(s in changed for s in source(p))


deps = subprocess.run(['Tools/EventAssembler/ea-dep', 'main.event', '-I', 'Tools/EventAssembler', '--add-missings'],
                      capture_output=True, text=True).stdout.split()
out = set(p for p in tracked if protected(p))
for d in deps:
    n = os.path.normpath(d)
    if n in out:
        out.add(d)
out = sorted(out)
if not args.make_args:
    print('\n'.join(out))
else:
    print('\n'.join(a for p in out for a in ('-o', p)))
    if args.rom_args:
        stale = False
        if args.rom and os.path.exists(args.rom):
            rom_time = os.path.getmtime(args.rom)
            stale = any(os.path.getmtime(p) > rom_time for p in out if os.path.exists(p))
        with open(args.rom_args, 'w') as f:
            f.write('-W\nmain.event\n' if stale else '')
