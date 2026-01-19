#include "common-chax.h"
#include "kernel-lib.h"
#include "jester_headers/custom-arrays.h"

typedef int (*KillUnitFunc_t)(struct Unit *unit);
// extern const KillUnitFunc_t gKillUnitHooks[];
extern KillUnitFunc_t const *const gpKillUnitHooks;

int GetDeadUnitCount(void)
{
    int count = 0;

    for (int i = 0; i < (int)ARRAY_COUNT(gDeadUnits); i++)
    {
        if (gDeadUnits[i] == 0x00)
            break;

        count++;
    }

    return count;
}

void AddDeadUnit(u8 unitId)
{
    for (int i = 0; i < (int)ARRAY_COUNT(gDeadUnits); i++)
    {
        if (gDeadUnits[i] == 0x00)  // sentinel
        {
            gDeadUnits[i] = unitId;
            return; // added
        }
    }
    // array full → ignore overflow
}

u8 GetLastDeadUnit(void)
{
    for (int i = ARRAY_COUNT(gDeadUnits) - 1; i >= 0; i--)
    {
        if (gDeadUnits[i] != 0x00)
            return gDeadUnits[i];
    }

    return 0x00; // none stored
}

LYN_REPLACE_CHECK(UnitKill);
void UnitKill(struct Unit *unit)
{
#if CHAX
	const KillUnitFunc_t *it;
	for (it = gpKillUnitHooks; *it; it++)
		(*it)(unit);
#endif

	AddDeadUnit(unit->index);

	if (UNIT_FACTION(unit) == FACTION_BLUE) {
		if (UNIT_IS_PHANTOM(unit))
			unit->pCharacterData = NULL;
		else 
		{	
			if (gpKernelDesignerConfig->casual_mode == true)
				unit->state |= US_HIDDEN;
			else
			{
				unit->state |= US_DEAD | US_HIDDEN;
				InitUnitsupports(unit);
			}
		}
	}
	else
	{
		if (gpKernelDesignerConfig->collect_dead_units == true)
		{
			unit->state |= US_DEAD | US_HIDDEN;
			unit->curHP = unit->maxHP;
		}
		else
			unit->pCharacterData = NULL;
	}
}
