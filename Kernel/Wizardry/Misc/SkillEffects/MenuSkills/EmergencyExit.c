#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"
#include "playst-expa.h"
#include "action-expa.h"
#include "jester_headers/miscellaneous.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_EmergencyExit) && (COMMON_SKILL_VALID(SID_EmergencyExit))

u8 EmergencyExit_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (gActiveUnit->curHP > (gActiveUnit->maxHP / 2))
        return MENU_NOTSHOWN;

    if (GetFactionUnitAmount(UNIT_FACTION(gActiveUnit)) < 2)
        return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

u8 EmergencyExit_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_No_Allies);
        return MENU_ACT_SND6B;
    }

    gActionData.unk08 = SID_EmergencyExit;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void callback_anim(ProcPtr proc)
{
    // Clear UI
    BG_Fill(gBG0TilemapBuffer, 0);
    BG_Fill(gBG1TilemapBuffer, 0);
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

	PlaySoundEffect(0x269);
	Proc_StartBlocking(ProcScr_DanceringAnim, proc);

	BG_SetPosition(
		BG_0,
		-SCREEN_TILE_IX(gActiveUnit->xPos - 1),
		-SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

/* Loop through the units in your faction to find the next replacement for your cursor */
static void callback_exec(ProcPtr proc)
{
    struct Unit * unit = GetUnit(gActiveUnit->index);
    HideUnitSprite(unit);
    unit->state |= US_HIDDEN;
    int faction = UNIT_FACTION(gActiveUnit);

    int i, amount = GetFactionUnitAmount(faction);

	for (i = 1; i <= amount; i++)
    {
		struct Unit * testUnit = GetUnit(faction + i);

		if (testUnit->pCharacterData != NULL && !(testUnit->state &= US_HIDDEN))
        {
            if (testUnit->pCharacterData->number != gActiveUnit->pCharacterData->number)
            {
			    gActiveUnit = testUnit;
                break;
            }
        }
	}

    gActionData.xMove = gActiveUnit->xPos;
    gActionData.yMove = gActiveUnit->yPos;

    gActionDataExpa.refrain_action = true;
    EndAllMus();
    
}

bool Action_EmergencyExit(ProcPtr parent)
{
	NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
	return true;
}
#endif