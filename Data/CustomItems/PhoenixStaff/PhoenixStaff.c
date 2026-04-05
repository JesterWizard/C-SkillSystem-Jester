#include "common-chax.h"
#include "item-sys.h"
#include "battle-system.h"
#include "constants/items.h"
#include "constants/texts.h"
#include "jester_headers/custom-arrays.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-structs.h"
#include "mapanim.h"
#include "popup.h"
#include "prepscreen.h"

#define PHOENIX_VISIBLE_COUNT 6
#define PHOENIX_SCROLLBAR_X 216
#define PHOENIX_SCROLLBAR_Y 32

static const int sPhoenixAdjTileOffsets[4][2] = {
	{-1, 0},
	{ 1, 0},
	{ 0,-1},
	{ 0, 1},
};

extern bool sPhoenixMenuActive;
extern u16 gUnknown_085A0D4C[];

extern u8 sDeadUnitCount;

static u8 PhoenixStaff_OnSelectTarget(ProcPtr proc, struct SelectTarget *target);

STATIC_DECLAR const struct MenuItemDef PhoenixStaffMenuItems[];
STATIC_DECLAR const struct MenuDef PhoenixStaffMenuDef;
STATIC_DECLAR u8 PhoenixStaffMenu_OnCancel(struct MenuProc *menu, struct MenuItemProc *item);
STATIC_DECLAR u8 PhoenixStaffMenu_HelpBox(struct MenuProc *menu, struct MenuItemProc *item);
STATIC_DECLAR u8 PhoenixStaffMenu_Usability(const struct MenuItemDef *self, int number);
STATIC_DECLAR int PhoenixStaffMenu_OnDraw(struct MenuProc *menu, struct MenuItemProc *item);
STATIC_DECLAR int PhoenixStaffMenu_OnSwitchIn(struct MenuProc *menu, struct MenuItemProc *item);
STATIC_DECLAR u8 PhoenixStaffMenu_OnSelected(struct MenuProc *menu, struct MenuItemProc *item);
static void PhoenixStaff_ResetMenuState(void);

/* ─── Validation ─── */

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

/* ─── Menu state helpers ─── */

static void PhoenixStaff_ResetMenuState(void)
{
	sPhoenixMenuActive = false;
	gList_Total = 0;
	gTopVisibleListIndex = 0;
}

static int PhoenixStaff_GetVisibleCount(void)
{
	if (sDeadUnitCount < PHOENIX_VISIBLE_COUNT)
		return sDeadUnitCount;

	return PHOENIX_VISIBLE_COUNT;
}

static bool PhoenixStaff_ShouldShowScrollBar(void)
{
	return (sDeadUnitCount > PHOENIX_VISIBLE_COUNT);
}

static void PhoenixStaff_UpdateScrollBar(void)
{
	if (!PhoenixStaff_ShouldShowScrollBar())
		return;

	UpdateMenuScrollBarConfig(
		8,
		gTopVisibleListIndex * 16,
		sDeadUnitCount,
		PHOENIX_VISIBLE_COUNT);
}

static void PhoenixStaff_StartScrollBar(struct MenuProc *menu)
{
	if (!PhoenixStaff_ShouldShowScrollBar())
	{
		return;
	}

	StartMenuScrollBar(menu);
	PutMenuScrollBarAt(PHOENIX_SCROLLBAR_X, PHOENIX_SCROLLBAR_Y);
	InitMenuScrollBarImg(0x7A60, 2);
	PhoenixStaff_UpdateScrollBar();
}

static void PhoenixStaff_EndScrollBar(void)
{
	if (!PhoenixStaff_ShouldShowScrollBar())
		return;

	EndMenuScrollBar();
}

/* ─── Unit list lookup (uses statics, no proc_parent cast) ─── */

static struct Unit *PhoenixStaff_GetUnitForVisibleIndex(int itemNumber)
{
	int index = gTopVisibleListIndex + itemNumber;

	if ((index < 0) || (index >= sDeadUnitCount))
		return NULL;

	return GetUnit(gDeadUnits[index]);
}

/* ─── Portrait (SummonPlus positions) ─── */

static void PhoenixStaff_ClearPortraitArea(void)
{
	TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 3, 5), 10, 9, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void PhoenixStaff_DrawPortrait(struct Unit *unit)
{
	if (!UNIT_IS_VALID(unit))
		return;

	PhoenixStaff_ClearPortraitArea();
	CallARM_FillTileRect(gBG1TilemapBuffer + 0x42, gUnknown_085A0D4C, 0x1000);
	PutFace80x72_Core(gBG0TilemapBuffer + 0x63 + 0x40, GetUnitPortraitId(unit), 0x200, 5);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

/* ─── Cancel / Select ─── */

static void PhoenixStaff_CancelAction(void)
{
	gActionData.unitActionType = 0;
	gActionData.unk08 = 0;
	gActionData.targetIndex = 0;
	PhoenixStaff_EndScrollBar();
	PhoenixStaff_ResetMenuState();
}

static bool PhoenixStaff_CanSelectVisibleIndex(int number)
{
	struct Unit *unit;

	if (number >= PhoenixStaff_GetVisibleCount())
		return false;

	unit = PhoenixStaff_GetUnitForVisibleIndex(number);
	return UNIT_IS_VALID(unit) && PhoenixStaff_CanUnitBePlacedAt(unit, gActionData.xOther, gActionData.yOther);
}

static bool PhoenixStaff_SelectUnit(struct MenuProc *menu, struct MenuItemProc *item)
{
	struct Unit *unit = PhoenixStaff_GetUnitForVisibleIndex(item->itemNumber);

	if (!UNIT_IS_VALID(unit) || !PhoenixStaff_CanUnitBePlacedAt(unit, gActionData.xOther, gActionData.yOther))
		return false;

	gActionData.targetIndex = unit->index;
	PhoenixStaff_EndScrollBar();
	PhoenixStaff_ResetMenuState();

	return true;
}

/* ─── Scroll handler (called from ProcessMenuDpadInput hook) ─── */

bool PhoenixStaff_HandleMenuScroll(struct MenuProc *menu)
{
	int visibleCount = PhoenixStaff_GetVisibleCount();
	int maxOffset = sDeadUnitCount - visibleCount;

	if (!sPhoenixMenuActive)
		return false;

	if (visibleCount <= 0)
		return false;

	if (sDeadUnitCount <= visibleCount)
		return false;

	if (gKeyStatusPtr->repeatedKeys & DPAD_UP) {
		if (menu->itemCurrent == 0) {
			if (gTopVisibleListIndex > 0) {
				gTopVisibleListIndex--;
				RedrawMenu(menu);
				DrawMenuItemHover(menu, menu->itemCurrent, TRUE);
				PhoenixStaff_UpdateScrollBar();
				return true;
			}

			return true;
		}
	}

	if (gKeyStatusPtr->repeatedKeys & DPAD_DOWN) {
		if (menu->itemCurrent == (visibleCount - 1)) {
			if (gTopVisibleListIndex < maxOffset) {
				gTopVisibleListIndex++;
				RedrawMenu(menu);
				DrawMenuItemHover(menu, menu->itemCurrent, TRUE);
				PhoenixStaff_UpdateScrollBar();
				return true;
			}

			return true;
		}
	}

	return false;
}

/* ─── Menu UI init / cleanup ─── */

static void PhoenixStaff_ClearUi(void)
{
	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
	HideSysHandCursor();
	EndMenuScrollBar();
	PhoenixStaff_ResetMenuState();
}

static void PhoenixStaff_InitRoster(void)
{
	sDeadUnitCount = GetDeadUnitCount();
	gList_Total = sDeadUnitCount;
	gTopVisibleListIndex = 0;
}

static bool PhoenixStaff_MenuRunning(ProcPtr proc)
{
	(void)proc;

	return sPhoenixMenuActive;
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

static void PhoenixStaff_OnInit(ProcPtr proc)
{
	struct MenuProc *menu;

	PhoenixStaff_InitRoster();

	if (sDeadUnitCount == 0) {
		PhoenixStaff_CancelAction();
		return;
	}

	gActionData.unk08 = ITEM_STAFF_PHOENIX;
	sPhoenixMenuActive = true;

	menu = StartOrphanMenu(&PhoenixStaffMenuDef);
	PhoenixStaff_StartScrollBar(menu);
	StartSubtitleHelp(menu, GetStringFromIndex(MSG_ITEM_PHOENIX_STAFF_SUBTITLE));
}

/* ─── Menu item definitions (SummonPlus layout) ─── */

STATIC_DECLAR const struct MenuItemDef PhoenixStaffMenuItems[] = {
	{ .isAvailable = PhoenixStaffMenu_Usability, .onDraw = PhoenixStaffMenu_OnDraw, .onSelected = PhoenixStaffMenu_OnSelected, .onSwitchIn = PhoenixStaffMenu_OnSwitchIn },
	{ .isAvailable = PhoenixStaffMenu_Usability, .onDraw = PhoenixStaffMenu_OnDraw, .onSelected = PhoenixStaffMenu_OnSelected, .onSwitchIn = PhoenixStaffMenu_OnSwitchIn },
	{ .isAvailable = PhoenixStaffMenu_Usability, .onDraw = PhoenixStaffMenu_OnDraw, .onSelected = PhoenixStaffMenu_OnSelected, .onSwitchIn = PhoenixStaffMenu_OnSwitchIn },
	{ .isAvailable = PhoenixStaffMenu_Usability, .onDraw = PhoenixStaffMenu_OnDraw, .onSelected = PhoenixStaffMenu_OnSelected, .onSwitchIn = PhoenixStaffMenu_OnSwitchIn },
	{ .isAvailable = PhoenixStaffMenu_Usability, .onDraw = PhoenixStaffMenu_OnDraw, .onSelected = PhoenixStaffMenu_OnSelected, .onSwitchIn = PhoenixStaffMenu_OnSwitchIn },
	{ .isAvailable = PhoenixStaffMenu_Usability, .onDraw = PhoenixStaffMenu_OnDraw, .onSelected = PhoenixStaffMenu_OnSelected, .onSwitchIn = PhoenixStaffMenu_OnSwitchIn },
	{ 0 }
};

STATIC_DECLAR const struct MenuDef PhoenixStaffMenuDef = {
	{15, 1, 12, 0},
	0,
	PhoenixStaffMenuItems,
	0, 0, 0,
	PhoenixStaffMenu_OnCancel,
	MenuAutoHelpBoxSelect,
	PhoenixStaffMenu_HelpBox
};

/* ─── Menu callbacks (SummonPlus draw pattern) ─── */

u8 PhoenixStaffMenu_Usability(const struct MenuItemDef *self, int number)
{
	(void)self;

	if (PhoenixStaff_CanSelectVisibleIndex(number))
		return MENU_ENABLED;

	if (number < PhoenixStaff_GetVisibleCount())
		return MENU_DISABLED;

	return MENU_NOTSHOWN;
}

int PhoenixStaffMenu_OnDraw(struct MenuProc *menu, struct MenuItemProc *item)
{
	struct Unit *unit = PhoenixStaff_GetUnitForVisibleIndex(item->itemNumber);
	bool canSelect;

	if (!UNIT_IS_VALID(unit))
		return 0;

	canSelect = PhoenixStaff_CanSelectVisibleIndex(item->itemNumber);

	ClearText(&item->text);
	Text_SetColor(&item->text, canSelect ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY);

	Text_DrawString(&item->text, GetStringFromIndex(unit->pCharacterData->nameTextId));

	if (item->itemNumber == menu->itemCurrent) {
		PhoenixStaff_DrawPortrait(unit);
	}

	PutText(&item->text, TILEMAP_LOCATED(gBG0TilemapBuffer, item->xTile + 1, item->yTile));

	BG_EnableSyncByMask(BG0_SYNC_BIT);

	return 0;
}

int PhoenixStaffMenu_OnSwitchIn(struct MenuProc *menu, struct MenuItemProc *item)
{
	struct Unit *unit = PhoenixStaff_GetUnitForVisibleIndex(item->itemNumber);

	if (UNIT_IS_VALID(unit))
		PhoenixStaff_DrawPortrait(unit);

	return 0;
}

u8 PhoenixStaffMenu_HelpBox(struct MenuProc *menu, struct MenuItemProc *item)
{
	struct Unit *unit = PhoenixStaff_GetUnitForVisibleIndex(item->itemNumber);

	if (!UNIT_IS_VALID(unit))
		return 0;

	StartHelpBox(item->xTile * 8, item->yTile * 8, unit->pCharacterData->descTextId);

	return 0;
}

u8 PhoenixStaffMenu_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	ProcPtr playerPhaseProc = Proc_Find(gProcScr_PlayerPhase);

	if (!PhoenixStaff_CanSelectVisibleIndex(item->itemNumber))
		return 0;

	if (!PhoenixStaff_SelectUnit(menu, item))
		return 0;

	PhoenixStaff_ClearUi();

	Proc_StartBlocking(ProcScr_PhoenixRevive, playerPhaseProc);

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

u8 PhoenixStaffMenu_OnCancel(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (!PhoenixStaff_CanSelectVisibleIndex(item->itemNumber))
		return 0;

	HideMoveRangeGraphics();
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);
	PhoenixStaff_CancelAction();

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static u8 PhoenixStaff_OnSelectTarget(ProcPtr proc, struct SelectTarget *target)
{
	gActionData.subjectIndex = gActiveUnit->index;
	gActionData.targetIndex = target->uid;
	gActionData.xOther = target->x;
	gActionData.yOther = target->y;
	HideMoveRangeGraphics();
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);

	return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
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
	// There is an issue with this rght now where the coordinates in gActionData are not respected and the light rune animation appears in a static position
	// StartLightRuneAnim(proc, gActionData.xOther, gActionData.yOther);
}

static bool PhoenixStaff_IsAnimRunning(ProcPtr proc)
{
	return Proc_Exists(ProcScr_LightRuneAnim);
}

static void PhoenixStaff_Exec(ProcPtr proc)
{
	struct Unit *unit = GetUnit(gActionData.subjectIndex);
	struct Unit *target = GetUnit(gActionData.targetIndex);


	if (!UNIT_IS_VALID(unit))
		return;

	if (!UNIT_IS_VALID(target))
		return;

	if (!PhoenixStaff_IsValidDeadUnit(target))
		return;

	BattleInitItemEffect(unit, gActionData.itemSlotIndex);
	BattleInitItemEffectTarget(unit);

	target->state &= ~(US_HIDDEN | US_UNSELECTABLE | US_DEAD);
	target->xPos = gActionData.xOther;
	target->yPos = gActionData.yOther;
	target->curHP = 1;
	target->rescue = 0;
	
	RefreshEntityBmMaps();
	RenderBmMap();
	RefreshUnitSprites();

	BattleApplyItemEffect(proc);
}

static void PhoenixStaff_ShowPopup(ProcPtr proc)
{
	struct Unit *target = GetUnit(gActionData.targetIndex);

	if (UNIT_IS_VALID(target)) {
		SetPopupUnit(target);
		NewPopup_Simple(PhoenixStaffRevivedPopup, 0x5A, 0, proc);
	}
}

static bool PhoenixStaff_PopupRunning(ProcPtr proc)
{
	return Proc_Exists(ProcScr_Popup);
}

static void PhoenixStaff_ShowExpBar(ProcPtr proc)
{
    struct Unit *unit = GetUnit(gActionData.subjectIndex);
    int expGain = StaffEXP(ITEM_STAFF_PHOENIX);

    if (!UNIT_IS_VALID(unit))
        return;

    if (expGain <= 0)
        return;

    // If the unit already got updated, use stored old exp instead of unit->exp here.
    struct MAExpBarProc *barProc = Proc_StartBlocking(ProcScr_MapAnimExpBar, proc);

    barProc->expFrom = unit->exp;
    barProc->expTo   = unit->exp + expGain;
    barProc->actorId = 0;
}

const struct PopupInstruction PhoenixStaffRevivedPopup[] = {
	POPUP_SOUND(0x5A),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
	POPUP_UNIT_NAME,
	POPUP_SPACE(2),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
	POPUP_MSG(MSG_UnitRevived),
	POPUP_END,
};

static bool PhoenixStaff_ExpBarRunning(ProcPtr proc)
{
	return Proc_Exists(ProcScr_MapAnimExpBar);
}

const struct ProcCmd ProcScr_PhoenixRevive[] = {
    PROC_CALL(PhoenixStaff_Anim),
    PROC_WHILE(PhoenixStaff_IsAnimRunning),
    PROC_CALL(PhoenixStaff_Exec),
    PROC_CALL(PhoenixStaff_ShowPopup),
    PROC_WHILE(PhoenixStaff_PopupRunning),
    PROC_CALL(PhoenixStaff_ShowExpBar),
	PROC_WHILE(PhoenixStaff_ExpBarRunning),
    PROC_END
};

const struct ProcCmd ProcScr_PhoenixStaff[] = {
	PROC_CALL(PhoenixStaff_OnInit),
	PROC_WHILE(PhoenixStaff_MenuRunning),
	PROC_CALL(PhoenixStaff_ClearUi),
	PROC_END,
};

bool IER_Usability_Phoenix(struct Unit *unit, int item)
{
	if (unit->state & US_CANTOING)
		return false;

	return PhoenixStaff_HasEligibleTargets(unit);
}

void IER_Effect_Phoenix(struct Unit *unit, int item)
{
	gActionData.unk08 = ITEM_STAFF_PHOENIX;
	gActionData.subjectIndex = unit->index;
	SetStaffUseAction(unit);

	MakeTargetListForPhoenix(unit);
	StartSubtitleHelp(
		NewTargetSelection_Specialized(&gSelectInfo_PutTrap, PhoenixStaff_OnSelectTarget),
		GetStringFromIndex(MSG_ITEM_PHOENIX_STAFF_SUBTITLE));
}

void IER_Action_Phoenix(ProcPtr proc, struct Unit *unit, int item)
{
	Proc_StartBlocking(ProcScr_PhoenixStaff, proc);
}