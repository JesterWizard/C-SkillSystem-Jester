#include "common-chax.h"
#include "battle-system.h"
#include "kernel-lib.h"
#include "bwl.h"
#include "jester_headers/custom-arrays.h"

typedef void (*PreBattleGenerateFunc) (void);
extern PreBattleGenerateFunc const* const gpPreBattleGenerateFuncs;

LYN_REPLACE_CHECK(InitObstacleBattleUnit);
void InitObstacleBattleUnit(void)
{
	ClearUnit(&gBattleTarget.unit);

    gBattleTarget.unit.index = 0;

    gBattleTarget.unit.pClassData = GetClassData(CLASS_OBSTACLE);

    gBattleTarget.unit.maxHP = GetROMChapterStruct(gPlaySt.chapterIndex)->mapCrackedWallHeath;
    gBattleTarget.unit.curHP = gActionData.trapType; // TODO: better

    gBattleTarget.unit.xPos  = gActionData.xOther;
    gBattleTarget.unit.yPos  = gActionData.yOther;

    switch (gBmMapTerrain[gBattleTarget.unit.yPos][gBattleTarget.unit.xPos]) {

    case TERRAIN_WALL_DAMAGED:
        gBattleTarget.unit.pCharacterData = GetCharacterData(CHARACTER_WALL);

        break;

    case TERRAIN_SNAG:
        gBattleTarget.unit.pCharacterData = GetCharacterData(CHARACTER_SNAG);
        gBattleTarget.unit.maxHP = 20;

        break;

    } // switch (gBmMapTerrain[gBattleTarget.unit.yPos][gBattleTarget.unit.xPos])
}

LYN_REPLACE_CHECK(ComputeBattleObstacleStats);
void ComputeBattleObstacleStats(void)
{
    gBattleActor.battleEffectiveHitRate = 100;
    gBattleActor.battleEffectiveCritRate = 0;

    gBattleTarget.battleSpeed = 0xFF;
    gBattleTarget.hpInitial = gBattleTarget.unit.curHP;

    gBattleTarget.wTriangleHitBonus = 0;
    gBattleTarget.wTriangleDmgBonus = 0;
}

/**
 * This is set an addition routine on start of function: `BattleGenerate()`
 * The goal of introducing this function is to make potential modification on battle unit status.
 * This function will be called once per battle calc.
 * As a comparison, pre-battle calc and battle-calc real routine will exec twice for both foe.
 */
void PreBattleGenerateHook(void)
{
	const PreBattleGenerateFunc* it;

	for (it = gpPreBattleGenerateFuncs; *it; it++)
		(*it)();
}

LYN_REPLACE_CHECK(BattleGenerate);
void BattleGenerate(struct Unit* actor, struct Unit* target)
{
#if CHAX
	PreBattleGenerateHook();
#endif

	ComputeBattleUnitStats(&gBattleActor, &gBattleTarget);
	ComputeBattleUnitStats(&gBattleTarget, &gBattleActor);

	ComputeBattleUnitEffectiveStats(&gBattleActor, &gBattleTarget);
	ComputeBattleUnitEffectiveStats(&gBattleTarget, &gBattleActor);

	if (target == NULL)
		ComputeBattleObstacleStats();

	if ((gBattleStats.config & BATTLE_CONFIG_REAL) && (gActionData.scriptedBattleHits))
		BattleUnwindScripted();
	else
		BattleUnwind();

	/* Finally fix on UI */
	ModifyBattleStatusForUI();
}
