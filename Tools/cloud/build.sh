#!/bin/bash
# Cloud build wrapper: builds fe8-kernel-dev.gba with the Ubuntu arm-none-eabi toolchain and the Python grit stand-in that .claude/hooks/session-start.sh sets up. Committed generated files (tracked .lyn.event files, tracked grit outputs of a tracked .png, and those in protected-outputs.txt) are marked old (make -o) unless their source changed, so a different compiler doesn't rewrite them. Prints errors, kernel text size, compiled files and BUILD rc=N.
# ea-dep names some targets through "Dir/../" paths, so every spelling it uses is protected too.
ROOT=$(git rev-parse --show-toplevel)
HERE="$ROOT/Tools/cloud"
cd "$ROOT"

: "${DEVKITPRO:?DEVKITPRO not set - run .claude/hooks/session-start.sh}"
[ -f "$ROOT/fe8.gba" ] || { echo "fe8.gba missing in $ROOT - put a clean FE8 ROM at the repo root" >&2; exit 1; }

LOG="${BUILD_LOG:-/tmp/c-skillsys-build.log}"
OLDLIST=$(mktemp)

python3 - "$HERE/protected-outputs.txt" > "$OLDLIST" <<'PY'
import os, subprocess, sys
P = sys.argv[1]
changed = set(l[3:].strip() for l in subprocess.run(['git', 'status', '--porcelain', '--untracked-files=all'],
              capture_output=True, text=True).stdout.splitlines())
tracked = set(subprocess.run(['git', 'ls-files', '*.lyn.event'], capture_output=True, text=True).stdout.split())
tracked |= set(l.strip() for l in open(P) if l.strip() and not l.strip().endswith('.lyn.event'))
# grit outputs of a tracked .png: gritshim.py output can differ from real grit, and a fresh clone's mtimes can make any .png look newer
pngs = set(subprocess.run(['git', 'ls-files', '*.png'], capture_output=True, text=True).stdout.split())
GRIT = ('.img.bin', '.map.bin', '.pal.bin', '.4bpp', '.4bpp.lz', '.gbapal', '.lz77')
def png_of(p):
    for e in GRIT:
        if p.endswith(e) and p[:-len(e)] + '.png' in pngs:
            return p[:-len(e)] + '.png'
    return None
tracked |= set(p for p in subprocess.run(['git', 'ls-files'] + ['*' + e for e in GRIT], capture_output=True, text=True).stdout.split()
               if png_of(p))
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
print('\n'.join(sorted(out)))
PY
OPTS=()
while read -r f; do OPTS+=(-o "$f"); done < "$OLDLIST"
{ make -j"$(nproc)" "${OPTS[@]}" pre_build && make -j"$(nproc)" "${OPTS[@]}" chax && make "${OPTS[@]}" post_chax; } > "$LOG" 2>&1
rc=$?
grep -E "error|Error|Assertion|warning" "$LOG" | head -40
grep "Kernel______Text" "$LOG"
grep "^\[CC \]" "$LOG"
tail -3 "$LOG"
git checkout -- Kernel/Wizardry/BuildVersionTitleScreen/BuildVersionTitleScreen_Installer.event
rm -f "$OLDLIST"
echo "BUILD rc=$rc"
exit $rc
