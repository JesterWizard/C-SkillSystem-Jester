#include "common-chax.h"
#include "debuff.h"
#include "status-getter.h"
#include "action-expa.h"
#include "unit-expa.h"
#include "skill-system.h"
#include "battle-system.h"
#include "constants/skills.h"
#include "jester_headers/macros.h"

#ifdef CONFIG_ITEM_INDEX_SKILL_STEALER
static bool SkillStealer_Held(struct Unit *unit)
{
	int i;

	for (i = 0; i < UNIT_ITEM_COUNT; i++)
		if (GetItemIndex(unit->items[i]) == CONFIG_ITEM_INDEX_SKILL_STEALER)
			return true;
	return false;
}

/*
 * The Skill Stealer copies a defeated boss's first skill (equipped skills come first in its list) as a scroll: the
 * skill itself, not the next one, and nothing when the boss has no skill a scroll can hold.
 */
static u16 SkillStealer_Scroll(struct Unit *boss)
{
	struct SkillList *list = GetUnitSkillList(boss);
	u16 scroll;
	int i;

	for (i = 0; i < list->amt; i++) {
		if (!COMMON_SKILL_VALID(list->sid[i]))
			continue;
		scroll = MakeSkillScrollItem(list->sid[i]);
		if (scroll)
			return scroll;
	}
	return 0;
}
#endif

bool PostAction_GetItem(ProcPtr parent)
{
	FORCE_DECLARE struct Unit *unit = gActiveUnit;

	if (!UNIT_IS_VALID(unit))
		return false;

	if (!UNIT_IS_VALID(unit) || UNIT_STONED(unit))
		return false;

	switch (gActionData.unitActionType) {
	case UNIT_ACTION_COMBAT:
	case CONFIG_UNIT_ACTION_EXPA_GaidenMagicCombat:
		if (gBattleActorGlobalFlag.enemy_defeated) {

#ifdef CONFIG_ITEM_INDEX_SKILL_STEALER
		if ((UNIT_CATTRIBUTES(&gBattleTarget.unit) & CA_BOSS) && SkillStealer_Held(unit)) {
			u16 scroll = SkillStealer_Scroll(&gBattleTarget.unit);

			if (scroll) {
				NewPopup_ItemGot(parent, unit, scroll);
				return true;
			}
		}
#endif


#if defined(SID_Despoil) && (COMMON_SKILL_VALID(SID_Despoil))
		if (SkillListTester(unit, SID_Despoil)) {
			NewPopup_ItemGot(parent, unit, ITEM_REDGEM);
			return true;
		}
#endif

#if defined(SID_GoldDigger) && (COMMON_SKILL_VALID(SID_GoldDigger))
		if (SkillListTester(unit, SID_GoldDigger) && gBattleActorGlobalFlag.enemy_defeated) {
			NewPopup_GoldGot(parent, unit, SKILL_EFF0(SID_GoldDigger));
			return true;
		}
#endif

#if defined(SID_MakeAKilling) && (COMMON_SKILL_VALID(SID_MakeAKilling))
		if (SkillListTester(unit, SID_MakeAKilling) && gBattleActorGlobalFlag.enemy_defeated)
		{
			if (Roll1RN(unit->lck))
			{
				NewPopup_GoldGot(parent, unit, SKILL_EFF0(SID_MakeAKilling) * 100);
				return true;
			}
		}
#endif
		}
		break;

	default:
		break;
	}
	return false;
}
