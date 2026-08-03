#include "common-chax.h"
#include "kernel-lib.h"
#include "kernel/traps.h"
#include "bmidoten.h"
#include "enemy-fog-vision.h"
#include "skill-system.h"
#include "constants/skills.h"

#define ENEMY_FOG_VISION_MAP_BUFFER_SIZE 0x800

extern u8 gBmMapEnemyVisionBuffer[];
extern u8 **gBmMapEnemyVision;

static bool EnemyFogVisionIsEnabled(void)
{
    return gpKernelDesignerConfig->enemy_fog_vision
        && gPlaySt.chapterVisionRange
        && gActiveUnit
        && UNIT_FACTION(gActiveUnit) != FACTION_BLUE;
}

static bool EnemyFogVisionMapFits(void)
{
    int width = gBmMapSize.x;
    int height = gBmMapSize.y;
    int required;

    if (width <= 0 || height <= 0)
        return false;

    required = width * height + height * sizeof(u8 *);

    return required <= ENEMY_FOG_VISION_MAP_BUFFER_SIZE;
}

void BuildEnemyFogVision(void)
{
    struct Unit *unit;
    struct Trap *trap;
    u8 **previousMap;
    int faction;
    int i;

    gBmMapEnemyVision = NULL;

    if (!EnemyFogVisionIsEnabled() || !EnemyFogVisionMapFits())
        return;

    faction = UNIT_FACTION(gActiveUnit);
    previousMap = gWorkingBmMap;

    BmMapInit(
        gBmMapEnemyVisionBuffer,
        &gBmMapEnemyVision,
        gBmMapSize.x,
        gBmMapSize.y);

    SetWorkingBmMap(gBmMapEnemyVision);
    BmMapFill(gBmMapEnemyVision, 0);

    for (i = 0; i < GetFactionUnitAmount(faction); ++i)
    {
        unit = GetUnit(faction + i + 1);

        if (!UNIT_IS_VALID(unit) || !unit->pClassData)
            continue;

        if (unit->state & (US_HIDDEN | US_UNAVAILABLE | US_RESCUED))
            continue;

        int fogBoost = 0;

#if defined(SID_HazeHunter) && (COMMON_SKILL_VALID(SID_HazeHunter))
        if (SkillTester(unit, SID_HazeHunter))
            fogBoost += 5;
#endif

        MapAddInRange(
            unit->xPos,
            unit->yPos,
            GetUnitFogViewRange(unit) + fogBoost,
            1);
    }

    for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap)
    {
        if (trap->type != TRAP_TOGGLE_TORCH)
            continue;

        if (trap->extra <= 0)
            continue;

        MapAddInRange(trap->xPos, trap->yPos, 5, 1);
    }

    SetWorkingBmMap(previousMap);
}

bool EnemyFogVisionCanSeeUnit(struct Unit *unit)
{
    if (!unit)
        return false;

    if (!EnemyFogVisionIsEnabled())
        return true;

    /*
     * A failed allocation must not turn every AI target into an invalid
     * pointer lookup. Falling back to omniscience preserves vanilla behavior
     * for maps larger than the reserved working buffer.
     */
    if (!gBmMapEnemyVision)
        return true;

    if (unit->xPos < 0 || unit->yPos < 0
        || unit->xPos >= gBmMapSize.x || unit->yPos >= gBmMapSize.y)
        return false;

    return gBmMapEnemyVision[unit->yPos][unit->xPos] != 0;
}

bool EnemyFogVisionCanTargetUnit(struct Unit *unit)
{
    if (!unit)
        return false;

    if (!gActiveUnit)
        return true;

    if (AreUnitsAllied(gActiveUnit->index, unit->index))
        return true;

    return EnemyFogVisionCanSeeUnit(unit);
}
