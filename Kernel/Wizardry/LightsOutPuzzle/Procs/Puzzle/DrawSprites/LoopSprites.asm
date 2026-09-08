@ Draw cursor and icons
.thumb

push  {r4-r7, r14}
mov   r4, r8
mov   r5, r9
mov   r6, r10
push  {r4-r6}
mov   r1, #0x14
ldr   r0, [r0, r1]
mov   r5, r0                                  @ Parent proc state, Puzzle


@ Set Affine matrix (0) if anim timer > 0
@ Also flip tiles once timer is halfway
mov   r0, #0x2B
ldrb  r4, [r5, r0]
cmp   r4, #0x0
ble   L1
  
  ldr   r1, =TileAnimSpeed
  ldrb  r1, [r1]
  lsr   r1, #0x1
  sub   r1, #0x1
  cmp   r4, r1
  bne   L3
  
    @ Timer is halfway done, flip tiles.
    mov   r0, r5
    bl    FlipTiles
  L3:
  
  @ Set Affine matrix
  ldr   r1, =TileAnimAffineSrc
  sub   r0, r4, #0x1
  lsl   r0, #0x3
  add   r0, r1
  ldr   r1, =whole_OAM_buffer
  add   r1, #0x6
  mov   r2, #0x1C
  mov   r3, #0x8
  swi   #0xF                        @ ObjAffineSet
L1:

@ Determine which ROM OAM to use
ldr   r4, =GetGameClock
bl    GOTO_R4
mov   r1, #0x20
swi   #0x6                          @ returns r0 % r1 in r1
cmp   r1, #0x8
bge   L5
  mov   r7, #0x0
  b     L8
    L5:
    cmp   r1, #0xA
    bge   L6
      mov   r7, #0x1
      b     L8
        L6:
        cmp   r1, #0x1E
        bge   L7
          mov   r7, #0x2
          b     L8
            L7:
            mov   r7, #0x1
L8:
mov   r6, #0x2F
ldrb  r6, [r5, r6]


@ Cursor & action prompts, don't draw if +0x29 bit 1 is set.
ldr   r4, =PushToSecondaryOAM
mov   r0, #0x29
ldrb  r1, [r5, r0]
mov   r2, #0x2
and   r1, r2
cmp   r1, #0x0
bne   L4

  @ Draw Cursor
  mov   r2, #0x2A
  ldrb  r2, [r5, r2]
  lsr   r3, r2, #0x4
  lsl   r2, #0x1C
  lsr   r2, #0x1C
  lsl   r0, r2, #0x5
  lsl   r2, #0x3
  add   r0, r2                        @ X, multiplied by 0x28 (tilesize)
  lsl   r1, r3, #0x5
  lsl   r3, #0x3
  add   r1, r3                        @ Y, multiplied by 0x28 (tilesize)
  ldr   r2, =PuzzleCursorOAM
  lsl   r3, r6, #0x7
  add   r2, r3
  lsl   r3, r7, #0x5
  add   r2, r3
  mov   r3, #0x0
  bl    GOTO_R4
  
  @ Draw action prompts, unless board size too big
  mov   r0, #0x2F
  ldrb  r0, [r5, r0]
  cmp   r0, #0x3
  bge   L4
    
    @ Draw "R: Info"
    mov   r0, #0x0
    mov   r1, #0x90
    ldr   r2, =RInfoOAM
    mov   r3, #0x20
    lsl   r3, #0x8
    bl    GOTO_R4
    
    @ Draw "B: Exit", unless puzzle can't be skipped.
    mov   r0, #0x29
    ldrb  r0, [r5, r0]
    mov   r1, #0x8
    tst   r0, r1
    bne   L4
    
      mov   r0, #0x0
      mov   r1, #0x80
      ldr   r2, =OAM32x16
      mov   r3, #0x21
      lsl   r3, #0x8
      add   r3, #0xB
      bl    GOTO_R4
      
L4:

@ Big Weapon Icons, don't draw if +0x29 bit 2 is set.
mov   r0, #0x29
ldrb  r1, [r0, r5]
mov   r2, #0x4
and   r1, r2
cmp   r1, #0x0
bne   Return

  mov   r0, #0x2C
  ldrb  r0, [r5, r0]
  mov   r1, #0x2D
  ldrb  r1, [r5, r1]

  lsl   r2, r0, #0x5
  lsl   r3, r0, #0x3
  add   r2, r3
  mov   r8, r2

  lsl   r2, r1, #0x5
  lsl   r3, r1, #0x3
  add   r2, r3
  mov   r9, r2

  ldr   r0, =BigWeaponIconsOAM
  lsl   r1, r6, #0x3
  add   r0, r1
  mov   r10, r0
  mov   r0, #0x30
  add   r5, r0

  mov   r6, #0x0
  mov   r7, #0x0

  Loop:
    mov   r0, r7
    mov   r1, r6
    ldrb  r2, [r5, #0x1]
    lsl   r2, #0x8
    add   r1, r2                    @ Affine flag
    mov   r2, r10
    ldrb  r3, [r5]
    lsl   r3, #0x2
    bl    GOTO_R4
    
    add   r5, #0x2
    add   r7, #0x28
    cmp   r7, r9
    blt   L2
      mov   r7, #0x0
      add   r6, #0x28
    L2:
    cmp   r6, r8
    blt   Loop


Return:
pop   {r4-r6}
mov   r8, r4
mov   r9, r5
mov   r10, r6
pop   {r4-r7}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
