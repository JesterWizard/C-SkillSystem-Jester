.macro dat value, name
    .global \name
    .type \name, object
    .set \name, \value
.endm

/* EWRAM for the promotion screen's growth preview text.
 *
 * Size: 128 bytes. The talk system keeps a pointer to the string and prints
 * it over the following frames, so it cannot live on the stack. The longest
 * text is 7 entries of at most 17 bytes, a [NL] and the end byte (121 bytes).
 *
 * Default placement: 0x0203AF00 .. 0x0203AF7F, inside FreeRamSpace2
 * (0x0203AAA4 .. 0x0203DDE0) on clean FE8U.
 *
 * Change the address below and rebuild (make) if another hack already uses
 * this range. Keep base + 128 bytes inside a free region.
 */
dat 0x0203AF00, ClassGrowthPreviewText
