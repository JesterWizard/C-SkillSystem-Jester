@ Setup puzzle proc vars
@   +0x29: byte, various flags.
@     0:    Ignore input if set.
@     1:    Don't draw cursor if set.
@     2:    Don't draw weapons icons if set.
@     3:    Puzzle can't be skipped if set.
@   +0x2A: byte, cursor pos. Lower nybble is X, higher is Y. Initialized to 0x0. 
@   +0x2B: byte, tile transformation animation timer. Initialized to 0x0.
@   +0x2C: byte, Puzzle Height. Initialized by ASMC.
@   +0x2D: byte, Puzzle Width. Initialized by ASMC.
@   +0x2E: byte, Icon count. Initialized by ASMC.
@   +0x2F: byte, [0-3] indicates which dimensions are used. Initialized by ASMC.
@   +0x30-0x57: (iconIndex, AffineFlag) structs, one for each tile.
@   +0x58-0x5F: 8 weaponID bytes indexed by iconIndex.
@   +0x60: byte, [0-2] indicates which tier the reward should be. Initialized by ASMC.
.thumb

push  {r4-r7, r14}
mov   r4, r8
push  {r4}
sub   sp, #0x10
mov   r5, r0


@ Init +0x2A-0x2B
mov   r0, #0x2A
add   r0, r5
mov   r1, #0x0
strb  r1, [r0]
strb  r1, [r0, #0x1]


@ Init Puzzle Tiles
mov   r2, #0x2C
ldrb  r2, [r5, r2]
mov   r3, #0x2D
ldrb  r3, [r5, r3]
mul   r3, r2

ldr   r4, =InitPuzzleTiles
mov   r0, #0x2F
ldrb  r0, [r5, r0]
lsl   r0, #0x6
mov   r1, #0x2E
ldrb  r1, [r5, r1]
sub   r1, #0x2
lsl   r1, #0x5
add   r4, r0
add   r4, r1

mov   r0, #0x30
mov   r1, #0x0

Loop:
  ldrb  r2, [r4, r1]
  strh  r2, [r5, r0]
  add   r0, #0x2
  add   r1, #0x1
  cmp   r1, r3
  blt   Loop


@ Init weaponIDs, random.
@ Store weaponflags on stack
ldr   r0, =PuzzleWeaponFlagsRAM
ldr   r0, [r0]
ldrb  r0, [r0]
mov   r1, sp
add   r1, #0x8
mov   r2, #0x0
str   r2, [r1]
str   r2, [r1, #0x4]
mov   r6, #0x1

Loop2:
  mov   r3, r0
  lsr   r3, r2
  and   r3, r6
  strb  r3, [r1, r2]
  add   r2, #0x1
  cmp   r2, #0x7
  ble   Loop2

@ Initialize weaponIDs
ldr   r4, =NextRN_N
mov   r6, sp
mov   r7, #0x2E
ldrb  r7, [r5, r7]
mov   r0, #0x58
mov   r8, r0

Loop3:
  mov   r0, #0x0
  mov   r2, #0x0
  
  Loop4:
    mov   r1, r2
    add   r1, #0x8
    ldrb  r1, [r6, r1]
    cmp   r1, #0x0
    bne   L1
      strb  r2, [r6, r0]
      add   r0, #0x1
    L1:
    add   r2, #0x1
    cmp   r2, #0x7
    ble   Loop4
  
  bl    GOTO_R4
  
  ldrb  r1, [r6, r0]
  mov   r2, r8
  strb  r1, [r5, r2]
  add   r2, #0x1
  mov   r8, r2
  mov   r2, #0x1
  add   r1, #0x8
  strb  r2, [r6, r1]
  sub   r7, #0x1
  cmp   r7, #0x0
  bgt   Loop3


@ Change BGM
@mov   r0, #0x9                          @ SongID
@mov   r1, #0x3                          @ Fadeout & Fadein rate?
@mov   r2, #0x0                          @ No idea
@ldr   r4, =Switch_BGM
@bl    GOTO_R4


@ Create DrawSprites child
ldr   r0, =Proc_DrawSprites
mov   r1, r5
ldr   r4, =New6C
bl    GOTO_R4


add   sp, #0x10
pop   {r4}
mov   r8, r4
pop   {r4-r7}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
