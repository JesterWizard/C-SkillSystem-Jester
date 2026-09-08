#include "common-chax.h"
#include "kernel-lib.h"
#include "map-anims.h"
#include "skill-system.h"
#include "event-rework.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "unit-expa.h"
#include "action-expa.h"
#include "strmag.h"
#include "bmtarget.h"
#include "playst-expa.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/macros.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_Gungnir) && (COMMON_SKILL_VALID(SID_Gungnir))

u8 Gungnir_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_Gungnir_Used))
        return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

u8 Gungnir_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_SKILL_Transform_FRtext);
        return MENU_ACT_SND6B;
    }

    gActionData.unk08 = SID_Gungnir;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    PlayStExpa_SetBit(PLAYSTEXPA_BIT_Gungnir_Used);

    PlaySoundEffect(0x6A);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static const EventScr EventScr_MenuSkillGungnir[] = {
    SHOOT_ARROW_DYNAMIC(10)
    NOFADE
    ENDA
};

bool Action_Gungnir(ProcPtr parent)
{
    gEventSlots[EVT_SLOT_B] = COORDS(gActiveUnit->xPos, gActiveUnit->yPos);
    ClearBg0Bg1();
    KernelCallEvent(EventScr_MenuSkillGungnir, EV_EXEC_CUTSCENE, parent);
    return true;
}
#endif