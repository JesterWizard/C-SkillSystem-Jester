@ Load in puzzle background
.thumb

push  {r4-r5, r14}
mov   r5, r0


@ Determine which PuzzleBG to load
mov   r0, #0x2F
ldrb  r0, [r5, r0]
lsl   r0, #0x2
add   pc, r0
lsl   r0, #0x0                  @ nop. We need to pad.

ldr   r0, =PuzzleBG3x3
b     L1
ldr   r0, =PuzzleBG3x4
b     L1
ldr   r0, =PuzzleBG4x4
b     L1
ldr   r0, =PuzzleBG4x5
L1:

@ Load PuzzleBG image into char block 2 (#0x6008000)
mov   r1, #0x6
lsl   r1, #0x18
mov   r2, #0x8
lsl   r2, #0xC
add   r1, r2
ldr   r4, =UnLZ77Decompress
bl    GOTO_R4

@ Load PuzzleBG palette
ldr   r0, =PuzzleBGPal
mov   r1, #0x20                           @ Overwrite BG palette 1
mov   r2, #0x20                           @ Load only one palette (1 short per colour)
ldr   r4, =CopyToPaletteBuffer
bl    GOTO_R4

@ Set up Screenblock (TSA)
ldr   r1, =gpBG2MapBuffer
mov   r2, #0x1
lsl   r2, #0xC                            @ Use BG palette 0x1
mov   r3, #0x0
mov   r4, #0x28
lsl   r4, #0x4                            @ BGTileSize

Loop:
  add   r0, r3, r2
  strh  r0, [r1]
  add   r1, #0x2
  add   r3, #0x1
  
  cmp   r3, r4
  ble   Loop
  
mov   r0, #0x1
ldr   r4, =EnableBackgroundSyncById 
bl    GOTO_R4


pop   {r4-r5}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
