@ Flip marked tiles, arguments:
@   r0 = Puzzle ProcState pointer
.thumb

push  {r4, r14}
mov   r4, r0


mov   r2, #0x2E
ldrb  r2, [r4, r2]
mov   r0, #0x2C
ldrb  r0, [r4, r0]
mov   r1, #0x2D
ldrb  r1, [r4, r1]
mul   r0, r1
mov   r1, #0x0
mov   r3, #0x30
add   r4, r3

Loop:
  ldrb  r3, [r4, #0x1]
  cmp   r3, #0x0
  beq   L1
  
    @ Tile is marked, flip tile.
    ldrb  r3, [r4]
    add   r3, #0x1
    cmp   r3, r2
    blt   L2
      mov   r3, #0x0
    L2:
    strb  r3, [r4]
    
  L1:
  add   r4, #0x2
  add   r1, #0x1
  cmp   r1, r0
  blt   Loop


pop   {r4}
pop   {r0}
bx    r0
