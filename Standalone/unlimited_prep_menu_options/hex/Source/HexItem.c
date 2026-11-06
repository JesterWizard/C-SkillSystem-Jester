/*
 * Hex Cards: the item-name and item-description hooks. The Installer hooks the vanilla
 * GetItemName and GetItemDescId with jumpToHack. A card shows its character's name and
 * description (the card type in the uses byte). Every other item gets the vanilla result:
 *
 *   GetItemName(item):   GetStringFromIndex(GetItemData(item & 0xFF)->nameTextId), then
 *                        StrInsertTact() (the vanilla tail, no argument), which is what
 *                        vanilla 0x080174F4 does.
 *   GetItemDescId(item): GetItemData(item & 0xFF)->descTextId (vanilla 0x08017518).
 */
#include "global.h"
#include "bmitem.h"
#include "bmunit.h"
#include "Hex.h"

char *HexItem_GetItemName(int item)
{
	if (IsHexCardItem(item))
		return GetStringFromIndex(HexCard_NameId(item));

	GetStringFromIndex(GetItemData(ITEM_INDEX(item))->nameTextId);
	return StrInsertTact();
}

int HexItem_GetItemDescId(int item)
{
	if (IsHexCardItem(item))
		return HexCard_DescId(item);

	return GetItemData(ITEM_INDEX(item))->descTextId;
}
