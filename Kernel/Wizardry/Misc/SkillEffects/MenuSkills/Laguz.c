#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"
#include "jester_headers/class-pairs.h"
#include "bwl.h"

u8 Transform_Laguz_Usability(const struct MenuItemDef *def, int number)
{
    if (gpKernelDesignerConfig->laguz_bars == true)
    {
        
        if (gActiveUnit->state & US_CANTOING)
            return MENU_NOTSHOWN;

        #if defined(SID_FormShift) && (COMMON_SKILL_VALID(SID_FormShift))
            if (SkillTester(gActiveUnit, SID_FormShift))
                return MENU_ENABLED;
        #endif

        #if defined(SID_HalfShift) && (COMMON_SKILL_VALID(SID_HalfShift))
            if (SkillTester(gActiveUnit, SID_HalfShift))
                return MENU_ENABLED;
        #endif

        FORCE_DECLARE struct NewBwl * bwl;
        bwl = GetNewBwl(UNIT_CHAR_ID(gActiveUnit));

        if (bwl->laguzBar != 30)
            return MENU_NOTSHOWN;

        return MENU_ENABLED;
    }

    return MENU_DISABLED;
}

u8 Transform_Laguz_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
        return MENU_ACT_SND6B;
    }

    gActionData.unk08 = 0x401;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

u8 Transform_Laguz_Effect(struct MenuProc * menu, struct MenuItemProc * item)
{
    FORCE_DECLARE struct NewBwl * bwl;
    bwl = GetNewBwl(UNIT_CHAR_ID(gActiveUnit));

    for (int i = 0; i < (int)ARRAY_COUNT(laguzPairs); i++)
    {

        if (gActiveUnit->pClassData->number == laguzPairs[i][0])
        {
            gActiveUnit->pClassData = GetClassData(laguzPairs[i][1]);
#if defined(SID_HalfShift) && (COMMON_SKILL_VALID(SID_HalfShift))
            if (SkillTester(gActiveUnit, SID_HalfShift) && bwl->laguzBar < 30)
            {
                SetUnitStatDebuff(gActiveUnit, UNIT_STAT_BUFF_LAGUZ_HALFSHIFT);
                bwl->laguzBar = 30;
                break;
            }
#endif
            SetUnitStatDebuff(gActiveUnit, UNIT_STAT_BUFF_LAGUZ);
            break;

        }
        else if (gActiveUnit->pClassData->number == laguzPairs[i][1])
        {
            gActiveUnit->pClassData = GetClassData(laguzPairs[i][0]);
            ClearUnitStatDebuff(gActiveUnit, UNIT_STAT_BUFF_LAGUZ);
            ClearUnitStatDebuff(gActiveUnit, UNIT_STAT_BUFF_LAGUZ_HALFSHIFT);
        }
    }

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}