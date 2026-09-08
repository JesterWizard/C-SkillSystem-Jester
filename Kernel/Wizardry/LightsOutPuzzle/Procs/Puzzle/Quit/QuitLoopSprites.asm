@ Draw button prompt sprites
.thumb

push  {r4-r5, r14}
ldr   r5, [r0, #0x14]


@ A
mov   r0, #0x68
mov   r1, #0x48
ldr   r2, =OAM16x16
mov   r3, #0x20
lsl   r3, #0x8
add   r3, #0x16
ldr   r4, =PushToSecondaryOAM
bl    GOTO_R4


@ B
mov   r0, #0x68
mov   r1, #0x58
ldr   r2, =OAM16x16
mov   r3, #0x20
lsl   r3, #0x8
add   r3, #0x18
ldr   r4, =PushToSecondaryOAM
bl    GOTO_R4


Return:
pop   {r4-r5}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
