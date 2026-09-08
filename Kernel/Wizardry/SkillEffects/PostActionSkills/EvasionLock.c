#include "common-chax.h"
#include "debuff.h"
#include "skill-system.h"
#include "kernel-lib.h"
#include "constants/skills.h"

bool PostAction_EvasionLock(ProcPtr parent)
{
	FORCE_DECLARE struct Unit *unit = gActiveUnit;

	if (!UnitAvaliable(gActiveUnit) || UNIT_STONED(gActiveUnit))
		return false;

	if (gActionData.unitActionType == UNIT_ACTION_COMBAT) {
#if defined(SID_EvasionLock) && (COMMON_SKILL_VALID(SID_EvasionLock))
		if (SkillListTester(unit, SID_EvasionLock)) {
			if (GetUnitStatusIndex(GetUnit(gBattleTarget.unit.index)) == 0)
				SetUnitStatus(GetUnit(gBattleTarget.unit.index), NEW_UNIT_STATUS_SHOCK);
		}
#endif
	}
	return false;
}
