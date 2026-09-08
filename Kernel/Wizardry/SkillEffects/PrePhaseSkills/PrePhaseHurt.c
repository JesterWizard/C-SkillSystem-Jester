#include "common-chax.h"
#include "debuff.h"
#include "kernel-lib.h"
#include "class-types.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "map-anims.h"
#include "jester_headers/custom-functions.h"

STATIC_DECLAR int GetPrePhaseHurtAmount(struct Unit *unit)
{
	int ret = 0;

#if defined(SID_Overdraft) && (COMMON_SKILL_VALID(SID_Overdraft))
	if (SkillTester(unit, SID_Overdraft))
		ret += Div(GetUnitMaxHp(unit) * 10, 100);
#endif

	return ret;
}

bool PrePhase_HurtAmount(ProcPtr proc)
{
	int damage;

	if (gPlaySt.chapterTurnNumber == 1)
		return false;

	FOR_UNITS_ONMAP_FACTION(gPlaySt.faction, unit, {
		if (unit->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
			continue;

		damage = GetUnitCurrentHp(unit) - GetPrePhaseHurtAmount(unit) <= 0 ? GetUnitCurrentHp(unit) - 1 : GetPrePhaseHurtAmount(unit);
		
		if (damage == 0)
			continue;

		CallMapAnim_Hurt(proc, unit, damage);
	})
	return false;
}