.thumb
.syntax unified

@ Hooked with jumpToHack at StatScreen_InitDisplay+0xEC (0x0808859C), where
@ vanilla does:
@     ldr  r0, =Pal_StatscreenBG
@     movs r1, #0x180
@     movs r2, #0x80
@     bl   CopyToPaletteBuffer      @ at 0x080885A4
@ We load r0 from GetStatScreenBgPal_Allegiance() instead and resume at the
@ vanilla bl, so CopyToPaletteBuffer(pal, 0x180, 0x80) still runs from there.
@ r3 is free (jumpToHack clobbers it; vanilla does not use it here).

.global StatScreen_LoadBgPal_Allegiance
.type StatScreen_LoadBgPal_Allegiance, %function
StatScreen_LoadBgPal_Allegiance:
    ldr  r3, =GetStatScreenBgPal_Allegiance
    bl   .Lcall_r3
    movs r1, #0xC0
    lsls r1, r1, #1
    movs r2, #0x80
    ldr  r3, =0x080885A5
    bx   r3

.Lcall_r3:
    bx   r3

.align 2
.ltorg
