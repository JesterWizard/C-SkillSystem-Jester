.thumb
.include 	"../src/_include/_Definitions.h.s"
.include 	"../src/_include/MokhaRAMSpace.s"

SET_FUNC_INLINE		MU_ExecCmd_FixForFreeMU
SET_FUNC_INLINE 	MU_CALL2_FixForFreeMU
SET_FUNC_INLINE 	MU_GetSpeed_FixForFreeMU




MU_ExecCmd_FixForFreeMU:
	@ r4: the MU running its move script (RunMuMoveScript)
	push	{r4-r7, lr}
	mov		r1, r8
	push	{r1}
	mov		r8, r4
	asr		r4, r0, #0x18
	
	ldr		r0, =FreeMovementControlProc
	blh		ProcFind
	mov		r5, r0					
	cmp		r0, #0
	beq		.EndMain
	
	@ Only the player's MU: a district's NPCs walk with MUs of their own (DistrictNpc.c), and the first MU
	@ found (ProcFind) could be one of theirs, which then took the player's keys
	mov		r6, r8
	ldr		r0, [r6, #0x2C]
	ldr		r1, =gActiveUnit
	ldr		r1, [r1]
	cmp		r0, r1
	bne		.EndMain
	
	ldr		r0,[r6, #0x34]			
	ldrb	r0,[r0, #4]
	cmp		r0, #1
	beq		.EndMain				@ 第一次则直接过
	
	blh		FMU_ChkKeyForMUExtra
	mov		r7, r0					@ r7 = 0L/1R/2D/3U
	cmp		r0, #0x10
	beq		.EndMain
	
	ldr		r0, =gProc_MuCtr
	blh		ProcFind
	cmp		r0, #0
	beq		.EndMain
	add		r0, #0x40
	ldrb	r5,[r0]					@ r5=x
	ldrb	r6,[r0, #1]				@ r6=y
	
	bl		PositionChange
	
	ldr		r0, =gActiveUnit
	ldr		r0,[r0]
	mov		r1, r5
	mov		r2, r6
	blh		FMU_CanUnitBeOnPos
	cmp		r0, #0
	beq		.EndMain
	
	@ <!> --------------- Set Here! ---------------------
	mov		r0, r8
	ldr		r3,[r0, #0x34]			
	mov		r0, #1
	strb	r0,[r3, #4]
	strb	r7,[r3, #5]
	
	ldr		r0, =gProc_MuCtr
	blh		ProcFind
	add		r0, #0x40
	strb	r5,[r0]
	strb	r6,[r0, #1]
	strb	r5,[r0, #2]
	strb	r6,[r0, #3]
	
	
	lsl		r4, r7, #0x18	@ Vanilla 0x0788AE
	asr		r4, #0x18
.EndMain:
	pop		{r1}
	mov		r8, r1
	mov		r0, r4			@ Vanilla 0x0788B0
	lsl		r0, #0x10
	lsr		r3, r0, #0x10
	mov		r1, #1
	lsl		r1, #0x10
	add		r0, r1
	asr		r0, #0x10
	
	pop		{r4-r7}
	pop		{r1}
	mov		lr, r1
	ldr		r1, =0x80788BF
	bx		r1
	
	
.ltorg	
.align	
MU_CALL2_FixForFreeMU:
	@ORG 0x8078D10, r6: the MU
	@ No MU moves the camera in free movement: the free movement camera (FreeMU_Camera.c) glides after the player
	@ itself. Vanilla's camera follow here jumped it on top of that, and a jump past a tile row skipped drawing the
	@ map row coming into view (UpdateBmMapDisplay draws one when the camera crosses a row): a blank row in a district
	@ until the map was redrawn. Outside free movement no camera either, as the build had it before.
	b		.ReturnCall2SkipCamera
	
	
	
	.ReturnCall2SkipCamera:
	ldr		r1, =0x8078D3D
	bx		r1
	
	
	
.ltorg
.align
MU_GetSpeed_FixForFreeMU:
@ 807947C
	push	{r4, lr}
	mov		r0, r4
	
	ldr		r0, =FreeMovementControlProc
	blh		ProcFind
	cmp		r0, #0
	beq		.MU_GetSpeed_ReturnNormal
	ldr		r0, =FreeMU_MovingSpeed
	ldrb	r0,[r0]
	lsl		r0, #0x18
	lsr		r0, #0x18
	pop		{r4, pc}
	
	.MU_GetSpeed_ReturnNormal:
	mov		r0, #0x4A
	add		r0, r4
	mov		r2, #0
	ldsh	r1,[r0, r2]
	mov		r0, #0x80
	and		r0, r1
	ldr		r2, =0x807948B
	bx		r2

	
	
.ltorg	
.align	
PositionChange:
	push	{lr}
	@ R7 = 0L/1R/2D/3U
	@ r5 = x
	@ r6 = y
.Left:
	cmp		r7, #0
	bne		.Right
	add		r5, #-1
	b		.EndPositionChange
.Right:
	cmp		r7, #1
	bne		.Down
	add		r5, #1
	b		.EndPositionChange
.Down:	
	cmp		r7, #2
	bne		.Up
	add		r6, #1
	b		.EndPositionChange
.Up:	
	cmp		r7, #3
	bne		.EndPositionChange
	add		r6, #-1
.EndPositionChange:
	pop		{pc}
	