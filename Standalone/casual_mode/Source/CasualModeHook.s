.thumb
.syntax unified

@ Hooked with jumpToHack at UnitKill+0x1C (0x08018418): the player-unit,
@ not-a-phantom branch of vanilla UnitKill, which does
@     unit->state |= US_DEAD | US_HIDDEN;      @ 0x08018418 .. 0x0801841F
@     InitUnitsupports(unit);                  @ 0x08018420 .. 0x08018427
@     goto end;                                @ 0x0801842C: pop {r0}; bx r0
@ r2 holds the unit. UnitKill pushed lr; r3 is free (jumpToHack clobbers it).
@ CasualMode_KillPlayerUnit does both vanilla steps when casual mode is off.

.global UnitKill_CasualMode
.type UnitKill_CasualMode, %function
UnitKill_CasualMode:
    movs r0, r2
    ldr  r3, =CasualMode_KillPlayerUnit
    bl   .Lcall_r3
    ldr  r3, =0x0801842D           @ UnitKill epilogue: pop {r0}; bx r0
.Lcall_r3:
    bx   r3

.align 2
.ltorg
