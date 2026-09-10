#include "global.h"
#include "bmio.h"
#include "bmlib.h"
#include "constants/songs.h"
#include "constants/video-global.h"
#include "fontgrp.h"
#include "hardware.h"
#include "player_interface.h"
#include "proc.h"
#include "rng.h"
#include "soundwrapper.h"
#include "uiutils.h"

#define GAMEOVER_QUOTE_LINES 3

struct GameOverQuote {
	const char *values[GAMEOVER_QUOTE_LINES];
};

extern const struct GameOverQuote GameOverQuotes[];
extern const struct GameOverQuote GameOverQuotesEnd[];

void GameOverScreen_Init_Quotes(struct ProcGameOverScreen *proc)
{
	int quoteCount;
	int chosenMessage;
	unsigned i;

	BMapDispSuspend();

	/* HUD procs keep writing BG0 (goal timer redraws every frame). */
	Proc_EndEach(gProcScr_UnitDisplay_MinimugBox);
	Proc_EndEach(gProcScr_UnitDisplay_Burst);
	Proc_EndEach(gProcScr_TerrainDisplay);
	Proc_EndEach(gProcScr_GoalDisplay);
	Proc_EndEach(gProcScr_PrepMap_MenuButtonDisplay);

	SetSecondaryHBlankHandler(NULL);
	SetWinEnable(0, 0, 0);

	StartBgm(SONG_GAME_OVER, 0);

	gLCDControlBuffer.dispcnt.mode = 0;
	gLCDControlBuffer.bg0cnt.priority = 0;
	gLCDControlBuffer.bg1cnt.priority = 1;
	gLCDControlBuffer.bg2cnt.priority = 2;
	gLCDControlBuffer.bg3cnt.priority = 3;

	gLCDControlBuffer.bg0cnt.screenSize = 0;
	gLCDControlBuffer.bg1cnt.screenSize = 0;
	gLCDControlBuffer.bg2cnt.screenSize = 0;
	gLCDControlBuffer.bg3cnt.screenSize = 0;
	gLCDControlBuffer.bg0cnt.colorMode = 0;
	gLCDControlBuffer.bg1cnt.colorMode = 0;
	gLCDControlBuffer.bg2cnt.colorMode = 0;
	gLCDControlBuffer.bg3cnt.colorMode = 0;
	gLCDControlBuffer.bg0cnt.mosaic = 0;
	gLCDControlBuffer.bg1cnt.mosaic = 0;
	gLCDControlBuffer.bg2cnt.mosaic = 0;
	gLCDControlBuffer.bg3cnt.mosaic = 0;
	gLCDControlBuffer.mosaic = 0;

	SetBackgroundTileDataOffset(0, 0);
	SetBackgroundTileDataOffset(1, 0);
	SetBackgroundTileDataOffset(2, 0);
	SetBackgroundTileDataOffset(3, 0);

	BG_SetPosition(BG_0, 0, 0);
	BG_SetPosition(BG_1, 0, 0);
	BG_SetPosition(BG_2, 0, 0);
	BG_SetPosition(BG_3, 0, 0);

	ApplyPalette(Pal_GameOverText1, BGPAL_GAMEOVER_4);
	Decompress(Img_ChapterIntroFog, BG_CHR_ADDR(BGCHR_BMFX_IMG));

	ClearBg0Bg1();

	PutScreenFogEffectOverlayed();
	PutScreenFogEffect();

	BG_EnableSyncByMask(BG0_SYNC_BIT | BG2_SYNC_BIT | BG3_SYNC_BIT);

	SetPrimaryHBlankHandler(GameOverScreenHBlank);

	SetBlendConfig(1, 14, 14, 0);

	SetBlendTargetA(0, 0, 1, 0, 0);
	SetBlendTargetB(0, 0, 0, 1, 0);

	MaybeResetSomePal();
	MaybeSmoothChangeSomePal(&PAL_BG_COLOR(BGPAL_GAMEOVER_TEXT, 0), BGPAL_GAMEOVER_TEXT, 1, +1);
	MaybeSmoothChangeSomePal(&PAL_BG_COLOR(BGPAL_GAMEOVER_4, 0), BGPAL_GAMEOVER_4, 1, +1);

	proc->counter1 = 21;

	for (i = 0; i < 10; ++i)
		CALLARM_ColorFadeTick();

	quoteCount = GameOverQuotesEnd - GameOverQuotes;
	if (quoteCount > 0) {
		chosenMessage = NextRN_N(quoteCount);

		/* Fog lives at BGCHR_BMFX_IMG (0x100). Reset the font allocator to 0x80
		 * so quote glyphs cannot overwrite the fog tiles and break wrapping. */
		ResetText();

		for (i = 0; i < GAMEOVER_QUOTE_LINES; i++) {
			const char *line = GameOverQuotes[chosenMessage].values[i];
			int xPx;
			int xTile;
			int xOff;
			int tileWidth;

			if (line == NULL || line[0] == '\0')
				continue;

			xPx = GetStringTextCenteredPos(DISPLAY_WIDTH, line);
			xTile = xPx / 8;
			xOff = xPx % 8;
			tileWidth = (GetStringTextLen(line) + xOff + 7) / 8;

			PutDrawText(
				NULL,
				gBG0TilemapBuffer + TILEMAP_INDEX(xTile, 7 + (i * 2)),
				TEXT_COLOR_SYSTEM_GOLD,
				xOff,
				tileWidth,
				line);
		}

		BG_EnableSyncByMask(BG0_SYNC_BIT);
	}

	EnablePaletteSync();
}
