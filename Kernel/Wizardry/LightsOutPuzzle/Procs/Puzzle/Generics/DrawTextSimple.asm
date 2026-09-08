@ Draw some white text without worrying about space management. Args:
@   r0: TextID
@   r1: BGID [0x0-0x3]
@   r2: TileID [0x0-0x400]
.thumb

push  {r4-r7, r14}
sub   sp, #0x8
mov   r5, r0
mov   r6, r1
mov   r7, r2


mov   r0, sp
mov   r1, #0x20
ldr   r4, =Text_Init
bl    GOTO_R4

mov   r0, r5
ldr   r4, =GetStringFromIndex
bl    GOTO_R4
ldr   r4, =FilterSomeTextFromStandardBuffer
bl    GOTO_R4
mov   r3, r0

mov   r0, sp
mov   r1, #0x0
mov   r2, #0x0
ldr   r4, =Text_InsertString
bl    GOTO_R4

mov   r0, sp
ldr   r1, =gpBG0MapBuffer
lsl   r2, r6, #0xB
add   r1, r2
lsl   r2, r7, #0x1
add   r1, r2
ldr   r4, =Text_Draw
bl    GOTO_R4


add   sp, #0x8
pop   {r4-r7}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
