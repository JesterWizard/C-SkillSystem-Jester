.macro dat value, name
    .global \name
    .type \name, object
    .set \name, \value
.endm

/* Page state (8) plus two extra help-box text slots (16).
 * Vanilla gHelpBoxSt only has three text slots.
 * 0x0203AAD8 .. 0x0203AAEF, after alpha_blend_movement_sprites' ghost buffer.
 */
dat 0x0203AAD8, sHelpBoxPageState
dat 0x0203AAE0, sHelpBoxExtraText
