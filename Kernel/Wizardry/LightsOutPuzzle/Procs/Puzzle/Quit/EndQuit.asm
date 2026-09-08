@ Wait until fade-out finished.
.thumb

push  {r4-r5, r14}
mov   r5, r0


@ Draw sprites
bl    QuitLoopSprites


@ End if fade-out finished
ldr   r4, =FadeOutExists
bl    GOTO_R4
cmp   r0, #0x0
bne   Return
  mov   r0, r5
  ldr   r4, =Break6CLoop
  bl    GOTO_R4
  bl    EndTutorial               @ Happens to clear the stuff we need cleared


Return:
pop   {r4-r5}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
