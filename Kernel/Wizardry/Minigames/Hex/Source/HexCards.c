#include "common-chax.h"
#include "kernel-lib.h"
#include "item-sys.h"
#include "hex.h"

#ifdef CONFIG_HEX_CARDS

#if CHAX
extern const int sExpaConvoyItemAmount;
#endif

static int ConvoySlotCount(void)
{
#if CHAX
	return sExpaConvoyItemAmount;
#else
	return CONVOY_ITEM_COUNT;
#endif
}

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

	for (i = 0; i < ConvoySlotCount(); i++) {
		if (CardMatches(convoy[i], type))
			count++;
	}

	for (i = 0; i < (int)ARRAY_COUNT(gUnitArrayBlue); i++) {
		struct Unit *unit = &gUnitArrayBlue[i];

		if (!UNIT_IS_VALID(unit) || (unit->state & US_DEAD))
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
	return ITEM_INDEX(item) == CONFIG_ITEM_INDEX_HEX_CARD;
}

/* Item word for a card of the given type: uses byte = type, index = CONFIG_ITEM_INDEX_HEX_CARD. 0 for a bad type. */
int MakeHexCardItem(int type)
{
	if (!TypeInRange(type))
		return 0;

	return (type << 8) | CONFIG_ITEM_INDEX_HEX_CARD;
}

int HexCard_Type(int item)
{
	int type = ITEM_USES(item);

	if (type >= HEX_CARD_TYPES)
		return 0;

	return type;
}

int HexCard_NameId(int item)
{
	return GetCharacterData(gpHexCardTypes[HexCard_Type(item)].pid)->nameTextId;
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

/* FALSE when the convoy is full. */
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
	for (i = 0; i < ConvoySlotCount(); i++) {
		if (CardMatches(convoy[i], type)) {
			RemoveItemFromConvoy(i);
			return true;
		}
	}

	for (i = 0; i < (int)ARRAY_COUNT(gUnitArrayBlue); i++) {
		struct Unit *unit = &gUnitArrayBlue[i];

		if (!UNIT_IS_VALID(unit) || (unit->state & US_DEAD))
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
		for (n = 0; n < gpHexCardTypes[type].starter; n++) {
			if (!HexCol_GiveToConvoy(type))
				return;
		}
	}
}

#endif /* CONFIG_HEX_CARDS */
