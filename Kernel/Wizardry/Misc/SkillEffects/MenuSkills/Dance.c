#include "common-chax.h"
#include "skill-system.h"

LYN_REPLACE_CHECK(DanceCommandUsability);
u8 DanceCommandUsability(const struct MenuItemDef *def, int number)
{
	// if (!(UNIT_CATTRIBUTES(gActiveUnit) & CA_DANCE))
	// 	return MENU_NOTSHOWN;

	gBmSt.um_tmp_item = ITEM_DANCE;
	return sub_80230F0(def);
}

//! FE8U = 0x08032358
LYN_REPLACE_CHECK(ActionDance);
s8 ActionDance(ProcPtr proc) {
    GetUnit(gActionData.targetIndex)->state &= ~( US_UNSELECTABLE | US_HAS_MOVED | US_HAS_MOVED_AI );

    BattleInitItemEffect(GetUnit(gActionData.subjectIndex), -1);
    BattleInitItemEffectTarget(GetUnit(gActionData.targetIndex));

    gBattleStats.config = BATTLE_CONFIG_REFRESH;

    BattleApplyMiscAction(proc);
	
	// This is a fix to allow units without the dancer class to use the dance skill
	if (UNIT_CATTRIBUTES(gActiveUnit) & CA_DANCE)
    	BeginBattleAnimations();

    return 0;
}

#if defined(SID_Dance) && (COMMON_SKILL_VALID(SID_Dance))
bool Action_Dance(ProcPtr parent)
{
    struct Unit *target = GetUnit(gActionData.targetIndex);
    if (!target || !target->pCharacterData)
        return false;
    target->state &= ~(US_UNSELECTABLE | US_HAS_MOVED | US_HAS_MOVED_AI);
    return true;
}
#endif