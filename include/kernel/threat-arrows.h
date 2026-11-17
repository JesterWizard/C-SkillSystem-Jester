#pragma once

#include "common-chax.h"

/*
 * Threat arrows (Kernel/Wizardry/ThreatArrows, Documentation/Features/ThreatArrows.md): while the player picks where to
 * move a unit, a flashing red arrow runs from each enemy that could attack the cursor's tile in the next enemy phase.
 */

/* Enemies counted per move selection (one bit each in a tile's mask), the nearest ones to the selected unit */
#define THREAT_ENEMY_MAX 32
/* The rows of the selected unit's move range that get masks, and the tiles in them */
#define THREAT_ROW_MAX 32
#define THREAT_TILE_MAX 384

/*
 * The masks, in kernel RAM (_kernel_malloc_overlay0, include/link/config-memmap.s): only live during a move selection.
 * Row r is map row (by + r) of the proc; its tiles x0 .. x0 + len - 1 have masks[off ..]. reach is the enemy being
 * worked on's movement map, a bit per tile (y * width + x), kept from one frame's step to the next.
 */
struct ThreatArrowsData {
	u8 rowX0[THREAT_ROW_MAX];
	u8 rowLen[THREAT_ROW_MAX];
	u16 rowOff[THREAT_ROW_MAX];
	u32 masks[THREAT_TILE_MAX];
	u8 reach[0x100];
};

extern struct ThreatArrowsData gThreatArrows;

/* Called each frame of a move selection, from DrawUpdatedPathArrow (Kernel/Wizardry/RemoveMovePath) */
void ThreatArrows_OnMoveSelect(void);
