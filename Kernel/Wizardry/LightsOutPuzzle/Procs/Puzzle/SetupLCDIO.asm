@ Prepares the backgrounds.
.thumb

push  {r4-r7, r14}


@ Move the ScreenBlocks to a more favourable position
ldr   r6, =LocScreenBlockBG0
ldr   r5, =gpDISPCNTbuffer
add   r5, #0xD                          @ BG0CNT
mov   r7, #0x1C                         @ ScreenBlock, also loop iterator because registerscarcity
mov   r0, #0x6
lsl   r0, #0x18
mov   r1, #0x8
lsl   r1, #0x8
mov   r2, #0xE0
lsl   r2, #0x8
mov   r4, #0xE0

Loop:
  add   r3, r0, r2
  str   r3, [r6]                          @ Update LocScreenBlockBGX
  ldrb  r3, [r5]
  and   r3, r4
  add   r3, r7
  strb  r3, [r5]                          @ Update Screen Base Block in BGXCNT
  add   r5, #0x4
  add   r7, #0x1
  add   r6, #0x4
  add   r2, r1
  cmp   r7, #0x1F
  ble   Loop


@ Display BG0, BG1, BG2, BG3, OBJ
ldr   r2, =gpDISPCNTbuffer                  @ DISPCNT
ldrh  r1, [r2]
mov   r0, #0x1F
lsl   r0, #0x8
mvn   r0, r0
and   r1, r0                                @ Clear Displaybits
mov   r0, #0x1F                             @ Enable BG0, BG1, BG2, BG3 & OBJ
lsl   r0, #0x8
orr   r1, r0
strh  r1, [r2]


@ Clear BG0, BG1, BG2, BG3
ldr   r0, =gpBG0MapBuffer
mov   r1, #0x0
ldr   r5, =BG_Fill
bl    GOTO_R5

ldr   r0, =gpBG1MapBuffer
mov   r1, #0x0
ldr   r5, =BG_Fill
bl    GOTO_R5

ldr   r0, =gpBG2MapBuffer
mov   r1, #0x0
ldr   r5, =BG_Fill
bl    GOTO_R5

ldr   r0, =gpBG3MapBuffer
mov   r1, #0x0
ldr   r5, =BG_Fill
bl    GOTO_R5

mov   r0, #0xF
ldr   r5, =EnableBackgroundSyncByMask
bl    GOTO_R5

@ Clear colour effects
ldr   r5, =SetDefaultColourEffects
bl    GOTO_R5


@ Set BG0, BG1, BG3 to use tiles1, BG2 to use tiles2
mov   r1, #0x0
mov   r0, #0x0
ldr   r5, =SetBackgroundTileDataOffset
bl    GOTO_R5
mov   r1, #0x0
mov   r0, #0x1
ldr   r5, =SetBackgroundTileDataOffset
bl    GOTO_R5
mov   r1, #0x80
lsl   r1, r1, #0x8                          @ Use Tiles2
mov   r0, #0x2
ldr   r5, =SetBackgroundTileDataOffset
bl    GOTO_R5
mov   r1, #0x0
mov   r0, #0x3
ldr   r5, =SetBackgroundTileDataOffset
bl    GOTO_R5


@ Reset VOFS & HOFS for BG0, BG1, BG2, BG3
ldr   r2, =gpDISPCNTbuffer
mov   r0, #0x0
mov   r1, #0x1C
mov   r3, #0x2A

OFSLoop:
  strb  r0, [r2, r1]
  add   r1, #0x2
  cmp   r1, r3
  ble   OFSLoop


@ Set BG0, BG1, BG2, BG3 priorities to 0, 1, 3, 2
ldr   r2, =gpDISPCNTbuffer
mov   r0, #0xFC

@ Set BG0 priority to 1
ldrb  r3, [r2, #0xC]                        @ BG0CNT
and   r3, r0
mov   r1, #0x0
orr   r3, r1                                @ priority 0
strb  r3, [r2, #0xC]

@ Set BG1 priority to 1
ldrb  r3, [r2, #0x10]                       @ BG1CNT
and   r3, r0
mov   r1, #0x1
orr   r3, r1                                @ priority 1
strb  r3, [r2, #0x10]

@ Set BG2 priority to 0
ldrb  r3, [r2, #0x14]                       @ BG2CNT
and   r3, r0
mov   r1, #0x3
orr   r3, r1                                @ priority 3
strb  r3, [r2, #0x14]

@ Set BG3 priority to 2
ldrb  r3, [r2, #0x18]                       @ BG3CNT
and   r3, r0
mov   r1, #0x2
orr   r3, r1                                @ priority 2
strb  r3, [r2, #0x18]


pop   {r4-r7}
pop   {r0}
bx    r0
GOTO_R5:
bx    r5
