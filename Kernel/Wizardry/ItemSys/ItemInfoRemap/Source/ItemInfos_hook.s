.INCLUDE "macros.inc"
.SYNTAX UNIFIED

/* 
** JESTER - As I've had to shift most of my hacks out of BL range to accomodate their size, 
** we need to use blh instead of bl for the functions here
*/

THUMB_FUNC_START DrawItemMenuLine_hook
DrawItemMenuLine_hook:
	@ 0x80168B4
	push {r0-r3}
	movs r0, r6
	// bl IsDuraItem
	blh IsDuraItem
	cmp r0, #1
	pop {r0-r3}
	beq 1f
	//bl PutNumberOrBlank
	blh PutNumberOrBlank
1:
	movs r0, r6
	// bl GetItemIconId
	blh GetItemIconId
	movs r1, r0
	ldr r0, =0x80168CB
	bx r0
