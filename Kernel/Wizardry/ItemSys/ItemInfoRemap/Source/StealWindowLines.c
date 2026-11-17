/*
 * The steal window's inventory list (the Steal command's target window), moved here from MiscFunctions.c (vanilla
 * RefreshUnitStealInventoryInfoWindow). Its uses line drew every item's uses with no infinite_durability check, so a
 * weapon's uses showed in the steal window.
 */
#include "common-chax.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "item-sys.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/Forging.h"

LYN_REPLACE_CHECK(RefreshUnitStealInventoryInfoWindow);
void RefreshUnitStealInventoryInfoWindow(struct Unit* unit)
{
    int i;
    int itemCount;
    int xPos;
    struct UnitInfoWindowProc* proc;

    itemCount = GetUnitItemCount(unit);

    xPos = GetUnitInfoWindowX(unit, 0xd);

    proc = UnitInfoWindow_DrawBase(0, unit, xPos, 0, 0xd, itemCount);

    bool stealPlus = false;

#if defined(SID_StealPlus) && (COMMON_SKILL_VALID(SID_StealPlus))
    if (SkillTester(gActiveUnit, SID_StealPlus))
        stealPlus = true;
#endif

    for (i = 0; i < itemCount; i++)
    {
        int yPos = 0 + i * 2 + 3;

        int item = unit->items[i];
        s8 stealable = IsItemStealable(item);

        // Skill holder cannot steal a unit's equipped weapon
        if (GetUnitEquippedWeaponSlot(GetUnit(gActionData.targetIndex)) == i && stealPlus)
            stealable = false;

        ClearText(proc->lines + i);

        Text_SetColor(proc->lines + i, stealable ? 0 : 1);
        Text_DrawString(proc->lines + i, GetItemName(item));

        PutText(proc->lines + i, gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 3, yPos));

        if (ShouldDisplayItemDurability(item)) /* infinite_durability hides weapon uses */
            PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 11, yPos), stealable ? 2 : 1, GetItemUses(item));

        if (gpKernelDesignerConfig->forge_mechanic == true)
        {
            struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];

            if (CanItemBeForged(item)) {
                /* Only a forged item has a gold +N */
                if (GetItemForgeCount(item) > 0) {
                    PutSpecialChar(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 8, yPos), stealable ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY, TEXT_SPECIAL_PLUS);
                    PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 9, yPos), stealable ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY, GetItemForgeCount(item));
                }
                if (ShouldDisplayItemDurability(item)) /* infinite_durability hides weapon uses */
                    PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 11, yPos), stealable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetForgedItemDurability(item));
            }
            else if (limits.maxCount == 0 && ShouldDisplayItemDurability(item)) {
                PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 11, yPos), stealable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
            }
        }

        DrawIcon(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 1, yPos), GetItemIconId(item), 0x4000);
    }

    return;
}
