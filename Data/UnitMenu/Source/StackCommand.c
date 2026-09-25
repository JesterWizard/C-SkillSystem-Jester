#include "common-chax.h"
#include "kernel-lib.h"
#include "item-sys.h"
#include "skill-system.h"
#include "bmmenu.h"
#include "constants/texts.h"
#include "jester_headers/Forging.h"

static bool IsGoldBagItem(int item)
{
	int iid = ITEM_INDEX(item);

	if (iid == ITEM_GOLD)
		return true;

	return iid >= ITEM_1G && iid <= ITEM_5000G;
}

static bool ItemCanBeStacked(int item)
{
	if (!item)
		return false;

	if (GetItemAttributes(item) & IA_UNBREAKABLE)
		return false;

	if (IsSkillScrollItem(item) || IsDuraItem(item) || IsGoldBagItem(item))
		return false;

	if (gpKernelDesignerConfig->forge_mechanic && CanItemBeForged(item))
		return false;

	return true;
}

static int CountItemSlots(struct Unit *unit, int iid)
{
	int i, count = 0;

	for (i = 0; i < UNIT_ITEM_COUNT; i++) {
		if (ITEM_INDEX(unit->items[i]) == iid)
			count++;
	}

	return count;
}

static int SumItemUses(struct Unit *unit, int iid)
{
	int i, total = 0;

	for (i = 0; i < UNIT_ITEM_COUNT; i++) {
		if (ITEM_INDEX(unit->items[i]) == iid)
			total += ITEM_USES(unit->items[i]);
	}

	return total;
}

static bool ItemIdIsStackableOnUnit(struct Unit *unit, int iid)
{
	int i;

	if (!iid)
		return false;

	for (i = 0; i < UNIT_ITEM_COUNT; i++) {
		if (ITEM_INDEX(unit->items[i]) == iid)
			return ItemCanBeStacked(unit->items[i]) &&
				CountItemSlots(unit, iid) >= 2 &&
				SumItemUses(unit, iid) <= 0xFF &&
				SumItemUses(unit, iid) > 0;
	}

	return false;
}

static int GetNthStackableIid(struct Unit *unit, int index)
{
	int i, nth = 0;

	for (i = 0; i < UNIT_ITEM_COUNT; i++) {
		int iid = ITEM_INDEX(unit->items[i]);
		int j;
		bool first = true;

		if (!iid || !ItemIdIsStackableOnUnit(unit, iid))
			continue;

		for (j = 0; j < i; j++) {
			if (ITEM_INDEX(unit->items[j]) == iid) {
				first = false;
				break;
			}
		}

		if (!first)
			continue;

		if (nth == index)
			return iid;

		nth++;
	}

	return 0;
}

static bool UnitHasStackableItems(struct Unit *unit)
{
	return GetNthStackableIid(unit, 0) != 0;
}

static void StackItemsOfId(struct Unit *unit, int iid)
{
	int i, first = -1, total = 0;

	for (i = 0; i < UNIT_ITEM_COUNT; i++) {
		if (ITEM_INDEX(unit->items[i]) != iid)
			continue;

		total += ITEM_USES(unit->items[i]);

		if (first < 0)
			first = i;
		else
			unit->items[i] = 0;
	}

	if (first < 0)
		return;

	if (total > 0xFF)
		total = 0xFF;

	unit->items[first] = iid | (total << 8);
	UnitRemoveInvalidItems(unit);
}

static u8 StackSub_Usability(const struct MenuItemDef *def, int number)
{
	if (!GetNthStackableIid(gActiveUnit, number))
		return MENU_NOTSHOWN;

	return MENU_ENABLED;
}

static int StackSub_Draw(struct MenuProc *menu, struct MenuItemProc *item)
{
	int iid = GetNthStackableIid(gActiveUnit, item->itemNumber);
	int packed = iid | (SumItemUses(gActiveUnit, iid) << 8);

	DrawItemMenuLine(
		&item->text,
		packed,
		TRUE,
		TILEMAP_LOCATED(BG_GetMapBuffer(menu->frontBg), item->xTile, item->yTile));

	return 0;
}

static u8 StackSub_OnSelect(struct MenuProc *menu, struct MenuItemProc *item)
{
	int iid = GetNthStackableIid(gActiveUnit, item->itemNumber);

	if (!ItemIdIsStackableOnUnit(gActiveUnit, iid))
		return MENU_ACT_SND6B;

	StackItemsOfId(gActiveUnit, iid);

	if (!UnitHasStackableItems(gActiveUnit))
		return (ItemMenu_ButtonBPressed(menu, item) & ~MENU_ACT_SND6B) | MENU_ACT_SND6A;

	RedrawMenu(menu);
	return MENU_ACT_SND6A;
}

static const struct MenuItemDef sItemStackMenuItems[] = {
	{"", 0, MSG_MenuCommand_Stack_DESC, 0, 0, StackSub_Usability, StackSub_Draw, StackSub_OnSelect, 0, 0, 0},
	{"", 0, MSG_MenuCommand_Stack_DESC, 0, 1, StackSub_Usability, StackSub_Draw, StackSub_OnSelect, 0, 0, 0},
	{"", 0, MSG_MenuCommand_Stack_DESC, 0, 2, StackSub_Usability, StackSub_Draw, StackSub_OnSelect, 0, 0, 0},
	{"", 0, MSG_MenuCommand_Stack_DESC, 0, 3, StackSub_Usability, StackSub_Draw, StackSub_OnSelect, 0, 0, 0},
	{"", 0, MSG_MenuCommand_Stack_DESC, 0, 4, StackSub_Usability, StackSub_Draw, StackSub_OnSelect, 0, 0, 0},
	MenuItemsEnd
};

static const struct MenuDef sItemStackMenuDef = {
	{1, 1, 14, 0},
	0,
	sItemStackMenuItems,
	0, 0, 0,
	ItemMenu_ButtonBPressed,
	MenuAutoHelpBoxSelect,
	MenuStdHelpBox
};

u8 StackCommandUsability(const struct MenuItemDef *def, int number)
{
	if (!gpKernelDesignerConfig->item_stack)
		return MENU_NOTSHOWN;

	if (!UNIT_IS_VALID(gActiveUnit))
		return MENU_NOTSHOWN;

	if (!UnitHasStackableItems(gActiveUnit))
		return MENU_NOTSHOWN;

	return MENU_ENABLED;
}

u8 StackCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem)
{
	struct MenuRect rect = sItemStackMenuDef.rect;

	if (menuItem->xTile >= 12)
		rect.x = 22 + 7 - rect.w;

	ResetIconGraphics();
	LoadIconPalettes(4);

	StartMenuAt(&sItemStackMenuDef, rect, NULL);

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}