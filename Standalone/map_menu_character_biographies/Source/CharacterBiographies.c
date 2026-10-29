#include "global.h"
#include "EAstdlib.h"
#include "EA_Standard_Library/Convo_Helpers.h"
#include "bm.h"
#include "bmio.h"
#include "bmudisp.h"
#include "bmunit.h"
#include "ctc.h"
#include "event.h"
#include "fontgrp.h"
#include "hardware.h"
#include "icon.h"
#include "player_interface.h"
#include "prepscreen.h"
#include "proc.h"
#include "rng.h"
#include "soundwrapper.h"
#include "sysutil.h"
#include "uimenu.h"
#include "uiutils.h"
#include "constants/songs.h"

ProcPtr StartUiCursorHand(ProcPtr);
extern CONST_DATA EventListScr EventScr_SetBackground[];

/*
 * Map menu "Biography" command: a scrolling list of characters, each with a
 * gold subtitle. Selecting one plays up to four CG text pages (background +
 * text ID) as an event, then returns to the list. B returns to the map.
 *
 * The list state lives in the proc, so no EWRAM is used.
 */

#define BIO_VISIBLE_COUNT 5
#define BIO_PAGE_COUNT 4

struct BiographyPage {
	u16 textId;
	u8 backgroundId;
	u8 pad;
};

struct BiographyEntry {
	u8 characterId;
	u8 songId;
	u16 pad;
	const char *subtitle;
	struct BiographyPage pages[BIO_PAGE_COUNT];
};

/* Defined in Biographies.event, terminated by characterId 0 */
extern const struct BiographyEntry gCharacterBiographies[];

enum {
	BIO_LABEL_INIT,
	BIO_LABEL_IDLE,
	BIO_LABEL_EVENT,
	BIO_LABEL_PRESS_B,
	BIO_LABEL_END,
};

struct BiographyProc {
	PROC_HEADER;

	u8 cursor;
	u8 top;
	u8 entry;
	u8 page;
};

extern const struct ProcCmd ProcScr_MapMenuBiography[];

static int CountBiographies(void)
{
	int count = 0;

	while (gCharacterBiographies[count].characterId != 0)
		count++;

	return count;
}

static void DrawBiographyList(struct BiographyProc *proc, int x, int y)
{
	int i, count = CountBiographies();

	for (i = 0; i < BIO_VISIBLE_COUNT; i++) {
		struct Text *text = &PrepItemSuppyTexts.th[i + 2];
		int index = proc->top + i;

		ClearText(text);

		if (index < count) {
			const struct BiographyEntry *bio = &gCharacterBiographies[index];

			Text_SetCursor(text, 0);
			Text_SetColor(text, TEXT_COLOR_SYSTEM_WHITE);
			Text_DrawString(text, GetStringFromIndex(GetCharacterData(bio->characterId)->nameTextId));

			if (bio->subtitle != NULL) {
				Text_SetCursor(text, 40);
				Text_SetColor(text, TEXT_COLOR_SYSTEM_GOLD);
				Text_DrawString(text, bio->subtitle);
			}
		}

		PutText(text, TILEMAP_LOCATED(gBG0TilemapBuffer, x, y + (i * 2)));
	}

	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void UpdateBiographyCursor(struct BiographyProc *proc)
{
	ShowSysHandCursor(56, 64 + ((proc->cursor - proc->top) * 16), 15, 0x800);
	UpdateMenuScrollBarConfig(10, proc->top * 16, CountBiographies(), BIO_VISIBLE_COUNT);
}

static void Biography_ResetCursor(struct BiographyProc *proc)
{
	proc->cursor = 0;
	proc->top = 0;
	proc->entry = 0;
	proc->page = 0;
}

static void Biography_Init(struct BiographyProc *proc)
{
	int i;

	proc->page = 0;

	StartBgm(SONG_LAUGHTER, NULL);

	ClearBg0Bg1();
	SetupBackgrounds(NULL);
	BG_Fill(BG_GetMapBuffer(0), 0);
	BG_Fill(BG_GetMapBuffer(1), 0);
	BG_Fill(BG_GetMapBuffer(2), 0);
	BG_Fill(BG_GetMapBuffer(3), 0);

	/* The cursor text shadow sits between the text and the frame */
	gLCDControlBuffer.bg0cnt.priority = 0;
	gLCDControlBuffer.bg1cnt.priority = 2;
	gLCDControlBuffer.bg2cnt.priority = 1;
	gLCDControlBuffer.bg3cnt.priority = 3;

	EndGreenText();
	ResetText();
	LoadUiFrameGraphics();
	LoadObjUIGfx();
	LoadIconPalettes(4);
	RestartMuralBackground();
	DrawUiFrame2(6, 7, 18, 12, 0);
	SetPrimaryHBlankHandler(PrepItemSupply_OnHBlank);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT);
	StartUiCursorHand(proc);
	ResetSysHandCursor(proc);
	DisplaySysHandCursorTextShadow(0x600, 1);

	for (i = 0; i < 8; i++)
		InitText(PrepItemSuppyTexts.th + i, 20);

	sub_8097668();
	BG_EnableSyncByMask(BG2_SYNC_BIT);

	PutDrawText(&PrepItemSuppyTexts.th[0], TILEMAP_LOCATED(gBG0TilemapBuffer, 6, 3), TEXT_COLOR_SYSTEM_WHITE, 15, 0, "Select a character to view");
	PutDrawText(&PrepItemSuppyTexts.th[1], TILEMAP_LOCATED(gBG0TilemapBuffer, 0, 0), TEXT_COLOR_SYSTEM_WHITE, 4, 0, "Biography");

	StartSysBrownBox(0x0, 0x7080, 0xF, 0xC00, 0x400, proc);
	EnableSysBrownBox(0, -40, -1, 1);
	StartMenuScrollBar(proc);
	PutMenuScrollBarAt(188, 64);
	InitMenuScrollBarImg(0x7A60, 2);

	UpdateBiographyCursor(proc);
	DrawBiographyList(proc, 8, 8);
}

static void Biography_Idle(struct BiographyProc *proc)
{
	u16 keys = gKeyStatusPtr->newKeys;
	int count = CountBiographies();
	bool moved = false;

	if ((keys & A_BUTTON) && count > 0) {
		Proc_Goto(proc, BIO_LABEL_EVENT);
		return;
	}

	if (keys & B_BUTTON) {
		SetPrimaryHBlankHandler(NULL);
		Proc_Goto(proc, BIO_LABEL_PRESS_B);
		return;
	}

	if ((keys & DPAD_UP) && proc->cursor > 0) {
		proc->cursor--;

		if (proc->cursor < proc->top)
			proc->top--;

		moved = true;
	}

	if ((keys & DPAD_DOWN) && proc->cursor + 1 < count) {
		proc->cursor++;

		if (proc->cursor >= proc->top + BIO_VISIBLE_COUNT)
			proc->top++;

		moved = true;
	}

	if (moved) {
		PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
		UpdateBiographyCursor(proc);
		DrawBiographyList(proc, 8, 8);
	}
}

/* ASMC: slot 2 = background, slot 3 = text ID, slot C = 0 when out of pages */
static void Biography_NextPageASMC(void)
{
	struct BiographyProc *proc = Proc_Find(ProcScr_MapMenuBiography);
	const struct BiographyPage *page;

	gEventSlots[EVT_SLOT_C] = 0;

	if (proc == NULL || proc->page >= BIO_PAGE_COUNT)
		return;

	page = &gCharacterBiographies[proc->entry].pages[proc->page];

	if (page->textId == 0)
		return;

	gEventSlots[EVT_SLOT_2] = page->backgroundId;
	gEventSlots[EVT_SLOT_3] = page->textId;
	gEventSlots[EVT_SLOT_C] = 1;
	proc->page++;
}

static const EventListScr EventScr_CharacterBiography[] = {
	EVBIT_MODIFY(0x4) // No skipping
	FADI(16)
LABEL(0)
	ASMC(Biography_NextPageASMC)
	BEQ(1, EVT_SLOT_C, EVT_SLOT_0)
	TEXT_CG_BIOGRAPHY
	GOTO(0)
LABEL(1)
	ENDA
};

static void Biography_StartEvent(struct BiographyProc *proc)
{
	const struct BiographyEntry *bio;

	proc->entry = proc->cursor;
	proc->page = 0;

	bio = &gCharacterBiographies[proc->entry];

	if (bio->songId == 0)
		StartBgm(NextRN_N(0x46), NULL);
	else
		StartBgm(bio->songId, NULL);

	CallEvent((const u16 *)EventScr_CharacterBiography, EV_EXEC_QUIET);
}

/* Clear the list screen before the CG event */
static void Biography_ClearScreen(void)
{
	SetPrimaryHBlankHandler(NULL);

	ClearBg0Bg1();
	BG_Fill(BG_GetMapBuffer(2), 0);
	BG_Fill(BG_GetMapBuffer(3), 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT | BG3_SYNC_BIT);

	ResetText();
	EndMenuScrollBar();
	EndSysBrownBox();
	EndMuralBackground_();
	HideSysHandCursor();
}

static void Biography_RestoreMapGraphics(void)
{
	EndMuralBackground_();
	ResetUnitSprites();
	RefreshBMapGraphics();
	EndMenuScrollBar();
	HideSysHandCursor();
	EndSysBrownBox();
	SyncUnitSpriteSheet();
	RefreshUnitSprites();
	InitPlayerPhaseInterface();
}

static void Biography_ForceBMapDispResume(void)
{
	while (gBmSt.gameGfxSemaphore)
		BMapDispResume();
}

const struct ProcCmd ProcScr_MapMenuBiography[] = {
	PROC_NAME("MapMenu_BIOGRAPHY"),
	PROC_YIELD,
	PROC_CALL(LockGame), // Hide the map cursor
	PROC_CALL_ARG(NewFadeOut, 16),
	PROC_WHILE(FadeOutExists),
	PROC_CALL(BMapDispSuspend), // Hide the unit map sprites
	PROC_YIELD,
	PROC_CALL(EndPlayerPhaseSideWindows),
	PROC_CALL(Biography_ResetCursor),

PROC_LABEL(BIO_LABEL_INIT),
	PROC_CALL(Biography_Init),
	PROC_CALL_ARG(NewFadeIn, 16),
	PROC_WHILE(FadeInExists),

PROC_LABEL(BIO_LABEL_IDLE),
	PROC_REPEAT(Biography_Idle),

PROC_LABEL(BIO_LABEL_EVENT),
	PROC_CALL(Biography_ClearScreen),
	PROC_CALL_ARG(NewFadeOut, 16),
	PROC_WHILE(FadeOutExists),
	PROC_CALL_ARG(NewFadeIn, 16),
	PROC_WHILE(FadeInExists),
	PROC_CALL(Biography_StartEvent),
	PROC_YIELD,
	PROC_WHILE(EventEngineExists),
	PROC_CALL_ARG(NewFadeOut, 16),
	PROC_WHILE(FadeOutExists),
	PROC_GOTO(BIO_LABEL_INIT),

PROC_LABEL(BIO_LABEL_PRESS_B),
	PROC_CALL_ARG(NewFadeOut, 16),
	PROC_WHILE(FadeOutExists),
	PROC_CALL(Biography_RestoreMapGraphics),
	PROC_CALL(SetAllUnitNotBackSprite),
	PROC_CALL(ResetUnitSpriteHover),
	PROC_CALL(Biography_ForceBMapDispResume),
	PROC_CALL(UnlockGame), // Show the map cursor again

PROC_LABEL(BIO_LABEL_END),
	PROC_END,
};

u8 MapMenu_BiographyCommand(struct MenuProc *menu, struct MenuItemProc *menuItem)
{
	(void)menu;
	(void)menuItem;

	Proc_Start(ProcScr_MapMenuBiography, PROC_TREE_3);

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}
