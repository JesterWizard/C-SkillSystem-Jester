#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "weapon-range.h"
#include "debuff.h"
#include "unit-expa.h"
#include "playst-expa.h"
#include "jester_headers/miscellaneous.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_Kamikaze) && (COMMON_SKILL_VALID(SID_Kamikaze))

u8 Kamikaze_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (!HasSelectTarget(gActiveUnit, MakeTargetListForAdjacentUnits))
		return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

int Kamikaze_Hover(struct MenuProc * menu, struct MenuItemProc * item)
{
    BmMapFill(gBmMapMovement, -1);
    BmMapFill(gBmMapRange, 0);
    GenerateUnitStandingReachRange(gActiveUnit, 0b11);
    DisplayMoveRangeGraphics(MOVLIMITV_MMAP_BLUE | MOVLIMITV_RMAP_RED);
    return 0;
}

int Kamikaze_Unhover(struct MenuProc * menu, struct MenuItemProc * menuItem)
{
    HideMoveRangeGraphics();
    return 0;
}

u8 Kamikaze_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_No_Adjacent_Units);
        return MENU_ACT_SND6B;
    }

    ClearBg0Bg1();

    gActionData.unk08 = SID_Kamikaze;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void callback_anim(ProcPtr proc)
{

}

static void callback_exec(ProcPtr proc)
{
    int i;

    MakeTargetListForAdjacentUnits(gActiveUnit);

    for (i = 0; i < GetSelectTargetCount(); i++)
    {
        struct Unit * unit = GetUnit(GetTarget(i)->uid);
        if (!UNIT_ALIVE(unit) && unit != gActiveUnit)
            continue;

        unit->curHP -= 10;

        if (unit->curHP <= 0)
            unit->curHP = 1;
    }



    gActiveUnit->curHP = 0;
    KillUnitOnCombatDeath(gActiveUnit, gActiveUnit);
}

bool Action_Kamikaze(ProcPtr parent)
{
	NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
	return true;
}
#endif