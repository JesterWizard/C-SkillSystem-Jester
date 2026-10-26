.macro dat value, name
    .global \name
    .type \name, object
    .set \name, \value
.endm

/* EWRAM cache of the selected language (4 bytes).
 *
 * gLanguageSelectCache: 0x4C53 in the high half marks it valid, low byte is
 * the language (0..5). It saves re-reading the SRAM global save header on
 * every text lookup when LANGUAGE_SELECT_TEXT_SWITCH is on.
 *
 * Occupies 0x0203BC00 .. 0x0203BC03, inside FreeRamSpace2
 * (0x0203AAA4 .. 0x0203DDE0 on clean FE8U).
 *
 * Change the address below and run make if another hack already uses it.
 */
dat 0x0203BC00, gLanguageSelectCache
