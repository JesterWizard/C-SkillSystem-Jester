.thumb
.include "macros.inc"
.include "gba.inc"

THUMB_FUNC_START MU_CALL2_FixForFreeMU_fix
MU_CALL2_FixForFreeMU_fix:
    @ORG 0x8078D10
    blh ProcFindFMU
    cmp r0, #0
    beq .ReturnCall2Normal

    ldr r0, =0x8078D23
    bx r0

    .ReturnCall2Normal:
    mov r0, r6
    add r0, #0x3E
    ldrb r0, [r0]
    cmp r0, #0
    beq .ReturnCall2SkipCamera
    ldr r0, =ProcScr_CamMove
    blh Proc_Find
    cmp r0, #0
    bne .ReturnCall2SkipCamera

    @ fix here
    ldr r0, =0x8078D23
    bx r0

    .ReturnCall2SkipCamera:
    ldr r1, =0x8078D3D
    bx r1
