@ Puts system icons, cursor and big weapon icons in VRAM
.thumb

push  {r4-r7, r14}
mov   r4, r8
mov   r5, r9
push  {r4-r5}
ldr   r5, [r0, #0x14]


@ Load System Icons, Puzzle cursor, Buttons A & B,
@ Movement arrows, Big weapon icons, Action Prompts.
ldr   r4, =LoadObjUIGfx
bl    GOTO_R4
ldr   r0, =PuzzleCursor
ldr   r1, =gGenericBuffer
ldr   r4, =UnLZ77Decompress
bl    GOTO_R4
ldr   r0, =gGenericBuffer
ldr   r1, =0x6010000
mov   r2, #0x2
mov   r3, #0x2
ldr   r4, =CopyTileGfxForObj
bl    GOTO_R4
ldr   r0, =ButtonPromptsGfx
ldr   r1, =gGenericBuffer
ldr   r4, =UnLZ77Decompress
bl    GOTO_R4
ldr   r0, =gGenericBuffer
ldr   r1, =0x60102C0
mov   r2, #0x2
mov   r3, #0x2
ldr   r4, =CopyTileGfxForObj
bl    GOTO_R4
ldr   r0, =gGenericBuffer+0x80
ldr   r1, =0x6010300
mov   r2, #0x2
mov   r3, #0x2
bl    GOTO_R4
ldr   r0, =MovementArrowsGfx
ldr   r1, =0x6010800
ldr   r4, =UnLZ77Decompress
bl    GOTO_R4
ldr   r0, =BigWeaponIcons
ldr   r1, =gGenericBuffer
ldr   r4, =UnLZ77Decompress
bl    GOTO_R4
ldr   r0, =ActionPromptsGfx
ldr   r1, =0x6012000
bl    GOTO_R4

ldr   r0, =gGenericBuffer
mov   r8, r0
ldr   r0, =0x6011000
mov   r9, r0
mov   r0, #0x2E
ldrb  r7, [r5, r0]
ldr   r4, =CopyTileGfxForObj
add   r5, #0x58
mov   r6, #0x0
Loop:
  ldrb  r0, [r5]
  lsl   r0, #0x9
  add   r0, r8
  lsl   r1, r6, #0x7
  add   r1, r9
  mov   r2, #0x4
  mov   r3, #0x4
  bl    GOTO_R4
  add   r5, #0x1
  add   r6, #0x1
  cmp   r6, r7
  blt   Loop


@ Load Palettes
ldr   r0, =PuzzleCursorPal
mov   r1, #0x20
lsl   r1, #0x4                            @ Overwrite OBJ palette 0
mov   r2, #0x20                           @ Load only one palette (1 short per colour)
ldr   r4, =CopyToPaletteBuffer
bl    GOTO_R4
ldr   r0, =ItemPalette
mov   r1, #0x22
lsl   r1, #0x4                            @ Overwrite OBJ palette 1
mov   r2, #0x20                           @ Load only one palette (1 short per colour)
bl    GOTO_R4
ldr   r0, =ButtonPromptsPalette
mov   r1, #0x24
lsl   r1, #0x4                            @ Overwrite OBJ palette 2
mov   r2, #0x20                           @ Load only one palette (1 short per colour)
bl    GOTO_R4


pop   {r4-r5}
mov   r8, r4
mov   r9, r5
pop   {r4-r7}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
