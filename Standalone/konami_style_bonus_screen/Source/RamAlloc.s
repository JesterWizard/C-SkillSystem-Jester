.macro dat value, name
    .global \name
    .type \name, object
    .set \name, \value
.endm

/* EWRAM for the title screen Konami code progress.
 *
 * Size: 2 bytes (1 used: gKonamiComboStep, 1 pad to keep the slot even-sized)
 * Default placement: 0x0203ADC0 .. 0x0203ADC1, inside FreeRamSpace2
 * (0x0203AAA4 .. 0x0203DDE0) on clean FE8U.
 *
 * Change the address below and rebuild if another hack already uses it.
 */
dat 0x0203ADC0, gKonamiComboStep
