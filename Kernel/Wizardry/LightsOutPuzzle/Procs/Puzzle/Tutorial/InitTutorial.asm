@ Set some stuff for tutorial sheet
.thumb

push  {r4-r7, r14}
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

ldr   r7, =PuzzleTut0
lsl   r7, #0x10
lsr   r7, #0x10

@ Setup textlines
mov   r6, sp
mov   r0, #0x0
strb  r0, [r6]
mov   r0, #0x1
strb  r0, [r6, #0x1]
mov   r0, #0x2
strb  r0, [r6, #0x2]
mov   r0, #0x3
strb  r0, [r6, #0x3]
mov   r0, #0x7
strb  r0, [r6, #0x4]
mov   r0, #0x8
strb  r0, [r6, #0x5]
mov   r0, #0x9
strb  r0, [r6, #0x6]
mov   r0, #0x9
strb  r0, [r6, #0x7]

@ Draw Cannot be skipped if puzzle cannot be skipped
mov   r1, #0x29
ldrb  r0, [r5, r1]
mov   r2, #0x8
and   r2, r0
lsr   r2, #0x3
mov   r4, #0x7
add   r4, r2
mov   r5, #0x0

Loop:
  mov   r0, r7
  mov   r1, #0x0
  ldrb  r2, [r6, r5]
  lsl   r2, #0x6
  bl    DrawTextSimple

  add   r7, #0x1
  add   r5, #0x1
  cmp   r5, r4
  blt   Loop


mov   r0, #0x59                 @ YUGIOH place card sfx
bl    PlaySFX


mov   r0, #0x0
ldr   r4, =EnableBackgroundSyncById 
bl    GOTO_R4


add   sp, #0x8
pop   {r4-r7}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
