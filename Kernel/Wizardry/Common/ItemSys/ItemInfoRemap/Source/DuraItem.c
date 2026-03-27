#include "common-chax.h"
#include "item-sys.h"
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
    default:
        return false;
    }
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
