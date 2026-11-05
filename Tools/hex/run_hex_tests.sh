#!/usr/bin/env bash
# Builds the Hex rules-engine host test (Tools/hex/test_hex.c with the game's HexCore.c) and runs it.
# Uses the host gcc and Tools/hex/shim/common-chax.h (the GBA types) in place of the repo's header.
# Exit status is the test's: 0 when every check passes.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUT="$(mktemp -d)"
trap 'rm -rf "$OUT"' EXIT

gcc -DHEX_HOST -std=gnu99 -Wall -Wextra -Werror \
	-I "$ROOT/Tools/hex/shim" -I "$ROOT/include/kernel" \
	-o "$OUT/test_hex" \
	"$ROOT/Tools/hex/test_hex.c" \
	"$ROOT/Kernel/Wizardry/Minigames/Hex/Source/HexCore.c"
"$OUT/test_hex"
