@ Draw button prompt sprites
.thumb

push  {r4-r5, r14}
ldr   r5, [r0, #0x14]


@ Diagram
mov   r0, #0x2E
ldrb  r0, [r0, r5]
sub   r0, #0x2
lsl   r0, #0x6
ldr   r2, =TutDiagramOAM
add   r2, r0
mov   r0, #0x0
mov   r1, #0x0
mov   r3, #0x0
ldr   r4, =PushToSecondaryOAM
bl    GOTO_R4


@ A
mov   r0, #0x58
mov   r1, #0x20
ldr   r2, =OAM16x16
mov   r3, #0x20
lsl   r3, #0x8
add   r3, #0x16
ldr   r4, =PushToSecondaryOAM
bl    GOTO_R4


@ B, if puzzle can be skipped.
mov   r0, #0x29
ldrb  r0, [r5, r0]
mov   r1, #0x8
tst   r0, r1
bne   Return

  mov   r0, #0x19
  mov   r1, #0x90
  ldr   r2, =OAM16x16
  mov   r3, #0x20
  lsl   r3, #0x8
  add   r3, #0x18
  ldr   r4, =PushToSecondaryOAM
  bl    GOTO_R4


Return:
pop   {r4-r5}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
