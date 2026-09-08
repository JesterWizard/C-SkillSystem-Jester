#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"
#include "jester_headers/miscellaneous.h"
#include "jester_headers/custom-functions.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_Acidic) && (COMMON_SKILL_VALID(SID_Acidic))

static void ForEachUnitAcidic(int x, int y, void(*func)(struct Unit* unit), u8 minRange, u8 maxRange) {
    InitTargets(x, y);

    MapAddInRange(x, y, maxRange, 1);
    MapAddInRange(x, y, minRange, -1);

    ForEachUnitInRange(func);

    return;
}

static void TryAddUnitToAcidicTargetList(struct Unit* unit) {

    bool isEnemy = gEventSlots[EVT_SLOT_9];

    if (AreUnitsAllied(gSubjectUnit->index, unit->index) && isEnemy) {
        return;
    }

    if (!AreUnitsAllied(gSubjectUnit->index, unit->index) && !isEnemy) {
        return;
    }

    if (unit->state & US_RESCUED) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);

    return;
}

static void MakeAcidicTargetList(struct Unit* unit) {
    int x = unit->xPos;
    int y = unit->yPos;

    gSubjectUnit = unit;

    u8 minRange = gEventSlots[EVT_SLOT_7];
    u8 maxRange = gEventSlots[EVT_SLOT_8];

    BmMapFill(gBmMapRange, 0);

    ForEachUnitAcidic(x, y, TryAddUnitToAcidicTargetList, minRange, maxRange);

    return;
}

u8 Acidic_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (!HasSelectTarget(gActiveUnit, MakeTargetListForAdjacentEnemies))
		return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

u8 Acidic_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{

    gActionData.unk08 = SID_Acidic;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    // return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
    return ItemCommandEffect(menu, item);
}

bool Action_Acidic(ProcPtr parent)
{
	return true;
}
#endif

#if defined(SID_AcidicPlus) && (COMMON_SKILL_VALID(SID_AcidicPlus))

    u8 AcidicPlus_Usability(const struct MenuItemDef * def, int number)
    {
        if (gActiveUnit->state & US_CANTOING)
            return MENU_NOTSHOWN;

        gEventSlots[EVT_SLOT_7] = 0; // Min range
        gEventSlots[EVT_SLOT_8] = 3; // Max range
        gEventSlots[EVT_SLOT_9] = TRUE; // isEnemy

        if (!HasSelectTarget(gActiveUnit, MakeAcidicTargetList))
            return MENU_NOTSHOWN;

        return MENU_ENABLED;
    }

    u8 AcidicPlus_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
    {

        gActionData.unk08 = SID_AcidicPlus;
        gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

        // return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
        return ItemCommandEffect(menu, item);
    }

    bool Action_AcidicPlus(ProcPtr parent)
    {
        return true;
    }
#endif