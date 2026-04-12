#include "common-chax.h"
#include "kernel-lib.h"
#include "Snek.h"

static void Snek_DrawStartSprite(void)
{
	CpuFastFill(0, (void *)0x06013000, 0xBE0);
	ApplyPalette(Pal_SnekPressStart, 0x10 + 2);

	Decompress(Gfx_SnekPressStart_1, gGenericBuffer);
	Copy2dChr(gGenericBuffer, (void *)0x6013000, 4, 2);

	Decompress(Gfx_SnekPressStart_2, gGenericBuffer);
	Copy2dChr(gGenericBuffer, (void *)0x6013080, 4, 2);

	Decompress(Gfx_SnekPressStart_3, gGenericBuffer);
	Copy2dChr(gGenericBuffer, (void *)0x6013100, 4, 2);

	Decompress(Gfx_SnekPressStart_4, gGenericBuffer);
	Copy2dChr(gGenericBuffer, (void *)0x6013180, 4, 2);

	/* Press Start - Parts 1, 2, 3, 4 */
	PutSprite(1, 64, 136,  gObject_32x16, OAM2_PAL(2) + OAM2_LAYER(0) + OAM2_CHR(0x181));
	PutSprite(1, 96, 136,  gObject_32x16, OAM2_PAL(2) + OAM2_LAYER(0) + OAM2_CHR(0x185));
	PutSprite(1, 128, 136, gObject_32x16, OAM2_PAL(2) + OAM2_LAYER(0) + OAM2_CHR(0x189));
	PutSprite(1, 160, 136, gObject_32x16, OAM2_PAL(2) + OAM2_LAYER(0) + OAM2_CHR(0x18D));
}

static void Snek_DrawGameSprites(void)
{
	Decompress(Gfx_SnekGameSheet_1, gGenericBuffer);
	Copy2dChr(gGenericBuffer, (void *)0x6013C00, 2, 4);

	Decompress(Gfx_SnekGameSheet_2, gGenericBuffer);
	Copy2dChr(gGenericBuffer, (void *)0x6013C40, 2, 4);

	Decompress(Gfx_SnekGameSheet_3, gGenericBuffer);
	Copy2dChr(gGenericBuffer, (void *)0x6013C80, 2, 4);

	Decompress(Gfx_SnekGameSheet_4, gGenericBuffer);
	Copy2dChr(gGenericBuffer, (void *)0x6013CC0, 2, 4);
	
	Decompress(Gfx_SnekGameSheet_5, gGenericBuffer);
	Copy2dChr(gGenericBuffer, (void *)0x6013D00, 2, 4);
}

static void Snek_DrawTopBarBackground(struct EventEngineProc * proc)
{
	Decompress(Gfx_TopBarBackground, gGenericBuffer);
	Copy2dChr(gGenericBuffer, (void *)0x6014160, 2, 4);

	PutSprite(1, 0, 0,  gObject_16x16, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x20B)); // Top Bar Background 1
	PutSprite(1, 16, 0, gObject_16x16, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x20B)); // Top Bar Background 2
	PutSprite(1, 32, 0, gObject_16x16, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x20B)); // Top Bar Background 3
	PutSprite(1, 48, 0, gObject_16x16, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x20B)); // Top Bar Background 4
	PutSprite(1, 64, 0, gObject_16x16, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x20B)); // Top Bar Background 5
	PutSprite(1, 80, 0, gObject_16x16, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x20B)); // Top Bar Background 6
	PutSprite(1, 96, 0, gObject_16x16, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x20B)); // Top Bar Background 7
	PutSprite(1, 112, 0, gObject_16x16, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x20B)); // Top Bar Background 8
	PutSprite(1, 128, 0, gObject_16x16, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x20B)); // Top Bar Background 9
	PutSprite(1, 144, 0, gObject_16x16, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x20B)); // Top Bar Background 10
	PutSprite(1, 160, 0, gObject_16x16, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x20B)); // Top Bar Background 11
	PutSprite(1, 176, 0, gObject_16x16, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x20B)); // Top Bar Background 12
	PutSprite(1, 192, 0, gObject_16x16, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x20B)); // Top Bar Background 13
	PutSprite(1, 208, 0, gObject_16x16, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x20B)); // Top Bar Background 14
	PutSprite(1, 224, 0, gObject_16x16, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x20B)); // Top Bar Background 15

	PutSprite(1, 12,  3,  gObject_16x8, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x240)); // Size

	// Left side box and numbers (current score)
	PutSprite(1, 28,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x244)); // | (Left)
	PutSprite(1, 28,  4,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x244)); // | (Left)
	PutSprite(1, 34,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x209)); // Empty Number
	PutSprite(1, 42,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x209)); // Empty Number
	PutSprite(1, 50,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x209)); // Empty Number
	PutSprite(1, 32,  0,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Top)
	PutSprite(1, 40,  0,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Top)
	PutSprite(1, 48,  0,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Top)
	PutSprite(1, 32,  10,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Bottom)
	PutSprite(1, 40,  10,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Bottom)
	PutSprite(1, 48,  10,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Bottom)
	PutSprite(1, 53,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x244)); // | (Right)
	PutSprite(1, 53,  4,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x244)); // | (Right)


	PutSprite(1, 60,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x243)); // "/"

	// Right side box and numbers (high score)
	PutSprite(1, 68,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x244)); // | (Left)
	PutSprite(1, 68,  4,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x244)); // | (Left)
	PutSprite(1, 74,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x209)); // Empty Number
	PutSprite(1, 82,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x209)); // Empty Number
	PutSprite(1, 90,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x209)); // Empty Number
	PutSprite(1, 72,  0,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Top)
	PutSprite(1, 80,  0,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Top)
	PutSprite(1, 88,  0,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Top)
	PutSprite(1, 72,  10,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Bottom)
	PutSprite(1, 80,  10,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Bottom)
	PutSprite(1, 88,  10,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Bottom)
	PutSprite(1, 93,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x244)); // | (Right)
	PutSprite(1, 93,  4,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x244)); // | (Right)

	PutSprite(1, 145, 3,  gObject_16x8, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x245)); // High
	PutSprite(1, 161, 3,  gObject_32x8, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x247)); // Score

	// High Score box and numbers
	PutSprite(1, 68 + 117,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x244)); // | (Left)
	PutSprite(1, 68 + 117,  4,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x244)); // | (Left)
	PutSprite(1, 74 + 117,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x209)); // Empty Number
	PutSprite(1, 82 + 117,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x209)); // Empty Number
	PutSprite(1, 90 + 117,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x209)); // Empty Number
	PutSprite(1, 72 + 117,  0,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Top)
	PutSprite(1, 80 + 117,  0,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Top)
	PutSprite(1, 88 + 117,  0,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Top)
	PutSprite(1, 72 + 117,  10,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Bottom)
	PutSprite(1, 80 + 117,  10,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Bottom)
	PutSprite(1, 88 + 117,  10,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x204)); // _ (Bottom)
	PutSprite(1, 93 + 117,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x244)); // | (Right)
	PutSprite(1, 93 + 117,  4,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x244)); // | (Right)
}

static void Snek_DrawGameBackground(struct EventEngineProc * proc)
{
	ApplyPalette(Pal_SnekGameBackground, 0);
	Decompress(Gfx_SnekGameBackground, (void *)0x06000160);
	CpuFastCopy(Tsa_SnekGameBackground, gBG2TilemapBuffer, 0x500);
	BG_EnableSyncByMask(BG2_SYNC_BIT);
	SetBlendConfig(BLEND_EFFECT_NONE, 0, 0x10, 0);
}

// Random;y draw a coin somewhere on the game background, which the player can collect for points if they run into it with the snek.
static void Snek_DrawGameCoin(struct EventEngineProc * proc)
{
	int x_width = 8;
	int y_width = 8;
	int x_pos;
	int y_pos;

	if (gSnekCoinPresent == false)
	{
		x_pos = NextRN_N(30);
		y_pos = NextRN_N(20);

		if (y_pos < 2)
			y_pos = 2; // Avoid drawing the coint in the top bar area where the score is displayed

		gSnekCoinCoordinates[0] = (x_pos << 8) | y_pos;
		gSnekCoinPresent = true;
	}
	else
	{
		x_pos = gSnekCoinCoordinates[0] >> 8;
		y_pos = gSnekCoinCoordinates[0] & 0xFF;

	}

	// Copper coin
	PutSprite(0, x_pos * x_width, y_pos * y_width, gObject_8x8, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x200));
}
extern u16 gSnekCoinCoordinates[1];

static void Snek_DrawSprites(struct EventEngineProc * proc)
{
	CpuFastFill(0, (void *)0x06013C00, 0x11E0);
	ApplyPalette(Pal_SnekGameSheet, 0x10 + 5);

	StartParallelWorker(Snek_DrawStartSprite, proc);
	StartParallelWorker(Snek_DrawGameSprites, proc);
	Snek_DrawGameBackground(proc);
	StartParallelWorker(Snek_DrawTopBarBackground, proc);
	StartParallelWorker(Snek_DrawGameCoin, proc);
}

static void Snek_Init(struct EventEngineProc * proc)
{
	Snek_DrawSprites(proc);
}

static void Snek_Loop(struct EventEngineProc * proc)
{
	if (gKeyStatusPtr->newKeys & B_BUTTON)
		Proc_Break(proc);
}

static const struct ProcCmd ProcScr_SnekMinigame[] = {
	PROC_CALL(Snek_Init),
	PROC_REPEAT(Snek_Loop),
	PROC_END,
};

void CallSnekMinigameASMC(struct EventEngineProc * proc)
{
	Proc_StartBlocking(ProcScr_SnekMinigame, proc);
}