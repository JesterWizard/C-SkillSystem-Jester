@   1
@  111
@ 11111
@  111
@   1

.thumb
.include 	"../../include/_Definitions.h.s"
.include 	"../../../../_FuncLib/MokhaRAMSpace.s"

@(x,y,Direction)
Func2_Absorption:
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
MoveDown:
Moveup:
	ldr		r0, =gMapMovement
	ldr		r0,[r0]
	mov		r1, #0x1
	neg		r1, r1
	_blh2	ClearMapWith|1
	add		r0, r4, #0
	add		r1, r5, #0
	mov		r2, #0
	mov		r3, #1
	push	{r4}
	_blh4	MapAddInRange			@ (r0=xPos, r1= yPos, r2=Range, r3=Value)
	pop		{r4}
	add		r0, r4, #0
	add		r1, r5, #1
	mov		r2, #0
	mov		r3, #1
	push	{r4}
	_blh4	MapAddInRange			@ (r0=xPos, r1= yPos, r2=Range, r3=Value)
	pop		{r4}
	add		r0, r4, #0
	add		r1, r5, #2
	mov		r2, #0
	mov		r3, #1
	push	{r4}
	_blh4	MapAddInRange			@ (r0=xPos, r1= yPos, r2=Range, r3=Value)
	pop		{r4}
	add		r0, r4, #0
	add		r1, r5, #-2
	mov		r2, #0
	mov		r3, #1
	push	{r4}
	_blh4	MapAddInRange			@ (r0=xPos, r1= yPos, r2=Range, r3=Value)
	pop		{r4}
	add		r0, r4, #0
	add		r1, r5, #-1
	mov		r2, #0
	mov		r3, #1
	push	{r4}
	_blh4	MapAddInRange			@ (r0=xPos, r1= yPos, r2=Range, r3=Value)
	pop		{r4}
	add		r0, r4, #0
	add		r1, r5, #2
	mov		r2, #0
	mov		r3, #1
	push	{r4}
	_blh4	MapAddInRange			@ (r0=xPos, r1= yPos, r2=Range, r3=Value)
	pop		{r4}
	
	add		r0, r4, #1
	add		r1, r5, #0
	mov		r2, #0
	mov		r3, #1
	push	{r4}
	_blh4	MapAddInRange			@ (r0=xPos, r1= yPos, r2=Range, r3=Value)
	pop		{r4}
	add		r0, r4, #1
	add		r1, r5, #1
	mov		r2, #0
	mov		r3, #1
	push	{r4}
	_blh4	MapAddInRange			@ (r0=xPos, r1= yPos, r2=Range, r3=Value)
	pop		{r4}
	add		r0, r4, #1
	add		r1, r5, #-1
	mov		r2, #0
	mov		r3, #1
	push	{r4}
	_blh4	MapAddInRange			@ (r0=xPos, r1= yPos, r2=Range, r3=Value)
	pop		{r4}

	add		r0, r4, #-1
	add		r1, r5, #0
	mov		r2, #0
	mov		r3, #1
	push	{r4}
	_blh4	MapAddInRange			@ (r0=xPos, r1= yPos, r2=Range, r3=Value)
	pop		{r4}
	add		r0, r4, #-1
	add		r1, r5, #1
	mov		r2, #0
	mov		r3, #1
	push	{r4}
	_blh4	MapAddInRange			@ (r0=xPos, r1= yPos, r2=Range, r3=Value)
	pop		{r4}
	add		r0, r4, #-1
	add		r1, r5, #-1
	mov		r2, #0
	mov		r3, #1
	push	{r4}
	_blh4	MapAddInRange			@ (r0=xPos, r1= yPos, r2=Range, r3=Value)
	pop		{r4}
	
	add		r0, r4, #2
	add		r1, r5, #0
	mov		r2, #0
	mov		r3, #1
	push	{r4}
	_blh4	MapAddInRange			@ (r0=xPos, r1= yPos, r2=Range, r3=Value)
	pop		{r4}
	
	add		r0, r4, #-2
	add		r1, r5, #0
	mov		r2, #0
	mov		r3, #1
	push	{r4}
	_blh4	MapAddInRange			@ (r0=xPos, r1= yPos, r2=Range, r3=Value)
	pop		{r4}
	
	b		TestEnd



	
TestEnd:
	pop		{r4-r7}
	pop		{r0}
	bx		r0
	
	
	
