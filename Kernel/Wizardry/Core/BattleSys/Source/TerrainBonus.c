#include "common-chax.h"
#include "skill-system.h"
#include "class-types.h"
#include "constants/skills.h"
#include "battle-system.h"
#include "kernel/traps.h"

STATIC_DECLAR const struct ClassData *GetJInfoForTerrainBonus(struct Unit *unit)
{
	/**
	 * As it is just used for gActiveUnit, so here we can use SkillListTester to replace SkillTester
	 */
	if (CheckClassFlier(UNIT_CLASS_ID(unit))) {
#if (defined(SID_WingedShield) && (COMMON_SKILL_VALID(SID_WingedShield)))
		if (SkillListTester(unit, SID_WingedShield))
			return GetClassData(gpKernelClassList_Cavalry[0]);
#endif

#if (defined(SID_Perch) && (COMMON_SKILL_VALID(SID_Perch)))
		if (SkillListTester(unit, SID_Perch))
			return GetClassData(gpKernelClassList_Cavalry[0]);
#endif
	}

	return unit->pClassData;
}

LYN_REPLACE_CHECK(SetBattleUnitTerrainBonuses);
void SetBattleUnitTerrainBonuses(struct BattleUnit *bu, int terrain)
{
	const struct ClassData *jinfo = GetJInfoForTerrainBonus(&bu->unit);

	bu->terrainId 		  = terrain;
    bu->terrainAvoid      = jinfo->pTerrainAvoidLookup[bu->terrainId];

	/* This is so we don't apply the numbers twice since they're applied in the getters as well */
	if (gpKernelDesignerConfig->display_terrain_bonuses_in_stat_screen == false)
	{
		bu->terrainDefense    = jinfo->pTerrainDefenseLookup[bu->terrainId];
		bu->terrainResistance = jinfo->pTerrainResistanceLookup[bu->terrainId];
	}

#if (defined(SID_Camouflage) && (COMMON_SKILL_VALID(SID_Camouflage)))
    if (BattleFastSkillTester(bu, SID_Camouflage))
        bu->terrainAvoid = (bu->terrainAvoid * 3)/2;
#endif
}

LYN_REPLACE_CHECK(SetBattleUnitTerrainBonusesAuto);
void SetBattleUnitTerrainBonusesAuto(struct BattleUnit *bu)
{
	SetBattleUnitTerrainBonuses(bu, GetEffectiveTerrainAt(bu->unit.xPos, bu->unit.yPos));
}

LYN_REPLACE_CHECK(AiGetTerrainCombatPositionScoreComponent);
int AiGetTerrainCombatPositionScoreComponent(int x, int y)
{
	const struct ClassData *jinfo = GetJInfoForTerrainBonus(gActiveUnit);
	int terrainId = GetEffectiveTerrainAt(x, y);

	return jinfo->pTerrainAvoidLookup[terrainId]
		 + jinfo->pTerrainDefenseLookup[terrainId]
		 + jinfo->pTerrainResistanceLookup[terrainId];
}
