@ Mark tile, arguments:
@   r0 = Puzzle ProcState pointer
@   r1 = TileX
@   r2 = TileY
.thumb

push  {r4, r14}
mov   r4, r0


mov   r0, #0x2D
ldrb  r0, [r4, r0]
mul   r2, r0
add   r1, r2

mov   r2, #0x30
add   r2, r4
lsl   r0, r1, #0x1
add   r1, r0, #0x1
mov   r3, #0x1
strb  r3, [r2, r1]              @ Mark, set Affine flag.


pop   {r4}
pop   {r0}
bx    r0
