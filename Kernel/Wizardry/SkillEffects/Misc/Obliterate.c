#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"

/*
 * Obliterate: a promoted unit instantly kills unpromoted enemies of a lower
 * level. bu and foe must be gBattleActor and gBattleTarget (either way round).
 * Callers: BattleCalcReal_ComputSkills (forecast), BattleUpdateBattleStats
 * (each strike) and CheckVantageOrder (strike first when defending).
 */
bool ObliterateApplies(struct BattleUnit *bu, struct BattleUnit *foe)
{
#if defined(SID_Obliterate) && (COMMON_SKILL_VALID(SID_Obliterate))
	if (gBattleStats.config & BATTLE_CONFIG_ARENA)
		return false;

	if (!BattleFastSkillTester(bu, SID_Obliterate))
		return false;

	if (!(UNIT_CATTRIBUTES(&bu->unit) & CA_PROMOTED))
		return false;

	if (UNIT_CATTRIBUTES(&foe->unit) & CA_PROMOTED)
		return false;

	if (AreUnitsAllied(bu->unit.index, foe->unit.index))
		return false;

	return foe->unit.level < bu->unit.level;
#else
	return false;
#endif
}
