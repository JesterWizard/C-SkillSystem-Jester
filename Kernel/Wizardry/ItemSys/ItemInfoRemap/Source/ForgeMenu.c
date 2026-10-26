#include "common-chax.h"
#include "gaiden-magic.h"
#include "item-sys.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "bmitem.h"
#include "jester_headers/Forging.h"
#include "icon-rework.h"

/*
** Forge menu UI. It lives in FreeSpaceDEMO (WIZARDRY_CUSTOM) rather than kernel .text,
** the forge data and rules it uses stay in ItemInfos.c.
*/

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
    NewPopup_VerySimple(MSG_WeaponForged, 0x5A, proc);
}

const struct MenuItemDef gForgeMenuItems[] = {
    {"", 0, 0, 0, 0, ForgeMenuItemUsability, DrawForgeMenuItem, ForgeMenuOnSelect, 0, ForgeMenuSwitchIn, 0},
    {"", 0, 0, 0, 0, ForgeMenuItemUsability, DrawForgeMenuItem, ForgeMenuOnSelect, 0, ForgeMenuSwitchIn, 0},
    {"", 0, 0, 0, 0, ForgeMenuItemUsability, DrawForgeMenuItem, ForgeMenuOnSelect, 0, ForgeMenuSwitchIn, 0},
    {"", 0, 0, 0, 0, ForgeMenuItemUsability, DrawForgeMenuItem, ForgeMenuOnSelect, 0, ForgeMenuSwitchIn, 0},
    {"", 0, 0, 0, 0, ForgeMenuItemUsability, DrawForgeMenuItem, ForgeMenuOnSelect, 0, ForgeMenuSwitchIn, 0},
    MenuItemsEnd
};

const int xCoord_menu = 13;
const int yCoord_menu = 8;
const int width_menu = 17;
const int height_menu = 0;

// the portrait isn't part of the menu's BGs, so it has to be removed however the menu closes
static void ForgeMenuEnd(struct MenuProc* menu) {
    EndFaceById(0);
}

static u8 ForgeMenuCancel(struct MenuProc* menu, struct MenuItemProc* menuItem) {
    ResetForgePreview(gActiveUnit->items[menu->itemCurrent]);
    return MenuCancelSelect(menu, menuItem);
}

/*
** R text: the item rows of the menu, plus the max forge skill line in the bottom left.
** The moving help box control proc is started with the forge menu as its parent, which is
** how the callbacks find the menu's hovered item (populate runs on a different proc).
*/
extern const struct HelpBoxInfo sForgeHbItems[UNIT_ITEM_COUNT];
extern const struct HelpBoxInfo sForgeHbSkill;
extern const struct HelpBoxInfo sForgeHbBack;
extern const struct ProcCmd ProcScr_ForgeSkillFocus[];

static int ForgeHbMenuCurrent(void) {
    struct Proc *parent = ((struct Proc *)Proc_Find(gProcScr_HelpBoxMoveCtrl))->proc_parent;

    // when opened from the skill line cursor, its proc sits between the help box and the menu
    if (Proc_Find(ProcScr_ForgeSkillFocus))
        parent = parent->proc_parent;

    return ((struct MenuProc *)parent)->itemCurrent;
}

static int ForgeHbSkill(void) {
    int item = gActiveUnit->items[ForgeHbMenuCurrent()];

    if (!CanItemBeForged(item))
        return 0;

    return gpConstSkillTable_Forge[GetItemIndex(item)];
}

static void HbPopulate_ForgeItem(struct HelpBoxProc* proc) {
    proc->item = gActiveUnit->items[proc->info->mid];
    proc->mid = GetItemDescId(proc->item);
}

static void HbRedirect_ForgeItem(struct HelpBoxProc* proc) {
    // items are packed at the top, so an empty row means we moved down past the last one
    if (!gActiveUnit->items[proc->info->mid])
        TryRelocateHbUp(proc);
}

static void HbRedirect_ForgeBack(struct HelpBoxProc* proc) {
    proc->info = &sForgeHbItems[ForgeHbMenuCurrent()];
}

static void HbRedirect_ForgeSkill(struct HelpBoxProc* proc) {
    if (!ForgeHbSkill())
        HbRedirect_ForgeBack(proc);
}

static void HbPopulate_ForgeSkill(struct HelpBoxProc* proc) {
    proc->mid = GetSkillDescMsg(ForgeHbSkill());
}

#define FORGE_HB_ITEM(i, up, down) \
    { up, down, &sForgeHbSkill, NULL, 14 * 8, (9 + 2 * (i)) * 8, i, HbRedirect_ForgeItem, HbPopulate_ForgeItem }

const struct HelpBoxInfo sForgeHbItems[UNIT_ITEM_COUNT] = {
    FORGE_HB_ITEM(0, NULL,               &sForgeHbItems[1]),
    FORGE_HB_ITEM(1, &sForgeHbItems[0], &sForgeHbItems[2]),
    FORGE_HB_ITEM(2, &sForgeHbItems[1], &sForgeHbItems[3]),
    FORGE_HB_ITEM(3, &sForgeHbItems[2], &sForgeHbItems[4]),
    FORGE_HB_ITEM(4, &sForgeHbItems[3], NULL),
};

const struct HelpBoxInfo sForgeHbSkill = {
    NULL, NULL, NULL, &sForgeHbBack, 2 * 8, 17 * 8, 0, HbRedirect_ForgeSkill, HbPopulate_ForgeSkill
};

// moving right off the skill line goes back to whichever item the menu is on
const struct HelpBoxInfo sForgeHbBack = {
    NULL, NULL, NULL, NULL, 0, 0, 0, HbRedirect_ForgeBack, NULL
};

static u8 ForgeMenuHelp(struct MenuProc* menu) {
    LoadHelpBoxGfx(NULL, -1);
    StartMovingHelpBox(&sForgeHbItems[menu->itemCurrent], (struct Proc *)menu);
    return 0;
}

/*
** Skill line cursor: pressing left once the preview can't go any lower moves the hand onto the
** max forge skill line, where A or R opens its help text. The proc blocks the menu while it's up,
** so the menu's own hand and input are paused until we hand control back.
*/
static void ForgeSkillFocus_Idle(struct Proc* proc) {
    u16 keys = gKeyStatusPtr->newKeys;
    struct MenuProc *menu = (struct MenuProc *)proc->proc_parent;

    DisplayUiHand(2 * 8, 17 * 8);

    if (keys & (A_BUTTON | R_BUTTON)) {
        Proc_Goto(proc, 1);
        return;
    }

    if (keys & (B_BUTTON | DPAD_RIGHT | DPAD_UP | DPAD_DOWN)) {
        PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
        DrawMenuItemHover(menu, menu->itemCurrent, TRUE);
        Proc_Goto(proc, 2);
    }
}

static void ForgeSkillFocus_StartHelp(struct Proc* proc) {
    LoadHelpBoxGfx(NULL, -1);
    StartMovingHelpBox(&sForgeHbSkill, proc);
}

const struct ProcCmd ProcScr_ForgeSkillFocus[] = {
    PROC_LABEL(0),
    PROC_REPEAT(ForgeSkillFocus_Idle),

    PROC_LABEL(1),
    PROC_CALL(ForgeSkillFocus_StartHelp),
    PROC_YIELD, // the key that closed the help box is still new this frame
    PROC_GOTO(0),

    PROC_LABEL(2),
    PROC_YIELD, // same for the key that leaves, so the menu doesn't act on it too
    PROC_END,
};

void ForgeMenuFocusSkill(struct MenuProc* menu) {
    int item = gActiveUnit->items[menu->itemCurrent];

    if (!CanItemBeForged(item) || !gpConstSkillTable_Forge[GetItemIndex(item)])
        return;

    PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
    DrawMenuItemHover(menu, menu->itemCurrent, FALSE);
    Proc_StartBlocking(ProcScr_ForgeSkillFocus, menu);
}

const struct MenuDef gForgeMenuDef = {
    {xCoord_menu, yCoord_menu, width_menu, height_menu},
    0,
    gForgeMenuItems,
    ForgeMenuInit,
    ForgeMenuEnd, 0,
    ForgeMenuCancel,
    ForgeMenuHelp,
    0
};

void ForgeMenuInit(struct MenuProc* proc) {
    ResetText();
    
    const int xCoord = 1;
    const int yCoord = 1;
    const int width = 12;
    const int height = 19;

    StartFace(0, GetUnitPortraitId(gActiveUnit), 0xAA, 0, 2);
    DrawUiFrame(gBG1TilemapBuffer, xCoord, yCoord, width, height, 0, 1);

    // OBJ palettes 8-10 are rewritten every frame on the map, so use 4 which is free while this menu is up
    StartSysBrownBox(6, 0x4800, 0x04, 0x800, 0x400, (struct Proc *) (proc));
    EnableSysBrownBox(1, 184, -1, 0);
    const char * str = "Forging";
    int xStart = ((8 * UNIT_PANEL_WIDTH) - GetStringTextLen(str)) / 2;
    PutDrawText(NULL, gBG0TilemapBuffer + TILEMAP_INDEX(24, 0), 0, xStart, UNIT_PANEL_WIDTH, str);
    
    struct Text *texts = gPrepItemTexts;
    
    InitText(&texts[0], 9);
    InitText(&texts[1], 9);
    InitText(&texts[2], 9);
    InitText(&texts[3], 9);
    InitText(&texts[4], 9);
    InitText(&texts[5], 9);
    InitText(&texts[6], 9);

    InitText(&texts[7], 14);
    InitText(&texts[8], 14);
    InitText(&texts[9], 14);
    InitText(&texts[10], 14);
    InitText(&texts[11], 14);

    InitText(&texts[12], 8);
    
    for(int i = 0; i < 12; i++) {
        ClearText(&texts[i]);
    }

    /*
    ** Hold the starting forge count for the weapon (this ASMC call) to prevent
    ** users from reducing it after it was already set in the last ASMC call
    */
    gEventSlots[EVT_SLOT_8] = GetItemForgeCount(gActiveUnit->items[0]); 
}

// Skill granted at the final forge level, in green once the forge being previewed reaches it
NOINLINE static void DrawForgeSkillLine(int item) {
    int sid = gpConstSkillTable_Forge[GetItemIndex(item)];

    ClearText(&gPrepItemTexts[12]);
    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 17), 1, 1, 0);

    if (!CanItemBeForged(item))
        return;

    if (!sid) {
        PutDrawText(&gPrepItemTexts[12], TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 17), TEXT_COLOR_SYSTEM_GRAY, 0, 0, "No skill");
        return;
    }

    DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 17), SKILL_ICON(sid), TILEREF(0, STATSCREEN_BGPAL_ITEMICONS + GetSkillIconPal(sid)));
    PutDrawText(&gPrepItemTexts[12], TILEMAP_LOCATED(gBG0TilemapBuffer, 4, 17),
        GetItemForgeCount(item) + 1 >= gForgeLimits[GetItemIndex(item)].maxCount ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_GRAY,
        0, 0, GetSkillNameStr(sid));
}

int ForgeMenuSwitchIn(struct MenuProc* menu, struct MenuItemProc* menuItem) {
    int item = gActiveUnit->items[menuItem->itemNumber];
    struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
    struct Text *texts = gPrepItemTexts;
    for(int i = 0; i < 12; i++) {
        ClearText(&texts[i]);
    }

    if (GetItemForgeCount(item) < limits.maxCount && gEventSlots[EVT_SLOT_7] == 0) {
        gEventSlots[EVT_SLOT_7] = (GetItemForgeCount(item) + 1) * limits.baseCost; // Initial gold value for the next level of forging;
    }

    PutDrawText(&texts[0], TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 2), TEXT_COLOR_SYSTEM_GOLD, 0, 0, "Mt");
    PutDrawText(&texts[1], TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 4), TEXT_COLOR_SYSTEM_GOLD, 0, 0, "Hit");
    PutDrawText(&texts[2], TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 6), TEXT_COLOR_SYSTEM_GOLD, 0, 0, "Wt");
    PutDrawText(&texts[3], TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 8), TEXT_COLOR_SYSTEM_GOLD, 0, 0, "Crit");
    PutDrawText(&texts[4], TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 15), TEXT_COLOR_SYSTEM_GOLD, 0, 0, "Cost");
    PutDrawText(&texts[5], TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 11), TEXT_COLOR_SYSTEM_GOLD, 0, 0, "Forges");
    PutDrawText(&texts[6], TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 13), TEXT_COLOR_SYSTEM_GOLD, 0, 0, "Gold");
    Text_InsertDrawNumberOrBlank(&texts[6], 0x3A, TEXT_COLOR_SYSTEM_BLUE, GetPartyGoldAmount());
    PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, 10, 13), TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_G);
    
    PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 2), TEXT_COLOR_SYSTEM_WHITE, TEXT_SPECIAL_ARROW);
    PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 4), TEXT_COLOR_SYSTEM_WHITE, TEXT_SPECIAL_ARROW);
    PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 6), TEXT_COLOR_SYSTEM_WHITE, TEXT_SPECIAL_ARROW);
    PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 8), TEXT_COLOR_SYSTEM_WHITE, TEXT_SPECIAL_ARROW);

    // Overwrite the values at x = 7, y = 11 with a 4x8 rectangle so we can update the new forge counts below
    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 11), 4, 1, 0);

    int count = GetItemForgeCount(item);
    
    if(limits.maxCount && (count < limits.maxCount)) {

        Text_InsertDrawNumberOrBlank(&texts[0], 0x1E, TEXT_COLOR_SYSTEM_BLUE, GetItemMight(item));
        Text_InsertDrawNumberOrBlank(&texts[1], 0x1E, TEXT_COLOR_SYSTEM_BLUE, GetItemHit(item));
        Text_InsertDrawNumberOrBlank(&texts[2], 0x1E, TEXT_COLOR_SYSTEM_BLUE, GetItemWeight(item));
        Text_InsertDrawNumberOrBlank(&texts[3], 0x1E, TEXT_COLOR_SYSTEM_BLUE, GetItemCrit(item));
        Text_InsertDrawNumberOrBlank(&texts[4], 0x3A, TEXT_COLOR_SYSTEM_BLUE, gEventSlots[EVT_SLOT_7]);

        // int forgedItem = item + (1 << 8);
        int forgedItem = item;

        int forgeSlot = ITEM_USES(item);
        if (!forgeSlot) {
            forgeSlot = InitFreeForgedItemSlot(item);
        }
        if (forgeSlot >= 0) { // ensure we found a valid forge ID
            // forgedItem = GetItemIndex(forgedItem) | forgeSlot << 8; // ensure the forge slot is set
            forgedItem = IncrementForgeCount(forgedItem, 1); // to show preview
        }

        Text_InsertDrawNumberOrBlank(&texts[0], 0x40, TEXT_COLOR_SYSTEM_GREEN, GetItemMight(forgedItem));
        Text_InsertDrawNumberOrBlank(&texts[1], 0x40, TEXT_COLOR_SYSTEM_GREEN, GetItemHit(forgedItem));
        Text_InsertDrawNumberOrBlank(&texts[2], 0x40, TEXT_COLOR_SYSTEM_GREEN, GetItemWeight(forgedItem));
        Text_InsertDrawNumberOrBlank(&texts[3], 0x40, TEXT_COLOR_SYSTEM_GREEN, GetItemCrit(forgedItem));

        PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 11), TEXT_COLOR_SYSTEM_BLUE, TEXT_SPECIAL_PLUS);
        PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 8, 11), TEXT_COLOR_SYSTEM_BLUE, count);
        PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, 9, 11), TEXT_COLOR_SYSTEM_WHITE, TEXT_SPECIAL_ARROW);
        PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, 10, 11), TEXT_COLOR_SYSTEM_GREEN, TEXT_SPECIAL_PLUS);
        PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 11, 11), TEXT_COLOR_SYSTEM_GREEN, count + 1);

        if (forgeSlot >= 0) {
            DecrementForgeCount(forgedItem, 1); // revert
        }
    }
    // If the item is at max forge count or cannot be forged, then show their text in green
    else {
        Text_InsertDrawNumberOrBlank(&texts[0], 0x1E, TEXT_COLOR_SYSTEM_GREEN, GetItemMight(item));
        Text_InsertDrawNumberOrBlank(&texts[1], 0x1E, TEXT_COLOR_SYSTEM_GREEN, GetItemHit(item));
        Text_InsertDrawNumberOrBlank(&texts[2], 0x1E, TEXT_COLOR_SYSTEM_GREEN, GetItemWeight(item));
        Text_InsertDrawNumberOrBlank(&texts[3], 0x1E, TEXT_COLOR_SYSTEM_GREEN, GetItemCrit(item));
        Text_InsertDrawNumberOrBlank(&texts[4], 0x3A, TEXT_COLOR_SYSTEM_GREEN, GetItemForgeCost(item));

        PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 11), TEXT_COLOR_SYSTEM_GREEN, TEXT_SPECIAL_PLUS);
        PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 8, 11), TEXT_COLOR_SYSTEM_GREEN, count);
    }

    DrawForgeSkillLine(item);

    // Because of some bullshit with graphics overlapping, we need to manually regenerate the menu items each time we move the cursor
    for(int i = 0; i < 5; i++) {
        if(gActiveUnit->items[i] != 0) {
            DrawItemForgeMenuLine(
                &texts[7 + i], 
                gActiveUnit->items[i], 
                TILEMAP_LOCATED(gBG0TilemapBuffer, 14, 9 + (i * 2))
            );
        }
    }

    BG_EnableSyncByMask(BG0_SYNC_BIT);

    return 0;
}

u8 ForgeMenuItemUsability(const struct MenuItemDef*, int number) {
    int item = gActiveUnit->items[number];
    
    if(item == 0) {
        return MENU_NOTSHOWN;
    }
    
    if(!(IsItemForgeable(item))) {
        return MENU_DISABLED;
    }
    
    return MENU_ENABLED;
}

int DrawForgeMenuItem(struct MenuProc* menu, struct MenuItemProc* menuItem) {
    int item = gActiveUnit->items[menuItem->itemNumber];
    
    DrawItemForgeMenuLine(&menuItem->text, item, TILEMAP_LOCATED(gBG0TilemapBuffer, menuItem->xTile, menuItem->yTile));
    
    BG_EnableSyncByMask(BG0_SYNC_BIT);

    return 0;
}

void DrawItemForgeMenuLine(struct Text* text, int item, u16* mapOut) {
    bool isForgeable = IsItemForgeable(item);
    Text_SetParams(text, 0, (isForgeable ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY));
    Text_DrawString(text, GetItemName(item));
    PutText(text, mapOut + 2);

    if (isForgeable) {
        PutNumberOrBlank(mapOut + 11, TEXT_COLOR_SYSTEM_GOLD, GetItemForgeCount(item));
        PutSpecialChar(mapOut + 12, TEXT_COLOR_SYSTEM_WHITE, TEXT_SPECIAL_SLASH);
        PutNumberOrBlank(mapOut + 14, TEXT_COLOR_SYSTEM_BLUE, GetForgedItemDurability(item));         
    }
    else
    {
        PutNumberOrBlank(mapOut + 11, TEXT_COLOR_SYSTEM_BLUE, GetItemUses(item));
        PutSpecialChar(mapOut + 12, TEXT_COLOR_SYSTEM_WHITE, TEXT_SPECIAL_SLASH);
        PutNumberOrBlank(mapOut + 14, TEXT_COLOR_SYSTEM_BLUE, GetItemMaxUses(item));
    }

    DrawIcon(mapOut, GetItemIconId(item), 0x4000);
}

u8 ForgeMenuOnSelect(struct MenuProc* menu, struct MenuItemProc* menuItem) {
    int item = gActiveUnit->items[menuItem->itemNumber];
    struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];

    if (IsItemForgeable(item)) {
        int forgeSlot = ITEM_USES(item);
        if (!forgeSlot) {
        forgeSlot = InitFreeForgedItemSlot(
            item); // returns ITEM_USES if no UseForgedItemDurability
        }
        if (forgeSlot >= 0) { // ensure we found a valid forge ID
            item = GetItemIndex(item) | forgeSlot << 8; // ensure the forge slot is set
            gPlaySt.partyGoldAmount -= GetItemForgeCost(item);
            //gActiveUnit->items[menuItem->itemNumber] = IncrementForgeCount(item, 1);
            SetItemForgeCount(item, GetItemForgeCount(item) + 1);
            // the skill list cache only compares item ids, so drop it in case the max forge skill was just unlocked
            ResetSkillLists();
            AnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
            return MENU_ACT_CLEAR | MENU_ACT_SND6A | MENU_ACT_END | MENU_ACT_SKIPCURSOR;
        }
    }

    if(limits.maxCount == 0) {
        MenuFrozenHelpBox(menu, ItemCannotBeForgedRText);
    }

    else if(GetItemForgeCount(item) >= limits.maxCount) {
        MenuFrozenHelpBox(menu, ItemAtMaxForgeCountRText);
    }
    
    else {
        MenuFrozenHelpBox(menu, NotEnoughGoldToForgeItemRText);
    }
    
    return MENU_ACT_SND6B;
}

void CallForgeMenuASMC(struct EventEngineProc* proc) {
    ClearBg0Bg1();
    SetDispEnable(1, 1, 1, 1, 1);
    SetTextFont(0);
    InitSystemTextFont();
    LoadUiFrameGraphics();
    ResetIconGraphics_();
    LoadIconPalettes(4); // callers like the debugger leave other palettes in BG 4/5
    gActionData.unk08 = 10000; // Set to a high value to check for when using the left and right arrow keys in the menu
    gEventSlots[EVT_SLOT_7] = 0;
    StartMenu(&gForgeMenuDef, proc);
}
