#include "common-chax.h"
#include "hooks.h"
#include "stat-screen.h"
#include "skill-system.h"
#include "bwl.h"
#include "utf8.h"
#include "icon-rework.h"
#include "constants/skills.h"
#include "jester_headers/custom-structs.h"

// ---------------------------------------------------------------------------
// Unit promotion array
// ---------------------------------------------------------------------------
const UnitPromotions unit_promotions[] =
{
    UNIT_PROMOS(
        CHARACTER_EIRIKA,
        PROMO(CLASS_EIRIKA_MASTER_LORD,  SID_Acrobat, SID_Fury, SID_Adept),
        PROMO(CLASS_EPHRAIM_MASTER_LORD,  SID_LimitBreakerPlus, SID_LullDefense),
        PROMO(CLASS_PALADIN,  SID_LuckySeven)
    ),

    {0},
};

// ---------------------------------------------------------------------------
// Page draw
// ---------------------------------------------------------------------------
void DrawPage7Rework(void)
{
    struct Unit *unit = gStatScreen.unit;
    int charId = UNIT_CHAR_ID(unit);
    
    for (int i = 14; i < 21; i++)
        ClearText(&gStatScreen.text[i]);
    for (int i = 29; i < 35; i++)
        ClearText(&gStatScreen.text[i]);
    
    // Find the unit's promotion data
    const UnitPromotions *promo_data = NULL;
    for (int i = 0; unit_promotions[i].key != 0; i++)
    {
        if (unit_promotions[i].key == charId)
        {
            promo_data = &unit_promotions[i];
            break;
        }
    }
    
    // If we found promotion data for this character, use it
    if (promo_data != NULL)
    {
        // Reset event slot 8 for units who have promotion options so their R Text can be viewed
        gEventSlots[EVT_SLOT_8] = 0;
        int text_idx = 14;
        int promo_y_offsets[] = {0x1, 0x6, 0xB};
        int skill_y_offsets[] = {0x3, 0x8, 0xD};
        
        ResetIconGraphics();
        
        // Iterate through promotions
        for (int i = 0; i < 3; i++)
        {
            // Check if this promotion exists (classId != 0)
            if (promo_data->promotions[i].classId != 0)
            {
                // Draw class name
                PutDrawText(&gStatScreen.text[text_idx++], 
                        gUiTmScratchA + TILEMAP_INDEX(0x6, promo_y_offsets[i]), 
                        TEXT_COLOR_SYSTEM_GOLD, 0, 0, 
                        GetStringFromIndex(GetClassData(promo_data->promotions[i].classId)->nameTextId));
                
                // Draw skill icons for this promotion
                int x_offset = 0x6;
                for (int j = 0; j < 3; j++)
                {
                    // Check if this skill exists (skill != 0)
                    if (promo_data->promotions[i].skills[j] != 0)
                    {
                        DrawIcon(gUiTmScratchA + TILEMAP_INDEX(x_offset, skill_y_offsets[i]), 
                                SKILL_ICON(promo_data->promotions[i].skills[j]), 
                                TILEREF(0, STATSCREEN_BGPAL_ITEMICONS));
                        x_offset += 2; // Move to next icon position
                    }
                }
            }
        }
    }
    else
    {
        PutDrawText(
            &gStatScreen.text[14], 
            gUiTmScratchA + TILEMAP_INDEX(0x5, 0x8), 
            TEXT_COLOR_SYSTEM_GOLD, 
            0, 
            0,
            "No promotions" 
        );

        // Keep this for checking in HbRedirect__SSItem so that R text isn't triggered when no promotions are available for the unit
        gEventSlots[EVT_SLOT_8] = 0x1000;
    }
}