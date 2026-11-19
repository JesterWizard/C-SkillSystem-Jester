#include "global.h"
#include "bmunit.h"
#include "bmitem.h"
#include "bmmind.h"
#include "bmshop.h"
#include "bmtrade.h"
#include "bmcontainer.h"
#include "bonusclaim.h"
#include "convoymenu.h"
#include "cp_common.h"
#include "hardware.h"
#include "popup.h"
#include "prepscreen.h"
#include "proc.h"
#include "statscreen.h"
#include "constants/classes.h"
#include "uimenu.h"

/*
 * Accessory slot (Commissions/accessory_slot/README.md).
 *
 * The items in AccessorySlotItems (AccessoryItems.event) are accessories. A unit carries at most four other items and
 * at most one accessory, and its accessory is always its last item: with four other items it is the fifth.
 * Inventories stay packed (no empty slots between items), as vanilla expects.
 *
 * Every vanilla way of adding an item goes through one of the hooks below: UnitAddItem refuses an item the unit can't
 * carry, so an item that is found, dropped or given brings up vanilla's "inventory full" menu, and the rest check
 * before they add (shops, steal, trades, the supply and list screens, bonus claims).
 */

#define SONG_ERROR 0x6C

#define ACC_MAX_REGULAR 4
#define ACC_MAX_ACCESSORY 1

/* gBmSt.um_tmp_item: the item being received while the "inventory full" menu is up */
#define ACC_NEW_ITEM (*(u16 *)((u8 *)&gBmSt + 0x2C))

/* Shop proc labels (bmshop.c) */
#define PL_SHOP_BUY_DONE 3
#define PL_SHOP_BUY_FULL_NO_INVENTORY 11

/* Item IDs, ending with 0 (AccessoryItems.event) */
extern const u8 AccessorySlotItems[];

/* Which item page row gets the blue "equipped" bar (AccessoryItems.event) */
extern const u8 AccessorySlotBar;

#define ACC_BAR_OFF       0
#define ACC_BAR_SLOT5     1
#define ACC_BAR_ACCESSORY 2

/* The bar vanilla draws under the equipped weapon (DisplayPage1) */
extern const u8 gUnknown_08A02250[];
void DisplayPage1(void);

void m4aSongNumStart(u16 n);
void SetBonusItemClaimed(int index);
void DrawBonusClaimItemText(int index);
int AddItemToConvoy(int item);

static bool IsAccessory(int item)
{
	const u8 *it;
	int id = ITEM_INDEX(item);

	if (id == 0)
		return false;

	for (it = AccessorySlotItems; *it != 0; it++)
		if (*it == id)
			return true;

	return false;
}

/*
 * Whether the unit can't wear the accessory: the item has a weapon lock (FEBuilder's item editor) that neither the
 * unit's character nor its class has, as vanilla CanUnitUseWeapon checks for weapons.
 */
static bool AccessoryLocked(struct Unit *unit, int item)
{
	static const u32 locks[][2] = {
		{ IA_LOCK_1, CA_LOCK_1 }, { IA_LOCK_2, CA_LOCK_2 }, { IA_LOCK_3, CA_LOCK_3 }, { IA_LOCK_4, CA_LOCK_4 },
		{ IA_LOCK_5, CA_LOCK_5 }, { IA_LOCK_6, CA_LOCK_6 }, { IA_LOCK_7, CA_LOCK_7 },
	};
	u32 itemAttr, unitAttr;
	unsigned int i;

	if (!IsAccessory(item))
		return false;

	itemAttr = GetItemAttributes(item);
	unitAttr = UNIT_CATTRIBUTES(unit);

	for (i = 0; i < ARRAY_COUNT(locks); i++)
		if ((itemAttr & locks[i][0]) && !(unitAttr & locks[i][1]))
			return true;

	if ((itemAttr & IA_UNUSABLE) && !IsItemUnsealedForUnit(unit, item))
		return true;

	return false;
}

/* Whether an inventory with these items keeps the rules: four other items and one accessory at most */
static bool InventoryAllowed(const u16 *items, int n)
{
	int i, regular = 0, accessory = 0;

	for (i = 0; i < n; i++) {
		if (items[i] == 0)
			continue;

		if (IsAccessory(items[i]))
			accessory++;
		else
			regular++;
	}

	return regular <= ACC_MAX_REGULAR && accessory <= ACC_MAX_ACCESSORY;
}

/* Whether the unit can take one more item */
static bool CanAddItem(struct Unit *unit, int item)
{
	u16 items[UNIT_ITEM_COUNT + 1];
	int i;

	for (i = 0; i < UNIT_ITEM_COUNT; i++)
		items[i] = unit->items[i];
	items[UNIT_ITEM_COUNT] = item;

	return GetUnitItemCount(unit) < UNIT_ITEM_COUNT && InventoryAllowed(items, UNIT_ITEM_COUNT + 1) &&
	       !AccessoryLocked(unit, item);
}

/* Whether slot slotA of unitA and slot slotB of unitB can swap items (unitB may be unitA) */
static bool SwapAllowed(struct Unit *unitA, int slotA, struct Unit *unitB, int slotB)
{
	u16 itemsA[UNIT_ITEM_COUNT], itemsB[UNIT_ITEM_COUNT];
	u16 itemA = unitA->items[slotA], itemB = unitB->items[slotB];
	int i;

	if (unitA == unitB)
		return true;

	for (i = 0; i < UNIT_ITEM_COUNT; i++) {
		itemsA[i] = unitA->items[i];
		itemsB[i] = unitB->items[i];
	}
	itemsA[slotA] = itemB;
	itemsB[slotB] = itemA;

	/* Only the item a unit receives is checked: one that a class change locked can still be traded away */
	if (AccessoryLocked(unitA, itemB) || AccessoryLocked(unitB, itemA))
		return false;

	return InventoryAllowed(itemsA, UNIT_ITEM_COUNT) && InventoryAllowed(itemsB, UNIT_ITEM_COUNT);
}

/*
 * Replaces vanilla UnitRemoveInvalidItems (0x08017984), which every removal, trade and break ends with: packs the
 * items, other items first, then the accessory.
 */
void AccessorySlot_UnitRemoveInvalidItems(struct Unit *unit)
{
	u16 regular[UNIT_ITEM_COUNT], accessory[UNIT_ITEM_COUNT];
	int i, nr = 0, na = 0, n = 0;

	for (i = 0; i < UNIT_ITEM_COUNT; i++) {
		u16 item = unit->items[i];

		if (item == 0)
			continue;

		if (IsAccessory(item))
			accessory[na++] = item;
		else
			regular[nr++] = item;
	}

	for (i = 0; i < nr; i++)
		unit->items[n++] = regular[i];
	for (i = 0; i < na; i++)
		unit->items[n++] = accessory[i];
	while (n < UNIT_ITEM_COUNT)
		unit->items[n++] = 0;
}

/* Replaces vanilla UnitAddItem (0x08017948): false if the unit can't carry the item */
bool AccessorySlot_UnitAddItem(struct Unit *unit, int item)
{
	int i;

	if (!CanAddItem(unit, item))
		return false;

	for (i = 0; i < UNIT_ITEM_COUNT; i++) {
		if (unit->items[i] == 0) {
			unit->items[i] = item;
			AccessorySlot_UnitRemoveInvalidItems(unit);
			return true;
		}
	}
	return false;
}

/*
 * The "inventory full" menus (gConvoyMenuItems, gSendToConvoyMenuItems): rows 0-4 are the unit's items. Only an item
 * of the same kind as the new one (accessory or not) can make room for it, and with fewer than five items the empty
 * rows are hidden.
 */
u8 AccessorySlot_ConvoyRowUsability(const struct MenuItemDef *def, int number)
{
	int item = gActiveUnit->items[number];

	(void)def;

	if (item == 0)
		return MENU_NOTSHOWN;

	/* A locked accessory can't be worn: only the new item itself can be sent or discarded */
	if (AccessoryLocked(gActiveUnit, ACC_NEW_ITEM))
		return MENU_DISABLED;

	if (IsAccessory(item) != IsAccessory(ACC_NEW_ITEM))
		return MENU_DISABLED;

	return MENU_ENABLED;
}

/* A disabled row is drawn gray (vanilla colors the rows by whether the unit can use the item) */
int AccessorySlot_ConvoyRowDraw(struct MenuProc *menu, struct MenuItemProc *row)
{
	if (row->availability != MENU_DISABLED)
		return SendToConvoyMenu_Draw(menu, row);

	DrawItemMenuLine(&row->text, gActiveUnit->items[row->itemNumber], FALSE,
			 gBG0TilemapBuffer + TILEMAP_INDEX(row->xTile, row->yTile));
	return 0;
}

/* A menu calls a row's select function even when the row is disabled */
u8 AccessorySlot_ConvoyRowSelected(struct MenuProc *menu, struct MenuItemProc *row)
{
	if (row->availability == MENU_DISABLED)
		return MENU_ACT_SND6B;

	return SendToConvoyMenu_Selected(menu, row);
}

u8 AccessorySlot_ConvoyRowSend(struct MenuProc *menu, struct MenuItemProc *row)
{
	if (row->availability == MENU_DISABLED)
		return MENU_ACT_SND6B;

	return SendToConvoyMenu_NormalEffect(menu, row);
}

/* Replaces vanilla Shop_TryAddItemToInventory (0x080B464C): "full" for the item being bought */
void AccessorySlot_Shop_TryAddItemToInventory(struct ProcShop *proc)
{
	int item = proc->shopItems[proc->head_loc];

	if (!CanAddItem(proc->unit, item)) {
		if (HasConvoyAccess()) {
			/* "Your hands are full. Send it to storage?" */
			StartShopDialogue(0x8BE, proc);
		} else {
			/* "Your hands are full." */
			StartShopDialogue(0x8C1, proc);
			Proc_Goto(proc, PL_SHOP_BUY_FULL_NO_INVENTORY);
		}
		return;
	}

	UnitAddItem(proc->unit, item);
	HandleShopBuyAction(proc);

	Proc_Goto(proc, PL_SHOP_BUY_DONE);
}

/* Replace vanilla's steal item menu (0x08024304, 0x08024348, 0x0802439C): an item the thief can't carry is grayed */
static bool CanStealItem(int item)
{
	return IsItemStealable(item) && CanAddItem(gActiveUnit, item);
}

u8 AccessorySlot_StealItemUsability(const struct MenuItemDef *def, int number)
{
	int item = GetUnit(gActionData.targetIndex)->items[number];

	(void)def;

	if (item == 0)
		return MENU_NOTSHOWN;

	if (!CanStealItem(item))
		return MENU_DISABLED;

	return MENU_ENABLED;
}

int AccessorySlot_StealItemDraw(struct MenuProc *menu, struct MenuItemProc *row)
{
	int item = GetUnit(gActionData.targetIndex)->items[row->itemNumber];

	(void)menu;
	DrawItemMenuLine(&row->text, item, CanStealItem(item), gBG0TilemapBuffer + TILEMAP_INDEX(row->xTile, row->yTile));
	return 0;
}

u8 AccessorySlot_StealItemEffect(struct MenuProc *menu, struct MenuItemProc *row)
{
	int item = GetUnit(gActionData.targetIndex)->items[row->itemNumber];

	if (row->availability == MENU_DISABLED) {
		/* "Weapons, magic, and staves can't be stolen." or "Your inventory is full." */
		MenuFrozenHelpBox(menu, IsItemStealable(item) ? 0x862 : 0x855);
		return MENU_ACT_SND6B;
	}

	gActionData.itemSlotIndex = row->itemNumber;
	gActionData.unitActionType = UNIT_ACTION_STEAL;

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

/* Replaces vanilla AiStartStealAction (0x0803A134): an enemy thief doesn't take what it can't carry */
void AccessorySlot_AiStartStealAction(ProcPtr proc)
{
	struct Unit *target = GetUnit(gAiDecision.targetId);
	u16 item = target->items[gAiDecision.itemSlot];

	if (!UnitAddItem(gActiveUnit, item))
		return;

	UnitRemoveItem(target, gAiDecision.itemSlot);
	NewPopup_ItemStealing(item, proc);
}

/* Replaces vanilla TradeMenu_ApplyItemSwap (0x0802D72C) */
void AccessorySlot_TradeMenu_ApplyItemSwap(struct TradeMenuProc *proc)
{
	struct Unit *unitA = proc->units[proc->hoverColumn];
	struct Unit *unitB = proc->units[proc->selectedColumn];
	u16 *itemA = &unitA->items[proc->hoverRow];
	u16 *itemB = &unitB->items[proc->selectedRow];
	u16 swp;

	if (!SwapAllowed(unitA, proc->hoverRow, unitB, proc->selectedRow)) {
		m4aSongNumStart(SONG_ERROR);
		return;
	}

	swp = *itemA;
	*itemA = *itemB;
	*itemB = swp;

	proc->hasTraded = TRUE;
	gActionData.unitActionType = UNIT_ACTION_TRADED;

	UnitRemoveInvalidItems(proc->units[0]);
	UnitRemoveInvalidItems(proc->units[1]);

	TradeMenu_RefreshItemText(proc);
}

/* Replaces vanilla PrepItemTrade_ApplyItemSwap (0x0809B538), the preparations' Trade */
void AccessorySlot_PrepItemTrade_ApplyItemSwap(struct Unit *unitA, int slotA, struct Unit *unitB, int slotB)
{
	u16 tmp;

	if (!SwapAllowed(unitA, slotA, unitB, slotB)) {
		m4aSongNumStart(SONG_ERROR);
		return;
	}

	tmp = unitA->items[slotA];
	unitA->items[slotA] = unitB->items[slotB];
	unitB->items[slotB] = tmp;

	UnitRemoveInvalidItems(unitA);
	UnitRemoveInvalidItems(unitB);
}

/* Replaces vanilla TryClaimBonusItem (0x080B11E4), the Bonus screen's items */
s8 AccessorySlot_TryClaimBonusItem(struct BonusClaimProc *proc)
{
	struct BonusClaimConfig *target = gpBonusClaimConfig + proc->submenuIndex;
	int entry = *((const s8 *)gpBonusClaimItemList + proc->menuIndex * 4);
	int item = MakeNewItem(*((const u8 *)gpBonusClaimData + entry * 0x14 + 2));
	bool toSupply = proc->submenuIndex == proc->targets - 1;

	if (!target->hasInventorySpace)
		return false;

	if (!toSupply && !CanAddItem(target->unit, item))
		return false;

	SetBonusItemClaimed(proc->menuIndex);
	DrawBonusClaimItemText(proc->menuIndex);

	if (toSupply)
		AddItemToConvoy(item);
	else
		UnitAddItem(target->unit, item);

	return true;
}

/*
 * The preparations' Supply (Take) and List (take into an empty row) put the selected list item into the unit's
 * first empty slot once its item count is under five. These count hooks give five when the item doesn't fit.
 */
static int TakeItemCount(struct Unit *unit, int listIndex)
{
	int count = GetUnitItemCount(unit);

	if (count >= UNIT_ITEM_COUNT || gUnknown_02012F56 == 0)
		return count;

	if (!CanAddItem(unit, gPrepScreenItemList[listIndex].item))
		return UNIT_ITEM_COUNT;

	return count;
}

int AccessorySlot_SupplyTakeCount(struct PrepItemSupplyProc *proc)
{
	return TakeItemCount(proc->unit, proc->idxPerPage[proc->currentPage]);
}

/* struct PrepItemListProc: unit +0x2C, unitInvIdx +0x30, currentPage +0x33, idxPerPage +0x38 */
#define LIST_UNIT(proc)       (*(struct Unit **)((u8 *)(proc) + 0x2C))
#define LIST_INV_INDEX(proc)  (*((u8 *)(proc) + 0x30))
#define LIST_INDEX(proc)      (((u16 *)((u8 *)(proc) + 0x38))[*((u8 *)(proc) + 0x33)])

int AccessorySlot_ListTakeCount(ProcPtr proc)
{
	return TakeItemCount(LIST_UNIT(proc), LIST_INDEX(proc));
}

/* The preparations' List: swap the unit's selected item for the list's */
bool AccessorySlot_ListSwapAllowed(ProcPtr proc)
{
	struct Unit *unit = LIST_UNIT(proc);
	u16 items[UNIT_ITEM_COUNT];
	int i;

	for (i = 0; i < UNIT_ITEM_COUNT; i++)
		items[i] = unit->items[i];
	items[LIST_INV_INDEX(proc)] = gPrepScreenItemList[LIST_INDEX(proc)].item;

	if (InventoryAllowed(items, UNIT_ITEM_COUNT) && !AccessoryLocked(unit, items[LIST_INV_INDEX(proc)]))
		return true;

	m4aSongNumStart(SONG_ERROR);
	return false;
}

/*
 * The stat screen's item page (gStatScreenDrawCallTable entry 1, Installer.event): vanilla's page, then the blue
 * "equipped" bar under slot 5 or under the accessory, as AccessorySlotBar says.
 */
void AccessorySlot_DisplayPage1(void)
{
	struct Unit *unit = gStatScreen.unit;
	int i, row = -1;

	DisplayPage1();

	if (UNIT_IS_GORGON_EGG(unit))
		return;

	switch (AccessorySlotBar) {
	case ACC_BAR_SLOT5:
		row = UNIT_ITEM_COUNT - 1;
		break;

	case ACC_BAR_ACCESSORY:
		for (i = 0; i < UNIT_ITEM_COUNT; i++)
			if (IsAccessory(unit->items[i]))
				row = i;
		break;
	}

	if (row >= 0)
		CallARM_FillTileRect(gUiTmScratchC + TILEMAP_INDEX(1, 2 + row * 2), gUnknown_08A02250,
				     TILEREF(0x40, STATSCREEN_BGPAL_3));
}

/*
 * The hooks in the middle of vanilla functions (Installer.event). Each replaces 8 bytes and goes back to the
 * instruction after them.
 */
asm(
	"	.syntax unified\n"
	"	.align 2\n"
	"	.thumb\n"

	/* sub_809E2C8 (Supply, Take) at 0x0809E2D0: ldr r0, [r7, #0x2C]; bl GetUnitItemCount; adds r2, r0, #0 */
	"	.global AccessorySlot_SupplyTakeHook\n"
	"	.thumb_func\n"
	"AccessorySlot_SupplyTakeHook:\n"
	"	adds r0, r7, #0\n"
	"	bl AccessorySlot_SupplyTakeCount\n"
	"	adds r2, r0, #0\n"
	"	ldr r3, =0x0809E2D9\n"
	"	bx r3\n"
	"	.ltorg\n"

	/* sub_809F498 (List, take) at 0x0809F4A0: adds r6, r0, #0; ldr r0, [r6, #0x2C]; bl GetUnitItemCount */
	"	.align 2\n"
	"	.global AccessorySlot_ListTakeHook\n"
	"	.thumb_func\n"
	"AccessorySlot_ListTakeHook:\n"
	"	adds r6, r0, #0\n"
	"	bl AccessorySlot_ListTakeCount\n"
	"	ldr r3, =0x0809F4A9\n"
	"	bx r3\n"
	"	.ltorg\n"

	/* sub_809FB70 (List, swap) at its start: push {r4-r7, lr}; adds r6, r0, #0; adds r7, r6, #0; adds r7, #0x33 */
	"	.align 2\n"
	"	.global AccessorySlot_ListSwapHook\n"
	"	.thumb_func\n"
	"AccessorySlot_ListSwapHook:\n"
	"	push {r4, lr}\n"
	"	adds r4, r0, #0\n"
	"	bl AccessorySlot_ListSwapAllowed\n"
	"	cmp r0, #0\n"
	"	bne 1f\n"
	"	pop {r4}\n"
	"	pop {r0}\n"
	"	bx r0\n"
	"1:\n"
	"	adds r0, r4, #0\n"
	"	pop {r4}\n"
	"	pop {r3}\n"
	"	mov lr, r3\n"
	"	push {r4, r5, r6, r7, lr}\n"
	"	adds r6, r0, #0\n"
	"	adds r7, r6, #0\n"
	"	adds r7, #0x33\n"
	"	ldr r3, =0x0809FB79\n"
	"	bx r3\n"
	"	.ltorg\n"
	"	.syntax divided\n"
);
