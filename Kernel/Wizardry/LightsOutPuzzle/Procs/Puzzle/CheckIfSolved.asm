@ Check if Puzzle has been solved. Arguments:
@   r0: Puzzle proc state pointer
@ Returns:
@   0x0 if puzzle has not been solved. 0x1, otherwise.
.thumb

push  {r4-r6, r14}
mov   r5, r0


mov   r6, #0x2D
ldrb  r6, [r5, r6]
mov   r1, #0x2C
ldrb  r1, [r5, r1]
mul   r6, r1

mov   r1, #0x30
ldrb  r2, [r5, r1]
add   r1, #0x2
mov   r0, #0x0
mov   r4, #0x1

Loop:
  ldrb  r3, [r5, r1]
  cmp   r3, r2
  bne   Return
    add   r1, #0x2
    add   r4, #0x1
    cmp   r4, r6
    blt   Loop

mov   r0, #0x1


Return:
pop   {r4-r6}
pop   {r1}
bx    r1
