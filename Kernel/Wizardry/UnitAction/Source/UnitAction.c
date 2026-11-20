#include "common-chax.h"
#include "action-expa.h"
#include "unit-expa.h"
#include "kernel-lib.h"
#include "battle-system.h"
#include "gaiden-magic.h"
#include "skill-system.h"
#include "debuff.h"
#include "big-unit.h"
#include "constants/skills.h"

bool PrePhase_ClearUnitsActedBit(ProcPtr proc)
{
	// FOR_UNITS_VALID_ALL(unit, {
	// 	ClearBitUES(unit, UES_BIT_ACTED);
	// })
	return false;
}

STATIC_DECLAR void TryChangeAction(void)
{
	int slot = gActionData.itemSlotIndex;

	/* Modify for special battle */
	if (!CheckUnbreakableSpecialSlot(slot))
		return;

	switch (slot) {
	case CHAX_BUISLOT_GAIDEN_BMAG1 ... CHAX_BUISLOT_GAIDEN_BMAG7:
	case CHAX_BUISLOT_GAIDEN_WMAG1 ... CHAX_BUISLOT_GAIDEN_WMAG7:
		TryChangeGaidenMagicAction();
		break;

	default:
		break;
	}
}

#if (defined(SID_Cover) && COMMON_SKILL_VALID(SID_Cover))
/*
 * Cover: when a unit is attacked, an ally next to it (not the attacker's) with Cover swaps places with it and takes the
 * fight, on either side's phase (every attack starts here, the player's and the AI's). The two must be able to stand
 * on each other's tiles; 2x2 units, units being carried and stoned ones don't cover. The healthiest of several takes
 * it.
 */
STATIC_DECLAR void TryCoverAttackedUnit(void)
{
	static const s8 adj[4][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
	struct Unit *target, *cover = NULL;
	int i, x, y;

	if (gActionData.targetIndex == 0)
		return;

	target = GetUnit(gActionData.targetIndex);
	if (!UNIT_IS_VALID(target) || BigUnit_IsBig(target))
		return;

	for (i = 0; i < 4; i++) {
		struct Unit *unit;

		x = target->xPos + adj[i][0];
		y = target->yPos + adj[i][1];
		if (x < 0 || y < 0 || x >= gBmMapSize.x || y >= gBmMapSize.y || gBmMapUnit[y][x] == 0)
			continue;

		unit = GetUnit(gBmMapUnit[y][x]);
		if (!UNIT_IS_VALID(unit) || unit == gActiveUnit || unit->curHP == 0 ||
		    (unit->state & (US_DEAD | US_RESCUED | US_UNAVAILABLE)) || UNIT_STONED(unit) || BigUnit_IsBig(unit))
			continue;

		if (!AreUnitsAllied(target->index, unit->index) || AreUnitsAllied(gActiveUnit->index, unit->index))
			continue;

		if (!SkillTester(unit, SID_Cover))
			continue;

		if (!CanUnitCrossTerrain(unit, gBmMapTerrain[target->yPos][target->xPos]) ||
		    !CanUnitCrossTerrain(target, gBmMapTerrain[y][x]))
			continue;

		if (cover == NULL || unit->curHP > cover->curHP)
			cover = unit;
	}

	if (cover == NULL)
		return;

	x = cover->xPos;
	y = cover->yPos;
	cover->xPos = target->xPos;
	cover->yPos = target->yPos;
	target->xPos = x;
	target->yPos = y;

	gBmMapUnit[cover->yPos][cover->xPos] = cover->index;
	gBmMapUnit[target->yPos][target->xPos] = target->index;
	gActionData.targetIndex = cover->index;

	RefreshUnitSprites();
}
#endif

LYN_REPLACE_CHECK(ApplyUnitAction);
unsigned int ApplyUnitAction(ProcPtr proc)
{
	UnitActionFunc_t it;

	gActiveUnit = GetUnit(gActionData.subjectIndex);
	gActiveUnit->state |= US_HAS_MOVED;

	switch (gActionData.unitActionType) {
	case UNIT_ACTION_COMBAT:
	case UNIT_ACTION_STAFF:
		TryChangeAction();
		break;
	}

#if (defined(SID_Cover) && COMMON_SKILL_VALID(SID_Cover))
	if (gActionData.unitActionType == UNIT_ACTION_COMBAT ||
	    gActionData.unitActionType == CONFIG_UNIT_ACTION_EXPA_GaidenMagicCombat)
		TryCoverAttackedUnit();
#endif

	if (gActionData.unitActionType >= CONFIG_UNIT_ACTION_AMT)
		return true;

	// SetBitUES(gActiveUnit, UES_BIT_ACTED);

	/* Well I think there should be set some data for action-expa during action routine */
	memset(&gActionDataExpa, 0, sizeof(gActionDataExpa));

	it = gpUnitActionTable[gActionData.unitActionType];
	if (!it)
		return true;

	return it(proc);
}

/* Misc action functions */
bool _ActionWait(ProcPtr proc)
{
	gActiveUnit->state |= US_HAS_MOVED;
	return true;
}

bool _ActionCombat(ProcPtr proc)
{
	if (gActionData.unitActionType == UNIT_ACTION_COMBAT) {
		int itemIdx = GetItemIndex(GetItemFromSlot(gActiveUnit, gActionData.itemSlotIndex));

		if (itemIdx == ITEM_NIGHTMARE) {
			ActionStaffDoorChestUseItem(proc);
			return false;
		}
	}
	return ActionCombat(proc);
}

bool _ActionStaffDoorChestUseItem(ProcPtr proc)
{
	ActionStaffDoorChestUseItem(proc);
	return false;
}

bool _ActionPick(ProcPtr proc)
{
	ActionPick(proc);
	return false;
}
