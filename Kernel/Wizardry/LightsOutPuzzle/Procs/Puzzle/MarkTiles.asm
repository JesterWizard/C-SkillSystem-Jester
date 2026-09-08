@ Change tiles. Arguments:
@   r0: Puzzle procstate pointer.
.thumb

push  {r4-r7, r14}
mov   r4, r8
push  {r4}
mov   r5, r0


@ Set ignore input flag, and set anim timer
mov   r0, #0x29
ldrb  r1, [r5, r0]
mov   r2, #0x1
orr   r1, r2
strb  r1, [r5, r0]
mov   r0, #0x2B
ldr   r1, =TileAnimSpeed
ldrb  r1, [r1]
strb  r1, [r5, r0]


@ Calc X & Y
mov   r0, #0x2C
ldrb  r0, [r5, r0]
sub   r0, #0x1
mov   r4, #0x2D
ldrb  r4, [r5, r4]
sub   r4, #0x1
mov   r8, r0
mov   r0, #0x2A
ldrb  r6, [r5, r0]
lsr   r7, r6, #0x4                  @ Y
lsl   r6, #0x1C
lsr   r6, #0x1C                     @ X


@   *
@ * 1 *
@   *
mov   r0, r5
mov   r1, r6
mov   r2, r7
bl    MarkTile

@   1
@ * * *
@   *
cmp   r7, #0x0
ble   L1
  mov   r0, r5
  mov   r1, r6
  sub   r2, r7, #0x1
  bl    MarkTile
L1:

@   *
@ 1 * *
@   *
cmp   r6, #0x0
ble   L2
  mov   r0, r5
  sub   r1, r6, #0x1
  mov   r2, r7
  bl    MarkTile
L2:

@   *
@ * * 1
@   *
cmp   r6, r4
bge   L3
  mov   r0, r5
  add   r1, r6, #0x1
  mov   r2, r7
  bl    MarkTile
L3:

@   *
@ * * *
@   1
cmp   r7, r8
bge   L4
  mov   r0, r5
  mov   r1, r6
  add   r2, r7, #0x1
  bl    MarkTile
L4:


pop   {r4}
mov   r8, r4
pop   {r4-r7}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
