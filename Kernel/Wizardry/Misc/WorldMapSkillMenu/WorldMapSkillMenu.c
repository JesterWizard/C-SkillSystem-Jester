#include "common-chax.h"
#include "worldmap.h"
#include "kernel-lib.h"
#include "prep-skill.h"
#include "skill-system.h"
#include "utf8.h"
#include "jester_headers/custom-structs.h"

extern u8 gSavedWorldMapUnitId;

#define WM_SKILL_VISIBLE_COUNT 5
#define WM_SKILL_ICON_COLS 4
#define WM_SKILL_ICON_MAX 12
#define SKILL_ICON(sid) ((5 << 8) + (sid))
struct WmSkillMenuProc {
	PROC_HEADER;
	/* 29 */ u8 listCursor;
	/* 2A */ u8 listTop;
	/* 2B */ u8 mode;
	/* 2C */ u8 iconCursor;
	/* 2D */ u8 iconCount;
	/* 2E */ u8 iconRows;
	/* 2F */ u8 _pad;
	/* 30 */ u8 unitCount;
	/* 31 */ u8 hoveredSkill;
	/* 32 */ u16 hoveredHelp;
};

enum {
	WM_SKILL_MODE_LIST = 0,
	WM_SKILL_MODE_ICONS = 1,
};

struct WmSkillMenuTransitionProc {
	PROC_HEADER;
};

static void StartWMNodeSkillMenuCore(struct MenuProc *menuProc);

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

static void WmSkillMenu_ClearTextArea(void)
{
	TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 0, 0), 30, 20, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT);
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

		PutUnitSprite(0, 20, 60 + (i * 16), unit);

		ClearText(&gPrepUnitTexts[i + 8]);
		PutDrawText(&gPrepUnitTexts[i + 8], TILEMAP_LOCATED(gBG0TilemapBuffer, 5, 8 + (i * 2)), TEXT_COLOR_SYSTEM_WHITE, 0, 0, GetStringFromIndex(unit->pCharacterData->nameTextId));
	}

	RefreshUnitSprites();
	SyncUnitSpriteSheet();
	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void WmSkillMenu_DrawSkillIcons(struct WmSkillMenuProc *proc)
{
	int i;
	struct Unit *unit = WmSkillMenu_GetUnit(proc->listCursor);
	int skillCount = WmSkillMenu_GetVisibleSkillCount(unit);

	TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 16, 6), 14, 8, 0);

	for (i = 0; i < skillCount; ++i) {
		u16 sid = WmSkillMenu_GetSkillId(unit, i);
		int tileX = 15 + ((i % WM_SKILL_ICON_COLS) * 2);
		int tileY = 9 + ((i / WM_SKILL_ICON_COLS) * 2);

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

	ShowSysHandCursor(16, 48 + (listRow * 16), 0x8, 0x800);
	if (proc->mode == WM_SKILL_MODE_ICONS)
		ShowSysHandCursor((iconX * 8) - 8, (iconY * 8), 0x8, 0x800);
}

static bool WmSkillMenu_HelpBoxActive(void)
{
	return Proc_Find(gProcScr_HelpBox) != NULL;
}

static void WmSkillMenu_OpenHoverHelp(struct WmSkillMenuProc *proc)
{
	struct Unit *unit = WmSkillMenu_GetUnit(proc->listCursor);
	u16 sid = WmSkillMenu_GetSkillId(unit, proc->iconCursor);

	if (!sid)
		return;

	LoadHelpBoxGfx(NULL, -1);
	StartHelpBox(17 * 8, 8 * 8, GetSkillDescMsg(sid));
	proc->hoveredSkill = sid;
	proc->hoveredHelp = GetSkillDescMsg(sid);
}

static void WmSkillMenu_CloseHoverHelp(void)
{
	if (Proc_Find(gProcScr_HelpBox) != NULL)
		CloseHelpBox();
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
    PutMenuScrollBarAt(2, 66); 
    InitMenuScrollBarImg(0x7A60, 2); 
	// StartMenuScrollBarExt(proc, 224, 11, 0x200, 4);

    /* Initial configuration to set the bar size/pos */
    UpdateMenuScrollBarConfig(
        PrepGetUnitAmount(),
        gTopVisibleListIndex * 16,
        PrepGetUnitAmount(),
        5
    );

    /* Display the transparent black banner behind the text */
    // SetPrimaryHBlankHandler(PrepItemSupply_OnHBlank);

	DrawUiFrame2(1, 6, 13, 13, 0);
 	TileMap_CopyRect(gUnknown_0201BBD8, TILEMAP_LOCATED(gBG1TilemapBuffer, 1, 3), 13, 4);

	DrawUiFrame2(16, 6, 13, 13, 0);
 	TileMap_CopyRect(gUnknown_0201BBD8, TILEMAP_LOCATED(gBG1TilemapBuffer, 16, 3), 13, 4);

    StartSysBrownBox(0x0, 0x7080, 0xf, 0xc00, 0x400, proc);
    EnableSysBrownBox(0, -20, -1, 1);

	InitText(&gPrepUnitTexts[7], 10);
	InitText(&gPrepUnitTexts[8], 10);
	InitText(&gPrepUnitTexts[9], 10);
	InitText(&gPrepUnitTexts[10], 10);
	InitText(&gPrepUnitTexts[11], 10);
	InitText(&gPrepUnitTexts[12], 10);

	PutDrawText(&gPrepUnitTexts[8], TILEMAP_LOCATED(gBG0TilemapBuffer, 4, 2), TEXT_COLOR_SYSTEM_WHITE, 0, 0, "Skill Change");
	PutDrawText(&gPrepUnitTexts[9], TILEMAP_LOCATED(gBG0TilemapBuffer, 4, 4), TEXT_COLOR_SYSTEM_GOLD, 0, 0, "Units");
	PutDrawText(&gPrepUnitTexts[10], TILEMAP_LOCATED(gBG0TilemapBuffer, 17, 4), TEXT_COLOR_SYSTEM_GOLD, 0, 0, "Skills");

	WmSkillMenu_ClearTextArea();
	WmSkillMenu_DrawSkillIcons(proc);
	WmSkillMenu_DrawSelection(proc);
}

static void WmSkillMenu_InitBexpStyleGraphics(struct WmSkillMenuProc *proc)
{
	NoCashGBAPrint("WM skill menu: init BEXP-style gfx\n");
	gGMData.sprite_disp = 0;
	gLCDControlBuffer.dispcnt.mode = 0;
	gLCDControlBuffer.dispcnt.bg3_on = 1;
	SetupBackgrounds(NULL);

	BG_Fill(BG_GetMapBuffer(0), 0);
	BG_Fill(BG_GetMapBuffer(1), 0);
	BG_Fill(BG_GetMapBuffer(2), 0);

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

	BG_SetPosition(0, 0, 0);
	BG_SetPosition(1, 0, 0);
	BG_SetPosition(3, 0, 0);

	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG3_SYNC_BIT);

	DrawUiFrame2(1, 7, 12, 13, 0);
	DrawUiFrame2(13, 7, 16, 6, 0);
	DrawUiFrame2(13, 13, 16, 7, 0);

	gLCDControlBuffer.dispcnt.win0_on = 1;
	gLCDControlBuffer.dispcnt.win1_on = 0;
	gLCDControlBuffer.dispcnt.objWin_on = 0;

	gLCDControlBuffer.win0_left = 128;
	gLCDControlBuffer.win0_top = 40;
	gLCDControlBuffer.win0_right = 224;
	gLCDControlBuffer.win0_bottom = 152;

	gLCDControlBuffer.wincnt.win0_enableBg0 = 1;
	gLCDControlBuffer.wincnt.win0_enableBg1 = 1;
	gLCDControlBuffer.wincnt.win0_enableBg2 = 1;
	gLCDControlBuffer.wincnt.win0_enableBg3 = 1;
	gLCDControlBuffer.wincnt.win0_enableObj = 1;

	gLCDControlBuffer.wincnt.wout_enableBg0 = 1;
	gLCDControlBuffer.wincnt.wout_enableBg1 = 1;
	gLCDControlBuffer.wincnt.wout_enableBg2 = 0;
	gLCDControlBuffer.wincnt.wout_enableBg3 = 1;
	gLCDControlBuffer.wincnt.wout_enableObj = 1;

	SetBlendConfig(0, 0, 0, 8);

	WmSkillMenu_ClearTextArea();
	WmSkillMenu_DrawUnitList(proc);
	WmSkillMenu_DrawSkillIcons(proc);
	WmSkillMenu_DrawSelection(proc);
	NoCashGBAPrint("WM skill menu: BEXP-style gfx ready\n");
}

static void WmSkillMenu_TransitionOpen(struct WmSkillMenuTransitionProc *transition)
{
	NoCashGBAPrint("WM skill menu: transition open -> start screen\n");
	StartWMNodeSkillMenuCore((struct MenuProc *)transition->proc_parent);
}

const struct ProcCmd ProcScr_WMNodeSkillMenuTransition[] = {
	PROC_NAME("WMNodeSkillMenuTransition"),
	PROC_YIELD,
	// PROC_CALL_ARG(NewFadeOut, 0x10),
	// PROC_WHILE(FadeOutExists),
	PROC_CALL(WmSkillMenu_TransitionOpen),
	PROC_CALL(WmSkillMenu_InitBexpStyleGraphics),
	// PROC_CALL_ARG(NewFadeIn, 0x10),
	// PROC_WHILE(FadeInExists),
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
	int redrawIcons = 0;
	int redrawList = 0;
	struct Unit *unit = WmSkillMenu_GetUnit(proc->listCursor);
	int skillCount = WmSkillMenu_GetVisibleSkillCount(unit);

	WmSkillMenu_DrawUnitList(proc);

	if (gKeyStatusPtr->newKeys & B_BUTTON) {
		if (WmSkillMenu_HelpBoxActive()) {
			WmSkillMenu_CloseHoverHelp();
			return;
		}

		WmSkillMenu_CloseHoverHelp();
		Proc_Break(proc);
		PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
		return;
	}

	if (proc->mode == WM_SKILL_MODE_LIST) {
		if (gKeyStatusPtr->newKeys & DPAD_UP) {
			if (proc->listCursor > 0) {
				proc->listCursor--;
				WmSkillMenu_ClampListCursor(proc);
				redrawList = 1;
				redrawIcons = 1;
				PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
			}
		}

		if (gKeyStatusPtr->newKeys & DPAD_DOWN) {
			if (proc->listCursor + 1 < proc->unitCount) {
				proc->listCursor++;
				WmSkillMenu_ClampListCursor(proc);
				redrawList = 1;
				redrawIcons = 1;
				PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
			}
		}

		if (gKeyStatusPtr->newKeys & A_BUTTON) {
			proc->mode = WM_SKILL_MODE_ICONS;
			proc->iconCursor = 0;
			WmSkillMenu_CloseHoverHelp();
			WmSkillMenu_OpenHoverHelp(proc);
			PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
		}
	}
	else {
		if (gKeyStatusPtr->newKeys & DPAD_LEFT) {
			if (proc->iconCursor > 0) {
				WmSkillMenu_CloseHoverHelp();
				proc->iconCursor--;
				WmSkillMenu_OpenHoverHelp(proc);
				PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
			}
		}

		if (gKeyStatusPtr->newKeys & DPAD_RIGHT) {
			if (proc->iconCursor + 1 < skillCount) {
				WmSkillMenu_CloseHoverHelp();
				proc->iconCursor++;
				WmSkillMenu_OpenHoverHelp(proc);
				PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
			}
		}

		if (gKeyStatusPtr->newKeys & DPAD_UP) {
			if (proc->iconCursor >= WM_SKILL_ICON_COLS) {
				WmSkillMenu_CloseHoverHelp();
				proc->iconCursor -= WM_SKILL_ICON_COLS;
				WmSkillMenu_OpenHoverHelp(proc);
				PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
			}
		}

		if (gKeyStatusPtr->newKeys & DPAD_DOWN) {
			if (proc->iconCursor + WM_SKILL_ICON_COLS < skillCount) {
				WmSkillMenu_CloseHoverHelp();
				proc->iconCursor += WM_SKILL_ICON_COLS;
				WmSkillMenu_OpenHoverHelp(proc);
				PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
			}
		}

		if (gKeyStatusPtr->newKeys & A_BUTTON) {
			WmSkillMenu_OpenHoverHelp(proc);
		}

		if (gKeyStatusPtr->newKeys & R_BUTTON) {
			proc->mode = WM_SKILL_MODE_LIST;
			WmSkillMenu_CloseHoverHelp();
			PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
		}
	}

	if (redrawList) {
		WmSkillMenu_DrawUnitList(proc);
	}

	if (redrawIcons) {
		unit = WmSkillMenu_GetUnit(proc->listCursor);
		skillCount = WmSkillMenu_GetVisibleSkillCount(unit);
		WmSkillMenu_DrawSkillIcons(proc);
		if (skillCount == 0)
			WmSkillMenu_CloseHoverHelp();
		else if (proc->mode == WM_SKILL_MODE_ICONS)
			WmSkillMenu_OpenHoverHelp(proc);
	}

	WmSkillMenu_DrawSelection(proc);
}

static void WmSkillMenu_OnEnd(struct WmSkillMenuProc *proc)
{
    WmSkillMenu_CloseHoverHelp();
    EndAllProcChildren(proc);
    gGMData.units[0].id = gSavedWorldMapUnitId;
    gGMData.sprite_disp = 1;
    ClearBg0Bg1();
}

const struct ProcCmd ProcScr_WMNodeSkillMenu[] = {
	PROC_NAME("WMNodeSkillMenu"),
	PROC_YIELD,
	PROC_SET_END_CB(WmSkillMenu_OnEnd),
	PROC_CALL(WmSkillMenu_InitGraphics),
	PROC_REPEAT(WmSkillMenu_Loop),
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
	ShowSysHandCursor(16, 48, 0x8, 0x800);

	NewFadeIn(0x10, proc);
}

void StartWMNodeSkillMenu(struct MenuProc *menuProc)
{
	NoCashGBAPrint("WM skill menu: start screen proc\n");
	StartWMNodeSkillMenuCore(menuProc);
}

void StartWMNodeSkillMenuTransition(struct MenuProc *menuProc)
{
	// Reset camera position to (0, 0) to prevent weird scrolling behavior during the transition
	// Will need to store the original camera position and restore it after the transition if we want to support opening the skill menu while the camera is scrolled away from the origin
	gGMData.xCamera = 0;
	gGMData.yCamera = 0;
	HideGmUnit(0); // Hide world map unit, will need to restpre later
	NoCashGBAPrint("WM skill menu: start transition\n");
    ProcPtr wmProc = Proc_Find(ProcScr_WorldMapMain);
	Proc_StartBlocking(ProcScr_WMNodeSkillMenuTransition, wmProc);
}