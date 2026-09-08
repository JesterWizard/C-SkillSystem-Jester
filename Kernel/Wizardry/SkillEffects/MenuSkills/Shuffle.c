#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"
#include "playst-expa.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/miscellaneous.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_Shuffle) && (COMMON_SKILL_VALID(SID_Shuffle))

u8 Shuffle_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_Shuffle_Used))
		return MENU_DISABLED;

    return MENU_ENABLED;
}

u8 Shuffle_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_No_Allies);
        return MENU_ACT_SND6B;
    }

    gActionData.unk08 = SID_Shuffle;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void callback_anim(ProcPtr proc)
{

}

static void callback_exec(ProcPtr proc)
{
    u32 faction = gPlaySt.faction;

#ifdef CONFIG_FOURTH_ALLEGIANCE
    int factionUnitCountLut[4] = { 62, 20, 50, 20 };
#else
    int factionUnitCountLut[3] = { 62, 20, 50 };
#endif

    int MAX_UNITS = factionUnitCountLut[faction >> 6];

    struct Unit *unitList[MAX_UNITS];
    int count = 0;

    // Collect units
    for (int i = 1; i < MAX_UNITS; i++) {

        struct Unit *unit = GetUnit(faction + i);
        
        if (!UNIT_IS_VALID(unit))
            break;

        unitList[count++] = unit;
    }

    if (count < 2) return;

    // Save first unit's position to temp
    s8 firstX = unitList[0]->xPos;
    s8 firstY = unitList[0]->yPos;

    // Shift all positions left
    for (int i = 0; i < count - 1; i++) {
        unitList[i]->xPos = unitList[i + 1]->xPos;
        unitList[i]->yPos = unitList[i + 1]->yPos;

        // 🔁 Sync gBattleActor if it's the skill user
        if (unitList[i] == gActiveUnit) {
            gActionData.xMove = unitList[i]->xPos;
            gActionData.yMove = unitList[i]->yPos;
        }
    }


    // Last unit gets first position
    unitList[count - 1]->xPos = firstX;
    unitList[count - 1]->yPos = firstY;

    if (unitList[count - 1] == gActiveUnit) {
        gActionData.xMove = firstX;
        gActionData.yMove = firstY;
    }

    // Clear UI
    BG_Fill(gBG0TilemapBuffer, 0);
    BG_Fill(gBG1TilemapBuffer, 0);
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

bool Action_Shuffle(ProcPtr parent)
{
    
#if defined(SID_ShufflePlus) && (COMMON_SKILL_VALID(SID_ShufflePlus))
	if (SkillTesterPlus(gActiveUnit, SID_ShufflePlus))
    {
        PlayStExpa_SetBit(PLAYSTEXPA_BIT_ShufflePlus_Used);
		Action_ShufflePlus(parent);
    }
    else
    {
        PlayStExpa_SetBit(PLAYSTEXPA_BIT_Shuffle_Used);
        NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
    }
#else
    PlayStExpa_SetBit(PLAYSTEXPA_BIT_Shuffle_Used);
	NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
#endif

	return true;
}
#endif