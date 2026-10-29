.macro dat value, name
    .global \name
    .type \name, object
    .set \name, \value
.endm

/* EWRAM for the half-body / 32-colour portrait bookkeeping: one
 * struct PortraitSlot (0x28 bytes) per face slot, 0xA0 bytes in all.
 *
 * Default placement: FreeRamSpace2 on clean FE8U (0x0203AAA4 .. 0x0203DDE0),
 * clear of Standalone/text_engine (0x0203AB00 .. 0x0203AC9F) and its
 * conversation log (0x0203B000 .. 0x0203B5FF).
 * Occupies 0x0203B800 .. 0x0203B89F.
 *
 * Change the address below and run make if another hack already uses it.
 * Keep it 4-byte aligned.  Standalone/text_engine doesn't use this file:
 * its copy of the portraits takes the address from its Installer.event.
 */
dat 0x0203B800, sPortraitSlots
