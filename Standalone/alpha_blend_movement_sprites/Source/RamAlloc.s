.macro dat value, name
    .global \name
    .type \name, object
    .set \name, \value
.endm

/* EWRAM scratch for pathfinding ghost OBJ data passed to PutSpriteExt.
 *
 * Size: 50 bytes = (1 + 8 * 3) * sizeof(u16)
 *   - word 0: sprite count (see DisplayBlendedMuAp, count <= 8)
 *   - words 1..24: up to 8 OAM entries, 3 halfwords each (ATTR0/1/2)
 *
 * Default placement: start of FreeRamSpace2 on clean FE8U.
 * Occupies 0x0203AAA4 .. 0x0203AAD5 (50 bytes inclusive).
 *
 * Common clean-FE8U EWRAM pools (pick an unused slice inside one):
 *   FreeRamSpace2: 0x0203AAA4 .. 0x0203DDE0  (~13 KB, grows upward from base)
 *   FreeRamSpace:  0x02026E30 .. 0x02028E58  (vanilla debug-print buffer)
 *   FreeRamSpace3: 0x02026AD0 .. 0x02026E30  (~864 B, grows downward from top)
 */

 /*
 * Change the address below and rebuild if another hack already uses this slot.
 * Keep the new base + 50 bytes inside a free region and clear of overlaps. 
 */
dat 0x0203AAA4, sPathfindingGhostObjBuf
