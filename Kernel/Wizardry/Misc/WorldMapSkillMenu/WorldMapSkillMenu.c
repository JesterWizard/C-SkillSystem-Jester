#include "common-chax.h"
#include "worldmap.h"
#include "kernel-lib.h"
#include "icon-rework.h"
#include "prep-skill.h"
#include "skill-system.h"
#include "utf8.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/custom-functions.h"

static struct WmSkillMenuProc *StartWMNodeSkillMenuCore(struct MenuProc *menuProc);

static int WmSkillMenu_GetUnitCount(void)
{
	return PrepGetUnitAmount();
}

static struct Unit *WmSkillMenu_GetUnit(int index)
{
	return GetUnitFromPrepList(index);
}

static int WmSkillMenu_GetSkillCount(struct Unit *unit)
{
	struct SkillList *list = GetUnitSkillList(unit);
	return list ? list->amt : 0;
}

static u16 WmSkillMenu_GetSkillId(struct Unit *unit, int index)
{
	struct SkillList *list = GetUnitSkillList(unit);

	if (list == NULL || index < 0 || index >= list->amt)
		return 0;

	return list->sid[index];
}

static int WmSkillMenu_GetVisibleSkillCount(struct Unit *unit)
{
	int skillCount = WmSkillMenu_GetSkillCount(unit);

	if (skillCount > WM_SKILL_ICON_MAX)
		skillCount = WM_SKILL_ICON_MAX;

	return skillCount;
}

static void WmSkillMenu_DrawUnitList(struct WmSkillMenuProc *proc)
{
	int i;
	int unitCount = proc->unitCount;

	ClearSprites();

	for (i = 0; i < WM_SKILL_VISIBLE_COUNT; ++i) {
		int unitIndex = proc->listTop + i;
		struct Unit *unit;

		if (unitIndex >= unitCount)
			continue;

		unit = WmSkillMenu_GetUnit(unitIndex);
		if (!UNIT_IS_VALID(unit))
			continue;

		PutUnitSprite(0, 20, 64 + (i * 16), unit);

		ClearText(&gPrepUnitTexts[i + 8]);
		PutDrawText(&gPrepUnitTexts[i + 8], TILEMAP_LOCATED(gBG0TilemapBuffer, 5, 8 + (i * 2)), TEXT_COLOR_SYSTEM_WHITE, 0, 0, GetStringFromIndex(unit->pCharacterData->nameTextId));
	}

	// R info sprite
	PutSprite(0, 200, 140, gObject_32x16, TILEREF(0xB, 0x0));
	PutSprite(0, 232, 140, gObject_8x16, TILEREF(0xF, 0x0));

	RefreshUnitSprites();
	SyncUnitSpriteSheet();
	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void WmSkillMenu_DrawSkillScreen(struct WmSkillMenuProc *proc)
{
	int i;
	struct Unit *unit = WmSkillMenu_GetUnit(proc->listCursor);
	int skillCount = WmSkillMenu_GetVisibleSkillCount(unit);

	TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 16, 6), 14, 8, 0);

	for (i = 0; i < skillCount; ++i) {
		u16 sid = WmSkillMenu_GetSkillId(unit, i);
		int tileX = 18 + ((i % WM_SKILL_ICON_COLS) * 2);
		int tileY = 8 + ((i / WM_SKILL_ICON_COLS) * 2);

		if (!sid)
			continue;

		DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, tileX, tileY), SKILL_ICON(sid), TILEREF(0, STATSCREEN_BGPAL_ITEMICONS + GetSkillIconPal(sid)));
	}

	BG_EnableSyncByMask(BG0_SYNC_BIT);
	proc->iconCount = skillCount;
	if (proc->iconCursor >= skillCount)
		proc->iconCursor = 0;
}

static void WmSkillMenu_DrawSelection(struct WmSkillMenuProc *proc)
{
	int listRow = proc->listCursor - proc->listTop;
	int iconX = 17 + ((proc->iconCursor % WM_SKILL_ICON_COLS) * 2);
	int iconY = 8 + ((proc->iconCursor / WM_SKILL_ICON_COLS) * 2);

	if (proc->mode == WM_SKILL_MODE_LIST)
		ShowSysHandCursor(16, 64 + (listRow * 16), 0x8, 0x800);
	else if (proc->iconCount > 0)
		ShowSysHandCursor((iconX * 8) + 4, (iconY * 8), 0x0, 0x800);
}

static bool WmSkillMenu_HelpBoxActive(void)
{
	return Proc_Find(gProcScr_HelpBox) != NULL;
}

static void WmSkillMenu_CloseHoverHelp(void)
{
	if (Proc_Find(gProcScr_HelpBox) != NULL)
		CloseHelpBox();
}

static void WmSkillMenu_OpenUnitHelp(struct WmSkillMenuProc *proc)
{
	struct Unit *unit = WmSkillMenu_GetUnit(proc->listCursor);

	if (!UNIT_IS_VALID(unit))
		return;

	LoadHelpBoxGfx(NULL, -1);
	StartHelpBox(17 * 8, 8 * 8, unit->pCharacterData->descTextId);
}

static void WmSkillMenu_OpenSkillHelp(struct WmSkillMenuProc *proc)
{
	struct Unit *unit = WmSkillMenu_GetUnit(proc->listCursor);
	u16 sid = WmSkillMenu_GetSkillId(unit, proc->iconCursor);

	if (!sid)
		return;

	LoadHelpBoxGfx(NULL, -1);
	StartHelpBox(17 * 8, 8 * 8, GetSkillDescMsg(sid));
}

static void WmSkillMenu_RefreshHelp(struct WmSkillMenuProc *proc)
{
	if (!WmSkillMenu_HelpBoxActive())
		return;


	if (proc->mode == WM_SKILL_MODE_LIST)
		WmSkillMenu_OpenUnitHelp(proc);
	else
		WmSkillMenu_OpenSkillHelp(proc);
}

static void WmSkillMenu_StartSelectedSkillScreen(struct WmSkillMenuProc *proc)
{
	StartWorldMapSelectSkillScreen((struct MenuProc *)proc, proc->startSkillScreen);
}

static void WmSkillMenu_InitGraphics(struct WmSkillMenuProc *proc)
{
	gSavedWorldMapUnitId = gGMData.units[0].id;
	HideGmUnit(0); // Hide world map unit, will need to restore later
	SetDispEnable(1, 1, 1, 1, 1);

	gLCDControlBuffer.dispcnt.mode = 0;
	SetupBackgrounds(NULL);

	CpuFastFill16(0, (void *)0x06010000, 0x5FE0);

    BG_Fill(gBG0TilemapBuffer, 0);
    BG_Fill(gBG1TilemapBuffer, 0);
    BG_Fill(gBG2TilemapBuffer, 0);
    BG_Fill(gBG3TilemapBuffer, 0);

	gLCDControlBuffer.bg0cnt.priority = 0;
	gLCDControlBuffer.bg1cnt.priority = 2;
	gLCDControlBuffer.bg2cnt.priority = 1;
	gLCDControlBuffer.bg3cnt.priority = 3;

	ResetFaces();
	ResetText();
	ResetUnitSprites();
	ResetIconGraphics_();
	LoadUiFrameGraphics();
	LoadObjUIGfx();
	LoadHelpBoxGfx((void *)0x06012000, -1);
	LoadIconPalettes(4);
	ApplyUnitSpritePalettes();
	RestartMuralBackground();

    StartMenuScrollBar(proc); 
    PutMenuScrollBarAt(14*8, 64); 
    InitMenuScrollBarImg(0x7A60, 4); 

    /* Initial configuration to set the bar size/pos */
    UpdateMenuScrollBarConfig(
        10,
		proc->listTop * 16,
        PrepGetUnitAmount(),
        5
    );

	DrawUiFrame2(1, 2, 13, 4, 2); // Top left
	DrawUiFrame2(1, 6, 13, 14, 0); // Bottom left

	DrawUiFrame2(16, 2, 13, 4, 2); // Top right
	DrawUiFrame2(16, 6, 13, 14, 0); // Bottom right

    StartSysBrownBox(0x0, 0x5800, 0x4, 0xc00, 0x400, proc);
    EnableSysBrownBox(0, -20, -1, 1);

    StartUiCursorHand(proc);
    ResetSysHandCursor(proc);
	DisplaySysHandCursorTextShadow(0x600, proc->mode == WM_SKILL_MODE_LIST);

	for (int i = 5; i < 20; ++i)
		InitText(&gPrepUnitTexts[i], 10);

	PutDrawText(&gPrepUnitTexts[5], TILEMAP_LOCATED(gBG0TilemapBuffer, 1, 0), TEXT_COLOR_SYSTEM_WHITE, 0, 0, "Manage Skills");
	PutDrawText(&gPrepUnitTexts[6], TILEMAP_LOCATED(gBG0TilemapBuffer, 6, 3), TEXT_COLOR_SYSTEM_GOLD, 0, 0, "Units");
	PutDrawText(&gPrepUnitTexts[7], TILEMAP_LOCATED(gBG0TilemapBuffer, 21, 3), TEXT_COLOR_SYSTEM_GOLD, 0, 0, "Skills");

	WmSkillMenu_DrawSkillScreen(proc);
	WmSkillMenu_DrawSelection(proc);

}

static void WmSkillMenu_ClampListCursor(struct WmSkillMenuProc *proc)
{
	if (proc->listCursor >= proc->unitCount)
		proc->listCursor = proc->unitCount ? proc->unitCount - 1 : 0;

	if (proc->listCursor < proc->listTop)
		proc->listTop = proc->listCursor;

	if (proc->listCursor >= proc->listTop + WM_SKILL_VISIBLE_COUNT)
		proc->listTop = proc->listCursor - (WM_SKILL_VISIBLE_COUNT - 1);
}

static void WmSkillMenu_Loop(struct WmSkillMenuProc *proc)
{
	int redrawList = 0;
	int redrawSkill = 0;

	WmSkillMenu_DrawUnitList(proc);

	if (gKeyStatusPtr->newKeys & B_BUTTON) {
		if (WmSkillMenu_HelpBoxActive()) {
			WmSkillMenu_CloseHoverHelp();
			return;
		}

		WmSkillMenu_CloseHoverHelp();
		EndAllProcChildren(proc);
		Proc_Goto(proc, 3);
		PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
		return;
	}

	if (proc->mode == WM_SKILL_MODE_LIST) {
		if (gKeyStatusPtr->newKeys & DPAD_UP) {
			if (proc->listCursor > 0) {
				proc->listCursor--;
				WmSkillMenu_ClampListCursor(proc);
				redrawList = 1;
				redrawSkill = 1;
				WmSkillMenu_RefreshHelp(proc);
				PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
			}
		}

		if (gKeyStatusPtr->newKeys & DPAD_DOWN) {
			if (proc->listCursor + 1 < proc->unitCount) {
				proc->listCursor++;
				WmSkillMenu_ClampListCursor(proc);
				redrawList = 1;
				redrawSkill = 1;
				WmSkillMenu_RefreshHelp(proc);
				PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
			}
		}

		if (gKeyStatusPtr->newKeys & DPAD_RIGHT) {
			bool helpBoxWasOpen = WmSkillMenu_HelpBoxActive();

			proc->mode = WM_SKILL_MODE_SKILL_SCREEN;
			proc->iconCursor = 0;
			redrawSkill = 1;
			PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);

			if (helpBoxWasOpen)
				WmSkillMenu_OpenSkillHelp(proc);
		}

		if (gKeyStatusPtr->newKeys & R_BUTTON)
			WmSkillMenu_OpenUnitHelp(proc);

		if (gKeyStatusPtr->newKeys & A_BUTTON) {
			bool helpBoxWasOpen = WmSkillMenu_HelpBoxActive();

			WmSkillMenu_CloseHoverHelp();
			proc->startSkillScreen = proc->listCursor;
			Proc_Goto(proc, 2);

			if (helpBoxWasOpen)
				WmSkillMenu_OpenUnitHelp(proc);
		}
	}
	else {
		if (proc->mode == WM_SKILL_MODE_SKILL_SCREEN) {
			int skillCount = WmSkillMenu_GetVisibleSkillCount(WmSkillMenu_GetUnit(proc->listCursor));

			if (gKeyStatusPtr->newKeys & DPAD_LEFT) {
				if (proc->iconCursor > 0) {
					proc->iconCursor--;
					PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
				}
				else {
					bool helpBoxWasOpen = WmSkillMenu_HelpBoxActive();

					proc->mode = WM_SKILL_MODE_LIST;
					redrawSkill = 1;
					PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);

					if (helpBoxWasOpen)
						WmSkillMenu_OpenUnitHelp(proc);
				}
			}

			if (gKeyStatusPtr->newKeys & DPAD_RIGHT) {
				if (skillCount > 0) {
					proc->iconCursor++;
					redrawSkill = 1;
					PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
					WmSkillMenu_RefreshHelp(proc);
				}
			}

			if (gKeyStatusPtr->newKeys & R_BUTTON) {
				if (WmSkillMenu_HelpBoxActive())
					WmSkillMenu_RefreshHelp(proc);
				else
					WmSkillMenu_OpenSkillHelp(proc);
			}

			if (gKeyStatusPtr->newKeys & DPAD_UP) {
				if (proc->iconCursor >= WM_SKILL_ICON_COLS) {
					proc->iconCursor -= WM_SKILL_ICON_COLS;
					redrawSkill = 1;
					PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
					WmSkillMenu_RefreshHelp(proc);
				}
			}
			if (gKeyStatusPtr->newKeys & DPAD_DOWN) {
				if (proc->iconCursor + WM_SKILL_ICON_COLS < skillCount) {
					proc->iconCursor += WM_SKILL_ICON_COLS;
					redrawSkill = 1;
					PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
					WmSkillMenu_RefreshHelp(proc);
				}
			}

			if (gKeyStatusPtr->newKeys & B_BUTTON) {
				bool helpBoxWasOpen = WmSkillMenu_HelpBoxActive();

				proc->mode = WM_SKILL_MODE_LIST;
				PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);

				if (helpBoxWasOpen)
					WmSkillMenu_OpenUnitHelp(proc);
				return;
			}

		}
	}

	if (redrawList) {
		UpdateMenuScrollBarConfig(
			10,
			proc->listTop * 16,
			proc->unitCount,
			WM_SKILL_VISIBLE_COUNT);
	}

	if (redrawSkill || proc->mode == WM_SKILL_MODE_SKILL_SCREEN)
		WmSkillMenu_DrawSkillScreen(proc);

	WmSkillMenu_DrawSelection(proc);
}

static void WmSkillMenu_OnEnd(struct WmSkillMenuProc *proc)
{
	Proc_EndEach(ProcScr_SlidingWallBg);
    WmSkillMenu_CloseHoverHelp();
    EndAllProcChildren(proc);
    gGMData.units[0].id = gSavedWorldMapUnitId;
    gGMData.sprite_disp = 1;
	gGMData.xCamera = gSavedWorldMapXCoordiate;
	gGMData.yCamera = gSavedWorldMapYCoordiate;
    ClearBg0Bg1();
	SetDefaultColorEffects();

 	returnToWorldMap_External();
}

const struct ProcCmd ProcScr_WMNodeSkillMenu[] = {
	PROC_NAME("WMNodeSkillMenu"),
	PROC_YIELD,
	PROC_SET_END_CB(WmSkillMenu_OnEnd),

PROC_LABEL(0),
	PROC_CALL(WmSkillMenu_InitGraphics),
	PROC_CALL_ARG(NewFadeIn, 0x10),
	PROC_WHILE(FadeInExists),

PROC_LABEL(1),
	PROC_REPEAT(WmSkillMenu_Loop),

PROC_LABEL(2),
	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_CALL(WmSkillMenu_StartSelectedSkillScreen),
	PROC_YIELD,
	PROC_GOTO(0),


PROC_LABEL(3),
    PROC_CALL_ARG(NewFadeOut, 0x10),
    PROC_WHILE(FadeOutExists),
	PROC_END,

PROC_LABEL(4),
	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_GOTO(0),
};

static struct WmSkillMenuProc *StartWMNodeSkillMenuCore(struct MenuProc *menuProc)
{
	struct WmSkillMenuProc *proc = Proc_StartBlocking(ProcScr_WMNodeSkillMenu, menuProc);

	MakePrepUnitList();
	proc->unitCount = WmSkillMenu_GetUnitCount();
	proc->listCursor = 0;
	proc->listTop = 0;
	proc->iconRows = 0;
	proc->mode = WM_SKILL_MODE_LIST;
	proc->iconCursor = 0;
	proc->iconCount = 0;
	proc->hoveredSkill = 0;
	proc->hoveredHelp = 0;

	return proc;
}

void StartWMNodeSkillMenu(struct MenuProc *menuProc)
{
	StartWMNodeSkillMenuCore(menuProc);
}

void StartWorldMapSkillMenu(struct MenuProc *menuProc)
{
	StartWMNodeSkillMenuCore(menuProc);
}

void StartWMNodeSkillMenuTransition(struct MenuProc *menuProc)
{
	// Reset camera position to (0, 0) to prevent weird scrolling behavior during the transition
	gSavedWorldMapXCoordiate = gGMData.xCamera;
	gSavedWorldMapYCoordiate = gGMData.yCamera;
	gGMData.xCamera = 0;
	gGMData.yCamera = 0;
    ProcPtr wmProc = Proc_Find(ProcScr_WorldMapMain);
	struct WmSkillMenuProc *proc = StartWMNodeSkillMenuCore(wmProc);
	Proc_Goto(proc, 4);
}