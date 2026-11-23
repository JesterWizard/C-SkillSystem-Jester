#!/bin/bash
# Cloud build wrapper: builds fe8-kernel-dev.gba with the Ubuntu arm-none-eabi toolchain and the Python grit stand-in that .claude/hooks/session-start.sh sets up. Committed generated files (tracked .lyn.event files, tracked grit outputs of a tracked .png, and those in protected-outputs.txt) are marked old (make -o) unless their source changed, so a different compiler doesn't rewrite them. Prints errors, kernel text size, compiled files and BUILD rc=N.
# The list comes from Tools/scripts/protected-outputs.py, which plain make uses too.
ROOT=$(git rev-parse --show-toplevel)
HERE="$ROOT/Tools/cloud"
cd "$ROOT"

: "${DEVKITPRO:?DEVKITPRO not set - run .claude/hooks/session-start.sh}"
[ -f "$ROOT/fe8.gba" ] || { echo "fe8.gba missing in $ROOT - put a clean FE8 ROM at the repo root" >&2; exit 1; }

LOG="${BUILD_LOG:-/tmp/c-skillsys-build.log}"

ROMARGS=$(mktemp)
mapfile -t OPTS < <(python3 Tools/scripts/protected-outputs.py "$HERE/protected-outputs.txt" --make-args --rom fe8-kernel-dev.gba --rom-args "$ROMARGS")
# ROM_OPTS (-W main.event when a kept file is newer than the ROM) go to the ROM's make only, or it is assembled twice
mapfile -t ROM_OPTS < "$ROMARGS"
rm -f "$ROMARGS"
{ make -j"$(nproc)" "${OPTS[@]}" pre_build && make -j"$(nproc)" "${OPTS[@]}" "${ROM_OPTS[@]}" chax && make "${OPTS[@]}" post_chax; } > "$LOG" 2>&1
rc=$?
# smoketest.py reads RAM addresses from config-memmap.o, which make deletes as an intermediate of the .lyn.event chain
[ $rc -eq 0 ] && { make "${OPTS[@]}" include/link/config-memmap.o >> "$LOG" 2>&1; rc=$?; }
grep -E "error|Error|Assertion|warning" "$LOG" | head -40
grep "Kernel______Text" "$LOG"
grep "^\[CC \]" "$LOG"
tail -3 "$LOG"
echo "BUILD rc=$rc"
exit $rc
