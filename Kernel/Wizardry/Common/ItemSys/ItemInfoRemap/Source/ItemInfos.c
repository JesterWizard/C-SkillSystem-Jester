#include "common-chax.h"
#include "item-sys.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "bmitem.h"
#include "jester_headers/Forging.h"
#include "bwl.h"

// use the macro ForgeBonus(mt, hit, crit, wt) to decide how an item improves each forge level
// each forge level, the item's stats will *increase* by the amount in gForgeBonuses / 6
// the number 6 was chosen to support halves and thirds of stats per level
const struct ForgeBonuses gForgeBonuses[NumOfForgables] = {
	{0.5 * ForgeLevels,  5 * ForgeLevels, 2.5 * ForgeLevels, -1 * ForgeLevels}
};

// "index" is the index of gForgeBonuses that the item gets bonuses from
// "max" is the maximum number of times an item can be forged
// "cost" is the amount of gold to forge once, multiplied by current forge count
// if an item has no entry in this table it is unforgeable
const struct ForgeLimits gForgeLimits[256] = {
    [ITEM_SWORD_IRON] = {
        .forgeIndex = 0, 
        .maxCount = 6, 
        .baseCost = 500
    }, // iron swords use index 0 forge bonuses, can be forged up to 6 times, cost 500 gold as a baseline
};

int GetForgedItemDurability(int item) {

    if (GetItemAttributes(item) & IA_UNBREAKABLE)
        return 0xFF;

    int id = ITEM_USES(item);
    if (id >= NumOfForgables)
        return 0;

    if (gForgedItemRam[id].unbreakable)
        return 0xFF;

    int uses = gForgedItemRam[id].uses;

    if (uses == 0) {
        // Fallback: If RAM is empty, treat as a fresh item
        return GetItemMaxUses(item); 
    }
    return uses;
}

void SetForgedItemDurability(int item, u8 value) {
    int id = ITEM_USES(item);
    if (id < 0 || id >= NumOfForgables)
        return;

    gForgedItemRam[id].uses = GetItemMaxUses(item) < value ? GetItemMaxUses(item) : value;
}

void MakeForgedItemUnbreakable(int item, bool state) {
    int id = ITEM_USES(item);
    if (id < 0 || id >= NumOfForgables)
        return;

    gForgedItemRam[id].unbreakable = state;
}

int SetForgedItemAfterUse(int item) {
    int uses = GetForgedItemDurability(item);
    
    if (!(gBattleStats.config & BATTLE_CONFIG_REAL)) {
        return uses;
    }

    uses -= 1;

    SetForgedItemDurability(item, uses);
    return uses;
}

void SetForgedItemDefaultUse(int item) {
    SetForgedItemDurability(item, GetItemMaxUses(item));
}

int InitFreeForgedItemSlot(int item) {
    for (int i = 1; i < NumOfForgables; ++i) {
        if (!GetForgedItemDurability(i << 8)) 
        { // if no durability, the item does not exist
            SetForgedItemDefaultUse(item | (i << 8));
            /* Ensure newly-allocated slot has deterministic defaults for
             * forge count and flags. Some memory backing the slot may be
             * non-zero (especially when mapped into convoy space), so
             * explicitly clear these fields. */
            gForgedItemRam[i].forgeCount = 0;
            gForgedItemRam[i].unbreakable = 0;
            gForgedItemRam[i].crit = 0;

            return i; // slot 0 would be 0 durability, so skip
        }
    }
    return -1;
}

int CanItemBeForged(int item) { // for item line drawing
    struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
    if (limits.maxCount == 0) {
        return false;
    }
    return true;
}

int GetItemForgeCount(int item) {
    struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
    int id = ITEM_USES(item);

    if (limits.maxCount == 0) {
        return 0;
    }

    return gForgedItemRam[id].forgeCount;
}

int SetItemForgeCount(int item, int val) {
    struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
    int id = ITEM_USES(item);

    if (limits.maxCount == 0) {
        return item;
    }
    
    gForgedItemRam[id].forgeCount = val;
    return item;
}

int IncrementForgeCount(int item, int amount) {
    int val = GetItemForgeCount(item);
    return SetItemForgeCount(item, val + amount);
}

int DecrementForgeCount(int item, int amount) {
    int val = GetItemForgeCount(item);
    return SetItemForgeCount(item, val - amount);
}

int GetItemForgeCost(int item) {
    struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
    int count = GetItemForgeCount(item);
    if (count >= limits.maxCount) {
        return 0;
    }

    return gEventSlots[EVT_SLOT_7];

    // return (count + 1) * limits.baseCost; // This is the cost for the (count+1)-th level only
}

bool IsItemForgeable(int item) { // do we have the cash
    struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
    int count = GetItemForgeCount(item);

    if (count >= limits.maxCount) {
        return false;
    }

    if (GetItemForgeCost(item) > (int)GetPartyGoldAmount()) {
        return false;
    }

    return true;
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

const struct MenuDef gForgeMenuDef = {
    {xCoord_menu, yCoord_menu, width_menu, height_menu},
    0,
    gForgeMenuItems,
    ForgeMenuInit,
    0, 0,
    MenuCancelSelect,
    0, 0
};

void ForgeMenuInit(struct MenuProc* proc) {
    ResetText();
    
    const int xCoord = 1;
    const int yCoord = 1;
    const int width = 12;
    const int height = 19;

    StartFace(0, GetUnitPortraitId(gActiveUnit), 0xAA, 0, 2);
    DrawUiFrame(gBG1TilemapBuffer, xCoord, yCoord, width, height, 0, 1);
    
    PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(9, 15), TEXT_COLOR_SYSTEM_BLUE, GetPartyGoldAmount());
    PutSpecialChar(gBG0TilemapBuffer + TILEMAP_INDEX(10, 15), TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_G);  

    StartSysBrownBox(6, 0x4800, 0x08, 0x800, 0x400, (struct Proc *) (proc));
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
    
    for(int i = 0; i < 12; i++) {
        ClearText(&texts[i]);
    }

    /*
    ** Hold the starting forge count for the weapon (this ASMC call) to prevent
    ** users from reducing it after it was already set in the last ASMC call
    */
    gEventSlots[EVT_SLOT_8] = GetItemForgeCount(gActiveUnit->items[0]); 
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
    PutDrawText(&texts[4], TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 17), TEXT_COLOR_SYSTEM_GOLD, 0, 0, " ");
    PutDrawText(&texts[5], TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 11), TEXT_COLOR_SYSTEM_GOLD, 0, 0, "Forges");
    PutDrawText(&texts[6], TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 13), TEXT_COLOR_SYSTEM_GOLD, 0, 0, "Gold Cost");
    
    PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 2), TEXT_COLOR_SYSTEM_WHITE, TEXT_SPECIAL_ARROW);
    PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 4), TEXT_COLOR_SYSTEM_WHITE, TEXT_SPECIAL_ARROW);
    PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 6), TEXT_COLOR_SYSTEM_WHITE, TEXT_SPECIAL_ARROW);
    PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 8), TEXT_COLOR_SYSTEM_WHITE, TEXT_SPECIAL_ARROW);
    PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 17), TEXT_COLOR_SYSTEM_WHITE, TEXT_SPECIAL_DASH);

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
    gActionData.unk08 = 10000; // Set to a high value to check for when using the left and right arrow keys in the menu
    gEventSlots[EVT_SLOT_7] = 0;
    StartMenu(&gForgeMenuDef, proc);
}

STATIC_DECLAR int MakeNewItemVanilla(int item)
{
	int uses = GetItemMaxUses(item);

	if (GetItemAttributes(item) & IA_UNBREAKABLE)
		uses = 0;

    if (gpKernelDesignerConfig->forge_mechanic == true)
    {
        struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
        if (limits.maxCount) {

            uses = InitFreeForgedItemSlot(item);
            if (uses < 0) {
                uses = 0; // need weapon usability routine to make 0 use weps unusable
            } // this only occurs when you exceed NumOfForgables
        }
    }

	return (uses << 8) + GetItemIndex(item);
}

LYN_REPLACE_CHECK(GetItemHit);
int GetItemHit(int item) {
	int result = GetItemData(ITEM_INDEX(item))->hit;

    if (gpKernelDesignerConfig->forge_mechanic == true)
    {
        struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
        struct ForgeBonuses bonuses = gForgeBonuses[limits.forgeIndex];
            
        int forgeHit = (GetItemForgeCount(item) * bonuses.hitBonus) / 6;
            
        result += forgeHit;		
    }

    return result;
}

LYN_REPLACE_CHECK(GetItemWeight);
int GetItemWeight(int item) {
	int result = GetItemData(ITEM_INDEX(item))->weight;

    if (gpKernelDesignerConfig->forge_mechanic == true)
    {
        struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
        struct ForgeBonuses bonuses = gForgeBonuses[limits.forgeIndex];
            
        int forgeWt = (GetItemForgeCount(item) * bonuses.wtBonus) / 6;
            
        result += forgeWt;

        if (result < 0)
            result = 0;
    }

    return result;
}

LYN_REPLACE_CHECK(GetItemCrit);
int GetItemCrit(int item) {
	int result = GetItemData(ITEM_INDEX(item))->crit;

    if (gpKernelDesignerConfig->forge_mechanic == true)
    {
        struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
        struct ForgeBonuses bonuses = gForgeBonuses[limits.forgeIndex];

        int forgeCrit = (GetItemForgeCount(item) * bonuses.critBonus) / 6;

        FORCE_DECLARE struct NewBwl * bwl = GetNewBwl(UNIT_CHAR_ID(GetUnit(gBattleActor.unit.index)));
        int id = ITEM_USES(item);
        result += gForgedItemRam[id].crit;

        result += forgeCrit;
    }

    return result;
}

LYN_REPLACE_CHECK(MakeNewItem);
int MakeNewItem(int item)
{
	if (IsDuraItem(item))
		return item;

	return MakeNewItemVanilla(item);
}

LYN_REPLACE_CHECK(GetItemNameWithArticle);
char *GetItemNameWithArticle(int item, s8 capitalize)
{
	return GetItemName(item);
}

LYN_REPLACE_CHECK(GetItemName);
char *GetItemName(int item)
{
	char *result;

#if CHAX
    if (IsTonicItem(item)) {
        return GetTonicItemName(item);
    }

	if (IsDuraItem(item))
		return GetDuraItemName(item);
#endif

	result = GetStringFromIndex(GetItemData(ITEM_INDEX(item))->nameTextId);
	result = StrInsertTact();
	return result;
}

LYN_REPLACE_CHECK(GetItemData);
const struct ItemData * GetItemData(int itemIndex) 
{
    return gItemData_New + itemIndex;
}

LYN_REPLACE_CHECK(GetItemAttributes);
int GetItemAttributes(int item) {
    return GetItemData(ITEM_INDEX(item))->attributes;
}

LYN_REPLACE_CHECK(GetItemType);
int GetItemType(int item) {
    if (!item)
        return 0xFF;

    return GetItemData(ITEM_INDEX(item))->weaponType;
}

LYN_REPLACE_CHECK(GetItemRequiredExp);
int GetItemRequiredExp(int item) {
    return GetItemData(ITEM_INDEX(item))->weaponRank;
}

LYN_REPLACE_CHECK(GetItemDescId);
int GetItemDescId(int item)
{
#if CHAX
    if (IsTonicItem(item)) {
        return GetTonicItemDescId(item);
    }

	if (IsDuraItem(item))
		return GetDuraItemDescId(item);
#endif

	return GetItemData(ITEM_INDEX(item))->descTextId;
}

LYN_REPLACE_CHECK(GetItemUseDescId);
int GetItemUseDescId(int item)
{
#if CHAX
    if (IsTonicItem(item)) {
        return GetTonicItemUseDescId(item);
    }

	if (IsDuraItem(item))
		return GetDuraItemUseDescId(item);
#endif

	return GetItemData(ITEM_INDEX(item))->useDescTextId;
}

LYN_REPLACE_CHECK(GetItemUses);
int GetItemUses(int item)
{
	if (GetItemAttributes(item) & IA_UNBREAKABLE)
		return 0xFF;

#if CHAX
	if (IsDuraItem(item))
		return 1;
#endif

    if (gpKernelDesignerConfig->forge_mechanic == true)
    {
        if (CanItemBeForged(item))
            return GetForgedItemDurability(item);
    }

	return ITEM_USES(item);
}

LYN_REPLACE_CHECK(GetItemMaxUses);
int GetItemMaxUses(int item)
{
	if (GetItemAttributes(item) & IA_UNBREAKABLE)
		return 0xFF;

#if CHAX
	if (IsDuraItem(item))
		return 1;
#endif

	return GetItemData(ITEM_INDEX(item))->maxUses;
}

LYN_REPLACE_CHECK(GetItemIconId);
int GetItemIconId(int item)
{
	if (!item)
		return -1;

#if CHAX
    if (IsTonicItem(item)) {
        return GetTonicItemIconId(item);
    }

	if (IsDuraItem(item))
		return GetDuraItemIconId(item);
#endif

    return GetItemData(ITEM_INDEX(item))->iconId;
}

LYN_REPLACE_CHECK(GetItemCost);
int GetItemCost(int item)
{
	if (GetItemAttributes(item) & IA_UNBREAKABLE)
		return GetItemData(ITEM_INDEX(item))->costPerUse;
	else
		return GetItemData(ITEM_INDEX(item))->costPerUse * GetItemUses(item);
}

LYN_REPLACE_CHECK(GetItemMaxCost);
int GetItemMaxCost(int item)
{
	return GetItemData(ITEM_INDEX(item))->costPerUse * GetItemMaxUses(item);
}

LYN_REPLACE_CHECK(GetItemMight);
int GetItemMight(int item) {
	int result = GetItemData(ITEM_INDEX(item))->might;

    if (gpKernelDesignerConfig->forge_mechanic == true)
    {
        struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
        struct ForgeBonuses bonuses = gForgeBonuses[limits.forgeIndex];
            
        int forgeMt = (GetItemForgeCount(item) * bonuses.mtBonus) / 6;
            
        result += forgeMt;
    }

    return result;
}

LYN_REPLACE_CHECK(DrawItemMenuLine);
void DrawItemMenuLine(struct Text * text, int item, s8 isUsable, u16 * mapOut)
{
    Text_SetParams(text, 0, (isUsable ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY));
    Text_DrawString(text, GetItemName(item));
    PutText(text, mapOut + 2);

    // Check if forge is active AND the item is forgeable
    if (gpKernelDesignerConfig->forge_mechanic == true && CanItemBeForged(item))
    {
        PutNumberOrBlank(mapOut + 11, isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, (GetForgedItemDurability(item) == 0 ? GetItemMaxUses(item) : GetForgedItemDurability(item)));
        PutSpecialChar(mapOut + 8, isUsable ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY, TEXT_SPECIAL_PLUS);
        PutNumberOrBlank(mapOut + 9, isUsable ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY, GetItemForgeCount(item));
    }
    else // Default behavior: display standard durability
    {
        #ifndef CONFIG_INFINITE_DURABILITY
        if (!IsDuraItem(item))
        {
            PutNumberOrBlank(mapOut + 11, isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
        }
        #endif
    }

    DrawIcon(mapOut, GetItemIconId(item), 0x4000);
}

LYN_REPLACE_CHECK(DrawItemMenuLineLong);
void DrawItemMenuLineLong(struct Text * text, int item, s8 isUsable, u16 * mapOut)
{
    Text_SetParams(text, 0, (isUsable ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY));
    Text_DrawString(text, GetItemName(item));

    PutText(text, mapOut + 2);

#ifndef CONFIG_INFINITE_DURABILITY
    if (!IsDuraItem(item))
    {
        PutNumberOrBlank(mapOut + 10, isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
        PutNumberOrBlank(mapOut + 13, isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemMaxUses(item));
        PutSpecialChar(mapOut + 11, isUsable ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY, TEXT_SPECIAL_SLASH);
    }
#endif

    if (gpKernelDesignerConfig->forge_mechanic == true)
    {
        struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
        if (CanItemBeForged(item)) {
            PutNumberOrBlank(mapOut + 13, isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetForgedItemDurability(item));
            PutSpecialChar(mapOut + 9, isUsable ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY, TEXT_SPECIAL_PLUS);
            PutNumberOrBlank(mapOut + 10, isUsable ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY, GetItemForgeCount(item));
        } else if (limits.maxCount == 0) {
            PutNumberOrBlank(mapOut + 10, isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
            PutNumberOrBlank(mapOut + 13, isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemMaxUses(item));
            PutSpecialChar(mapOut + 11, isUsable ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY, TEXT_SPECIAL_SLASH);
        }
    }

    DrawIcon(mapOut, GetItemIconId(item), 0x4000);
}

LYN_REPLACE_CHECK(DrawItemMenuLineNoColor);
void DrawItemMenuLineNoColor(struct Text * text, int item, u16 * mapOut)
{
    Text_SetCursor(text, 0);
    Text_DrawString(text, GetItemName(item));

    PutText(text, mapOut + 2);

#ifndef CONFIG_INFINITE_DURABILITY
    if (!IsDuraItem(item))
    {
        PutNumberOrBlank(mapOut + 11, Text_GetColor(text), GetItemUses(item));
    }
#endif

    if (gpKernelDesignerConfig->forge_mechanic == true)
    {
        struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
        if (CanItemBeForged(item)) {
            PutSpecialChar(mapOut + 8, Text_GetColor(text), TEXT_SPECIAL_PLUS);
            PutNumberOrBlank(mapOut + 9, Text_GetColor(text), GetItemForgeCount(item));
            PutNumberOrBlank(mapOut + 11, Text_GetColor(text), GetForgedItemDurability(item));
        } else if (limits.maxCount == 0) {
            PutNumberOrBlank(mapOut + 11, Text_GetColor(text), GetItemUses(item));
        }
    }
    
    DrawIcon(mapOut, GetItemIconId(item), 0x4000);
}

LYN_REPLACE_CHECK(DrawItemStatScreenLine);
void DrawItemStatScreenLine(struct Text * text, int item, int nameColor, u16 * mapOut)
{
    int color;

    ClearText(text);

    color = nameColor;
    Text_SetColor(text, color);

    Text_DrawString(text, GetItemName(item));

#ifndef CONFIG_INFINITE_DURABILITY
    if (!IsDuraItem(item))
    {
        color = (nameColor == TEXT_COLOR_SYSTEM_GRAY) ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE;
        PutSpecialChar(mapOut + 12, color, TEXT_SPECIAL_SLASH);

        color = (nameColor != TEXT_COLOR_SYSTEM_GRAY) ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY;
        PutNumberOrBlank(mapOut + 11, color, GetItemUses(item));
        PutNumberOrBlank(mapOut + 14, color, GetItemMaxUses(item));
    }
#endif

    if (gpKernelDesignerConfig->forge_mechanic == true)
    {
        struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
        if (CanItemBeForged(item)) {
            color = (nameColor == TEXT_COLOR_SYSTEM_GRAY) ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_GOLD;
            PutSpecialChar(mapOut + 10, color, TEXT_SPECIAL_PLUS);
            PutNumberOrBlank(mapOut + 11, color, GetItemForgeCount(item));

            color = (nameColor == TEXT_COLOR_SYSTEM_GRAY) ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_BLUE;
            PutNumberOrBlank(mapOut + 14, color, GetForgedItemDurability(item));
        } else if (limits.maxCount == 0) {
            color = (nameColor == TEXT_COLOR_SYSTEM_GRAY) ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE;
            PutSpecialChar(mapOut + 12, color, TEXT_SPECIAL_SLASH);

            color = (nameColor != TEXT_COLOR_SYSTEM_GRAY) ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY;
            PutNumberOrBlank(mapOut + 11, color, GetItemUses(item));
            PutNumberOrBlank(mapOut + 14, color, GetItemMaxUses(item));
        }
        PutText(text, mapOut + 2);
    }

    PutText(text, mapOut + 2);

    DrawIcon(mapOut, GetItemIconId(item), 0x4000);
}

LYN_REPLACE_CHECK(GetItemAfterUse);
u16 GetItemAfterUse(int item)
{
    if (GetItemAttributes(item) & IA_UNBREAKABLE)
        return item;

#if CHAX
    if (IsDuraItem(item))
        return 0;
#endif

    if (gpKernelDesignerConfig->forge_mechanic == true)
    {
        if (CanItemBeForged(item))
        {
            struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
            if (limits.maxCount) {
                if (!SetForgedItemAfterUse(item)) { // out of uses, so delete the item
                    return 0;
                }
                return item; // items that have a nonzero forge count don't lose uses
            }
        }
    }

    item -= (1 << 8);
    if (item < (1 << 8))
        return 0;

    return item;
}

LYN_REPLACE_CHECK(StartShopScreen);
void StartShopScreen(struct Unit *unit, const u16 *inventory, u8 shopType, ProcPtr parent)
{
	struct ProcShop *proc;
	const u16 *shopItems;
	int i;

	EndPlayerPhaseSideWindows();

	if (parent)
		proc = Proc_StartBlocking(gProcScr_Shop, parent);
	else
		proc = Proc_Start(gProcScr_Shop, PROC_TREE_3);

	proc->shopType = shopType;
	proc->unit = unit;

	shopItems = gDefaultShopInventory;
	if (inventory != 0)
		shopItems = inventory;

#if CHAX
	for (i = 0; i < SHOP_ITEMS_MAX_AMT; i++) {
		u16 _item = *shopItems++;

		if (IsDuraItem(_item))
			proc->shopItems[i] = _item;
		else
			proc->shopItems[i] = MakeNewItem(_item);
	}
#else
	for (i = 0; i <= SHOP_ITEMS_MAX_AMT; i++)
		proc->shopItems[i] = MakeNewItem(*shopItems++);
#endif

	UpdateShopItemCounts(proc);
}

LYN_REPLACE_CHECK(RefreshUnitInventoryInfoWindow);
void RefreshUnitInventoryInfoWindow(struct Unit* unit) {
    int i;
    int xPos;
    int itemCount;

    struct UnitInfoWindowProc* proc;

    itemCount = GetUnitItemCount(unit);

    xPos = GetUnitInfoWindowX(unit, 0xd);

    proc = UnitInfoWindow_DrawBase(0, unit, xPos, 0, 0xd, itemCount != 0 ? itemCount : 1);

    if (itemCount == 0) {
        int offset;

        ClearText(proc->lines + 0);
        Text_InsertDrawString(proc->lines + 0, 0, 1, GetStringFromIndex(0x5a8)); // TODO: msgid "Nothing[.]"

        offset = TILEMAP_INDEX(xPos+3, 0+3);
        PutText(proc->lines + 0, gBG0TilemapBuffer + offset);

        return;
    }

    for (i = 0; i < itemCount; i++) {
        int yPos = 0 + i * 2 + 3;

        int item = unit->items[i];

        ClearText(proc->lines + i);
        Text_DrawString(proc->lines + i, GetItemName(item));

        PutText(proc->lines + i, gBG0TilemapBuffer + TILEMAP_INDEX(xPos+3, yPos));

        if (gpKernelDesignerConfig->forge_mechanic == true)
        {
            struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
            if (CanItemBeForged(item)) {
                PutSpecialChar(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 8, yPos), TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_PLUS);
                PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 9, yPos), TEXT_COLOR_SYSTEM_GOLD, GetItemForgeCount(item));
                PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 11, yPos), TEXT_COLOR_SYSTEM_BLUE, GetForgedItemDurability(item));
            } else if (limits.maxCount == 0) {
                PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 11, yPos), TEXT_COLOR_SYSTEM_BLUE, GetItemUses(item));
            }
        }

        DrawIcon(gBG0TilemapBuffer + TILEMAP_INDEX(xPos+1, yPos), GetItemIconId(item), 0x4000);
    }
    return;
}
LYN_REPLACE_CHECK(RefreshHammerneUnitInfoWindow);
void RefreshHammerneUnitInfoWindow(struct Unit* unit) {
    int i;
    int color;
    int xPos;
    int itemCount;
    struct UnitInfoWindowProc* proc;

    itemCount = GetUnitItemCount(unit);

    xPos = GetUnitInfoWindowX(unit, 0x10);

    proc = UnitInfoWindow_DrawBase(0, unit, xPos, 0, 0x10, itemCount);

    for (i = 0; i < itemCount; i++) {
        int yPos = 0 + i * 2 + 3;

        int item = unit->items[i];

        color = IsItemHammernable(item) ? 0 : 1;

        ClearText(proc->lines + i);

        Text_SetColor(proc->lines + i, color);
        Text_DrawString(proc->lines + i, GetItemName(item));

        PutText(proc->lines + i, gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 3, yPos));
		
        if (gpKernelDesignerConfig->forge_mechanic == true)
        {     
            struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
            if (CanItemBeForged(item)) {
                PutSpecialChar(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 11, yPos), IsItemHammernable(item) ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY, TEXT_SPECIAL_PLUS);
                PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 12, yPos), IsItemHammernable(item) ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY, GetItemForgeCount(item));
                PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 14, yPos), color, GetForgedItemDurability(item));
            } else if (limits.maxCount == 0) {
                PutSpecialChar(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 12, yPos), color, TEXT_SPECIAL_SLASH);
            }
        }
        else
        {
            PutSpecialChar(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 12, yPos), color, TEXT_SPECIAL_SLASH);

            color = IsItemHammernable(item) ? 2 : 1;

            PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 11, yPos), color, GetItemUses(item));
            PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 14, yPos), color, GetItemMaxUses(item));	
        }

		DrawIcon(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 1, yPos), GetItemIconId(item), 0x4000);
    }

    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

LYN_REPLACE_CHECK(GetWeaponLevelFromExp);
int GetWeaponLevelFromExp(int wexp) {
    if (wexp < WPN_EXP_E)
        return WPN_LEVEL_0;

    if (wexp < WPN_EXP_D)
        return WPN_LEVEL_E;

    if (wexp < WPN_EXP_C)
        return WPN_LEVEL_D;

    if (wexp < WPN_EXP_B)
        return WPN_LEVEL_C;

    if (wexp < WPN_EXP_A)
        return WPN_LEVEL_B;

    if (wexp < WPN_EXP_S)
        return WPN_LEVEL_A;

    return WPN_LEVEL_S;
}

LYN_REPLACE_CHECK(GetItemDisplayRankString);
char* GetItemDisplayRankString(int item) {
    int rankTextIdLookup[] = {
        // TODO: TEXT ID CONSTANTS
        0x52C, 0x52D, 0x52E, 0x52F, // --, E, D, C
        0x530, 0x531, 0x532, 0x533, // B, A, S, Prf
    };

    // reuse of the same variable for different purposes :/
    int var = GetItemRequiredExp(item);

    if ((GetItemAttributes(item) & IA_LOCK_ANY) && GetWeaponLevelFromExp(var) == WPN_LEVEL_0)
        var = 7;
    else
        var = GetWeaponLevelFromExp(var);

    return GetStringFromIndex(rankTextIdLookup[var]);
};


LYN_REPLACE_CHECK(PrepUnit_DrawUnitItems);
void PrepUnit_DrawUnitItems(struct Unit *unit)
{
    int i, cnt;

    ResetIconGraphics_();
    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 1, 5), 0xB, 0xA, 0);

    cnt = GetUnitItemCount(unit);
    for (i = 0; i < cnt; i++) {
        int item = unit->items[i];

        DrawIcon(
            TILEMAP_LOCATED(gBG0TilemapBuffer, 1, 5 + 2 * i),
            GetItemIconId(item),
            TILEREF(0, BGPAL_ICONS)
        );

        ClearText(&gPrepUnitTexts[i + 0xE]);

        PutDrawText(
            &gPrepUnitTexts[i + 0xE],
            TILEMAP_LOCATED( gBG0TilemapBuffer, 3, 5 + 2 * i),
            IsItemDisplayUsable(unit, item) ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY, 0, 0, GetItemName(item)
        );
        if (gpKernelDesignerConfig->forge_mechanic == true)
        {
            struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
            if (CanItemBeForged(item)) {
                PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, 8, 5 + 2 * i), IsItemDisplayUsable(unit, item) ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY, TEXT_SPECIAL_PLUS);
                PutNumberOrBlank(TILEMAP_LOCATED(gBG0TilemapBuffer, 9, 5 + 2 * i), IsItemDisplayUsable(unit, item) ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY, GetItemForgeCount(item));
                PutNumberOrBlank(TILEMAP_LOCATED(gBG0TilemapBuffer, 11, 5 + 2 * i), IsItemDisplayUsable(unit, item) ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetForgedItemDurability(item));
            } else if (limits.maxCount == 0) {
                PutNumberOrBlank(TILEMAP_LOCATED(gBG0TilemapBuffer, 11, 5 + 2 * i), IsItemDisplayUsable(unit, item) ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
            }
        // //#else
        //     PutNumberOrBlank(TILEMAP_LOCATED(gBG0TilemapBuffer, 11, 5 + 2 * i), IsItemDisplayUsable(unit, item) ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
        }
    }

    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

LYN_REPLACE_CHECK(IsItemDisplayUsable);
s8 IsItemDisplayUsable(struct Unit* unit, int item) {
    
    if (GetItemAttributes(item) & IA_WEAPON)
        return CanUnitUseWeapon(unit, item);

    if (GetItemAttributes(item) & IA_STAFF)
        return CanUnitUseStaff(unit, item);

    if (GetItemUseEffect(item)) {
        if (unit->statusIndex == UNIT_STATUS_SLEEP)
            return FALSE;

        if (unit->statusIndex == UNIT_STATUS_BERSERK)
            return FALSE;

        if (!(UNIT_CATTRIBUTES(unit) & CA_THIEF) && GetItemIndex(item) == ITEM_LOCKPICK)
            return FALSE;

        if (!(UNIT_CATTRIBUTES(unit) & CA_REFRESHER) && IsItemDanceRing(item))
            return FALSE;
    }

    return TRUE;
}

LYN_REPLACE_CHECK(GetItemMaxRange);
int GetItemMaxRange(int item) {
    return GetItemData(ITEM_INDEX(item))->encodedRange & 0xF;
}

LYN_REPLACE_CHECK(GetItemMinRange);
int GetItemMinRange(int item) {
    return GetItemData(ITEM_INDEX(item))->encodedRange >> 4;
}

LYN_REPLACE_CHECK(GetItemAwardedExp);
int GetItemAwardedExp(int item) {
    return GetItemData(ITEM_INDEX(item))->weaponExp;
}