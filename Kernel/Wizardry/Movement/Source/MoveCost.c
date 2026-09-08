#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"

extern EWRAM_OVERLAY(0) s8 sTmpMovCostTable[0x41];

STATIC_DECLAR void CopyTerrainTable(s8 *dst, const s8 *src)
{
	int i;

	for (i = 0; i < TERRAIN_COUNT; i++)
		dst[i] = src[i];
}

LYN_REPLACE_CHECK(GetUnitMovementCost);
const s8 *GetUnitMovementCost(struct Unit *unit)
{
    if (unit->state & US_IN_BALLISTA)
	{
        return Unk_TerrainTable_0880BC18;
	}

	int movIndex = 0;

	switch (gPlaySt.chapterWeatherId) {
	case WEATHER_RAIN:
		movIndex = 1;
		break;

	case WEATHER_SNOW:
	case WEATHER_SNOWSTORM:
		movIndex = 2;
		break;

	default:
		movIndex = 0;
		break;
	}

#if defined(SID_Weatherman) && (COMMON_SKILL_VALID(SID_Weatherman))
	if (SkillTester(unit, SID_Weatherman) && movIndex != 0)
		return unit->pClassData->pMovCostTable[0];
#endif

	return unit->pClassData->pMovCostTable[movIndex];


// 	CopyTerrainTable(sTmpMovCostTable, cost_src);

// 	/* I need to remove this at some point, it's a worse copy of an existing skill I already have */
// #if (defined(SID_SeaWays) && COMMON_SKILL_VALID(SID_SeaWays))
// 	if (SkillTester(unit, SID_SeaWays)) {
// 		if (sTmpMovCostTable[TERRAIN_RIVER] < 0)
// 			sTmpMovCostTable[TERRAIN_RIVER] = 2;

// 		if (sTmpMovCostTable[TERRAIN_SEA] < 0)
// 			sTmpMovCostTable[TERRAIN_SEA] = 2;

// 		if (sTmpMovCostTable[TERRAIN_LAKE] < 0)
// 			sTmpMovCostTable[TERRAIN_LAKE] = 2;
// 	}
// #endif

	// return sTmpMovCostTable;
}
