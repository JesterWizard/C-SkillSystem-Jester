#include "common-chax.h"
#include "debuff.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "strmag.h"
#include "debuff.h"
#include "bwl.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-arrays.h"

extern int (*gpExternalPrePhaseHealCalc)(int cur, struct Unit *unit);

FORCE_DECLARE static int has_ally(struct Unit *unit)
{
	int i;

	for (i = 0; i < ARRAY_COUNT_RANGE2x2; i++) {
		int _x = unit->xPos + gVecs_2x2[i].x;
		int _y = unit->yPos + gVecs_2x2[i].y;
		struct Unit *unit_ally = GetUnitAtPosition(_x, _y);

		if (!UNIT_IS_VALID(unit_ally))
			continue;

		if (unit_ally->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
			continue;

		if (AreUnitsAllied(unit->index, unit_ally->index)) {
			i = 1;
#if defined(SID_Amaterasu) && (COMMON_SKILL_VALID(SID_Amaterasu))
			if (SkillTester(unit_ally, SID_Amaterasu))
				i += 1 << 2;
#endif
			return i;
		}
	}
	return false;
}

STATIC_DECLAR int GetPrePhaseHealAmount(struct Unit *unit)
{
	int ret = 0;
	int res = has_ally(unit);

	if (res) {
#if defined(SID_Camaraderie) && (COMMON_SKILL_VALID(SID_Camaraderie))
		if (SkillTester(unit, SID_Camaraderie))
			ret += Div(GetUnitMaxHp(unit) * SKILL_EFF0(SID_Camaraderie), 100);
#endif

#if defined(SID_Amaterasu) && (COMMON_SKILL_VALID(SID_Amaterasu))
		if (res & 1 << 2)
			ret += Div(GetUnitMaxHp(unit) * SKILL_EFF0(SID_Amaterasu), 100);
#endif
	} else {
#if defined(SID_Relief) && (COMMON_SKILL_VALID(SID_Relief))
		if (SkillTester(unit, SID_Relief))
			ret += Div(GetUnitMaxHp(unit) * SKILL_EFF0(SID_Relief), 100);
#endif
	}

#if defined(SID_RenewalPlus) && (COMMON_SKILL_VALID(SID_RenewalPlus))
    if (SkillTesterPlus(unit, SID_RenewalPlus))
        ret += Div(GetUnitMaxHp(unit) * SKILL_EFF0(SID_RenewalPlus), 100);
#endif

#if defined(SID_Renewal) && (COMMON_SKILL_VALID(SID_Renewal))
	if (SkillTester(unit, SID_Renewal))
		ret += Div(GetUnitMaxHp(unit) * SKILL_EFF0(SID_Renewal), 100);
#endif

#if defined(SID_Imbue) && (COMMON_SKILL_VALID(SID_Imbue))
	if (SkillTester(unit, SID_Imbue))
		ret += GetUnitMagic(unit);
#endif

#if defined(SID_Forager) && (COMMON_SKILL_VALID(SID_Forager))
	if (SkillTester(unit, SID_Forager)) {
		const unsigned int terrainId = gBmMapTerrain[unit->yPos][unit->xPos];

		if (terrainId == TERRAIN_PLAINS || terrainId == TERRAIN_FOREST || terrainId == TERRAIN_MOUNTAIN)
			ret += Div(GetUnitMaxHp(unit) * SKILL_EFF0(SID_Forager), 100);
	}
#endif

#if defined(SID_RainDish) && (COMMON_SKILL_VALID(SID_RainDish))
	if (SkillTester(unit, SID_RainDish) && gPlaySt.chapterWeatherId == WEATHER_RAIN)
		ret += Div(GetUnitMaxHp(unit) * SKILL_EFF0(SID_Renewal), 100);
#endif

#if defined(SID_Geomancy) && (COMMON_SKILL_VALID(SID_Geomancy))
    if (SkillTester(unit, SID_Geomancy))
    {
        switch (gBmMapTerrain[unit->yPos][unit->xPos])
        {
        case TERRAIN_FORT:
        case TERRAIN_FLOOR_17:
        case TERRAIN_FLOOR_18:
        case TERRAIN_WALL_1A:
        case TERRAIN_WALL_1B:
        case TERRAIN_RUBBLE:
        case TERRAIN_PILLAR:
        case TERRAIN_DOOR:
        case TERRAIN_THRONE:
        case TERRAIN_CHEST_20:
        case TERRAIN_CHEST_21:
        case TERRAIN_ROOF:
        case TERRAIN_STAIRS:
        case TERRAIN_TILE_2E:
        case TERRAIN_GLACIER:
        case TERRAIN_GUNNELS:
            ret += 10;
            break;

        default:
            break;
        }
    }
#endif

#if defined(SID_GoodyBasket) && (COMMON_SKILL_VALID(SID_GoodyBasket))
    if (SkillTester(unit, SID_GoodyBasket))
        ret += Div(GetUnitMaxHp(unit) * unit->lck, 100);
#endif

#if defined(SID_ManaTransfusion) && (COMMON_SKILL_VALID(SID_ManaTransfusion))
    if (SkillTester(unit, SID_ManaTransfusion))
    {
		struct NewBwl* bwl;
		bwl = GetNewBwl(unit->pCharacterData->number);

		if (bwl != NULL)
			ret += gMpSystemPInfoConfigList[unit->pCharacterData->number].idleGeneration;
	}
#endif

    if (GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_RENEWAL)
	{
        ret += Div(GetUnitMaxHp(unit) * SKILL_EFF0(SID_Renewal), 100);
	}

	if (gpExternalPrePhaseHealCalc)
		ret = gpExternalPrePhaseHealCalc(ret, unit);

	return ret;
}

LYN_REPLACE_CHECK(MakeTerrainHealTargetList);
void MakeTerrainHealTargetList(int faction)
{
	InitTargets(0, 0);

	FOR_UNITS_ONMAP_FACTION(gPlaySt.faction, unit, {
		int terrainId;
		int amount;

		if (unit->state & (US_DEAD | US_NOT_DEPLOYED | US_RESCUED | US_BIT16))
			continue;

		terrainId = gBmMapTerrain[unit->yPos][unit->xPos];

		if (GetUnitCurrentHp(unit) != GetUnitMaxHp(unit)) {
			/* Heal */
			int terrain_heal = Div(GetTerrainHealAmount(terrainId) * GetUnitMaxHp(unit), 100);
			int skill_heal = GetPrePhaseHealAmount(unit);

			amount = terrain_heal + skill_heal;
			if (amount != 0)
				AddTarget(unit->xPos, unit->yPos, unit->index, amount);
		}

		if (GetTerrainHealsStatus(terrainId) == 0)
			continue;

		if (GetUnitStatusIndex(unit) == UNIT_STATUS_NONE)
			continue;

		if (GetUnitStatusIndex(unit) == UNIT_STATUS_13)
			SetUnitStatus(unit, UNIT_STATUS_PETRIFY);

		AddTarget(unit->xPos, unit->yPos, unit->index, -1);
	})
}
