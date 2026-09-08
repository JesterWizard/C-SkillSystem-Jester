@ Puzzle idle routine
.thumb

push  {r4-r7, r14}
mov   r5, r0


@ Dimensions
mov   r6, #0x2D
ldrb  r6, [r5, r6]
mov   r7, #0x2C
ldrb  r7, [r5, r7]


@ Ignore input if +0x29 bit 0 is set and anim timer is not yet 0
@ Also end puzzle if it's been solved.
mov   r0, #0x29
ldrb  r2, [r5, r0]
mov   r1, #0x1
and   r1, r2
cmp   r1, #0x0
beq   L5

  @ Decrease and check if anim timer is 0
  mov   r3, #0x2B
  ldrb  r1, [r5, r3]
  sub   r1, #0x1
  strb  r1, [r5, r3]
  cmp   r1, #0x0
  bgt   L6
  
    @ Unset ignore input flag
    mov   r1, #0x1
    mvn   r1, r1
    and   r2, r1
    strb  r2, [r5, r0]
    
    @ Clear Affine flags for tiles
    mov   r0, #0x31
    mov   r1, #0x0
    mov   r2, #0x0
    mov   r3, r6
    mul   r3, r7
    Loop:
      strb  r2, [r5, r0]
      add   r0, #0x2
      add   r1, #0x1
      cmp   r1, r3
      blt   Loop

    @ Check if all tiles match
    mov   r0, r5
    bl    CheckIfSolved
    cmp   r0, #0x0
    beq   L5
    
      @ Puzzle has been solved, Don't draw cursor.
      mov   r0, #0x29
      ldrb  r1, [r0, r5]
      mov   r2, #0x2
      orr   r1, r2
      strb  r1, [r0, r5]
      
      @ Set Leg weapon obtained flag
      mov   r0, #0x30
      ldrb  r0, [r5, r0]
      mov   r1, #0x58
      add   r0, r1
      ldrb  r0, [r5, r0]
      ldr   r1, =PuzzleWeaponFlagsRAM
      ldr   r1, [r1]
      ldrb  r2, [r1]
      mov   r3, #0x1
      lsl   r3, r0
      orr   r2, r3
      strb  r2, [r1]
      
      @ Determine reward and store in slot 12
      mov   r1, #0x60
      ldrb  r1, [r5, r1]                  @ Reward tier
      lsl   r1, #0x3
      add   r0, r1
      mov   r1, #0xBC                     @ Hard-coded, this is where Leg weapons should be.
      add   r0, r1
      ldr   r1, =SlotC
      str   r0, [r1]
      
      @ Break loop
      mov   r0, r5
      ldr   r4, =Break6CLoop
      bl    GOTO_R4

@ Label Return was out of range.
L6:
b     Return
L5:


@ Check input
ldr   r0, =KeyStatusBuffer
ldrh  r4, [r0, #0x6]
mov   r2, #0x0
mov   r3, #0x0

@ CheckRight
mov   r0, #0x10
tst   r0, r4
beq   CheckLeft
add   r2, #0x1
b     MoveCursor

  CheckLeft:
  mov   r0, #0x20
  tst   r0, r4
  beq   CheckUp
  sub   r2, #0x1
  b     MoveCursor
    
    CheckUp:
    mov   r0, #0x40
    tst   r0, r4
    beq   CheckDown
    sub   r3, #0x1
    b     MoveCursor
    
      CheckDown:
      mov   r0, #0x80
      tst   r0, r4
      beq   CheckA
      add   r3, #0x1
      b     MoveCursor
      
        CheckA:
        ldr   r0, =KeyStatusBuffer
        ldrh  r4, [r0, #0x8]
        mov   r0, #0x1
        tst   r0, r4
        beq   CheckR
          mov   r0, r5
          bl    MarkTiles
          mov   r0, #0x5E                 @ YUGIOH Select sfx
          bl    PlaySFX
          b     Return
          
            CheckR:
            mov   r0, #0x80
            lsl   r0, #0x1
            tst   r0, r4
            beq   CheckB
              ldr   r0, =Proc_Tutorial
              mov   r1, r5
              ldr   r4, =NewBlocking6C
              bl    GOTO_R4
              b     Return
              
                CheckB:
                mov   r0, #0x2
                tst   r0, r4
                beq   Return
                  
                  @ Can't quit if +0x29 bit 3 is set
                  mov   r0, #0x29
                  ldrb  r0, [r5, r0]
                  mov   r1, #0x8
                  tst   r0, r1
                  beq   L7
                    mov   r0, #0x5F
                    bl    PlaySFX
                    b     Return
                  
                      L7:
                      ldr   r0, =Proc_Quit
                      mov   r1, r5
                      ldr   r4, =NewBlocking6C
                      bl    GOTO_R4
                      b     Return


MoveCursor:
mov   r0, #0x2A
ldrb  r0, [r5, r0]
lsr   r1, r0, #0x4                  @ Y
lsl   r0, #0x1C
lsr   r0, #0x1C                     @ X

add   r0, r2
add   r1, r3

cmp   r0, r6
blt   L1
  mov   r0, #0x0
  b     L4
    L1:
    cmp   r0, #0x0
    bge   L2
      sub   r0, r6, #0x1
      b     L4
        L2:
        cmp   r1, r7
        blt   L3
          mov   r1, #0x0
          b     L4
            L3:
            cmp   r1, #0x0
            bge   L4
              sub   r1, r7, #0x1
L4:
lsl   r1, #0x4
add   r0, r1
mov   r1, #0x2A
strb  r0, [r5, r1]

mov   r0, #0x58                 @ We play the YUGIOH cursor move sound.
bl    PlaySFX


Return:
pop   {r4-r7}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
