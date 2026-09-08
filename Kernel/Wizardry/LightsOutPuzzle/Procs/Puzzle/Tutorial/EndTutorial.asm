@ Prepares the backgrounds.
.thumb

push  {r4, r14}


@ Re-enable cursor sprite and weapon icons
ldr   r0, [r0, #0x14]
mov   r1, #0x29
ldrb  r2, [r0, r1]
mov   r3, #0x6
mvn   r3, r3
and   r2, r3
strb  r2, [r0, r1]


@ Clear BG0,
ldr   r0, =gpBG0MapBuffer
mov   r1, #0x0
ldr   r4, =BG_Fill
bl    GOTO_R4


@ Clear colour effects
ldr   r0, =gpDISPCNTbuffer
add   r0, #0x3C
mov   r1, #0x0
strh  r1, [r0]                  @ BLDCNT
strh  r1, [r0, #0xA]            @ BLDY


@ Deallocate text. Apparently unnecessary,
@ also re-sets palette 0 which we don't want
@ when quitting, otherwise BG2 will display
@ garbage data for 1 frame.
@ldr   r4, =Font_InitForUIDefault
@bl    GOTO_R4


mov   r0, #0x0
ldr   r4, =EnableBackgroundSyncById 
bl    GOTO_R4


pop   {r4}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
