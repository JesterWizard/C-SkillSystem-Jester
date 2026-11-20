#include "common-chax.h"
#include "skill-system.h"
#include "debuff.h"
#include "kernel-lib.h"
#include "battle-system.h"
#include "big-unit.h"
#include "constants/skills.h"

/*
 * Knockback: after a fight the unit started, a surviving foe next to it is pushed one tile straight back, onto a free
 * tile it could walk on. Anchor holds the foe in place, and 2x2 units are not moved.
 */
bool PostAction_Knockback(ProcPtr parent)
{
#if defined(SID_Knockback) && (COMMON_SKILL_VALID(SID_Knockback))
	struct Unit *target;
	int dx, dy, x, y;

	switch (gActionData.unitActionType) {
	case UNIT_ACTION_COMBAT:
	case CONFIG_UNIT_ACTION_EXPA_GaidenMagicCombat:
		break;

	default:
		return false;
	}

	if (!UnitAvaliable(gActiveUnit) || UNIT_STONED(gActiveUnit))
		return false;

	if (!SkillListTester(gActiveUnit, SID_Knockback))
		return false;

	if ((u8)gBattleTarget.unit.index == 0)
		return false;

	target = GetUnit((u8)gBattleTarget.unit.index);
	if (!UNIT_IS_VALID(target) || target->curHP == 0 || (target->state & (US_DEAD | US_RESCUED | US_UNAVAILABLE)))
		return false;

	if (AreUnitsAllied(gActiveUnit->index, target->index) || BigUnit_IsBig(target))
		return false;

#if defined(SID_Anchor) && (COMMON_SKILL_VALID(SID_Anchor))
	if (SkillTester(target, SID_Anchor))
		return false;
#endif

	dx = target->xPos - gActiveUnit->xPos;
	dy = target->yPos - gActiveUnit->yPos;
	if (ABS(dx) + ABS(dy) != 1)
		return false;

	x = target->xPos + dx;
	y = target->yPos + dy;
	if (x < 0 || y < 0 || x >= gBmMapSize.x || y >= gBmMapSize.y)
		return false;

	if (gBmMapUnit[y][x] != 0 || !CanUnitCrossTerrain(target, gBmMapTerrain[y][x]))
		return false;

	target->xPos = x;
	target->yPos = y;

	RefreshEntityBmMaps();
	RefreshUnitSprites();

	if (CheckKernelHookSkippingFlag())
		return false;

	NewMuSkillAnimOnActiveUnitWithDeamon(parent, SID_Knockback, NULL, NULL);
	return true;
#endif

	return false;
}
