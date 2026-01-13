#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"
#include "unit-expa.h"
#include "action-expa.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_Switcher) && (COMMON_SKILL_VALID(SID_Switcher))

u8 Switcher_Usability(const struct MenuItemDef *def, int number)
{
    return MENU_ENABLED;
}

u8 Switcher_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
        return MENU_ACT_SND6B;
    }

    gActionData.unk08 = SID_Switcher;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static void callback_anim(ProcPtr proc)
{
    // PlaySoundEffect(0x269);
    // Proc_StartBlocking(ProcScr_DanceringAnim, proc);

    // BG_SetPosition(
    //     BG_0,
    //     -SCREEN_TILE_IX(gActiveUnit->xPos - 1),
    //     -SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

static void callback_exec(ProcPtr proc)
{
    if (CheckBitUES(gActiveUnit, UES_BIT_SWITCHER_SKILL_USED))
        ClearBitUES(gActiveUnit, UES_BIT_SWITCHER_SKILL_USED);
    else
        SetBitUES(gActiveUnit, UES_BIT_SWITCHER_SKILL_USED);

    gActionDataExpa.refrain_action = true;
    EndAllMus();
    StartStatusHealEffect(gActiveUnit, proc);
}

bool Action_Switcher(ProcPtr parent)
{
    NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
    return true;
}
#endif