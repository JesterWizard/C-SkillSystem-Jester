#include "common-chax.h"
#include "worldmap.h"
#include "kernel-lib.h"
#include "prep-skill.h"
#include "skill-system.h"
#include "utf8.h"
#include "jester_headers/custom-structs.h"

enum {
	CUSTOM_PROC_PRESS_B = 0,
};

extern u8 gSavedWorldMapUnitId;

static void StartWMNodeSkillMenuCore(struct MenuProc *menuProc);
static void FillWorldMapBg3(void);

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
		int tileY = 6 + ((i / WM_SKILL_ICON_COLS) * 2);

		if (!sid)
			continue;

		DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, tileX, tileY), SKILL_ICON(sid), TILEREF(0, STATSCREEN_BGPAL_ITEMICONS + GetSkillIconPal(sid)));
		// PutDrawText(&gPrepUnitTexts[i + 13], TILEMAP_LOCATED(gBG0TilemapBuffer, tileX + 2, tileY), TEXT_COLOR_SYSTEM_WHITE, 0, 0, GetSkillNameStr(sid));
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
		ShowSysHandCursor((iconX * 8) + 4, (iconY * 8) - 16, 0x0, 0x800);
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

// extern u16 gUnknown_0201B458[]; // Magvel minimap
// extern u16 gUnknown_0201B758[]; // Unit minimug and name for worldmap
extern u16 gUnknown_0201BBD8[]; // Two sections of frame for worldmap
// extern u16 gUnknown_0201B864[]; // World map unit level graphic
// extern u16 gUnknown_0201B7DA[]; // World map unit minimug, name and level

static void WmSkillMenu_InitGraphics(struct WmSkillMenuProc *proc)
{
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
	ResetIconGraphics_();
	LoadUiFrameGraphics();
	LoadObjUIGfx();
	LoadHelpBoxGfx((void *)0x06012000, -1);
	LoadIconPalettes(4);
	ApplyUnitSpritePalettes();
	StartMuralBackgroundExt(proc, NULL, 0, 0, 0);

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

	DrawUiFrame2(1, 6, 13, 14, 0);
 	TileMap_CopyRect(gUnknown_0201BBD8, TILEMAP_LOCATED(gBG1TilemapBuffer, 1, 3), 13, 4);

	DrawUiFrame2(16, 4, 13, 16, 0);
	TileMap_CopyRect(gUnknown_0201BBD8, TILEMAP_LOCATED(gBG1TilemapBuffer, 16, 1), 13, 4);

    StartSysBrownBox(0x0, 0x5800, 0x4, 0xc00, 0x400, proc);
    EnableSysBrownBox(0, -20, -1, 1);

    StartUiCursorHand(proc);
    ResetSysHandCursor(proc);
	DisplaySysHandCursorTextShadow(0x600, proc->mode == WM_SKILL_MODE_LIST);

	for (int i = 5; i < 20; ++i)
		InitText(&gPrepUnitTexts[i], 10);

	PutDrawText(&gPrepUnitTexts[5], TILEMAP_LOCATED(gBG0TilemapBuffer, 1, 0), TEXT_COLOR_SYSTEM_WHITE, 0, 0, "Manage Skills");
	PutDrawText(&gPrepUnitTexts[6], TILEMAP_LOCATED(gBG0TilemapBuffer, 6, 4), TEXT_COLOR_SYSTEM_WHITE, 0, 0, "Units");
	PutDrawText(&gPrepUnitTexts[7], TILEMAP_LOCATED(gBG0TilemapBuffer, 21, 2), TEXT_COLOR_SYSTEM_WHITE, 0, 0, "Skills");

	WmSkillMenu_DrawSkillScreen(proc);
	WmSkillMenu_DrawSelection(proc);

}

static void WmSkillMenu_TransitionOpen(struct WmSkillMenuTransitionProc *transition)
{
	StartWMNodeSkillMenuCore((struct MenuProc *)transition->proc_parent);
}

const struct ProcCmd ProcScr_WMNodeSkillMenuTransition[] = {
	PROC_NAME("WMNodeSkillMenuTransition"),
	PROC_YIELD,
	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_CALL(WmSkillMenu_TransitionOpen),
	PROC_END,
};

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

	WmSkillMenu_DrawUnitList(proc);

	if (gKeyStatusPtr->newKeys & B_BUTTON) {
		if (WmSkillMenu_HelpBoxActive()) {
			WmSkillMenu_CloseHoverHelp();
			return;
		}

		WmSkillMenu_CloseHoverHelp();
		Proc_Goto(proc, CUSTOM_PROC_PRESS_B);
		PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
		return;
	}

	if (proc->mode == WM_SKILL_MODE_LIST) {
		if (gKeyStatusPtr->newKeys & DPAD_UP) {
			if (proc->listCursor > 0) {
				proc->listCursor--;
				WmSkillMenu_ClampListCursor(proc);
				redrawList = 1;
				WmSkillMenu_RefreshHelp(proc);
				PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
			}
		}

		if (gKeyStatusPtr->newKeys & DPAD_DOWN) {
			if (proc->listCursor + 1 < proc->unitCount) {
				proc->listCursor++;
				WmSkillMenu_ClampListCursor(proc);
				redrawList = 1;
				WmSkillMenu_RefreshHelp(proc);
				PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
			}
		}

		if (gKeyStatusPtr->newKeys & DPAD_RIGHT) {
			int skillCount = WmSkillMenu_GetVisibleSkillCount(WmSkillMenu_GetUnit(proc->listCursor));

			proc->mode = WM_SKILL_MODE_SKILL_SCREEN;
			proc->iconCursor = 0;
			PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);

			if (skillCount > 0) {
				if (WmSkillMenu_HelpBoxActive())
					WmSkillMenu_RefreshHelp(proc);
				else
					WmSkillMenu_OpenSkillHelp(proc);
			}
		}

		if (gKeyStatusPtr->newKeys & R_BUTTON)
			WmSkillMenu_OpenUnitHelp(proc);
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
					proc->mode = WM_SKILL_MODE_LIST;
					WmSkillMenu_CloseHoverHelp();
					PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
					WmSkillMenu_OpenUnitHelp(proc);
				}
			}

			if (gKeyStatusPtr->newKeys & DPAD_RIGHT) {
				if (skillCount > 0) {
					proc->iconCursor++;
					PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
					WmSkillMenu_RefreshHelp(proc);
				}
			}

			if (gKeyStatusPtr->newKeys & DPAD_UP) {
				if (proc->iconCursor >= WM_SKILL_ICON_COLS) {
					proc->iconCursor -= WM_SKILL_ICON_COLS;
					PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
					WmSkillMenu_RefreshHelp(proc);
				}
			}
			if (gKeyStatusPtr->newKeys & DPAD_DOWN) {
				if (proc->iconCursor + WM_SKILL_ICON_COLS < skillCount) {
					proc->iconCursor += WM_SKILL_ICON_COLS;
					PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
					WmSkillMenu_RefreshHelp(proc);
				}
			}

			if (gKeyStatusPtr->newKeys & B_BUTTON) {
				proc->mode = WM_SKILL_MODE_LIST;
				WmSkillMenu_CloseHoverHelp();
				PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
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

	if (proc->mode != WM_SKILL_MODE_LIST)
		WmSkillMenu_DrawSkillScreen(proc);

	WmSkillMenu_DrawSelection(proc);
}

extern u8 gUnknown_08A83364[];
extern u16 gUnknown_08A95FE4[];
extern u16 gUnknown_08A96064[];

void FillWorldMapBg3(void)
{
    int i;

    // Clear BG3 tilemap buffer
    BG_Fill(gBG3TilemapBuffer, 0);

    // Copy the full worldmap tile graphics into BG3 VRAM
    for (i = 0; i < 0x20; i++)
    {
        CpuFastCopy(
            gUnknown_08A83364 + (i * 0x780),
            (void *)(0x06008000 + (i * 0x400)),
            0x400);
    }

    // Copy the palettemap into the BG3 map buffer
    Decompress(gUnknown_08A96064, gUnknown_020087A0);

    // If you want to apply palettes like the original worldmap init
    ApplyPalettes(gUnknown_08A95FE4, 9, 4);
    EnablePaletteSync();

    // Reset BG3 position
    BG_SetPosition(BG_3, 0, 0);
    BG_EnableSyncByMask(BG3_SYNC_BIT);
}

static void WmSkillMenu_OnEnd(struct WmSkillMenuProc *proc)
{
	ProcPtr wmProc;

    WmSkillMenu_CloseHoverHelp();
    EndAllProcChildren(proc);
    FillWorldMapBg3();
    gGMData.units[0].id = gSavedWorldMapUnitId;
    gGMData.sprite_disp = 1;
    ClearBg0Bg1();
	SetDefaultColorEffects();

	wmProc = Proc_Find(ProcScr_WorldMapMain);
	if (wmProc != NULL)
		NewFadeIn(0x10, wmProc);
}

const struct ProcCmd ProcScr_WMNodeSkillMenu[] = {
	PROC_NAME("WMNodeSkillMenu"),
	PROC_YIELD,
	PROC_SET_END_CB(WmSkillMenu_OnEnd),
	PROC_CALL(WmSkillMenu_InitGraphics),
	PROC_CALL_ARG(NewFadeIn, 0x10),
	PROC_WHILE(FadeInExists),
	PROC_REPEAT(WmSkillMenu_Loop),
PROC_LABEL(CUSTOM_PROC_PRESS_B),
    PROC_CALL_ARG(NewFadeOut, 0x10),
    PROC_WHILE(FadeOutExists),
	PROC_END,
};

static void StartWMNodeSkillMenuCore(struct MenuProc *menuProc)
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
}

void StartWMNodeSkillMenu(struct MenuProc *menuProc)
{
	StartWMNodeSkillMenuCore(menuProc);
}

void StartWMNodeSkillMenuTransition(struct MenuProc *menuProc)
{
	// Reset camera position to (0, 0) to prevent weird scrolling behavior during the transition
	// Will need to store the original camera position and restore it after the transition if we want to support opening the skill menu while the camera is scrolled away from the origin
	gGMData.xCamera = 0;
	gGMData.yCamera = 0;
	gSavedWorldMapUnitId = gGMData.units[0].id;
	HideGmUnit(0); // Hide world map unit, will need to restore later
    ProcPtr wmProc = Proc_Find(ProcScr_WorldMapMain);
	Proc_StartBlocking(ProcScr_WMNodeSkillMenuTransition, wmProc);
}