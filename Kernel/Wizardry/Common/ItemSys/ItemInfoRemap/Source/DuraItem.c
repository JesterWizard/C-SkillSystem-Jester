#include "common-chax.h"
#include "item-sys.h"
#include "constants/texts.h"
#include "skill-system.h"

bool IsDuraItem(int item)
{
    switch (ITEM_INDEX(item)) {
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_1
    case CONFIG_ITEM_INDEX_SKILL_SCROLL_1:
        return true;
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_2
    case CONFIG_ITEM_INDEX_SKILL_SCROLL_2:
        return true;
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_3
    case CONFIG_ITEM_INDEX_SKILL_SCROLL_3:
        return true;
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_4
    case CONFIG_ITEM_INDEX_SKILL_SCROLL_4:
        return true;
#endif
#ifdef CONFIG_ITEM_INDEX_ARMS_SCROLL
    case CONFIG_ITEM_INDEX_ARMS_SCROLL:
        return true;
#endif
    case ITEM_TONIC:
        return true;
    default:
        return false;
    }
}

static const char *const sTonicNames[] = {
	"HP Tonic",
	"STR Tonic",
	"SKL Tonic",
	"SPD Tonic",
	"DEF Tonic",
	"RES Tonic",
	"LCK Tonic",
	"MAG Tonic",
	"OMNI Tonic",
};

static const int sTonicIconIds[] = {
	ITEM_ICON_TONIC_HP,
	ITEM_ICON_TONIC_STR,
	ITEM_ICON_TONIC_SKL,
	ITEM_ICON_TONIC_SPD,
	ITEM_ICON_TONIC_DEF,
	ITEM_ICON_TONIC_RES,
	ITEM_ICON_TONIC_LCK,
	ITEM_ICON_TONIC_MAG,
	ITEM_ICON_TONIC_OMNI,
};

static const int sTonicDescIds[] = {
	MSG_ITEM_TONIC_HP_DESC,
	MSG_ITEM_TONIC_STR_DESC,
	MSG_ITEM_TONIC_SKL_DESC,
	MSG_ITEM_TONIC_SPD_DESC,
	MSG_ITEM_TONIC_DEF_DESC,
	MSG_ITEM_TONIC_RES_DESC,
	MSG_ITEM_TONIC_LCK_DESC,
	MSG_ITEM_TONIC_MAG_DESC,
	MSG_ITEM_TONIC_OMNI_DESC,
};

static const int sTonicUseDescIds[] = {
	MSG_ITEM_TONIC_HP_USEDESC,
	MSG_ITEM_TONIC_STR_USEDESC,
	MSG_ITEM_TONIC_SKL_USEDESC,
	MSG_ITEM_TONIC_SPD_USEDESC,
	MSG_ITEM_TONIC_DEF_USEDESC,
	MSG_ITEM_TONIC_RES_USEDESC,
	MSG_ITEM_TONIC_LCK_USEDESC,
	MSG_ITEM_TONIC_MAG_USEDESC,
	MSG_ITEM_TONIC_OMNI_USEDESC,
};

bool IsTonicItem(int item)
{
	return ITEM_INDEX(item) == ITEM_TONIC;
}

static int TonicIndex(int item)
{
	int tonic = ITEM_USES(item);
	if (tonic < 1 || tonic > 9)
		return 0;

	return tonic - 1;
}

char *GetTonicItemName(int item)
{
	return (char *)sTonicNames[TonicIndex(item)];
}

int GetTonicItemDescId(int item)
{
	return sTonicDescIds[TonicIndex(item)];
}

int GetTonicItemUseDescId(int item)
{
	return sTonicUseDescIds[TonicIndex(item)];
}

int GetTonicItemIconId(int item)
{
	return sTonicIconIds[TonicIndex(item)];
}

char *GetDuraItemName(int item)
{
	if (IsSkillScrollItem(item))
		return GetSkillScrollItemName(item);

#ifdef CONFIG_ITEM_INDEX_ARMS_SCROLL
	if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_ARMS_SCROLL) {
		char *result = GetStringFromIndex(GetItemData(ITEM_INDEX(item))->nameTextId);
		result = StrInsertTact();
		return result;
	}
#endif

	Errorf("Invalid dura-item: %x", item);
	return NULL;
}

int GetDuraItemDescId(int item)
{
	if (IsSkillScrollItem(item))
		return GetSkillScrollItemDescId(item);

#ifdef CONFIG_ITEM_INDEX_ARMS_SCROLL
	if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_ARMS_SCROLL)
		return GetItemData(ITEM_INDEX(item))->descTextId;
#endif

	Errorf("Invalid dura-item: %x", item);
	return 0;
}

int GetDuraItemUseDescId(int item)
{
	if (IsSkillScrollItem(item))
		return GetSkillScrollItemUseDescId(item);

#ifdef CONFIG_ITEM_INDEX_ARMS_SCROLL
	if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_ARMS_SCROLL)
		return GetItemData(ITEM_INDEX(item))->useDescTextId;
#endif

	Errorf("Invalid dura-item: %x", item);
	return 0;
}

int GetDuraItemIconId(int item)
{
	if (IsSkillScrollItem(item))
		return GetSkillScrollItemIconId(item);

#ifdef CONFIG_ITEM_INDEX_ARMS_SCROLL
	if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_ARMS_SCROLL)
		return GetItemData(ITEM_INDEX(item))->iconId;
#endif

	Errorf("Invalid dura-item: %x", item);
	return 0;
}
