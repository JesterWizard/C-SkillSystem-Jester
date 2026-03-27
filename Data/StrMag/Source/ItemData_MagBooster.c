#include "common-chax.h"
#include "strmag.h"
#include "item-sys.h"
#include "constants/texts.h"

/*
** JESTER - Apparently I need this active both here and in the IERevampTable.c for it to work, maybe I'll figure it out one day.
** If I remove the entry here, the effect doesn't active. If I remove the one in IERevampTable.c, the item is removed
*/

const struct ItemData ItemData_MagBooster = {
	.nameTextId = MSG_ITEM_MAGBOOSTER_NAME,
	.descTextId = MSG_ITEM_MAGBOOSTER_DESC,
	.useDescTextId = MSG_ITEM_MAGBOOSTER_USEDESC,
	.number = CONFIG_ITEM_INDEX_MAG_BOOSTER,
	.weaponType = ITYPE_ITEM,
	.pStatBonuses = &ItemBonus_MagBooster,
	.maxUses = 1,
	.encodedRange = 0x11,
	.costPerUse = 8000,
	.iconId = ITEM_ICON_MAG_BOOSTER,
	.useEffectId = IER_BOOSTER_POW,
};
