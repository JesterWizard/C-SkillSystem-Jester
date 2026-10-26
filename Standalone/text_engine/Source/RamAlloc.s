.macro dat value, name
    .global \name
    .type \name, object
    .set \name, \value
.endm

/* EWRAM state for the text engine, 0x1A0 bytes in all.
 *
 * Default placement: FreeRamSpace2 on clean FE8U, after the slices used by
 * alpha_blend_movement_sprites (0x0203AAA4..0x0203AAD5) and
 * text_box_extension_layout (0x0203AAD8..0x0203AAEF).
 * Occupies 0x0203AB00 .. 0x0203AC9F.
 *
 * Common clean-FE8U EWRAM pools (pick an unused slice inside one):
 *   FreeRamSpace2: 0x0203AAA4 .. 0x0203DDE0  (~13 KB, grows upward from base)
 *   FreeRamSpace:  0x02026E30 .. 0x02028E58  (vanilla debug-print buffer)
 *   FreeRamSpace3: 0x02026AD0 .. 0x02026E30  (~864 B, grows downward from top)
 *
 * Change the base below and rebuild if another hack already uses this range.
 * Keep each symbol 4-byte aligned.
 */
.set TextEngineRamBase, 0x0203AB00

/* Dialogue wave / screen tear / static: per-scanline BG offsets (0xF0) */
dat TextEngineRamBase + 0x000, sTextEngineWaveOffsets
/* Speaker nameplate text and position (0xA0) */
dat TextEngineRamBase + 0x0F0, sTextEngineNameplateState
/* Screen earthquake offset (2, padded to 4) */
dat TextEngineRamBase + 0x190, sTextEngineEarthquakeOffset
/* Scene layout flags (4) */
dat TextEngineRamBase + 0x194, sTextEngineSceneLayoutState
/* Big text flags (4) */
dat TextEngineRamBase + 0x198, sTextEngineBigTextState
/* Auto-advance frames (4) */
dat TextEngineRamBase + 0x19C, sTextEngineAutoAdvanceState
