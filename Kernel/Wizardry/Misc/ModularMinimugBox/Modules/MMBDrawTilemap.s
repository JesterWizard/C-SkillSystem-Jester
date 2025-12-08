
.thumb

.include "../CommonDefinitions.inc"

MMBDrawTilemap:

	.global	MMBDrawTilemap
	.type	MMBDrawTilemap, %function

	.set MMBTilemap,				EALiterals + 0
	.set MMBTilemapPaletteIndex,	EALiterals + 4

	@ Inputs:
	@ r0: pointer to proc state
	@ r1: pointer to unit in RAM

	push	{r4, lr}

	mov		r4, r1

	@ Draw the tilemap

	ldr		r0, =WindowBufferBG1
	ldr		r1, MMBTilemap
	mov		r2, #0xC0 @ 0x3000
	lsl		r2, r2, #0x06
	ldr		r3, =DrawTilemap
	mov		lr, r3
	bllr

	@ fetch palette based on allegiance

	mov		r1, r4
	mov		r0, #UnitDeploymentNumber @ allegiance byte
	ldsb	r0, [r1, r0]
	mov		r1, #0xC0
	and		r0, r1
	ldr		r1, MMBTilemapPaletteIndex
	ldr		r2, =GetPaletteByAllegiance
	mov		lr, r2
	bllr

	@ @ fetch palette based on gplayst.config.windowcolor 
	@ ldr		r0,=#0x202BCF0  // Struct - gPlaySt
	@ add 	r0, #0x41		// Config window color byte
	@ ldrb 	r0, [r0]		// Load the contents of the byte
	
	@ cmp 	r0, #0
	@ beq 	LoadBluePalette
	@ cmp 	r0, #4
	@ ble 	LoadRedPalette
	@ cmp 	r0, #8
	@ ble 	LoadGreenPalette
	@ cmp 	r0, #12
	@ ble 	LoadGreyPalette

	@ LoadBluePalette:
	@ ldr		r0, =#0x85b6bb4
	@ b 		End
	@ LoadRedPalette:
	@ ldr		r0, =#0x85b6bd4
	@ b 		End
	@ LoadGreenPalette:
	@ ldr		r0, =#0x85b6bf4
	@ b 		End
	@ LoadGreyPalette:
	@ ldr		r0, =#0x85b6c14
	@ b 		End

	@ End:
	@ mov 	r1, #0x60
	@ mov		r2, #0x20
	@ ldr		r3, =#0x8000DB8 // CopyToPaletteBuffer
	@ mov		lr, r3
	@ bllr

	pop		{r4}
	pop		{r0}
	bx		r0

.ltorg

EALiterals:
	@ MMBTilemap
	@ MMBTilemapPaletteIndex


