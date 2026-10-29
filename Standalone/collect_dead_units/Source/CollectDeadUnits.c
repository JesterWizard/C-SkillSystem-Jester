/*
 * Collect Dead Units for clean FE8U.
 *
 * Vanilla erases a dying enemy or NPC from the unit array at once
 * (unit->pCharacterData = NULL). With this patch the unit stays in its slot,
 * marked dead and hidden, until the end of the chapter, the same way vanilla
 * keeps dead player units. Events and other patches can then still find and
 * inspect it (or bring it back).
 * Ported from the C Skill System's UnitKill (collect_dead_units flag).
 */
#include "global.h"
#include "bmunit.h"

/*
 * UnitKill itself is hooked in Source/CollectDeadUnitsHook.s: a dying unit
 * that isn't a player unit gets unit->state |= US_DEAD | US_HIDDEN in place
 * of vanilla's unit->pCharacterData = NULL.
 */

/*
 * Replaces vanilla GetFreeUnit (FE8U: 0x08017838).
 *
 * Dead enemies and NPCs now keep their slot until the chapter ends, so a map
 * that loads more than 50 enemies (or 20 NPCs) over the chapter can run out of
 * slots, which vanilla never expects. When no slot is empty, reuse the slot of
 * a unit that is already dead instead of failing.
 */
struct Unit * GetFreeUnit_CollectDeadUnits(int faction)
{
    int i;
    int last = faction + 0x40;

    for (i = faction + 1; i < last; i++)
    {
        struct Unit * unit = GetUnit(i);

        if (unit != NULL && unit->pCharacterData == NULL)
            return unit;
    }

    if (faction == FACTION_BLUE)
        return NULL;

    for (i = faction + 1; i < last; i++)
    {
        struct Unit * unit = GetUnit(i);

        if (unit != NULL && unit->pCharacterData != NULL && (unit->state & US_DEAD))
        {
            ClearUnit(unit);
            return unit;
        }
    }

    return NULL;
}
