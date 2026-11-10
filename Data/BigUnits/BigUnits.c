#include "common-chax.h"
#include "big-unit.h"

/*
 * 2x2 units (Documentation/Features/BigUnits.md): a unit whose character or class is listed here loads as a 2x2 block
 * of four corners, each with its own HP, drawn as one map sprite at twice its size. Each list ends with 0; there is
 * room for 15 entries, and the free bytes are zero.
 *
 * Example: CHARACTER_ONEILL makes the prologue's boss a 2x2 unit (smoketest.py's bigunit test does that in a test ROM).
 */

const u8 gBigUnitCharacters[0x10] = {
	0,
};

const u8 gBigUnitClasses[0x10] = {
	0,
};
