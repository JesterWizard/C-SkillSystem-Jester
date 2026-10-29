.thumb
.syntax unified

@ Hooked with jumpToHack at the start of vanilla UnitKill (0x080183FC), r0 = unit.
@
@ Vanilla UnitKill:
@   080183FC  push  {lr}                 \
@   080183FE  adds  r2, r0, #0            | overwritten by jumpToHack,
@   08018400  movs  r1, #11               | redone below
@   08018402  ldrsb r1, [r2, r1]         /
@   08018404  movs  r0, #0xC0
@   08018406  ands  r1, r0
@   08018408  cmp   r1, #0
@   0801840A  bne   0x08018428            @ not a player unit
@   0801840C  ...                         @ player unit: phantom check, then
@                                         @ state |= US_DEAD | US_HIDDEN, InitUnitsupports
@   08018428  movs  r0, #0                @ not a player unit:
@   0801842A  str   r0, [r2]              @   unit->pCharacterData = NULL
@   0801842C  pop   {r0}
@   0801842E  bx    r0
@
@ Player units go back into vanilla code at 0x0801840C (so a patch that hooks
@ the player branch, like casual_mode at 0x08018418, still runs). Other units
@ are marked US_DEAD | US_HIDDEN instead of being erased.
@ r3 is free (jumpToHack clobbers it).

.global UnitKill_CollectDeadUnits
.type UnitKill_CollectDeadUnits, %function
UnitKill_CollectDeadUnits:
    push  {lr}
    adds  r2, r0, #0
    movs  r1, #11
    ldrsb r1, [r2, r1]              @ unit->index
    movs  r0, #0xC0
    ands  r1, r0
    cmp   r1, #0
    bne   .Lnot_player

    ldr   r3, =0x0801840D           @ vanilla player branch (r1 = 0, r2 = unit, lr pushed)
    bx    r3

.Lnot_player:
    ldr   r0, [r2, #0xC]
    movs  r1, #5                    @ US_DEAD | US_HIDDEN
    orrs  r0, r1
    str   r0, [r2, #0xC]
    pop   {r0}
    bx    r0

.align 2
.ltorg
