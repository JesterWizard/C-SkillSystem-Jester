#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"
#include "playst-expa.h"
#include "jester_headers/miscellaneous.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_SealedZone) && (COMMON_SKILL_VALID(SID_SealedZone))

u8 SealedZone_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_SealedZone_Used))
		return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

u8 SealedZone_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_No_Allies);
        return MENU_ACT_SND6B;
    }

    gActionData.unk08 = SID_SealedZone;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void callback_anim(ProcPtr proc)
{
    /* Needed here to remove the menu as fast as possible */
    ClearBg0Bg1();
    StartLightRuneAnim(proc, gActiveUnit->xPos, gActiveUnit->yPos);
}

static void callback_exec(ProcPtr proc)
{
    int centerX = gActiveUnit->xPos;
    int centerY = gActiveUnit->yPos;
    
    // Loop through a 5x5 area centered on the unit
    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            // Only place runes on the outer ring (exclude inner 3x3)
            if ((abs(dx) == 2 || abs(dy) == 2)) {
                int x = centerX + dx;
                int y = centerY + dy;
                
                if (!GetUnitAtPosition(x, y))
                    AddLightRune(x, y, TRAP_MAPSPRITE_PAL_LIGHT_RUNE);
            }
        }
    }
}

bool Action_SealedZone(ProcPtr parent)
{
    PlayStExpa_SetBit(PLAYSTEXPA_BIT_SealedZone_Used);

	NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
	return true;
}
#endif