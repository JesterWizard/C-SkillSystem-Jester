/*
 * Hex Cards: the card item and the collection (count, total, give, take, starters).
 * Port of Kernel/Wizardry/Minigames/Hex/Source/HexCards.c. Uses vanilla convoy and unit
 * functions only (FE-CLib). The convoy is CONVOY_ITEM_COUNT slots, the same as vanilla.
 */
#include "global.h"
#include "bmitem.h"
#include "bmunit.h"
#include "bmcontainer.h"
#include "Hex.h"

#define HEX_UNIT_IS_LIVING(aUnit) (UNIT_IS_VALID(aUnit) && !((aUnit)->state & US_DEAD))

static bool CardMatches(int item, int type)
{
	if (!IsHexCardItem(item))
		return false;

	return type < 0 || HexCard_Type(item) == type;
}

static bool TypeInRange(int type)
{
	return type >= 0 && type < HEX_CARD_TYPES;
}

/* Cards in the convoy and in the inventory of every living player unit. type < 0 counts every card. */
static int CountCards(int type)
{
	int count = 0;
	u16 *convoy = GetConvoyItemArray();
	int i, j;

	for (i = 0; i < CONVOY_ITEM_COUNT; i++) {
		if (CardMatches(convoy[i], type))
			count++;
	}

	for (i = 0; i < (int)ARRAY_COUNT(gUnitArrayBlue); i++) {
		struct Unit *unit = &gUnitArrayBlue[i];

		if (!HEX_UNIT_IS_LIVING(unit))
			continue;

		for (j = 0; j < UNIT_ITEM_COUNT; j++) {
			if (CardMatches(unit->items[j], type))
				count++;
		}
	}

	return count;
}

/* Close the gap a removed item leaves. The slot is never the equipped weapon (cards cannot be equipped). */
static void RemoveUnitItemSlot(struct Unit *unit, int slot)
{
	int k;

	for (k = slot; k < UNIT_ITEM_COUNT - 1; k++)
		unit->items[k] = unit->items[k + 1];

	unit->items[UNIT_ITEM_COUNT - 1] = 0;
}

bool IsHexCardItem(int item)
{
	return ITEM_INDEX(item) == HEX_CARD_ITEM;
}

/* Item word for a card of the given type: uses byte = type, index = HEX_CARD_ITEM. 0 for a bad type. */
int MakeHexCardItem(int type)
{
	if (!TypeInRange(type))
		return 0;

	return (type << 8) | HEX_CARD_ITEM;
}

/* The uses byte is the type. A type past the table is shown as type 0 (Ross). */
int HexCard_Type(int item)
{
	int type = ITEM_USES(item);

	if (type >= HEX_CARD_TYPES)
		return 0;

	return type;
}

/* The character the card is named after: the name and description come from that character. */
int HexCard_NameId(int item)
{
	return GetCharacterData(gHexCardTypes[HexCard_Type(item)].pid)->nameTextId;
}

int HexCard_DescId(int item)
{
	return GetCharacterData(gHexCardTypes[HexCard_Type(item)].pid)->descTextId;
}

int HexCol_Count(int type)
{
	if (!TypeInRange(type))
		return 0;

	return CountCards(type);
}

int HexCol_Total(void)
{
	return CountCards(-1);
}

/* FALSE when the convoy is full. AddItemToConvoy returns the slot, or -1 when full. */
bool HexCol_GiveToConvoy(int type)
{
	int item = MakeHexCardItem(type);

	if (!item)
		return false;

	return AddItemToConvoy(item) >= 0;
}

/* Convoy first, then the inventory of a living player unit. FALSE when no card of that type is owned. */
bool HexCol_TakeOne(int type)
{
	u16 *convoy;
	int i, j;

	if (!TypeInRange(type))
		return false;

	convoy = GetConvoyItemArray();
	for (i = 0; i < CONVOY_ITEM_COUNT; i++) {
		if (CardMatches(convoy[i], type)) {
			RemoveItemFromConvoy(i);
			return true;
		}
	}

	for (i = 0; i < (int)ARRAY_COUNT(gUnitArrayBlue); i++) {
		struct Unit *unit = &gUnitArrayBlue[i];

		if (!HEX_UNIT_IS_LIVING(unit))
			continue;

		for (j = 0; j < UNIT_ITEM_COUNT; j++) {
			if (CardMatches(unit->items[j], type)) {
				RemoveUnitItemSlot(unit, j);
				return true;
			}
		}
	}

	return false;
}

/* Starter copies go to the convoy, and only when the player owns no cards at all. */
void HexCol_GiveStarters(void)
{
	int type, n;

	if (HexCol_Total() > 0)
		return;

	for (type = 0; type < HEX_CARD_TYPES; type++) {
		for (n = 0; n < gHexCardTypes[type].starter; n++) {
			if (!HexCol_GiveToConvoy(type))
				return;
		}
	}
}
