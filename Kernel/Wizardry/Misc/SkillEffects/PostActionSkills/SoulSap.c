#include "common-chax.h"
#include "skill-system.h"
#include "map-anims.h"
#include "debuff.h"
#include "kernel-lib.h"
#include "constants/skills.h"

STATIC_DECLAR void ExecSkillSoulSapEffectAnim(ProcPtr proc)
{
	(void)proc;
	struct Unit *unit = gActiveUnit;

	BmMapFill(gBmMapMovement, -1);
	BmMapFill(gBmMapRange, 0);
	GenerateUnitStandingReachRange(unit, 0b110);

	InitTargets(unit->xPos, unit->yPos);
	ForEachUnitInRange(AddUnitToTargetListIfNotAllied);
}

STATIC_DECLAR void SkillSoulSapPostAnimEffect(ProcPtr proc)
{
	(void)proc;
	int i;
	int heal_amt = 0;
	struct Unit *unit = gActiveUnit;
#if defined(SID_SoulSap) && (COMMON_SKILL_VALID(SID_SoulSap))
	int perc = SKILL_EFF0(SID_SoulSap);
#else
	int perc = 10;
#endif

	for (i = 0; i < GetSelectTargetCount(); i++) {
		struct SelectTarget *starget = GetTarget(i);
		struct Unit *tunit = GetUnit(starget->uid);

		int max_hp = GetUnitMaxHp(tunit);
		int damage_amt = Div(max_hp * perc, 100);

		// Make sure enemy units don't die from this skill
		if (tunit->curHP <= damage_amt) {
			heal_amt += tunit->curHP - 1;
			tunit->curHP = 1;
		} else {
			tunit->curHP = tunit->curHP - damage_amt;
			heal_amt += damage_amt;
		}
	}

	AddUnitHp(unit, heal_amt);
}

STATIC_DECLAR const struct ProcCmd ProcScr_PostAction_SoulSap[] = {
	PROC_NAME("PostAction_SoulSap"),
	PROC_YIELD,
	PROC_CALL(SkillSoulSapPostAnimEffect),
	PROC_END
};

bool PostAction_SoulSap(ProcPtr parent)
{
	FORCE_DECLARE struct Unit *unit = gActiveUnit;

	if (!UNIT_IS_VALID(unit))
		return false;

#if defined(SID_SoulSap) && (COMMON_SKILL_VALID(SID_SoulSap))
	if (!SkillListTester(unit, SID_SoulSap))
#else
	if (1)
#endif
		return false;

	if (!UnitAvaliable(gActiveUnit) || UNIT_STONED(gActiveUnit))
		return false;

	ExecSkillSoulSapEffectAnim(parent);
	if (GetSelectTargetCount() == 0)
		return false;

	Proc_StartBlocking(ProcScr_PostAction_SoulSap, parent);
	return true;
}
