@ Start a Lights Out Puzzle
.thumb

push  {r4, r14}

mov   r1, r0
ldr   r0, =Proc_Puzzle
ldr   r4, =NewBlocking6C
bl    GOTO_R4


@ Set Dimensions
ldr   r1, =Slot1
ldr   r1, [r1]
mov   r2, #0x2F
strb  r1, [r0, r2]

cmp   r1, #0x0
bne   L1
  mov   r2, #0x3
  mov   r3, #0x3
  b     L4
    L1:
    cmp   r1, #0x1
    bne   L2
      mov   r2, #0x3
      mov   r3, #0x4
      b     L4
        L2:
        cmp   r1, #0x2
        bne   L3
          mov   r2, #0x4
          mov   r3, #0x4
          b     L4
            L3:
            mov   r2, #0x4
            mov   r3, #0x5
L4:

mov   r1, #0x2C
strb  r2, [r0, r1]
mov   r1, #0x2D
strb  r3, [r0, r1]


@ Set Icon count
ldr   r1, =Slot2
ldr   r1, [r1]
mov   r2, #0x2E
strb  r1, [r0, r2]


@ Set whether puzzle can be skipped or not
ldr   r1, =Slot3
ldr   r1, [r1]
cmp   r1, #0x0
beq   L5
  mov   r1, #0x8
L5:
mov   r2, #0x29
strb  r1, [r0, r2]


@ Set reward Tier
ldr   r1, =Slot4
ldr   r1, [r1]
mov   r2, #0x60
strb  r1, [r0, r2]


pop   {r4}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
