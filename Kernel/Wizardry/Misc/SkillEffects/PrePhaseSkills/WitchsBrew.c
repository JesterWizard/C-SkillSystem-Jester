#include "common-chax.h"
#include "debuff.h"
#include "kernel-lib.h"
#include "class-types.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "jester_headers/custom-functions.h"

void Popup_GotItem(ProcPtr proc)
{
	NewPopup_Simple(PopupScr_GotItem, 0x60, 0x0, proc);
};

const struct ProcCmd ProcPopup_GotItem[] = {
	PROC_CALL(Popup_GotItem),
	PROC_YIELD,
	PROC_END
};

static const u8 healingItems[] = {
	ITEM_VULNERARY,
	ITEM_ELIXIR,
};

static const size_t healingItemsCount = sizeof(healingItems) / sizeof(healingItems[0]);

bool PrePhaseFunc_WitchsBrew(ProcPtr proc)
{
#if (defined(SID_WitchsBrew) && (COMMON_SKILL_VALID(SID_WitchsBrew)))

	FOR_UNITS_ONMAP_FACTION(gPlaySt.faction, unit, {
		if (gPlaySt.chapterTurnNumber > SKILL_EFF0(SID_WitchsBrew))
			continue;

		if (unit->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
			continue;

		if (!SkillTester(unit, SID_WitchsBrew))
			continue;

		u8 chosenHealingItem = healingItems[k_umod(NextRN_100(), healingItemsCount)];

		if (NextRN_100() <= unit->lck)
		{
			if (unit->items[4] == ITEM_NONE)
			{
				SetPopupItem(chosenHealingItem);
				Proc_StartBlocking(ProcPopup_GotItem, proc);
				UnitAddItem(unit, MakeNewItem(chosenHealingItem));
			}
			else
			{
				NewPopup_ItemGot(proc, unit, chosenHealingItem);
			}
		}
	})
#endif
	return false;
}
