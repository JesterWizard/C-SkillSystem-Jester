.macro dat value, name
    .global \name
    .type \name, object
    .set \name, \value
.endm

/* EWRAM for the Hex screen (Source/HexScreen.c).
 *
 * 0x200 bytes, 0x0203C100 .. 0x0203C2FF inclusive: gHexScreenState (struct HexScreenState, checked to fit at compile time).
 * The parent patch (../../Source/RamAlloc.s) uses 0x0203C000 .. 0x0203C013. Change the base address and rebuild if another
 * hack already uses this range.
 */

dat 0x0203C100, gHexScreenState
