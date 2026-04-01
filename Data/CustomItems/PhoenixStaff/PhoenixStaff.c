#include "common-chax.h"
#include "item-sys.h"
#include "battle-system.h"
#include "constants/items.h"
#include "constants/texts.h"
#include "jester_headers/custom-arrays.h"
#include "jester_headers/custom-functions.h"
#include "bmfx.h"
#include "mapanim.h"
#include "popup.h"
#include "bmitemuse.h"
#include "bmtarget.h"
#include "bm.h"
#include "bmudisp.h"
#include "prepscreen.h"
#include "proc.h"
#include "sysutil.h"

enum {
	PHOENIX_STATE_INIT = 0,
	PHOENIX_STATE_IDLE = 1,
	PHOENIX_STATE_END = 2,
};

#define PHOENIX_VISIBLE_COUNT 5
#define PHOENIX_FRAME_X 13
#define PHOENIX_FRAME_Y 4
#define PHOENIX_LIST_X 16
#define PHOENIX_LIST_Y 5
#define PHOENIX_TEXT_X 16
#define PHOENIX_CURSOR_X 104
#define PHOENIX_SCROLLBAR_X 200
#define PHOENIX_SCROLLBAR_Y 40

static const int sPhoenixAdjTileOffsets[4][2] = {
	{-1, 0},
	{ 1, 0},
	{ 0,-1},
	{ 0, 1},
};

struct PhoenixStaffProc {
	PROC_HEADER;
	u8 cursor;
	u8 scrollTop;
	u8 unitCount;
	u8 selectedUnit;
	u8 placeX;
	u8 placeY;
	u8 unitIds[ARRAY_COUNT(gDeadUnits)];
	struct Text texts[8];
};

extern const struct ProcCmd ProcScr_PhoenixStaff[];
extern const struct ProcCmd ProcScr_PhoenixRevive[];
static u8 PhoenixStaff_OnSelectTarget(ProcPtr proc, struct SelectTarget *target);
extern struct PopupInstruction const PhoenixStaffRevivedPopup[];

static bool PhoenixStaff_IsValidDeadUnit(struct Unit *unit)
{
	return UNIT_IS_VALID(unit) && (UNIT_FACTION(unit) == FACTION_BLUE) && (unit->state & US_DEAD);
}

static bool PhoenixStaff_CanUnitBePlacedAt(struct Unit *unit, int x, int y)
{
	if (!PhoenixStaff_IsValidDeadUnit(unit))
		return false;

	return Generic_CanUnitBeOnPos(unit, x, y, -1, -1);
}

static bool PhoenixStaff_CanAnyDeadUnitBePlacedAt(int x, int y)
{
	for (int i = (int)ARRAY_COUNT(gDeadUnits) - 1; i >= 0; i--) {
		u8 unitId = gDeadUnits[i];
		struct Unit *unit;

		if (unitId == 0)
			continue;

		unit = GetUnit(unitId);
		if (!PhoenixStaff_CanUnitBePlacedAt(unit, x, y))
			continue;

		return true;
	}

	return false;
}

static void PhoenixStaff_ClearUi(void)
{
	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
	ResetUnitSprites();
	RefreshUnitSprites();
	SyncUnitSpriteSheet();
	HideSysHandCursor();
	EndMenuScrollBar();
}

static void PhoenixStaff_StartMenu(ProcPtr proc)
{
	(void)proc;
	Proc_Start(ProcScr_PhoenixStaff, PROC_TREE_3);
}

static bool PhoenixStaff_MenuRunning(ProcPtr proc)
{
	(void)proc;
	return Proc_Exists(ProcScr_PhoenixStaff);
}

static void PhoenixStaff_StartRevive(ProcPtr proc)
{
	if (gActionData.targetIndex == 0)
		return;

	Proc_StartBlocking(ProcScr_PhoenixRevive, proc);
}

static void MakeTargetListForPhoenix(struct Unit *subject)
{
	gSubjectUnit = subject;
	InitTargets(subject->xPos, subject->yPos);
	BmMapFill(gBmMapRange, 0);

	for (int i = 0; i < 4; i++) {
		int x = subject->xPos + sPhoenixAdjTileOffsets[i][0];
		int y = subject->yPos + sPhoenixAdjTileOffsets[i][1];

		if (x < 0 || y < 0 || x >= gBmMapSize.x || y >= gBmMapSize.y)
			continue;

		if (PhoenixStaff_CanAnyDeadUnitBePlacedAt(x, y))
			AddTarget(x, y, subject->index, 0);
	}
}

static void PhoenixStaff_InitRoster(struct PhoenixStaffProc *proc)
{
	int count = 0;
	int placeX = gActionData.xOther;
	int placeY = gActionData.yOther;

	proc->cursor = 0;
	proc->scrollTop = 0;
	proc->selectedUnit = 0;
	proc->unitCount = 0;
	proc->placeX = placeX;
	proc->placeY = placeY;

	for (int i = (int)ARRAY_COUNT(gDeadUnits) - 1; i >= 0; i--) {
		u8 unitId = gDeadUnits[i];
		struct Unit *unit;
		bool duplicate = false;

		if (unitId == 0)
			continue;

		for (int j = 0; j < count; j++) {
			if (proc->unitIds[j] == unitId) {
				duplicate = true;
				break;
			}
		}

		if (duplicate)
			continue;

		unit = GetUnit(unitId);
		if (!PhoenixStaff_CanUnitBePlacedAt(unit, proc->placeX, proc->placeY))
			continue;

		proc->unitIds[count++] = unitId;
	}

	proc->unitCount = count;

	for (int i = 0; i < 8; i++)
		InitText(&proc->texts[i], 20);
}

static void PhoenixStaff_DrawRoster(struct PhoenixStaffProc *proc)
{
	ClearSprites();

	for (int i = 0; i < PHOENIX_VISIBLE_COUNT; i++) {
		int index = proc->scrollTop + i;
		int rowY = PHOENIX_LIST_Y + (i * 2);

		ClearText(&proc->texts[i]);
		TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, PHOENIX_LIST_X, rowY), 12, 2, 0);

		if (index >= proc->unitCount)
			continue;

		{
			struct Unit *unit = GetUnit(proc->unitIds[index]);
			int color = (index == proc->cursor) ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_WHITE;

			if (!PhoenixStaff_IsValidDeadUnit(unit))
				continue;

			Text_SetColor(&proc->texts[i], color);
			PutDrawText(
				&proc->texts[i],
				TILEMAP_LOCATED(gBG0TilemapBuffer, PHOENIX_TEXT_X, rowY),
				color,
				0,
				0,
				GetStringFromIndex(unit->pCharacterData->nameTextId));

			PutUnitSprite(0, 24, (rowY * 8) + 4, unit);
		}
	}

	RefreshUnitSprites();
	SyncUnitSpriteSheet();

	if (proc->unitCount > PHOENIX_VISIBLE_COUNT) {
		UpdateMenuScrollBarConfig(
			10,
			proc->scrollTop * 16,
			proc->unitCount,
			PHOENIX_VISIBLE_COUNT);
	}

	if ((proc->cursor >= proc->scrollTop) && (proc->cursor < proc->scrollTop + PHOENIX_VISIBLE_COUNT))
		ShowSysHandCursor(PHOENIX_CURSOR_X, PHOENIX_SCROLLBAR_Y + ((proc->cursor - proc->scrollTop) * 16), 10, 0x800);

	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void PhoenixStaff_InitUi(struct PhoenixStaffProc *proc)
{
	gLCDControlBuffer.dispcnt.mode = 0;
	// SetupBackgrounds(NULL);
	// BG_Fill(BG_GetMapBuffer(0), 0);
	// BG_Fill(BG_GetMapBuffer(1), 0);

	// gLCDControlBuffer.bg0cnt.priority = 0;
	// gLCDControlBuffer.bg1cnt.priority = 2;
	// gLCDControlBuffer.bg2cnt.priority = 1;
	// gLCDControlBuffer.bg3cnt.priority = 3;

	ResetText();
	LoadUiFrameGraphics();
	LoadObjUIGfx();
	ApplyUnitSpritePalettes();
	StartUiCursorHand(proc);
	ResetSysHandCursor(proc);
	DisplaySysHandCursorTextShadow(0x600, 1);

	DrawUiFrame2(PHOENIX_FRAME_X, PHOENIX_FRAME_Y, 12, 12, 0);
	StartMenuScrollBar(proc);
	PutMenuScrollBarAt(PHOENIX_SCROLLBAR_X, PHOENIX_SCROLLBAR_Y);
	InitMenuScrollBarImg(0x7A60, 2);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
	PhoenixStaff_DrawRoster(proc);
}

static void PhoenixStaff_EndUi(struct PhoenixStaffProc *proc)
{
	(void)proc;
	PhoenixStaff_ClearUi();
	ResetText();
}

static void PhoenixStaff_HandleSelect(struct PhoenixStaffProc *proc)
{
	if (proc->unitCount == 0)
		return;

	proc->selectedUnit = proc->unitIds[proc->cursor];
	gActionData.targetIndex = proc->selectedUnit;
	PhoenixStaff_EndUi(proc);
	Proc_Goto(proc, PHOENIX_STATE_END);
}

static void PhoenixStaff_HandleCancel(struct PhoenixStaffProc *proc)
{
	gActionData.unitActionType = 0;
	gActionData.unk08 = 0;
	gActionData.targetIndex = 0;
	PhoenixStaff_EndUi(proc);
	Proc_Goto(proc, PHOENIX_STATE_END);
}

static u8 PhoenixStaff_OnSelectTarget(ProcPtr proc, struct SelectTarget *target)
{
	gActionData.subjectIndex = gActiveUnit->index;
	gActionData.xOther = target->x;
	gActionData.yOther = target->y;
	gActionData.unk08 = ITEM_STAFF_PHOENIX;

	return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A |
		TARGETSELECTION_ACTION_CLEARBGS;
}

static void PhoenixStaff_OnInit(struct PhoenixStaffProc *proc)
{
	PhoenixStaff_InitRoster(proc);

	if (proc->unitCount == 0) {
		PhoenixStaff_HandleCancel(proc);
		return;
	}

	PhoenixStaff_InitUi(proc);
}

static void PhoenixStaff_OnLoop(struct PhoenixStaffProc *proc)
{
	bool scrolled = false;

	if (proc->unitCount == 0) {
		Proc_Goto(proc, PHOENIX_STATE_END);
		return;
	}

	if (gKeyStatusPtr->newKeys & B_BUTTON) {
		PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
		PhoenixStaff_HandleCancel(proc);
		return;
	}

	if (gKeyStatusPtr->newKeys & A_BUTTON) {
		PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
		PhoenixStaff_HandleSelect(proc);
		return;
	}

	if (gKeyStatusPtr->repeatedKeys & DPAD_UP) {
		if (proc->cursor > 0) {
			proc->cursor--;
			scrolled = true;
		}
	}

	if (gKeyStatusPtr->repeatedKeys & DPAD_DOWN) {
		if (proc->cursor < proc->unitCount - 1) {
			proc->cursor++;
			scrolled = true;
		}
	}

	if (proc->cursor < proc->scrollTop)
		proc->scrollTop = proc->cursor;

	if (proc->cursor >= proc->scrollTop + PHOENIX_VISIBLE_COUNT)
		proc->scrollTop = proc->cursor - PHOENIX_VISIBLE_COUNT + 1;

	if (scrolled) {
		PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
		PhoenixStaff_DrawRoster(proc);
	}
}

static bool PhoenixStaff_HasEligibleTargets(struct Unit *unit)
{
	for (int i = 0; i < 4; i++) {
		int x = unit->xPos + sPhoenixAdjTileOffsets[i][0];
		int y = unit->yPos + sPhoenixAdjTileOffsets[i][1];

		if (x < 0 || y < 0 || x >= gBmMapSize.x || y >= gBmMapSize.y)
			continue;

		if (PhoenixStaff_CanAnyDeadUnitBePlacedAt(x, y))
			return true;
	}

	return false;
}

static void PhoenixStaff_Anim(ProcPtr proc)
{
	StartLightRuneAnim(proc, gActionData.xOther, gActionData.yOther);
}

static bool PhoenixStaff_IsAnimRunning(ProcPtr proc)
{
	return Proc_Exists(ProcScr_LightRuneAnim);
}

static void PhoenixStaff_Exec(ProcPtr proc)
{
	struct Unit *unit = GetUnit(gActionData.subjectIndex);
	struct Unit *target = GetUnit(gActionData.targetIndex);

	if (!UNIT_IS_VALID(unit) || !PhoenixStaff_IsValidDeadUnit(target))
		return;

	BattleInitItemEffect(unit, gActionData.itemSlotIndex);
	BattleInitItemEffectTarget(target);

	target->state &= ~(US_HIDDEN | US_UNSELECTABLE | US_DEAD);
	target->xPos = gActionData.xOther;
	target->yPos = gActionData.yOther;
	target->curHP = 1;
	target->rescue = 0;

	RemoveDeadUnit(target->index);

	RefreshEntityBmMaps();
	RenderBmMap();
	RefreshUnitSprites();

	BattleApplyItemEffect(proc);
}

static void PhoenixStaff_ShowPopup(ProcPtr proc)
{
	struct Unit *target = GetUnit(gActionData.targetIndex);

	if (PhoenixStaff_IsValidDeadUnit(target)) {
		SetPopupUnit(target);
		NewPopup_Simple(PhoenixStaffRevivedPopup, 0x5A, 0, proc);
	}
}

static bool PhoenixStaff_PopupRunning(ProcPtr proc)
{
	return Proc_Exists(ProcScr_Popup);
}

struct PopupInstruction const PhoenixStaffRevivedPopup[] = {
	POPUP_SOUND(0x5A),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
	POPUP_UNIT_NAME,
	POPUP_SPACE(2),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
	POPUP_MSG(MSG_UnitRevived),
	POPUP_END,
};

STATIC_DECLAR const struct ProcCmd ProcScr_PhoenixRevive[] = {
	PROC_CALL(PhoenixStaff_Anim),
	PROC_WHILE(PhoenixStaff_IsAnimRunning),
	PROC_CALL(PhoenixStaff_Exec),
	PROC_CALL(PhoenixStaff_ShowPopup),
	PROC_WHILE(PhoenixStaff_PopupRunning),
	PROC_END,
};

STATIC_DECLAR const struct ProcCmd ProcScr_PhoenixAction[] = {
	PROC_CALL(PhoenixStaff_StartMenu),
	PROC_WHILE(PhoenixStaff_MenuRunning),
	PROC_CALL(PhoenixStaff_StartRevive),
	PROC_END,
};

STATIC_DECLAR const struct ProcCmd ProcScr_PhoenixStaff[] = {
	PROC_NAME("PhoenixStaff"),
	PROC_YIELD,
	PROC_SET_END_CB(PhoenixStaff_EndUi),
	PROC_CALL(PhoenixStaff_OnInit),

	PROC_LABEL(PHOENIX_STATE_INIT),
	PROC_GOTO(PHOENIX_STATE_IDLE),

	PROC_LABEL(PHOENIX_STATE_IDLE),
	PROC_REPEAT(PhoenixStaff_OnLoop),

	PROC_LABEL(PHOENIX_STATE_END),
	PROC_END,
};

bool IER_Usability_Phoenix(struct Unit *unit, int item)
{
	(void)item;

	if (unit->state & US_CANTOING)
		return false;

	return PhoenixStaff_HasEligibleTargets(unit);
}

void IER_Effect_Phoenix(struct Unit *unit, int item)
{
	(void)item;

	gActionData.unk08 = ITEM_STAFF_PHOENIX;
	gActionData.subjectIndex = unit->index;
	gActionData.targetIndex = 0;
	SetStaffUseAction(unit);

	MakeTargetListForPhoenix(unit);
	StartSubtitleHelp(
		NewTargetSelection_Specialized(&gSelectInfo_PutTrap, PhoenixStaff_OnSelectTarget),
		GetStringFromIndex(MSG_ITEM_PHOENIX_STAFF_SUBTITLE));
}

void IER_Action_Phoenix(ProcPtr proc, struct Unit *unit, int item)
{
	(void)unit;
	(void)item;
	Proc_StartBlocking(ProcScr_PhoenixAction, proc);
}