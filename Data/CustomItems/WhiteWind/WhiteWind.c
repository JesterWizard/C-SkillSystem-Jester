#include "common-chax.h"
#include "item-sys.h"
#include "battle-system.h"
#include "constants/items.h"

int WhiteWindHealAmountGetter(int old, struct Unit *actor, struct Unit *target)
{
	int item;

	if (!actor || !target)
		return old;

	if (gActionData.subjectIndex != actor->index)
		return old;

	item = GetItemFromSlot(actor, gActionData.itemSlotIndex);
	if (GetItemIndex(item) != ITEM_STAFF_WHITE_WIND)
		return old;

	return GetUnitMaxHp(actor);
}