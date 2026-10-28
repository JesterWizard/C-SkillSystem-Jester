#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "post-action.h"
#include "constants/skills.h"

/**
 * Insurance: after a battle, the player unit with this skill pays the party
 * SKILL_EFF0 gold for every point of damage it took. It works whether the
 * unit attacked (damage dealt by its target) or was attacked (damage dealt
 * by the acting unit).
 */
bool PostAction_Insurance(ProcPtr parent)
{
#if defined(SID_Insurance) && (COMMON_SKILL_VALID(SID_Insurance))
	struct Unit *unit;
	int damage;

	switch (gActionData.unitActionType) {
	case UNIT_ACTION_COMBAT:
	case CONFIG_UNIT_ACTION_EXPA_GaidenMagicCombat:
		break;

	default:
		return false;
	}

	if (UNIT_IS_VALID(gActiveUnit) && UNIT_FACTION(gActiveUnit) == FACTION_BLUE) {
		unit = gActiveUnit;
		damage = gPostActionCommonBuffer.total_damage_target;
	} else {
		unit = GetUnit(gActionData.targetIndex);
		damage = gPostActionCommonBuffer.total_damage_actor;
	}

	if (!UNIT_IS_VALID(unit) || UNIT_FACTION(unit) != FACTION_BLUE)
		return false;

	if (damage <= 0 || !SkillListTester(unit, SID_Insurance))
		return false;

	NewPopup_GoldGot(parent, unit, damage * SKILL_EFF0(SID_Insurance));
	return true;
#else
	return false;
#endif
}
