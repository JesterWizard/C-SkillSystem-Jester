#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"

/*
 * Speed that Fast Hands adds with the unit's equipped weapon, for the stat
 * screen. The battle side is in ComputeBattleUnitSpeed (PreBattleCalc.c);
 * this runs the same calculation the stat screen uses for its battle
 * preview, so the number shown matches battle.
 */
int GetFastHandsSpdBonus(struct Unit *unit)
{
#if defined(SID_FastHands) && (COMMON_SKILL_VALID(SID_FastHands))
	int bonus;

	if (!SkillTester(unit, SID_FastHands))
		return 0;

	BattleGenerateUiStats(unit, GetUnitEquippedWeaponSlot(unit));
	bonus = gBattleActor.battleSpeed - gBattleActor.unit.spd;

	return bonus > 0 ? bonus : 0;
#else
	return 0;
#endif
}
