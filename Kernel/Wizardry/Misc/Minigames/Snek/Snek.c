#include "common-chax.h"
#include "kernel-lib.h"
#include "Snek.h"

static void Snek_ShowUnitSprites(void);

void Snek_SaveHighScore(u8 *dst, const u32 size)
{
	static const u16 sSnekHighScoreMagic = 0x534B;
	u16 buffer[2];

	buffer[0] = gSnekHighScore;
	buffer[1] = sSnekHighScoreMagic;
	WriteAndVerifySramFast(buffer, dst, sizeof(buffer));
}

void Snek_LoadHighScore(u8 *src, const u32 size)
{
	u16 buffer[2];
	static const u16 sSnekHighScoreMagic = 0x534B;

	ReadSramFast(src, buffer, sizeof(buffer));
	if (buffer[1] == sSnekHighScoreMagic) {
		gSnekHighScore = buffer[0];
	} else {
		gSnekHighScore = 0;
		gSnekHighScoreMagic = sSnekHighScoreMagic;
	}
}

static void Snek_DrawStartSprite(void)
{
	CpuFastFill(0, (void *)0x06013000, 0xBE0);
	ApplyPalette(Pal_SnekPressStart, 0x10 + 2);

	Decompress(Gfx_SnekPressStart, gGenericBuffer);
	Copy2dChr(gGenericBuffer, (void *)0x6013000, 16, 2);

	/* Press Start - Parts 1, 2, 3, 4 */
	// PutSprite(1, 64, 136,  gObject_32x16, OAM2_PAL(2) + OAM2_LAYER(0) + OAM2_CHR(0x181));
	// PutSprite(1, 96, 136,  gObject_32x16, OAM2_PAL(2) + OAM2_LAYER(0) + OAM2_CHR(0x185));
	// PutSprite(1, 128, 136, gObject_32x16, OAM2_PAL(2) + OAM2_LAYER(0) + OAM2_CHR(0x189));
	// PutSprite(1, 160, 136, gObject_32x16, OAM2_PAL(2) + OAM2_LAYER(0) + OAM2_CHR(0x18D));
}

static void Snek_DrawGameSprites(void)
{
	Decompress(Gfx_SnekGameSheet, gGenericBuffer);
	Copy2dChr(gGenericBuffer, (void *)0x6013C00, 10, 4);
}

static void Snek_DrawHighScore(struct EventEngineProc * proc)
{
	int score = gSnekHighScore;
	int hundreds = k_umod((score / 100), 10);
	int tens = k_umod((score / 10), 10);
	int ones = k_umod(score, 10);

	PutSprite(1, 74 + 117,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x220 + hundreds));
	PutSprite(1, 82 + 117,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x220 + tens));
	PutSprite(1, 90 + 117,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x220 + ones));
}

static void Snek_DrawTopBarBackground(struct EventEngineProc * proc)
{
	static const u8 sTopBarXPositions[] = {
		0, 16, 32, 48, 64, 80, 96, 112,
		128, 144, 160, 176, 192, 208, 224,
	};
	unsigned int i;

	Decompress(Gfx_TopBarBackground, gGenericBuffer);
	Copy2dChr(gGenericBuffer, (void *)0x6014160, 2, 4);

	for (i = 0; i < ARRAY_COUNT(sTopBarXPositions); ++i)
		PutSprite(1, sTopBarXPositions[i], 0, gObject_16x16, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x20B));

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

	// Right side box and numbers (max obtainable score)
	PutSprite(1, 68,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x244)); // | (Left)
	PutSprite(1, 68,  4,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x244)); // | (Left)
	PutSprite(1, 74,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x229)); // Empty Number
	PutSprite(1, 82,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x229)); // Empty Number
	PutSprite(1, 90,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x229)); // Empty Number
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
	Snek_DrawHighScore(proc);
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

// Randomly draw a coin somewhere on the game background, which the player can collect for points if they run into it with the snek.
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

static const u16 * Snek_GetBendSprite(int prev_dx, int prev_dy, int next_dx, int next_dy)
{
	const u16 * sprite = gObject_8x8;

	/* Horizontal segment entering the bend */
	if (prev_dx < 0 && next_dy > 0)        // from left, turning down
		sprite = gObject_8x8_HFlipped;
	else if (prev_dx < 0 && next_dy < 0)   // from left, turning up
		sprite = gObject_8x8_HFlipped_VFlipped;
	else if (prev_dx > 0 && next_dy > 0)    // from right, turning down
		sprite = gObject_8x8;
	else if (prev_dx > 0 && next_dy < 0)    // from right, turning up
		sprite = gObject_8x8_HFlipped;
	/* Vertical segment entering the bend */
	else if (prev_dy < 0 && next_dx > 0)    // from up, turning right
		sprite = gObject_8x8_HFlipped;
	else if (prev_dy < 0 && next_dx < 0)    // from up, turning left
		sprite = gObject_8x8;
	else if (prev_dy > 0 && next_dx > 0)    // from down, turning right
		sprite = gObject_8x8_HFlipped_VFlipped;
	else if (prev_dy > 0 && next_dx < 0)    // from down, turning left
		sprite = gObject_8x8_VFlipped;

	return sprite;
}

static void Snek_HandleCoinCollision(void)
{
	int snake_x = gSnekSnakeX;
	int snake_y = gSnekSnakeY;
	int coin_x = (gSnekCoinCoordinates[0] >> 8) * 8;
	int coin_y = (gSnekCoinCoordinates[0] & 0xFF) * 8;

	if (!gSnekCoinPresent)
		return;

	if (snake_x == coin_x && snake_y == coin_y) {
		gSnekCoinPresent = false;
		gSnekCurrentScore += 4;
		if (gSnekCurrentScore >= 999)
			gSnekCurrentScore = 999;
		if (gSnekSnakeBodyLength < 32) {
			gSnekSnakeBodyX[gSnekSnakeBodyLength] = gSnekSnakeBodyX[gSnekSnakeBodyLength - 1];
			gSnekSnakeBodyY[gSnekSnakeBodyLength] = gSnekSnakeBodyY[gSnekSnakeBodyLength - 1];
			++gSnekSnakeBodyLength;
		}
	}
}

static void Snek_ShowUnitSprites(void)
{
	int i;

	for (i = 1; i < 0xC0; ++i) {
		struct Unit * unit = GetUnit(i);

		if (!UNIT_IS_VALID(unit))
			continue;

		unit->state &= ~US_HIDDEN;
		ShowUnitSprite(unit);
	}

	RefreshUnitSprites();
	SyncUnitSpriteSheet();
}

static void Snek_DrawSnake(void)
{
	int i;

	for (i = gSnekSnakeBodyLength - 1; i >= 0; --i) {
		const u16 * sprite = gObject_8x8;
		int chr = 0x1E1;
		int x = gSnekSnakeBodyX[i];
		int y = gSnekSnakeBodyY[i];

		if (i == 0) {
			chr = (gSnekSnakeState[SNEK_STATE_DIR] == SNEK_DIR_UP || gSnekSnakeState[SNEK_STATE_DIR] == SNEK_DIR_DOWN) ? 0x1E5 : 0x1E0;
			if (gSnekSnakeState[SNEK_STATE_DIR] == SNEK_DIR_RIGHT || gSnekSnakeState[SNEK_STATE_DIR] == SNEK_DIR_DOWN)
				sprite = gObject_8x8_HFlipped_VFlipped;
		} else if (i == gSnekSnakeBodyLength - 1) {
			int dx = gSnekSnakeBodyX[i - 1] - x;
			int dy = gSnekSnakeBodyY[i - 1] - y;

			if (dx != 0)
				chr = 0x1E8;
			else
				chr = 0x1E7;

			if (dx > 0 || dy > 0)
				sprite = gObject_8x8_HFlipped_VFlipped;
		} else {
			int prev_dx = x - gSnekSnakeBodyX[i - 1];
			int prev_dy = y - gSnekSnakeBodyY[i - 1];
			int next_dx = gSnekSnakeBodyX[i + 1] - x;
			int next_dy = gSnekSnakeBodyY[i + 1] - y;
			int bends_corner = (prev_dx != 0 && next_dy != 0) || (prev_dy != 0 && next_dx != 0);

			if (bends_corner) {
				const u16 * bend_sprite;

				chr = 0x1E3;
				bend_sprite = Snek_GetBendSprite(prev_dx, prev_dy, next_dx, next_dy);
				sprite = bend_sprite;
			} else if (prev_dx != 0 || next_dx != 0) {
				chr = 0x1E1;
			} else {
				chr = 0x1E6;
			}
		}

		PutSprite(0, x, y, sprite, OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(chr));
	}
}

static void Snek_DrawCurrentScore(struct EventEngineProc * proc)
{
	int score = gSnekCurrentScore;
	int hundreds = k_umod((score / 100), 10);
	int tens = k_umod((score / 10), 10);
	int ones = k_umod(score, 10);

	PutSprite(1, 34,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x220 + hundreds));
	PutSprite(1, 42,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x220 + tens));
	PutSprite(1, 50,  3,  gObject_8x8,  OAM2_PAL(5) + OAM2_LAYER(0) + OAM2_CHR(0x220 + ones));
}

static void Snek_HideUnitSprites(void)
{
	int i;

	for (i = 1; i < 0xC0; ++i) {
		struct Unit * unit = GetUnit(i);

		if (!UNIT_IS_VALID(unit))
			continue;

		unit->state |= US_HIDDEN;
		HideUnitSprite(unit);
	}

	RefreshUnitSprites();
	SyncUnitSpriteSheet();
}

static void Snek_DrawSprites(struct EventEngineProc * proc)
{
	CpuFastFill(0, (void *)0x06013C00, 0x11E0);
	ApplyPalette(Pal_SnekGameSheet, 0x10 + 5);

	Snek_DrawGameBackground(proc);
}

static void Snek_Init(struct EventEngineProc * proc)
{
	gSnekSnakeState[SNEK_STATE_X] = 120;
	gSnekSnakeState[SNEK_STATE_Y] = 72;
	gSnekSnakeState[SNEK_STATE_DIR] = SNEK_DIR_LEFT;
	gSnekSnakeState[SNEK_STATE_TIMER] = 0;
	gSnekSnakeX = 120;
	gSnekSnakeY = 72;
	gSnekLastTravelDirection = SNEK_DIR_LEFT;
	gSnekLastPressedDirection = SNEK_DIR_LEFT;
	if (gSnekHighScoreMagic != 0x534B) {
		gSnekHighScore = 0;
		gSnekHighScoreMagic = 0x534B;
	}
	gSnekSnakeBodyLength = 3;
	gSnekSnakeBodyX[0] = 120;
	gSnekSnakeBodyY[0] = 72;
	gSnekSnakeBodyX[1] = 128;
	gSnekSnakeBodyY[1] = 72;
	gSnekSnakeBodyX[2] = 136;
	gSnekSnakeBodyY[2] = 72;

	Snek_HideUnitSprites();
	Snek_DrawSprites(proc);
}

static void Snek_EndProc(struct EventEngineProc * proc)
{
	// Set the high score for the next game and reset the current score
	if (gSnekCurrentScore > gSnekHighScore)
		gSnekHighScore = gSnekCurrentScore;

	gSnekCurrentScore = 0;
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);
	
	Snek_ShowUnitSprites();
	Proc_Break(proc);
}

static void Snek_Loop(struct EventEngineProc * proc)
{;
	if (gSnekSnakeX < 0 || gSnekSnakeX >= 240 || gSnekSnakeY < 16 || gSnekSnakeY >= 160) {
		Snek_EndProc(proc);
		return;
	}


	if (gKeyStatusPtr->newKeys & B_BUTTON)
	{
		Proc_Goto(proc, PL_GAME_SNEK_PRESS_B);
		return;
	}

	if (gKeyStatusPtr->newKeys & DPAD_RIGHT)
	{
		gSnekLastTravelDirection = gSnekSnakeState[SNEK_STATE_DIR];
		gSnekLastPressedDirection = SNEK_DIR_RIGHT;
		if (gSnekSnakeState[SNEK_STATE_DIR] != SNEK_DIR_LEFT)
			gSnekSnakeState[SNEK_STATE_DIR] = SNEK_DIR_RIGHT;
	}
	else if (gKeyStatusPtr->newKeys & DPAD_LEFT)
	{
		gSnekLastTravelDirection = gSnekSnakeState[SNEK_STATE_DIR];
		gSnekLastPressedDirection = SNEK_DIR_LEFT;
		if (gSnekSnakeState[SNEK_STATE_DIR] != SNEK_DIR_RIGHT)
			gSnekSnakeState[SNEK_STATE_DIR] = SNEK_DIR_LEFT;
	}
	else if (gKeyStatusPtr->newKeys & DPAD_UP)
	{
		gSnekLastTravelDirection = gSnekSnakeState[SNEK_STATE_DIR];
		gSnekLastPressedDirection = SNEK_DIR_UP;
		if (gSnekSnakeState[SNEK_STATE_DIR] != SNEK_DIR_DOWN)
			gSnekSnakeState[SNEK_STATE_DIR] = SNEK_DIR_UP;
	}
	else if (gKeyStatusPtr->newKeys & DPAD_DOWN)
	{
		gSnekLastTravelDirection = gSnekSnakeState[SNEK_STATE_DIR];
		gSnekLastPressedDirection = SNEK_DIR_DOWN;
		if (gSnekSnakeState[SNEK_STATE_DIR] != SNEK_DIR_UP)
			gSnekSnakeState[SNEK_STATE_DIR] = SNEK_DIR_DOWN;
	}

	if (++gSnekSnakeState[SNEK_STATE_TIMER] >= 16) {
		gSnekSnakeState[SNEK_STATE_TIMER] = 0;

		if (gSnekSnakeBodyLength > 1) {
			int i;

			for (i = gSnekSnakeBodyLength - 1; i > 0; --i) {
				gSnekSnakeBodyX[i] = gSnekSnakeBodyX[i - 1];
				gSnekSnakeBodyY[i] = gSnekSnakeBodyY[i - 1];
			}
		}

		switch (gSnekSnakeState[SNEK_STATE_DIR]) {
		case SNEK_DIR_RIGHT:
			gSnekSnakeX += 8;
			break;

		case SNEK_DIR_LEFT:
			gSnekSnakeX -= 8;
			break;

		case SNEK_DIR_UP:
			gSnekSnakeY -= 8;
			break;

		case SNEK_DIR_DOWN:
			gSnekSnakeY += 8;
			break;
		}

		gSnekSnakeState[SNEK_STATE_X] = (u8)gSnekSnakeX;
		gSnekSnakeState[SNEK_STATE_Y] = (u8)gSnekSnakeY;
		gSnekSnakeBodyX[0] = gSnekSnakeX;
		gSnekSnakeBodyY[0] = gSnekSnakeY;

		{
			int i;

		for (i = 1; i < gSnekSnakeBodyLength; ++i) {
			if (gSnekSnakeBodyX[0] == gSnekSnakeBodyX[i] && gSnekSnakeBodyY[0] == gSnekSnakeBodyY[i]) {
				Snek_EndProc(proc);
				return;
			}
		}
		}
	}

	Snek_HandleCoinCollision();

	CpuFastFill(0, (void *)0x06017000, 0xC60);
	
	Snek_DrawStartSprite();
	Snek_DrawGameSprites();
	Snek_DrawTopBarBackground(proc);
	Snek_DrawGameCoin(proc);
	Snek_DrawCurrentScore(proc);
	Snek_DrawSnake();
}

static const struct ProcCmd ProcScr_SnekMinigame[] = {
	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_CALL(Snek_Init),
	PROC_CALL_ARG(NewFadeIn, 0x10),
    PROC_WHILE(FadeInExists),
	PROC_REPEAT(Snek_Loop),

PROC_LABEL(PL_GAME_SNEK_PRESS_B),
    PROC_CALL_ARG(NewFadeOut, 0x10),
    PROC_WHILE(FadeOutExists),
	PROC_END,
};

void CallSnekMinigameASMC(struct EventEngineProc * proc)
{
	Proc_StartBlocking(ProcScr_SnekMinigame, proc);
}

void Snek_SetOutcomeEventSlots(void)
{
	if (gSnekCurrentScore > gSnekHighScore)
		gEventSlots[EVT_SLOT_7] = 0;
	else if (gSnekCurrentScore == gSnekHighScore)
		gEventSlots[EVT_SLOT_7] = 1;
	else
		gEventSlots[EVT_SLOT_7] = 2;

	gEventSlots[EVT_SLOT_8] = 0;
	gEventSlots[EVT_SLOT_9] = 1;
}