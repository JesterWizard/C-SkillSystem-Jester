#!/usr/bin/env bash
# TEST ONLY. Builds the Hex Cards test ROM from your own clean FE8U ROM, in one Event Assembler run:
#   parent test/ToChapter4.event (chapter-4 start) -> parent Installer.event -> hex/Installer.event
# Run `python3 test/make_ch4_entry.py <clean fe8.gba>` in the parent folder first (see the parent test/README.md).
#
# Usage: hex/test/make_test_rom.sh <clean fe8.gba> <out.gba>
# The clean ROM is copied to <out.gba> and that copy is patched; the clean ROM is only read.
set -euo pipefail

if [ "$#" -ne 2 ]; then
	echo "usage: $0 <clean fe8.gba> <out.gba>" >&2
	exit 2
fi

HERE=$(cd "$(dirname "$0")" && pwd)           # .../unlimited_prep_menu_options/hex/test
HEX=$(cd "$HERE/.." && pwd)                   # .../hex
PARENT=$(cd "$HEX/.." && pwd)                 # .../unlimited_prep_menu_options
REPO=$(cd "$PARENT/../.." && pwd)             # repo root
EA=$REPO/Tools/EventAssembler
CLEAN=$(cd "$(dirname "$1")" && pwd)/$(basename "$1")
OUT=$(cd "$(dirname "$2")" && pwd)/$(basename "$2")

if [ ! -f "$PARENT/test/Ch4Entry.bin" ]; then
	echo "missing $PARENT/test/Ch4Entry.bin: run python3 $PARENT/test/make_ch4_entry.py <clean fe8.gba> first" >&2
	exit 1
fi

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

cp "$CLEAN" "$OUT"

cat > "$WORK/test.event" <<EOF
#include "Extensions/Hack Installation.txt"
#include "$PARENT/test/ToChapter4.event"
#include "$PARENT/Installer.event"
#include "$HEX/Installer.event"
EOF

# -I dirs resolve the relative includes: test/Ch4Entry.bin and Source/*.lyn.event of each patch.
"$EA/ColorzCore" A FE8 -input:"$WORK/test.event" -output:"$OUT" \
	-I:"$EA" -I:"$PARENT" -I:"$HEX"

echo "built $OUT"
