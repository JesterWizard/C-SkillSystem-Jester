.macro dat value, name
    .global \name
    .type \name, object
    .set \name, \value
.endm

/* EWRAM for the Unlimited Prep Menu Options patch.
 *
 * Total: 20 bytes, 0x0203C000 .. 0x0203C013 inclusive, in FreeRamSpace2 (clean FE8U).
 * Even-sized slots:
 *   0x0203C000 .. 0x0203C00F  gPrepMenuVisibleTableSlots[16]
 *                              one byte per visible row: row key | 0x80 if grey
 *   0x0203C010                gPrepMenuTopVisibleIndex (u8, padded to 2 bytes)
 *   0x0203C012                gPrepMenuBuildingList   (u8, padded to 2 bytes)
 *
 * Change the base address and rebuild if another hack already uses this range.
 * The range must stay free of other FreeRamSpace2 users (0x0203AAA4..0x0203B5FF
 * is taken by alpha_blend_movement_sprites and other standalones).
 */

dat 0x0203C000, gPrepMenuVisibleTableSlots
dat 0x0203C010, gPrepMenuTopVisibleIndex
dat 0x0203C012, gPrepMenuBuildingList
