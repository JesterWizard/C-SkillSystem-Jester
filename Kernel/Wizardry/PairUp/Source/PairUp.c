#include "common-chax.h"
#include "pair-up.h"
#include "kernel-lib.h"
#include "bmmap.h"
#include "bmtarget.h"
#include "bmudisp.h"
#include "bmmenu.h"
#include "uiselecttarget.h"
#include "unitinfowindow.h"
#include "action-expa.h"
#include "status-getter.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"

#pragma GCC optimize("Os", "no-jump-tables")

#define PAIR_UP_RESCUE_MARKER_ADDR ((volatile u8 *) 0x0203F101)
#define PAIR_UP_VIEWED_UNIT_ADDR   ((volatile u8 *) 0x0203A956)
#define PAIR_UP_MODE_PAIR_UP       1
#define PAIR_UP_MODE_SHELTER       2

#define PAIR_UP_PREVIEW_X          16
#define PAIR_UP_PREVIEW_Y          8
#define PAIR_UP_PREVIEW_WIDTH      14
#define PAIR_UP_PREVIEW_HEIGHT     10
#define PAIR_UP_PREVIEW_STAT_COUNT 8

static bool PairUp_IsInMap(int x, int y);
static void PairUp_MakeTargetList(struct Unit *unit, bool shelter);
static void PairUp_MakeTransferTargetList(struct Unit *unit);
static void PairUp_ClearStatPreview(void);
static void PairUp_DrawStatPreview(const struct Unit *target);
static void PairUp_DrawUnitInfoWindows(
	ProcPtr parent,
	const struct Unit *target);
extern const struct SelectInfo gSelectInfo_PairUp;
extern const struct ProcCmd gProcScr_UnitInfoWindow[];

static bool PairUp_IsValidUnit(const struct Unit *unit)
{
	return unit && UNIT_IS_VALID(unit) && (u8) unit->index != 0;
}

static bool PairUp_IsRescuer(const struct Unit *unit)
{
	return PairUp_IsValidUnit(unit) && (unit->state & US_RESCUING);
}

static bool PairUp_IsRescued(const struct Unit *unit)
{
	return PairUp_IsValidUnit(unit) && (unit->state & US_RESCUED);
}

static struct Unit *PairUp_GetRescuePartner(const struct Unit *unit)
{
	struct Unit *partner;

	if (!PairUp_IsValidUnit(unit) || !unit->rescue)
		return NULL;

	partner = GetUnit(unit->rescue);
	if (!PairUp_IsValidUnit(partner) || partner == unit)
		return NULL;

	if (partner->rescue != (u8) unit->index)
		return NULL;

	return partner;
}

static bool PairUp_AreAdjacent(
	const struct Unit *left,
	const struct Unit *right)
{
	int dx;
	int dy;

	if (!PairUp_IsValidUnit(left) || !PairUp_IsValidUnit(right))
		return false;

	dx = left->xPos - right->xPos;
	dy = left->yPos - right->yPos;

	if (dx < 0)
		dx = -dx;
	if (dy < 0)
		dy = -dy;

	return dx + dy == 1;
}

static bool PairUp_IsInMap(int x, int y)
{
	return gBmMapSize.x > 0 && gBmMapSize.y > 0
		&& x >= 0 && y >= 0
		&& x < gBmMapSize.x && y < gBmMapSize.y;
}

static void PairUp_SetMode(u8 mode)
{
	*PAIR_UP_RESCUE_MARKER_ADDR = mode;
}

static u8 PairUp_GetMode(void)
{
	u8 mode = *PAIR_UP_RESCUE_MARKER_ADDR;

	return mode == PAIR_UP_MODE_SHELTER
		? PAIR_UP_MODE_SHELTER
		: PAIR_UP_MODE_PAIR_UP;
}

static void PairUp_SetRescueState(
	struct Unit *leader,
	struct Unit *support)
{
	leader->state &= ~(US_RESCUED | US_HIDDEN);
	leader->state |= US_RESCUING;
	leader->rescue = support->index;

	support->state &= ~US_RESCUING;
	support->state |= US_RESCUED | US_HIDDEN;
	support->rescue = leader->index;
	support->xPos = leader->xPos;
	support->yPos = leader->yPos;

	if (gBmMapUnit)
		gBmMapUnit[leader->yPos][leader->xPos] = leader->index;
}

static bool PairUp_IsValidTarget(const struct Unit *unit)
{
	return PairUp_IsValidUnit(unit)
		&& !(unit->state & US_UNAVAILABLE)
		&& unit->statusIndex != UNIT_STATUS_BERSERK
		&& !UNIT_IS_PHANTOM(unit);
}

static void PairUp_AddPairTarget(struct Unit *unit)
{
	struct Unit *active = gActiveUnit;

	if (!PairUp_IsValidTarget(unit) || !PairUp_IsValidUnit(active))
		return;

	if (PairUp_GetMode() == PAIR_UP_MODE_SHELTER) {
		if (PairUp_CanPair(active, unit))
			AddTarget(unit->xPos, unit->yPos, unit->index, 0);
	} else {
		if (PairUp_CanPair(unit, active))
			AddTarget(unit->xPos, unit->yPos, unit->index, 0);
	}
}

static void PairUp_MakeTargetList(struct Unit *unit, bool shelter)
{
	PairUp_SetMode(shelter ? PAIR_UP_MODE_SHELTER : PAIR_UP_MODE_PAIR_UP);
	InitTargets(0, 0);
	gSubjectUnit = unit;
	BmMapFill(gBmMapRange, 0);

	ForEachAdjacentUnit(unit->xPos, unit->yPos, PairUp_AddPairTarget);
}

static void PairUp_ClearRescueState(
	struct Unit *leader,
	struct Unit *support)
{
	if (leader) {
		leader->state &= ~US_RESCUING;
		leader->rescue = 0;
	}

	if (support) {
		support->state &= ~(US_RESCUED | US_HIDDEN);
		support->rescue = 0;
	}
}

static bool PairUp_CanCanto(const struct Unit *unit)
{
	return PairUp_IsValidUnit(unit)
		&& (UNIT_CATTRIBUTES(unit) & CA_CANTO);
}

static u8 PairUp_RescueUsabilityCore(
	const struct MenuItemDef *def,
	int number,
	bool shelter,
	bool requireCanto)
{
	(void) def;
	(void) number;

	if (!gpKernelDesignerConfig->pair_up_enabled)
		return MENU_NOTSHOWN;

	if (!PairUp_IsValidTarget(gActiveUnit)
		|| (gActiveUnit->state & US_HAS_MOVED))
		return MENU_NOTSHOWN;

	if (requireCanto && !PairUp_CanCanto(gActiveUnit))
		return MENU_NOTSHOWN;

	PairUp_MakeTargetList(gActiveUnit, shelter);
	return GetSelectTargetCount() ? MENU_ENABLED : MENU_NOTSHOWN;
}

LYN_REPLACE_CHECK(RescueUsability);
u8 RescueUsability(const struct MenuItemDef *def, int number)
{
	return PairUp_RescueUsabilityCore(def, number, false, false);
}

u8 PairUpCheck(const struct MenuItemDef *def, int number)
{
	return PairUp_RescueUsabilityCore(def, number, true, false);
}

u8 RescueCheck(const struct MenuItemDef *def, int number)
{
	return PairUp_RescueUsabilityCore(def, number, false, true);
}

u8 PairUp_Usability(const struct MenuItemDef *def, int number)
{
	return PairUp_RescueUsabilityCore(def, number, false, false);
}

u8 Shelter_Usability(const struct MenuItemDef *def, int number)
{
	return PairUp_RescueUsabilityCore(def, number, true, false);
}

u8 CantoCheck(void)
{
	return PairUp_CanCanto(gActiveUnit) ? MENU_ENABLED : MENU_NOTSHOWN;
}

LYN_REPLACE_CHECK(ActionRescue);
s8 ActionRescue(ProcPtr proc)
{
	struct Unit *actor;
	struct Unit *target;
	bool shelter;

	(void) proc;

	if (!gpKernelDesignerConfig->pair_up_enabled)
		return true;

	actor = GetUnit(gActionData.subjectIndex);
	target = GetUnit(gActionData.targetIndex);
	if (!PairUp_IsValidUnit(actor) || !PairUp_IsValidUnit(target))
		return true;

	TryRemoveUnitFromBallista(actor);
	shelter = PairUp_GetMode() == PAIR_UP_MODE_SHELTER;

	if (shelter) {
		if (!PairUp_CanPair(actor, target))
			return true;

		Make6CKOIDO(
			actor,
			GetSomeFacingDirection(
				target->xPos,
				target->yPos,
				actor->xPos,
				actor->yPos),
			0,
			proc);
		UnitRescue(target, actor);
		HideUnitSprite(actor);

		gActionData.xMove = target->xPos;
		gActionData.yMove = target->yPos;
		gActionData.subjectIndex = target->index;
		gActionData.targetIndex = actor->index;
		gActiveUnit = target;
		gActiveUnitId = target->index;

		if (!(target->state & US_HAS_MOVED))
			gActionDataExpa.refrain_action = true;

		Proc_EndEach((const struct ProcCmd *) 0x089A2C48);
	} else {
		if (!PairUp_CanPair(target, actor))
			return true;

		UnitRescue(actor, target);
		gActionData.xMove = actor->xPos;
		gActionData.yMove = actor->yPos;
		gActiveUnit = actor;
	}

	RefreshEntityBmMaps();
	RefreshUnitSprites();

	if (shelter) {
		gActiveUnit = target;
		gActiveUnitId = target->index;
		*PAIR_UP_VIEWED_UNIT_ADDR = target->index;
	}

	return true;
}

bool PairUp_IsPaired(const struct Unit *unit)
{
	return PairUp_IsRescuer(unit) || PairUp_IsRescued(unit);
}

bool PairUp_IsLeader(const struct Unit *unit)
{
	return PairUp_IsRescuer(unit);
}

bool PairUp_IsSupport(const struct Unit *unit)
{
	return PairUp_IsRescued(unit);
}

struct Unit *PairUp_GetPartner(const struct Unit *unit)
{
	return PairUp_GetRescuePartner(unit);
}

struct Unit *PairUp_GetLeader(const struct Unit *unit)
{
	if (PairUp_IsRescuer(unit))
		return (struct Unit *) unit;

	if (PairUp_IsRescued(unit))
		return PairUp_GetRescuePartner(unit);

	return NULL;
}

struct Unit *PairUp_GetSupport(const struct Unit *unit)
{
	if (!PairUp_IsRescuer(unit))
		return NULL;

	return PairUp_GetRescuePartner(unit);
}

bool PairUp_CanPair(struct Unit *support, struct Unit *leader)
{
	if (!PairUp_IsValidTarget(support)
		|| !PairUp_IsValidTarget(leader)
		|| support == leader
		|| !PairUp_AreAdjacent(support, leader)
		|| !IsSameAllegiance(support->index, leader->index))
		return false;

	if (PairUp_IsPaired(support) || PairUp_IsPaired(leader))
		return false;

	if ((!UnitOnMapAvaliable(support) && support != gActiveUnit)
		|| (!UnitOnMapAvaliable(leader) && leader != gActiveUnit))
		return false;

	if ((support->state | leader->state)
		& (US_IN_BALLISTA | US_RESCUING | US_RESCUED))
		return false;

	return true;
}

bool PairUp_Attach(struct Unit *support, struct Unit *leader)
{
	if (!PairUp_CanPair(support, leader))
		return false;

	PairUp_SetRescueState(leader, support);
	return true;
}

bool PairUp_Separate(struct Unit *leader, int x, int y)
{
	struct Unit *support;

	if (!PairUp_IsLeader(leader))
		return false;

	support = PairUp_GetSupport(leader);
	if (!PairUp_IsValidUnit(support)
		|| !PairUp_IsInMap(x, y)
		|| !gBmMapUnit
		|| gBmMapUnit[y][x]
		|| !Generic_CanUnitBeOnPos(support, x, y, -1, -1))
		return false;

	PairUp_ClearRescueState(leader, support);
	support->xPos = x;
	support->yPos = y;
	gBmMapUnit[y][x] = support->index;
	return true;
}

static void PairUp_AddTransferTarget(struct Unit *unit)
{
	if (PairUp_CanTransfer(gActiveUnit, unit))
		AddTarget(unit->xPos, unit->yPos, unit->index, 0);
}

static void PairUp_MakeTransferTargetList(struct Unit *unit)
{
	InitTargets(0, 0);
	gSubjectUnit = unit;
	BmMapFill(gBmMapRange, 0);

	ForEachAdjacentUnit(unit->xPos, unit->yPos, PairUp_AddTransferTarget);
}

bool PairUp_CanTransfer(struct Unit *leader, struct Unit *target)
{
	if (!PairUp_IsLeader(leader)
		|| !PairUp_GetSupport(leader)
		|| !PairUp_IsValidTarget(target)
		|| target == leader
		|| !PairUp_AreAdjacent(leader, target)
		|| !IsSameAllegiance(leader->index, target->index)
		|| (target->state & (US_IN_BALLISTA | US_RESCUED)))
		return false;

	if (!UnitOnMapAvaliable(target) && target != gActiveUnit)
		return false;

	if (PairUp_IsLeader(target) && !PairUp_GetSupport(target))
		return false;

	return true;
}

bool PairUp_Transfer(struct Unit *leader, struct Unit *target)
{
	struct Unit *support;
	struct Unit *targetSupport;

	if (!PairUp_CanTransfer(leader, target))
		return false;

	support = PairUp_GetSupport(leader);
	targetSupport = PairUp_IsLeader(target)
		? PairUp_GetSupport(target)
		: NULL;

	PairUp_ClearRescueState(leader, support);

	if (targetSupport)
		PairUp_ClearRescueState(target, targetSupport);

	PairUp_SetRescueState(target, support);

	if (targetSupport)
		PairUp_SetRescueState(leader, targetSupport);

	RefreshEntityBmMaps();
	RefreshUnitSprites();
	return true;
}

bool PairUp_Switch(struct Unit *leader)
{
	struct Unit *support;
	s8 x;
	s8 y;

	if (!PairUp_IsLeader(leader))
		return false;

	support = PairUp_GetSupport(leader);
	if (!PairUp_IsValidUnit(support))
		return false;

	x = leader->xPos;
	y = leader->yPos;

	leader->state &= ~(US_RESCUING | US_HIDDEN);
	leader->state |= US_RESCUED | US_HIDDEN;
	leader->rescue = support->index;

	support->state &= ~(US_RESCUED | US_HIDDEN);
	support->state |= US_RESCUING;
	support->rescue = leader->index;
	support->xPos = x;
	support->yPos = y;

	if (gBmMapUnit)
		gBmMapUnit[y][x] = support->index;

	gActionData.subjectIndex = support->index;
	gActionData.targetIndex = leader->index;
	gActionData.unitActionType = UNIT_ACTION_TRADED;
	gActiveUnit = support;
	gActiveUnitId = support->index;
	*PAIR_UP_VIEWED_UNIT_ADDR = support->index;

	EndAllMus();
	HideUnitSprite(leader);
	RefreshUnitSprites();
	return true;
}

static int PairUp_GetEffectiveStat(const struct Unit *unit, int stat)
{
	struct Unit *mutableUnit = (struct Unit *) unit;

	switch (stat) {
	case PAIR_UP_STAT_POW:
		return PowGetter(mutableUnit);

	case PAIR_UP_STAT_MAG:
		return MagGetter(mutableUnit);

	case PAIR_UP_STAT_SKL:
		return SklGetter(mutableUnit);

	case PAIR_UP_STAT_SPD:
		return SpdGetter(mutableUnit);

	case PAIR_UP_STAT_LCK:
		return LckGetter(mutableUnit);

	case PAIR_UP_STAT_DEF:
		return DefGetter(mutableUnit);

	case PAIR_UP_STAT_RES:
		return ResGetter(mutableUnit);

	case PAIR_UP_STAT_MOV:
		return MovGetter(mutableUnit);

	default:
		return 0;
	}
}

static int PairUp_ThirtyPercent(int stat)
{
	if (stat >= 0)
		return stat * 30 / 100;

	return -((-stat * 30 + 99) / 100);
}

int PairUp_GetStatBonus(const struct Unit *unit, int stat)
{
	if (!PairUp_IsValidUnit(unit))
		return 0;

	switch (stat) {
	case PAIR_UP_STAT_POW:
	case PAIR_UP_STAT_MAG:
	case PAIR_UP_STAT_SKL:
	case PAIR_UP_STAT_SPD:
	case PAIR_UP_STAT_LCK:
	case PAIR_UP_STAT_DEF:
	case PAIR_UP_STAT_RES:
	case PAIR_UP_STAT_MOV:
		return PairUp_ThirtyPercent(PairUp_GetEffectiveStat(unit, stat));

	default:
		return 0;
	}
}

int PairUp_RescueStatScale(int status, const struct Unit *unit, int stat)
{
	struct Unit *support;

	if (!PairUp_IsRescuer(unit))
		return status;

	support = PairUp_GetSupport(unit);
	if (!support)
		return status;

	return status + PairUp_GetStatBonus(support, stat);
}

static int PairUp_GetPreviewLabelId(int stat)
{
	if (stat == PAIR_UP_STAT_POW)
		return 0x4FE;

	if (stat == PAIR_UP_STAT_MAG)
		return 0x4FF;

	if (stat == PAIR_UP_STAT_SKL)
		return 0x4EC;

	if (stat == PAIR_UP_STAT_SPD)
		return 0x4ED;

	if (stat == PAIR_UP_STAT_LCK)
		return 0x4EE;

	if (stat == PAIR_UP_STAT_DEF)
		return 0x4EF;

	if (stat == PAIR_UP_STAT_MOV)
		return 0x4F6;

	return 0x4F0;
}

static int PairUp_GetPreviewStat(int index)
{
	switch (index) {
	case 0:
		return PAIR_UP_STAT_POW;

	case 1:
		return PAIR_UP_STAT_LCK;

	case 2:
		return PAIR_UP_STAT_MAG;

	case 3:
		return PAIR_UP_STAT_DEF;

	case 4:
		return PAIR_UP_STAT_SKL;

	case 5:
		return PAIR_UP_STAT_RES;

	case 6:
		return PAIR_UP_STAT_SPD;

	default:
		return PAIR_UP_STAT_MOV;
	}
}

static void PairUp_ClearStatPreview(void)
{
	TileMap_FillRect(
		TILEMAP_LOCATED(
			gBG0TilemapBuffer,
			PAIR_UP_PREVIEW_X,
			PAIR_UP_PREVIEW_Y),
		PAIR_UP_PREVIEW_WIDTH,
		PAIR_UP_PREVIEW_HEIGHT,
		0);
	TileMap_FillRect(
		TILEMAP_LOCATED(
			gBG1TilemapBuffer,
			PAIR_UP_PREVIEW_X,
			PAIR_UP_PREVIEW_Y),
		PAIR_UP_PREVIEW_WIDTH,
		PAIR_UP_PREVIEW_HEIGHT,
		0);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static void PairUp_DrawStatPreview(const struct Unit *target)
{
	const struct Unit *partner = target;
	struct Text texts[PAIR_UP_PREVIEW_STAT_COUNT];
	int i;

	if (PairUp_GetMode() == PAIR_UP_MODE_SHELTER)
		partner = gActiveUnit;

	if (!PairUp_IsValidUnit(partner))
		return;

	DrawUiFrame2(
		PAIR_UP_PREVIEW_X,
		PAIR_UP_PREVIEW_Y,
		PAIR_UP_PREVIEW_WIDTH,
		PAIR_UP_PREVIEW_HEIGHT,
		0);

	for (i = 0; i < PAIR_UP_PREVIEW_STAT_COUNT; i++) {
		int column = i / 4;
		int row = i % 4;
		int stat = PairUp_GetPreviewStat(i);
		int bonus = PairUp_GetStatBonus(partner, stat);
		int labelX = PAIR_UP_PREVIEW_X + (column == 0 ? 1 : 7);
		int bonusX = PAIR_UP_PREVIEW_X + (column == 0 ? 4 : 10);
		u16 *tilemap;
		struct Text *text = &texts[i];

		InitText(text, 5);
		ClearText(text);
		Text_InsertDrawString(
			text,
			0,
			TEXT_COLOR_SYSTEM_GOLD,
			GetStringFromIndex(PairUp_GetPreviewLabelId(stat)));
		tilemap = TILEMAP_LOCATED(
			gBG0TilemapBuffer,
			labelX,
			PAIR_UP_PREVIEW_Y + 1 + row * 2);
		PutText(
			text,
			tilemap);
		tilemap = TILEMAP_LOCATED(
			gBG0TilemapBuffer,
			bonusX,
			PAIR_UP_PREVIEW_Y + 1 + row * 2);
		PutSpecialChar(
			tilemap,
			TEXT_COLOR_SYSTEM_BLUE,
			TEXT_SPECIAL_PLUS);
		PutNumber(tilemap + 1, TEXT_COLOR_SYSTEM_BLUE, bonus);
	}

	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

void PairUp_SelectionOnConstruction(ProcPtr proc)
{
	NewUnitInfoWindow(proc);
	NewUnitInfoWindow(proc);
	StartSpriteRefresher(
		proc,
		2,
		0,
		0,
		gObject_16x16_VFlipped,
		6);
	StartSubtitleHelp(proc, GetStringFromIndex(0x868));
}

static struct UnitInfoWindowProc *PairUp_GetUnitInfoWindow(
	ProcPtr parent,
	int number)
{
	struct ProcFindIterator iterator;
	struct UnitInfoWindowProc *window;

	Proc_FindBegin(&iterator, gProcScr_UnitInfoWindow);
	while ((window = (struct UnitInfoWindowProc *) Proc_FindNext(&iterator))) {
		if (window->proc_parent != parent)
			continue;

		if (number-- == 0)
			return window;
	}

	return NULL;
}

static void PairUp_DrawUnitInfoWindows(
	ProcPtr parent,
	const struct Unit *target)
{
	struct UnitInfoWindowProc *activeWindow;
	struct UnitInfoWindowProc *targetWindow;
	int x;

	activeWindow = PairUp_GetUnitInfoWindow(parent, 0);
	targetWindow = PairUp_GetUnitInfoWindow(parent, 1);
	if (!activeWindow || !targetWindow)
		return;

	x = PAIR_UP_PREVIEW_X;
	ClearBg0Bg1();
	UnitInfoWindow_DrawBase(
		activeWindow,
		gActiveUnit,
		x,
		0,
		PAIR_UP_PREVIEW_WIDTH,
		0);
	UnitInfoWindow_DrawBase(
		targetWindow,
		(struct Unit *) target,
		x,
		4,
		PAIR_UP_PREVIEW_WIDTH,
		0);
	MoveSpriteRefresher(
		0,
		(x + 4) * 8,
		(4 - 2) * 8 + 7);
}

u8 PairUp_SelectionOnSwitchIn(
	ProcPtr proc,
	struct SelectTarget *target)
{
	struct Unit *unit = GetUnit(target->uid);

	PairUp_ClearStatPreview();
	ChangeActiveUnitFacing(target->x, target->y);
	PairUp_DrawUnitInfoWindows(proc, unit);

	PairUp_DrawStatPreview(unit);
	return 0;
}

void PairUp_SelectionOnEnd(ProcPtr proc)
{
	(void) proc;
	PairUp_ClearStatPreview();
}

u8 PairUp_SelectionOnSelect(
	ProcPtr proc,
	struct SelectTarget *target)
{
	u8 result = RescueSelection_OnSelect(proc, target);

	PairUp_ClearStatPreview();
	return result;
}

u8 PairUp_SelectionOnCancel(
	ProcPtr proc,
	struct SelectTarget *target)
{
	u8 result = GenericSelection_BackToUM(proc, target);

	PairUp_ClearStatPreview();
	return result;
}

static u8 PairUp_OnSelectedCore(
	struct MenuProc *menu,
	struct MenuItemProc *item,
	bool shelter)
{
	(void) menu;

	if (!gpKernelDesignerConfig->pair_up_enabled
		|| item->availability == MENU_DISABLED)
		return MENU_ACT_SND6B;

	ClearBg0Bg1();
	PairUp_MakeTargetList(gActiveUnit, shelter);
	BmMapFill(gBmMapMovement, -1);

	StartSubtitleHelp(
		NewTargetSelection(&gSelectInfo_PairUp),
		GetStringFromIndex(MSG_SKILL_Common_Target));
	PlaySoundEffect(0x6A);

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

u8 PairUp_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	return PairUp_OnSelectedCore(menu, item, false);
}

u8 Shelter_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	return PairUp_OnSelectedCore(menu, item, true);
}

static u8 PairUp_TransferOnSelect(
	ProcPtr proc,
	struct SelectTarget *target)
{
	struct Unit *leader = GetUnit(gActionData.subjectIndex);
	struct Unit *recipient = GetUnit(target->uid);

	gActionData.targetIndex = target->uid;
	gActionData.xOther = target->x;
	gActionData.yOther = target->y;

	if (!PairUp_Transfer(leader, recipient))
		return GenericSelection_BackToUM(proc, target);

	gActionData.unitActionType = UNIT_ACTION_TRADED;
	HideMoveRangeGraphics();
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);

	return TARGETSELECTION_ACTION_ENDFAST
		| TARGETSELECTION_ACTION_END
		| TARGETSELECTION_ACTION_SE_6A
		| TARGETSELECTION_ACTION_CLEARBGS;
}

u8 PairUp_TransferUsability(const struct MenuItemDef *def, int number)
{
	(void) def;
	(void) number;

	if (!gpKernelDesignerConfig->pair_up_enabled
		|| !gActiveUnit
		|| (gActiveUnit->state & US_HAS_MOVED)
		|| !PairUp_IsLeader(gActiveUnit))
		return MENU_NOTSHOWN;

	PairUp_MakeTransferTargetList(gActiveUnit);
	return GetSelectTargetCount() ? MENU_ENABLED : MENU_NOTSHOWN;
}

u8 PairUp_TransferEffect(
	struct MenuProc *menu,
	struct MenuItemProc *item)
{
	(void) menu;

	if (!gpKernelDesignerConfig->pair_up_enabled
		|| item->availability == MENU_DISABLED)
		return MENU_ACT_SND6B;

	ClearBg0Bg1();
	PairUp_MakeTransferTargetList(gActiveUnit);
	BmMapFill(gBmMapMovement, -1);

	StartSubtitleHelp(
		NewTargetSelection_Specialized(
			&gSelectInfo_Rescue,
			PairUp_TransferOnSelect),
		GetStringFromIndex(MSG_SKILL_Common_Target));
	PlaySoundEffect(0x6A);

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

u8 PairUp_SwitchUsability(const struct MenuItemDef *def, int number)
{
	(void) def;
	(void) number;

	if (!gpKernelDesignerConfig->pair_up_enabled
		|| !gActiveUnit
		|| (gActiveUnit->state & US_HAS_MOVED)
		|| !PairUp_IsLeader(gActiveUnit)
		|| !PairUp_GetSupport(gActiveUnit))
		return MENU_NOTSHOWN;

	return MENU_ENABLED;
}

u8 PairUp_SwitchEffect(
	struct MenuProc *menu,
	struct MenuItemProc *item)
{
	(void) menu;

	if (!gpKernelDesignerConfig->pair_up_enabled
		|| item->availability == MENU_DISABLED)
		return MENU_ACT_SND6B;

	if (!PairUp_Switch(gActiveUnit))
		return MENU_ACT_SND6B;

	return MENU_ACT_SKIPCURSOR
		| MENU_ACT_END
		| MENU_ACT_SND6A
		| MENU_ACT_CLEAR;
}
