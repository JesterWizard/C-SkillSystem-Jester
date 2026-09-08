@ Some BLD and WINDOW stuff
.thumb

push  {r4-r5, r14}
sub   sp, #0x8
ldr   r5, [r0, #0x14]


@ Don't draw cursor, weapon icons, button prompts
mov   r1, #0x29
ldrb  r2, [r5, r1]
mov   r3, #0x6
orr   r2, r3
strb  r2, [r5, r1]


@ Darken BG2, BG3 & OBJ
ldr   r0, =gpDISPCNTbuffer
add   r0, #0x3C
mov   r1, #0xCC
strh  r1, [r0]

mov   r1, #0x7
strh  r1, [r0, #0xA]            @ BLDY


@ Draw text
ldr   r4, =Font_InitForUIDefault
bl    GOTO_R4

ldr   r0, =PuzzleQuit
lsl   r0, #0x10
lsr   r0, #0x10
mov   r1, #0x0
mov   r2, #0xE5
bl    DrawTextSimple

mov   r0, #0x84
lsl   r0, #0x4
add   r0, #0x3
mov   r1, #0x0
mov   r2, #0x12
lsl   r2, #0x4
add   r2, #0xF
bl    DrawTextSimple

mov   r0, #0x84
lsl   r0, #0x4
add   r0, #0x4
mov   r1, #0x0
mov   r2, #0x16
lsl   r2, #0x4
add   r2, #0xF
bl    DrawTextSimple


mov   r0, #0x59                 @ YUGIOH place card sfx
bl    PlaySFX


mov   r0, #0x0
ldr   r4, =EnableBackgroundSyncById 
bl    GOTO_R4


add   sp, #0x8
pop   {r4-r5}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
