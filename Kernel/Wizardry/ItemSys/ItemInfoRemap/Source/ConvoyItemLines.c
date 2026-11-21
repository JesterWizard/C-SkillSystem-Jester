/*
 * The prep item list's line drawing for the convoy (Supply) and the other lists of prep items, moved here from
 * MiscFunctions.c (vanilla 0x0809D300, its scrolling twin 0x0809D47C, 0x08099F7C and DrawPrepScreenItems at 0x0809B74C).
 * Their forge branches drew a forgeable item's durability with no infinite_durability check, so a weapon's uses showed in the convoy on the world map and
 * in the prep screens.
 */
#include "common-chax.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "prepscreen.h"
#include "item-sys.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/Forging.h"

//! FE8U = 0x0809D300 (DisplayConvoyItemStrings?)
LYN_REPLACE_CHECK(sub_809D300);
void sub_809D300(struct Text* textBase, u16* tm, int yLines, struct Unit* unit)
{
    int i;

    TileMap_FillRect(tm, 12, 31, 0);

    if (gUnknown_02012F56 == 0) {
        ClearText(textBase);
        Text_InsertDrawString(textBase, 0, 1, GetStringFromIndex(0x5a8)); // TODO: msgid "Nothing[.]"
        PutText(textBase, tm + 3);
        return;
    }

    for (i = yLines; (i < yLines + 7) && (i < gUnknown_02012F56); i++) {
        struct Text* th = textBase + (i & 7);
        int item = gPrepScreenItemList[i].item;
        int unusable = !IsItemDisplayUsable(unit, item);

        ClearText(th);

        Text_InsertDrawString(
            th,
            0,
            unusable,
            GetItemName(item)
        );

        DrawIcon(tm + TILEMAP_INDEX(1, i * 2 & 0x1f), GetItemIconId(item), 0x4000);

        PutText(th, tm + TILEMAP_INDEX(3, i * 2 & 0x1f));

        if (ShouldDisplayItemDurability(item))
        {
            PutNumberOrBlank(tm + TILEMAP_INDEX(12, i * 2 & 0x1f), !unusable ? 2 : 1, GetItemUses(item));
        }

        if (gpKernelDesignerConfig->forge_mechanic == true)
        {
            struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
            if (CanItemBeForged(item)) {
                /* Only a forged item has a gold +N */
                if (GetItemForgeCount(item) > 0) {
                    PutSpecialChar(tm + TILEMAP_INDEX(8, i * 2 & 0x1f),
                        !unusable ? TEXT_COLOR_SYSTEM_GOLD
                        : TEXT_COLOR_SYSTEM_GRAY,
                        TEXT_SPECIAL_PLUS);
                    PutNumberOrBlank(tm + TILEMAP_INDEX(9, i * 2 & 0x1f),
                        !unusable ? TEXT_COLOR_SYSTEM_GOLD
                        : TEXT_COLOR_SYSTEM_GRAY,
                        GetItemForgeCount(item));
                }
                if (ShouldDisplayItemDurability(item)) /* infinite_durability hides weapon uses */
                    PutNumberOrBlank(tm + TILEMAP_INDEX(12, i * 2 & 0x1f), !unusable ? 2 : 1,
                        GetForgedItemDurability(item));
            }
            else if (limits.maxCount == 0 && ShouldDisplayItemDurability(item)) {
                PutNumberOrBlank(tm + TILEMAP_INDEX(12, i * 2 & 0x1f), !unusable ? 2 : 1,
                    GetItemUses(item));
            }
        }
    }

    return;
}

//! FE8U = 0x0809D47C
LYN_REPLACE_CHECK(sub_809D47C);
void sub_809D47C(struct Text* textBase, u16* tm, int yLines, struct Unit* unit)
{
    if (gUnknown_02012F56 > yLines) {
        int y = (yLines * 2) & 0x1f;
        struct Text* th = textBase + (yLines & 7);
        int item = gPrepScreenItemList[yLines].item;
        int unusable = !IsItemDisplayUsable(unit, item);

        /* When scrolling down the infuse list, ensure all the items are white */
        if(Proc_Find(ProcScr_PrepItemListScreen_INFUSE))
            unusable = TEXT_COLOR_SYSTEM_WHITE;

        int offset = TILEMAP_INDEX(0, y);
        TileMap_FillRect(tm + offset, 12, 1, 0);

        ClearText(th);
        Text_InsertDrawString(th, 0, unusable, GetItemName(item));
        DrawIcon(tm + offset + 1, GetItemIconId(item), 0x4000);
        PutText(th, tm + offset + 3);

        if (ShouldDisplayItemDurability(item))
        {
            PutNumberOrBlank(tm + offset + 12, !unusable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
        }

        if (gpKernelDesignerConfig->forge_mechanic == true)
        {
            struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
            if (CanItemBeForged(item)) {
                /* Only a forged item has a gold +N */
                if (GetItemForgeCount(item) > 0) {
                    PutSpecialChar(tm + offset + 8,
                        !unusable ? TEXT_COLOR_SYSTEM_GOLD
                        : TEXT_COLOR_SYSTEM_GRAY,
                        TEXT_SPECIAL_PLUS);
                    PutNumberOrBlank(tm + offset + 9,
                        !unusable ? TEXT_COLOR_SYSTEM_GOLD
                        : TEXT_COLOR_SYSTEM_GRAY,
                        GetItemForgeCount(item));
                }
                if (ShouldDisplayItemDurability(item)) /* infinite_durability hides weapon uses */
                    PutNumberOrBlank(tm + offset + 12, !unusable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY,
                        GetForgedItemDurability(item));
            }
            else if (limits.maxCount == 0 && ShouldDisplayItemDurability(item)) {
                PutNumberOrBlank(tm + offset + 12, !unusable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
            }
        }
    }
}

//! FE8U = 0x08099F7C
LYN_REPLACE_CHECK(sub_8099F7C);
void sub_8099F7C(struct Text* th, u16* tm, struct Unit* unit, u16 flags) {
    int itemCount;
    int i;

    TileMap_FillRect(tm, 12, 20, 0);

    if ((flags & 2) != 0) {
        ResetIconGraphics();
    }

    if (!unit) {
        return;
    }

    itemCount = GetUnitItemCount(unit);

    for (i = 0; i < itemCount; th++, i++) {
        u16 item = unit->items[i];

        int isUnusable = ((flags & 4) != 0)
            ? !CanUnitUseItemPrepScreen(unit, item)
            : !IsItemDisplayUsable(unit, item);

        if ((flags & 1) == 0) {
            ClearText(th);
            Text_SetColor(th, isUnusable);
            Text_SetCursor(th, 0);
            Text_DrawString(th, GetItemName(item));
        }

        DrawIcon(tm + i * 0x40, GetItemIconId(item), 0x4000);

        PutText(th, tm + 2 + i * 0x40);

        if (gpKernelDesignerConfig->forge_mechanic == true)
        {
            struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
            if (CanItemBeForged(item)) {
                /* Only a forged item has a gold +N */
                if (GetItemForgeCount(item) > 0) {
                    PutSpecialChar(tm + 8 + i * 0x40,
                        !isUnusable ? TEXT_COLOR_SYSTEM_GOLD
                        : TEXT_COLOR_SYSTEM_GRAY,
                        TEXT_SPECIAL_PLUS);
                    PutNumberOrBlank(tm + 9 + i * 0x40,
                        !isUnusable ? TEXT_COLOR_SYSTEM_GOLD
                        : TEXT_COLOR_SYSTEM_GRAY,
                        GetItemForgeCount(item));
                }
                if (ShouldDisplayItemDurability(item)) /* infinite_durability hides weapon uses */
                    PutNumberOrBlank(tm + 11 + i * 0x40,
                        !isUnusable ? TEXT_COLOR_SYSTEM_BLUE
                        : TEXT_COLOR_SYSTEM_GRAY,
                        GetForgedItemDurability(item));
            }
            else if (limits.maxCount == 0 && ShouldDisplayItemDurability(item)) {
                PutNumberOrBlank(tm + 11 + i * 0x40,
                    !isUnusable ? TEXT_COLOR_SYSTEM_BLUE
                    : TEXT_COLOR_SYSTEM_GRAY,
                    GetItemUses(item));
            }
        }
        if (ShouldDisplayItemDurability(item))
        {
            PutNumberOrBlank(tm + 11 + i * 0x40, !isUnusable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
        }
    }

    return;
}

//! FE8U = 0x0809B74C
LYN_REPLACE_CHECK(DrawPrepScreenItems);
void DrawPrepScreenItems(u16* tm, struct Text* th, struct Unit* unit, u8 checkPrepUsability) {
    s8 isUsable;
    int i;
    int itemCount;

    TileMap_FillRect(tm, 11, 9, 0);

    itemCount = GetUnitItemCount(unit);

    for (i = 0; i < itemCount; i++) {
        int item = unit->items[i];

        if (checkPrepUsability != 0) {
            isUsable = CanUnitUseItemPrepScreen(unit, item);
        }
        else {
            isUsable = IsItemDisplayUsable(unit, item);
        }

        ClearText(th);
        PutDrawText(
            th,
            tm + i * 0x40 + 2,
            !isUsable ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE,
            0,
            0,
            GetItemName(item)
        );

        if (ShouldDisplayItemDurability(item))
        {
            PutNumberOrBlank(tm + i * 0x40 + 0xB, isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
        }

        if (gpKernelDesignerConfig->forge_mechanic == true)
        {
            struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
            if (CanItemBeForged(item)) {
                /* Only a forged item has a gold +N */
                if (GetItemForgeCount(item) > 0) {
                    PutSpecialChar(tm + i * 0x40 + 8,
                        isUsable ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY,
                        TEXT_SPECIAL_PLUS);
                    PutNumberOrBlank(tm + i * 0x40 + 9,
                        isUsable ? TEXT_COLOR_SYSTEM_GOLD
                        : TEXT_COLOR_SYSTEM_GRAY,
                        GetItemForgeCount(item));
                }
                if (ShouldDisplayItemDurability(item)) /* infinite_durability hides weapon uses */
                    PutNumberOrBlank(tm + i * 0x40 + 0xB,
                        isUsable ? TEXT_COLOR_SYSTEM_BLUE
                        : TEXT_COLOR_SYSTEM_GRAY,
                        GetForgedItemDurability(item));
            }
            else if (limits.maxCount == 0 && ShouldDisplayItemDurability(item)) {
                PutNumberOrBlank(tm + i * 0x40 + 0xB,
                    isUsable ? TEXT_COLOR_SYSTEM_BLUE
                    : TEXT_COLOR_SYSTEM_GRAY,
                    GetItemUses(item));
            }
        }

        DrawIcon(tm + i * 0x40, GetItemIconId(item), 0x4000);

        th++;
    }

    return;
}
