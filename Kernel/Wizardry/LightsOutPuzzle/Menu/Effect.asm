@ Puzzle menu command effect routine.
@ We just set the active unit's action to that of visiting.
@ This will allow us to take advantage of stuff "visit" does.
.thumb

push  {r4-r7, r14}


@ Active unit performs visiting action
ldr   r0, =ActionData
mov   r1, #0x10
strb  r1, [r0, #0x11]


mov   r0, #0x17
pop   {r4-r7}
pop   {r1}
bx    r1
