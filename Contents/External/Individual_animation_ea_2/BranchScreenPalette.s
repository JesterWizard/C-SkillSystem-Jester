.thumb
@ Branching from 0x080CD104 (LoadBattleSpritesForBranchScreen, the promotion screen's class previews), right after
@ GetBattleAnimationId: r0 is the animation index (the ID minus 1).
@
@ Vanilla then looks the class up in the character's palettes (gAnimCharaPalConfig) and uses the character palette if
@ there is one. A custom animation (CustomAnimeTable, Contents/BattleAnims; index 0xC9 and up) has its own palettes, as
@ in battle (gBanimUniquePal -1): the character palette made for the vanilla class sprite drew it in the wrong colours
@ (Joshua's Swordmaster). For those, skip the lookup; the palette stays -1.
@
@ Replaces 0x080CD104-0x080CD10B: mov r8, r0; movs r1, #0; ldr r5, =gAnimCharaPalConfig; lsls r4, r4, #16.
.align 2
    mov  r8, r0
    cmp  r0, #0xC9
    bhs  custom             @ unsigned: an invalid index (0xFFFF) also skips; its palette is -1 either way

    movs r1, #0
    ldr  r5, =0x080CD1A0    @ the function's own literal for gAnimCharaPalConfig
    ldr  r5, [r5]
    lsl  r4, r4, #16
    ldr  r3, =0x080CD10D    @ asrs r4, r4, #16 and the lookup loop
    bx   r3

custom:
    ldr  r3, =0x080CD131    @ after the loop: the palette in [sp, #0x58] is still -1
    bx   r3

.ltorg
