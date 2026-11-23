#!/usr/bin/env python3
# post_chax check of the vanilla functions the build replaces (Documentation/FeatureTypes/Hooks.md). Run from the repo
# root. One process instead of a few per file: the shell version this replaces took about 16 s a build, this a fraction
# of a second, with the same checks and messages.
#
# Round 1: every LYN_REPLACE_CHECK(X) in a .c file needs a LynJump.event in the same directory that mentions X.
# Round 2: every POIN X within three lines of an "ORG $" line of a .lyn.event file (a lyn jump) needs
#          LYN_REPLACE_CHECK(X); in the .c file of the same name.
# Standalone/ and .claude/ (worktree copies) are skipped.
import os
import re
import sys

SKIP = ('./Standalone/', './.claude/')


def files(ext):
    for root, dirs, names in os.walk('.'):
        if (root + '/').startswith(SKIP):
            dirs[:] = []
            continue
        dirs.sort()
        for n in sorted(names):
            p = os.path.join(root, n)
            if n.endswith(ext) and not os.path.islink(p):
                yield p


def read(p):
    with open(p, encoding='latin-1') as f:
        return f.read()


ret = 0

# Round 1: c files
for src in files('.c'):
    text = read(src)
    if 'LYN_REPLACE_CHECK' not in text:
        continue
    jump = os.path.join(os.path.dirname(src), 'LynJump.event')
    jump_text = read(jump) if os.path.exists(jump) else None
    for line in text.splitlines():
        if 'LYN_REPLACE_CHECK' not in line:
            continue
        parts = re.split(r'[()]', line)
        func = parts[1] if len(parts) > 1 else ''
        if jump_text is None:
            print('[ERROR] lynjump file not in the same directory as the C source file: %s in file %s' % (func, src))
            ret = 1
        elif not func or func not in jump_text:
            print('[Error] C function not referenced in lynjump file: %s in file %s' % (func, src))
            ret = 1

# Round 2: lyn files
for lyn in files('.lyn.event'):
    lines = read(lyn).splitlines()
    near = set()
    for i, line in enumerate(lines):
        if line.startswith('ORG $'):
            near.update(range(i, min(i + 4, len(lines))))
    funcs = []
    for i in sorted(near):
        m = re.search(r'POIN (.+)', lines[i])
        if m:
            funcs.append(m.group(1))
    if not funcs:
        continue
    src = lyn.replace('.lyn.event', '.c')
    src_text = read(src) if os.path.exists(src) else ''
    for func in funcs:
        if 'LYN_REPLACE_CHECK(%s);' % func not in src_text:
            print('[ERROR] LYN_REPLACE_CHECK function not found for: %s in file %s' % (func, src))
            ret = 1

sys.exit(ret)
