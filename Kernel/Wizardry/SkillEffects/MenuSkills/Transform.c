#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"
#include "jester_headers/class-pairs.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_Transform) && (COMMON_SKILL_VALID(SID_Transform))

u8 Transform_Usability(const struct MenuItemDef *def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

u8 Transform_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
        return MENU_ACT_SND6B;
    }

    gActionData.unk08 = SID_Transform;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static void callback_anim(ProcPtr proc)
{
    PlaySoundEffect(0x269);
    Proc_StartBlocking(ProcScr_DanceringAnim, proc);

    BG_SetPosition(
        BG_0,
        -SCREEN_TILE_IX(gActiveUnit->xPos - 1),
        -SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

static void callback_exec(ProcPtr proc)
{
    for (int i = 0; i < (int)ARRAY_COUNT(transformationPairs); i++)
    {
        if (gActiveUnit->pClassData->number == transformationPairs[i][0])
        {
            gActiveUnit->pClassData = GetClassData(transformationPairs[i][1]);
            SetUnitStatDebuff(gActiveUnit, UNIT_STAT_BUFF_TRANSFORM);
            gActiveUnit->curHP += 7;
            gActiveUnit->maxHP += 7;
            break;
        }
        else if (gActiveUnit->pClassData->number == transformationPairs[i][1])
        {
            gActiveUnit->pClassData = GetClassData(transformationPairs[i][0]);
            ClearUnitStatDebuff(gActiveUnit, UNIT_STAT_BUFF_TRANSFORM);
            gActiveUnit->maxHP -= 7;
            break;
        }
    }
}

bool Action_Transform(ProcPtr parent)
{
    NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
    return true;
}
#endif