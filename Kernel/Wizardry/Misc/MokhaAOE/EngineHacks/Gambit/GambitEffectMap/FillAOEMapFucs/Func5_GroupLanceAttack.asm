@ 111

.thumb
.include 	"../../include/_Definitions.h.s"
.include 	"../../../../_FuncLib/MokhaRAMSpace.s"
.include	"_definations_ADDMAP.h.s"

@(x,y,Direction)
Test_DrawMapDefault:
	push	{r4-r7,lr}
	mov		r4, r0		@Target.x
	mov		r5, r1		@Target.y
	cmp		r2, #4
	blt		Normal
	mov		r2, #0
Normal:	
	mov		r6, r2		@Direction
	
		@r0=Direction
	lsl		r0, r6, #2
	ldr		r1, =jpt_UnitDirection
	add		r0, r1
	ldr		r0,[r0]
	bx		r0
	
.ltorg
.align	
jpt_UnitDirection:
.word Moveleft+0x8000001
.word Moveright+0x8000001
.word MoveDown+0x8000001
.word Moveup+0x8000001

Moveleft:	
Moveright:
	ldr		r0, =gMapMovement
	ldr		r0,[r0]
	mov		r1, #0x1
	neg		r1, r1
	_blh2	ClearMapWith|1
	AddMapINLINE 0,0
	AddMapINLINE 0,1
	AddMapINLINE 0,-1
	b		TestEnd
	
	
MoveDown:	
Moveup:
	ldr		r0, =gMapMovement
	ldr		r0,[r0]
	mov		r1, #0x1
	neg		r1, r1
	_blh2	ClearMapWith|1
	AddMapINLINE 0,0
	AddMapINLINE 1,0
	AddMapINLINE -1,0
	b		TestEnd

	
TestEnd:
	pop		{r4-r7}
	pop		{r0}
	bx		r0
	
	
	
