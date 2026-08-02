#include "common-chax.h"
#include "pair-up.h"
#include "kernel-lib.h"
#include "bmmap.h"
#include "bmtarget.h"
#include "bmudisp.h"
#include "bmmenu.h"
#include "uiselecttarget.h"
#include "action-expa.h"
#include "jester_headers/custom-functions.h"

#pragma GCC optimize("Os", "no-jump-tables")

#define PAIR_UP_RESCUE_MARKER_ADDR ((volatile u8 *) 0x0203F101)
#define PAIR_UP_RESCUE_MARKER      2

static bool PairUp_IsInMap(int x, int y);

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
	bool requireCanto)
{
	(void) def;
	(void) number;

	if (!gActiveUnit || (gActiveUnit->state & US_HAS_MOVED))
		return MENU_NOTSHOWN;

	*PAIR_UP_RESCUE_MARKER_ADDR = 1;

	if (requireCanto && !PairUp_CanCanto(gActiveUnit))
		return MENU_NOTSHOWN;

	MakeRescueTargetList(gActiveUnit);
	return GetSelectTargetCount() ? MENU_ENABLED : MENU_NOTSHOWN;
}

LYN_REPLACE_CHECK(RescueUsability);
u8 RescueUsability(const struct MenuItemDef *def, int number)
{
	*PAIR_UP_RESCUE_MARKER_ADDR = 1;
	return PairUp_RescueUsabilityCore(def, number, false);
}

u8 PairUpCheck(const struct MenuItemDef *def, int number)
{
	*PAIR_UP_RESCUE_MARKER_ADDR = PAIR_UP_RESCUE_MARKER;
	return PairUp_RescueUsabilityCore(def, number, false);
}

u8 RescueCheck(const struct MenuItemDef *def, int number)
{
	*PAIR_UP_RESCUE_MARKER_ADDR = 1;
	return PairUp_RescueUsabilityCore(def, number, true);
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

	(void) proc;

	actor = GetUnit(gActionData.subjectIndex);
	target = GetUnit(gActionData.targetIndex);
	if (!PairUp_IsValidUnit(actor) || !PairUp_IsValidUnit(target))
		return true;

	TryRemoveUnitFromBallista(actor);

	if (*PAIR_UP_RESCUE_MARKER_ADDR == PAIR_UP_RESCUE_MARKER) {
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

		if (!(target->state & US_HAS_MOVED))
			gActionDataExpa.refrain_action = true;

		Proc_EndEach((const struct ProcCmd *) 0x089A2C48);
	} else {
		UnitRescue(actor, target);
		gActionData.xMove = actor->xPos;
		gActionData.yMove = actor->yPos;
		gActiveUnit = actor;
	}

	RefreshEntityBmMaps();
	RefreshUnitSprites();
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
	if (!PairUp_IsValidUnit(support)
		|| !PairUp_IsValidUnit(leader)
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

	if (!CanUnitRescue(leader, support))
		return false;

	if (support->statusIndex == UNIT_STATUS_BERSERK
		|| leader->statusIndex == UNIT_STATUS_BERSERK
		|| UNIT_IS_PHANTOM(support)
		|| UNIT_IS_PHANTOM(leader))
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

	leader->state &= ~US_RESCUING;
	leader->state |= US_RESCUED | US_HIDDEN;
	leader->rescue = support->index;

	support->state &= ~US_RESCUED;
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

	((void (*)(void)) 0x080790A5)();
	HideUnitSprite(leader);
	RefreshUnitSprites();
	return true;
}

int PairUp_RescueStatScale(int status, const struct Unit *unit, int stat)
{
	int half;

	if (!PairUp_IsRescuer(unit))
		return status;

	half = status >= 0 ? status / 2 : -((-status) / 2);

	if (stat == PAIR_UP_STAT_POW || stat == PAIR_UP_STAT_SKL)
		return half;

	return status + half;
}
