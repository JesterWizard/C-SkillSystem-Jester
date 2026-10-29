@ Banim C07 (attack start) entry for vanilla BattleAIS_ExecCommands.
@
@ The dispatcher at 0x08058C10 loads the case address for command N from the
@ table at 0x08058C44 and jumps to it with r7 = anim (r8/r9 hold its loop
@ state). Installer.event points entry 7 here.
@
@ CritCutin_OnC07(anim) returns 0 to run the vanilla C07 case (0x08059048),
@ nonzero to skip it and continue with the next queued command (0x080596CC,
@ the same place every vanilla case ends up).

	.syntax unified
	.thumb
	.align 2

	.global CritCutin_C07Thunk
	.type CritCutin_C07Thunk, %function
CritCutin_C07Thunk:
	adds r0, r7, #0
	ldr r3, =CritCutin_OnC07
	bl .Lbx_r3
	cmp r0, #0
	bne .Lhandled
	ldr r3, =0x08059049 @ vanilla C07 case
	bx r3
.Lhandled:
	ldr r3, =0x080596CD @ next queued command
	bx r3
.Lbx_r3:
	bx r3

	.ltorg
