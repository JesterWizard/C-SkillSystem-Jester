/*
 * Casual Mode for clean FE8U.
 *
 * A player unit that dies is only taken off the map for the rest of the
 * chapter: it keeps its supports and comes back at the next chapter.
 * Ported from the C Skill System's UnitKill (casual_mode designer-config flag).
 */
#include "global.h"
#include "bmunit.h"
#include "bmreliance.h"
#include "eventinfo.h"

/* Settings from Installer.event */
extern const u8 gCasualModeFlag[]; /* u16: 0 = always on, else event flag ID */

static bool IsCasualModeActive(void)
{
    int flag = gCasualModeFlag[0] | (gCasualModeFlag[1] << 8);

    if (flag == 0)
        return TRUE;

    return CheckFlag(flag) ? TRUE : FALSE;
}

/*
 * Called from the UnitKill hook (Source/CasualModeHook.s) for a dying player
 * unit that isn't a phantom, in place of vanilla's
 *     unit->state |= US_DEAD | US_HIDDEN;
 *     InitUnitsupports(unit);
 */
void CasualMode_KillPlayerUnit(struct Unit * unit)
{
    if (IsCasualModeActive())
    {
        /* Off the map and out of play for this chapter, but not dead.
         * ChapterChangeUnitCleanup only keeps US_DEAD between chapters,
         * so the unit is back for the next chapter. */
        unit->state |= US_HIDDEN | US_NOT_DEPLOYED;
        return;
    }

    unit->state |= US_DEAD | US_HIDDEN;
    InitUnitsupports(unit);
}
