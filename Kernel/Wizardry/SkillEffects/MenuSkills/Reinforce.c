#include "common-chax.h"
#include "map-anims.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "unit-expa.h"
#include "action-expa.h"
#include "playst-expa.h"
#include "event-rework.h"
#include "weapon-range.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_Reinforce) && (COMMON_SKILL_VALID(SID_Reinforce))

bool hasDuplicateItems(struct Unit * unit) {
    for (int i = 0; i < 5; i++) {
        if (unit->items[i] == 0) continue; // Skip empty slots
        for (int j = i + 1; j < 5; j++) {
            if (unit->items[j] == 0) continue; // Skip empty slots
            if (ITEM_INDEX(unit->items[i]) == ITEM_INDEX(unit->items[j])) {
                return true; // Found a duplicate
            }
        }
    }
    return false; // No duplicates found
}

u8 Reinforce_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (!hasDuplicateItems(gActiveUnit))
        return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

u8 Reinforce_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    gActionData.unk08 = SID_Reinforce;
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
    struct Unit * unit = gActiveUnit;
    // Loop through all 5 of a unit's items
    for (int i = 0; i < 5; i++) {
        if (unit->items[i] == 0) continue; // Skip empty slots
    
        int totalUses = ITEM_USES(unit->items[i]); // Get uses of current item
    
        // Look for duplicates of this item in remaining slots
        for (int j = i + 1; j < 5; j++) {
            if (unit->items[j] == 0) continue; // Skip empty slots in inner loop too
            if (ITEM_INDEX(unit->items[j]) == ITEM_INDEX(unit->items[i])) { // Found duplicate
                // Add the uses of the duplicate item
                totalUses += ITEM_USES(unit->items[j]);
            
                // Delete the duplicate item
                unit->items[j] = 0;
            }
        }

        // Cap to prevent overflow
        if (totalUses > 254)
            totalUses = 254;
    
        // Save the aggregated uses back to the original item
        // Extract itemID and recombine with new uses
        u8 itemID = unit->items[i] & 0xFF; // Extract itemID
        
        u16 newValue = (totalUses << 8) | itemID; // Recombine into a u16 value
        unit->items[i] = newValue;
    }

    // Compact the inventory - shift items up to fill gaps
    int writePos = 0;
    for (int readPos = 0; readPos < 5; readPos++) {
        if (unit->items[readPos] != 0) {
            unit->items[writePos] = unit->items[readPos];
            if (writePos != readPos) {
                unit->items[readPos] = 0; // Clear the old position
            }
            writePos++;
        }
    }
}

bool Action_Reinforce(ProcPtr parent)
{
    NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
    return true;
}

#endif