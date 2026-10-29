#include "global.h"
#include "bmunit.h"
#include "bmreliance.h"
#include "bmsave.h"
#include "eventinfo.h"

/**
 * Replaces vanilla IsExtraSupportViewerEnabled (0x080A332C).
 * Its only caller is InitSaveMenuChoice, so the Extras > Support entry
 * is always added to the title main menu.
 */
bool IsExtraSupportViewerEnabled_Unlocked(void)
{
	return TRUE;
}

/**
 * Replaces vanilla GGM_IsCharacterKnown (0x080A37F0).
 * Every character counts as "met", so the Support viewer lists everyone.
 */
bool GGM_IsCharacterKnown_Unlocked(int index, struct GlobalSaveInfo *buf)
{
	(void)index;
	(void)buf;
	return TRUE;
}

/**
 * Replaces vanilla GetGlobalSupportListFromSave (0x080A35EC).
 * Instead of reading which support conversations were seen from the global
 * save, every support partner that has a conversation in the support talk
 * list is reported as rank A (3), so C/B/A can all be viewed.
 */
void GetGlobalSupportListFromSave_Unlocked(int unitId, u8 *data, struct GlobalSaveInfo *info)
{
	const struct SupportData *supp = GetCharacterData(unitId)->pSupportData;
	int i, count = 0;

	(void)info;

	if (supp != NULL) {
		count = supp->supportCount;

		if (count > UNIT_SUPPORT_MAX_COUNT)
			count = UNIT_SUPPORT_MAX_COUNT;

		for (i = 0; i < count; i++)
			data[i] = GetSupportTalkEntry(unitId, supp->characters[i]) ? 3 : 0;
	}

	for (i = count; i < UNIT_SUPPORT_MAX_COUNT; i++)
		data[i] = 0;
}
