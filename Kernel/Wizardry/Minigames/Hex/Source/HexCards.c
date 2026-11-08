#include "common-chax.h"
#include "kernel-lib.h"
#include "item-sys.h"
#include "hex.h"

#ifdef CONFIG_HEX_CARDS

#if CHAX
extern const int sExpaConvoyItemAmount;
#endif

/*
 * The collection lives in the card binder (gHexSave, Kernel/Wizardry/Minigames/Hex/Source/HexSave.c). A Hex Card item is only a
 * way to receive a card: HexCol_MigrateItems moves the items in the convoy and in the player units' inventories into the binder,
 * and the Hex screen runs it when it opens.
 */

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

/* The name text of card type t: the character's name, or the class name for a monster card. */
int HexCardType_NameId(int type)
{
	const struct HexCardType *info = &gpHexCardTypes[type];

	if (info->pid == 0)
		return GetClassData(info->jid)->nameTextId;

	return GetCharacterData(info->pid)->nameTextId;
}

int HexCard_NameId(int item)
{
	return HexCardType_NameId(HexCard_Type(item));
}

/* Copies of a card type in the binder. 0 for a bad type. */
int HexCol_Count(int type)
{
	if (!TypeInRange(type))
		return 0;

	return HexBinder_Get(type);
}

/* Copies of every card type in the binder. */
int HexCol_Total(void)
{
	return HexBinder_Total();
}

/* Adds a copy to the binder. FALSE at HEX_BINDER_MAX or for a bad type. */
bool HexCol_Give(int type)
{
	return HexBinder_Add(type);
}

/* Takes a copy out of the binder (the loss stake). FALSE when none is owned or for a bad type. */
bool HexCol_TakeOne(int type)
{
	return HexBinder_Remove(type);
}

/*
 * Moves every Hex Card item in the convoy and in the inventory of each living player unit into the binder. An item stays where
 * it is when its type is already at HEX_BINDER_MAX copies.
 */
void HexCol_MigrateItems(void)
{
	u16 *convoy = GetConvoyItemArray();
	int i, j;

	i = 0;
	while (i < ConvoySlotCount()) {
		/* RemoveItemFromConvoy shifts the later items down, so the same index is checked again. */
		if (CardMatches(convoy[i], -1) && HexBinder_Add(HexCard_Type(convoy[i])))
			RemoveItemFromConvoy(i);
		else
			i++;
	}

	for (i = 0; i < (int)ARRAY_COUNT(gUnitArrayBlue); i++) {
		struct Unit *unit = &gUnitArrayBlue[i];

		if (!UNIT_IS_VALID(unit) || (unit->state & US_DEAD))
			continue;

		j = 0;
		while (j < UNIT_ITEM_COUNT) {
			/* RemoveUnitItemSlot shifts the later items down, so the same slot is checked again. */
			if (CardMatches(unit->items[j], -1) && HexBinder_Add(HexCard_Type(unit->items[j])))
				RemoveUnitItemSlot(unit, j);
			else
				j++;
		}
	}
}

/* Starter copies go to the binder, and only when the player owns no cards at all. */
void HexCol_GiveStarters(void)
{
	int type, n;

	if (HexCol_Total() > 0)
		return;

	for (type = 0; type < HEX_CARD_TYPES; type++) {
		for (n = 0; n < gpHexCardTypes[type].starter; n++) {
			if (!HexCol_Give(type))
				return;
		}
	}
}

#endif /* CONFIG_HEX_CARDS */
