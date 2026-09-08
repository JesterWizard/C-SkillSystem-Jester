@ Puzzle menu command usability routine.
.thumb

push  {r4-r7, r14}
mov   r5, #0x3


@ Disregard summon
ldr   r0, =ActiveUnit
ldr   r1, [r0]
cmp   r1, #0x0
beq   Return
  ldr   r0, [r1, #0x4]
  cmp   r0, #0x0
  beq   Return
    ldrb  r0, [r0, #0x4]
    cmp   r0, #0x51                         @ Phantom class
    beq   Return


@ Character should be moving/canto-ing
ldr   r0, [r1, #0xC]
mov   r2, #0x40
tst   r0, r2
bne   Return

  mov   r0, #0x10
  ldsb  r0, [r1, r0]                        @ X
  mov   r2, #0x11
  ldsb  r1, [r1, r2]                        @ Y
  ldr   r4, =GetAvailableLocaCommandAt
  bl    GOTO_R4
  cmp   r0, #0x1A
  bne   Return
    mov   r5, #0x1


Return:
mov   r0, r5
pop   {r4-r7}
pop   {r1}
bx    r1
GOTO_R4:
bx    r4
