@ Resets LCDIO stuff that isn't covered by other routines
@ Unused.
.thumb

push  {r4-r7, r14}


pop   {r4-r7}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
