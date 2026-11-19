#!/bin/bash
# One-time setup after cloning: checks the committed tools, marks the committed generated files as up to date and
# checks devkitARM, the ROM and the Python packages. Downloads nothing. Safe to run again. See Documentation/Setup.md.
set -u
ROOT=$(git -C "$(dirname "$(readlink -f "$0")")" rev-parse --show-toplevel) || exit 1
cd "$ROOT" || exit 1
fail=0
log() { echo "[setup] $*"; }

# 1. Every tool the build uses (EventAssembler, FE-CLib-Mokha, FE-PyTools, check_patch) is committed to this
#    repository; nothing is downloaded. Restore any that were replaced or deleted.
for d in Tools/EventAssembler Tools/FE-CLib-Mokha Tools/FE-PyTools Tools/check_patch; do
	[ -n "$(git ls-files "$d" | head -1)" ] && [ -z "$(git status --porcelain -- "$d" | grep -v '^??')" ] ||
		{ log "ERROR: $d differs from the committed copy (git checkout -- $d)"; fail=1; }
done

# 2. Tools/FE-CLib-Mokha/reference holds the FE8U symbol files lyn links against. They are committed, pinned to
#    the names the C code uses; never replace them with upstream FE-CLib-Mokha's, which rename symbols
#    (e.g. ItemEffectiveness_088ADEC2) and drop the local labels the lyn protectors use to size functions.
CLIB_REF=Tools/FE-CLib-Mokha/reference
CLIB_FILES="fireemblem8.ref.o fireemblem8.ref.s fireemblem8.sym"
for f in $CLIB_FILES; do
	[ -s "$CLIB_REF/$f" ] || { log "ERROR: $CLIB_REF/$f is missing (git checkout -- $CLIB_REF)"; fail=1; }
done
grep -q ItemEffectiveness_088ADEC2 "$CLIB_REF/fireemblem8.sym" 2>/dev/null ||
	{ log "ERROR: $CLIB_REF was replaced by another version (git checkout -- $CLIB_REF)"; fail=1; }
# Date them in the past so they do not look newer than the committed .lyn.event files.
for f in $CLIB_FILES; do [ -f "$CLIB_REF/$f" ] && touch -d '2000-01-01' "$CLIB_REF/$f"; done

# 3. The committed .lyn.event files are build output, and some carry hand fixes (see CLAUDE.md). A fresh clone
#    gives files arbitrary timestamps, so make could regenerate them all and lose those fixes. Mark them up to
#    date; make still rebuilds any whose .c you change later.
git ls-files -z '*.lyn.event' | xargs -0 touch
log "marked the committed .lyn.event files up to date"

# 4. Checks
if [ -z "${DEVKITPRO:-}" ] || [ -z "${DEVKITARM:-}" ] || [ ! -x "${DEVKITARM:-/nonexistent}/bin/arm-none-eabi-gcc" ]; then
	log "WARNING: DEVKITPRO/DEVKITARM not set, or devkitARM is not installed (Setup.md step 2)"
	fail=1
fi
FE8_SHA1=c25b145e37456171ada4b0d440bf88a19f4d509f
if [ ! -f fe8.gba ]; then
	log "WARNING: put a clean FE8U ROM at the repository root as fe8.gba"
	fail=1
elif [ "$(sha1sum fe8.gba | awk '{print $1}')" != "$FE8_SHA1" ]; then
	log "WARNING: fe8.gba is not a clean FE8U ROM (expected sha1 $FE8_SHA1)"
	fail=1
fi
for t in Tools/EventAssembler/ColorzCore Tools/EventAssembler/Tools/lyn Tools/EventAssembler/ea-dep; do
	[ -x "$t" ] || { log "WARNING: $t is missing or not executable"; fail=1; }
done
python3 -c 'import PIL, six, numpy' 2>/dev/null ||
	{ log "WARNING: missing Python packages (pip install Pillow six numpy)"; fail=1; }

if [ $fail -eq 0 ]; then
	log "done - run make"
else
	log "finished with problems (see above)"
fi
exit $fail
