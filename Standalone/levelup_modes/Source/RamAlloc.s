.macro dat value, name
    .global \name
    .type \name, object
    .set \name, \value
.endm

/* EWRAM state for level-up mode 1 (separate level-up RNG stream).
 *
 * Size: 8 bytes = 4 * sizeof(u16)
 *   - words 0..2: RNG seeds
 *   - word 3:     check value; when it does not match, the seeds are
 *                 re-drawn from the vanilla RNG on the next level-up
 *
 * Occupies 0x0203AE20 .. 0x0203AE27 (8 bytes inclusive), inside clean
 * FE8U FreeRamSpace2 (0x0203AAA4 .. 0x0203DDE0).
 *
 * Only modes set to 1 read or write it. Change the address below and
 * rebuild if another hack already uses this slot.
 */
dat 0x0203AE20, sLvupModesRandSeeds
