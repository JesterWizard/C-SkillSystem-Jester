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

#if defined(SID_ShufflePlus) && (COMMON_SKILL_VALID(SID_ShufflePlus))

u8 ShufflePlus_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_ShufflePlus_Used))
		return MENU_DISABLED;

    return MENU_ENABLED;
}

u8 ShufflePlus_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_No_Allies);
        return MENU_ACT_SND6B;
    }

    gActionData.unk08 = SID_ShufflePlus;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void callback_anim(ProcPtr proc)
{

}

static void callback_exec(ProcPtr proc)
{
#ifdef CONFIG_FOURTH_ALLEGIANCE
    int factionUnitCountLut[4] = { 62, 20, 50, 20 };
    int factionStarts[4] = { 0x01, 0x41, 0x81, 0xC1 };
    int numFactions = 4;
#else
    int factionUnitCountLut[3] = { 62, 20, 50 };
    int factionStarts[3] = { 0x01, 0x41, 0x81 };
    int numFactions = 3;
#endif

    for (int f = 0; f < numFactions; f++) {
        int start = factionStarts[f];
        int limit = factionUnitCountLut[f];

        struct Unit *unitList[limit];
        int count = 0;

        // Collect all valid units for this faction
        for (int i = 0; i < limit; i++) {
            struct Unit *unit = GetUnit(start + i);

            if (!UNIT_IS_VALID(unit))
                break;

            // Only include deployed units
            if (unit->xPos < 0 || unit->yPos < 0)
                continue;

            unitList[count++] = unit;
        }

        if (count < 2)
            continue; // Nothing to rotate

        // Save first unit's position
        s8 firstX = unitList[0]->xPos;
        s8 firstY = unitList[0]->yPos;

        // Shift all positions left
        for (int i = 0; i < count - 1; i++) {
            unitList[i]->xPos = unitList[i + 1]->xPos;
            unitList[i]->yPos = unitList[i + 1]->yPos;

            // Sync if it's the skill user
            if (unitList[i] == gActiveUnit) {
                gActionData.xMove = unitList[i]->xPos;
                gActionData.yMove = unitList[i]->yPos;
            }
        }

        // Wrap last unit to first position
        unitList[count - 1]->xPos = firstX;
        unitList[count - 1]->yPos = firstY;

        if (unitList[count - 1] == gActiveUnit) {
            gActionData.xMove = firstX;
            gActionData.yMove = firstY;
        }
    }

    // Clear UI
    BG_Fill(gBG0TilemapBuffer, 0);
    BG_Fill(gBG1TilemapBuffer, 0);
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

bool Action_ShufflePlus(ProcPtr parent)
{
    PlayStExpa_SetBit(PLAYSTEXPA_BIT_ShufflePlus_Used);

	NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
	return true;
}
#endif