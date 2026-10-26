#include "common-chax.h"
#include "gaiden-magic.h"
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

static bool ShouldDisplayItemDurability(int item)
{
    if (!gpKernelDesignerConfig->infinite_durability)
        return true;

    return !(GetItemAttributes(item) & IA_WEAPON);
}

// noinline: it is called all over this file and inlining it costs far more kernel text than the calls
NOINLINE int GetItemForgeCount(int item) {
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

int GetForgedItemSkill(int item) {
    struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
    int sid = gpConstSkillTable_Forge[GetItemIndex(item)];
    int id = ITEM_USES(item);

    if (!gpKernelDesignerConfig->forge_mechanic)
        return 0;

    if (limits.maxCount == 0 || sid == 0)
        return 0;

    // slot 0 is never allocated, so a zero uses byte means the item was never forged
    if (id == 0 || id >= NumOfForgables)
        return 0;

    if (gForgedItemRam[id].forgeCount < limits.maxCount)
        return 0;

    return sid;
}

// Undo the levels previewed with left/right on this item and clear the running cost total
void ResetForgePreview(int item) {
    struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];

    gEventSlots[EVT_SLOT_7] = 0;

    if (limits.maxCount == 0 || ITEM_USES(item) == 0)
        return;

    SetItemForgeCount(item, gEventSlots[EVT_SLOT_8]);

    if ((int)gEventSlots[EVT_SLOT_8] < limits.maxCount - 1)
        MakeForgedItemUnbreakable(item, false);
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
        int id = ITEM_USES(item);

        // only forgeable weapons hold a forge slot in their uses byte (0 = none was free),
        // every other item keeps its real durability there
        if (limits.maxCount && id != 0 && id < NumOfForgables)
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

LYN_REPLACE_CHECK(GetItemStatBonuses);
const struct ItemStatBonuses *GetItemStatBonuses(int item)
{
    if (!item)
        return NULL;

    return GetItemData(ITEM_INDEX(item))->pStatBonuses;
}

LYN_REPLACE_CHECK(GetItemHpBonus);
int GetItemHpBonus(int item)
{
    const struct ItemStatBonuses *bonuses = GetItemStatBonuses(item);
    return bonuses ? bonuses->hpBonus : 0;
}

LYN_REPLACE_CHECK(GetItemPowBonus);
int GetItemPowBonus(int item)
{
    const struct ItemStatBonuses *bonuses = GetItemStatBonuses(item);
    return bonuses ? bonuses->powBonus : 0;
}

LYN_REPLACE_CHECK(GetItemSklBonus);
int GetItemSklBonus(int item)
{
    const struct ItemStatBonuses *bonuses = GetItemStatBonuses(item);
    return bonuses ? bonuses->sklBonus : 0;
}

LYN_REPLACE_CHECK(GetItemSpdBonus);
int GetItemSpdBonus(int item)
{
    const struct ItemStatBonuses *bonuses = GetItemStatBonuses(item);
    return bonuses ? bonuses->spdBonus : 0;
}

LYN_REPLACE_CHECK(GetItemDefBonus);
int GetItemDefBonus(int item)
{
    const struct ItemStatBonuses *bonuses = GetItemStatBonuses(item);
    return bonuses ? bonuses->defBonus : 0;
}

LYN_REPLACE_CHECK(GetItemResBonus);
int GetItemResBonus(int item)
{
    const struct ItemStatBonuses *bonuses = GetItemStatBonuses(item);
    return bonuses ? bonuses->resBonus : 0;
}

LYN_REPLACE_CHECK(GetItemLckBonus);
int GetItemLckBonus(int item)
{
    const struct ItemStatBonuses *bonuses = GetItemStatBonuses(item);
    return bonuses ? bonuses->lckBonus : 0;
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

LYN_REPLACE_CHECK(GetWeaponTypeDisplayString);
char *GetWeaponTypeDisplayString(int wpnType)
{
    static const u16 vanillaTextIds[] = {
        [ITYPE_SWORD] = 0x505,
        [ITYPE_LANCE] = 0x506,
        [ITYPE_AXE]   = 0x507,
        [ITYPE_BOW]   = 0x508,
        [ITYPE_STAFF] = 0x509,
        [ITYPE_ANIMA] = 0x50A,
        [ITYPE_LIGHT] = 0x50B,
        [ITYPE_DARK]  = 0x50C,
        [ITYPE_BLLST] = 0x50E,
        [ITYPE_ITEM]  = 0x50D,
        [ITYPE_DRAGN] = 0x50F,
    };

    if (wpnType == ITYPE_KNIFE)
        return GetStringFromIndex(MSG_WTYPE_KNIFE);

    if (wpnType == ITYPE_GUN)
        return GetStringFromIndex(MSG_WTYPE_GUN);

    if (wpnType < 0 || wpnType >= (int)ARRAY_COUNT(vanillaTextIds))
        return GetStringFromIndex(0x50D); /* Item */

    return GetStringFromIndex(vanillaTextIds[wpnType]);
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
        if (ShouldDisplayItemDurability(item)) // non-weapons always show durability; weapons obey infinite durability
        {
            if (!IsDuraItem(item))
            {
                PutNumberOrBlank(mapOut + 11, isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
            }
        }
    }

    DrawIcon(mapOut, GetItemIconId(item), 0x4000);
}

LYN_REPLACE_CHECK(DrawItemMenuLineLong);
void DrawItemMenuLineLong(struct Text * text, int item, s8 isUsable, u16 * mapOut)
{
    Text_SetParams(text, 0, (isUsable ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY));
    Text_DrawString(text, GetItemName(item));

    PutText(text, mapOut + 2);

    if (ShouldDisplayItemDurability(item))
    {
        if (!IsDuraItem(item))
        {
            PutNumberOrBlank(mapOut + 10, isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
            PutNumberOrBlank(mapOut + 13, isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemMaxUses(item));
            PutSpecialChar(mapOut + 11, isUsable ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY, TEXT_SPECIAL_SLASH);
        }
    }

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

    if (ShouldDisplayItemDurability(item))
    {
        if (!IsDuraItem(item))
        {
            PutNumberOrBlank(mapOut + 11, Text_GetColor(text), GetItemUses(item));
        }
    }

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

    if (ShouldDisplayItemDurability(item))
    {
        if (!IsDuraItem(item))
        {
            color = (nameColor == TEXT_COLOR_SYSTEM_GRAY) ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE;
            PutSpecialChar(mapOut + 12, color, TEXT_SPECIAL_SLASH);

            color = (nameColor != TEXT_COLOR_SYSTEM_GRAY) ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY;
            PutNumberOrBlank(mapOut + 11, color, GetItemUses(item));
            PutNumberOrBlank(mapOut + 14, color, GetItemMaxUses(item));
        }
    }

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

		/* Three Houses magic: no tomes or staves for sale */
		while (_item != ITEM_NONE && IsGaidenMagicUsesSystem() && IsThreeHousesMagicItem(_item))
			_item = *shopItems++;

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
