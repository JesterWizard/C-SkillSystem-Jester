#include "common-chax.h"
#include "skill-system.h"
#include "battle-system.h"
#include "constants/skills.h"

STATIC_DECLAR bool CheckSkillActivateExt(struct Unit *unit, int sid, int rate, bool (*tester)(struct Unit *unit, const u16 sid))
{
	if (!tester(unit, sid))
		return false;

#if (defined(SID_RightfulArch) && (COMMON_SKILL_VALID(SID_RightfulArch)))
	if (tester(unit, SID_RightfulArch))
		return true;
#endif

#if (defined(SID_RightfulKing) && (COMMON_SKILL_VALID(SID_RightfulKing)))
	if (tester(unit, SID_RightfulKing))
		rate += SKILL_EFF0(SID_RightfulKing);
#endif

#if (defined(SID_RightfulGod) && (COMMON_SKILL_VALID(SID_RightfulGod)))
	if (tester(unit, SID_RightfulGod))
		rate += SKILL_EFF0(SID_RightfulGod);
#endif

#if (defined(SID_Hero) && (COMMON_SKILL_VALID(SID_Hero)))
	if (tester(unit, SID_Hero))
		if ((unit->curHP * 2) < unit->maxHP)
			rate += SKILL_EFF0(SID_Hero);
#endif

#if (defined(SID_Domino) && (COMMON_SKILL_VALID(SID_Domino)))
	for (int i = 0; i < ARRAY_COUNT_RANGE3x3; i++)
	{
		int _x = unit->xPos + gVecs_3x3[i].x;
		int _y = unit->yPos + gVecs_3x3[i].y;

		struct Unit *unit_ally = GetUnitAtPosition(_x, _y);

		if (!UNIT_IS_VALID(unit_ally))
			continue;

		if (unit_ally->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
			continue;

		if (tester(unit_ally, SID_Domino), AreUnitsAllied(unit->index, unit_ally->index)) {
			rate += 10;
			break;
		}
	}
#endif

#if (defined(SID_SereneGrace) && (COMMON_SKILL_VALID(SID_SereneGrace)))
    if (tester(unit, SID_SereneGrace))
        rate *= 2;
#endif


	LIMIT_AREA(rate, 0, 100);

	if (Roll2RN(rate))
		return true;

	return false;
}

bool CheckSkillActivate(struct Unit *unit, int sid, int rate)
{
	return CheckSkillActivateExt(unit, sid, rate, SkillTester);
}

bool CheckActiveUnitSkillActivate(int sid, int rate)
{
	return CheckSkillActivateExt(gActiveUnit, sid, rate, SkillListTester);
}

bool CheckBattleSkillActivate(struct BattleUnit *actor, struct BattleUnit *target, int sid, int rate)
{
	if (gBattleStats.config & BATTLE_CONFIG_SIMULATE)
		return false;

#if (defined(SID_Foresight) && (COMMON_SKILL_VALID(SID_Foresight)))
	if (BattleFastSkillTester(target, SID_Foresight))
		return false;
#endif

	return CheckSkillActivateExt(&actor->unit, sid, rate, (bool (*)(struct Unit *unit, const u16 sid))BattleFastSkillTester);
}
