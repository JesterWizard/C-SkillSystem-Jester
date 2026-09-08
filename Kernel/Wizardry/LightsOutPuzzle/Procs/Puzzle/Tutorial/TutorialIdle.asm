@ Ends tutorial if certain input is given.
.thumb

push  {r4-r5, r14}
mov   r5, r0


@ Draw sprites
bl    TutLoopSprites


@ Check input
ldr   r1, =KeyStatusBuffer
ldrh  r4, [r1, #0x8]
mov   r1, #0x81
lsl   r1, #0x1
tst   r1, r4
beq   Return

  @ R or B was pressed, end tutorial
  mov   r0, #0x59                 @ YUGIOH place card sfx
  bl    PlaySFX
  mov   r0, r5
  ldr   r4, =Break6CLoop
  bl    GOTO_R4


Return:
pop   {r4-r5}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
