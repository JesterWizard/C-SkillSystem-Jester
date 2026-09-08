@ Clears all BG (screen & char block) and OBJ tiles
.thumb

push  {r4, r14}
sub   sp, #0x4

mov   r0, #0x0
str   r0, [sp]
mov   r0, sp
mov   r1, #0x6
lsl   r1, #0x18
mov   r2, #0x18
lsl   r2, #0xA
mov   r3, #0x1
lsl   r3, #0x18
orr   r2, r3
ldr   r4, =CpuFastSet
bl    GOTO_R4

add   sp, #0x4
pop   {r4}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
