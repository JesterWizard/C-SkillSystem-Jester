#include "common-chax.h"
#include "bwl.h"
#include "lvup.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "gaiden-magic.h"
#include "constants/texts.h"

#define LOCAL_TRACE 0

/**
 * Slot ops
 */
static void SortRamSkillList(struct Unit * unit)
{
    int i, cnt = 0;
    u16 buf[UNIT_RAM_SKILLS_LEN] = {0};

    for (i = 0; i < UNIT_RAM_SKILLS_LEN; i++) {
        u16 sid = GET_SKILL(unit, i);
        if (EQUIP_SKILL_VALID(sid))
            buf[cnt++] = sid;
    }

    for (i = 0; i < UNIT_RAM_SKILLS_LEN; i++)
        SET_SKILL(unit, i, i < cnt ? buf[i] : 0);
}

inline int GetSkillSlot(struct Unit * unit, int sid)
{
    for (int i = 0; i < RAM_SKILL_LEN_EXT; i++)
        if (GET_SKILL(unit, i) == sid)
            return i;

    return -1;
}

inline int GetFreeSkillSlot(struct Unit * unit)
{
    for (int i = 0; i < RAM_SKILL_LEN_EXT; i++)
        if (!EQUIP_SKILL_VALID(GET_SKILL(unit, i)))
            return i;

    return -1;
}

bool CanRemoveSkill(struct Unit * unit, const u16 sid)
{
    if (!EQUIP_SKILL_VALID(sid))
        return false;

    for (int i = 0; i < UNIT_RAM_SKILLS_LEN; i++)
        if (GET_SKILL(unit, i) == sid)
            return true;

    return false;
}

int RemoveSkill(struct Unit * unit, const u16 sid)
{
    if (!EQUIP_SKILL_VALID(sid))
        return -1;

    for (int i = 0; i < UNIT_RAM_SKILLS_LEN; i++) {
        if (GET_SKILL(unit, i) == sid) {
            SET_SKILL(unit, i, 0);
            SortRamSkillList(unit);
            ResetSkillLists();
            return 0;
        }
    }
    return -1;
}

int AddSkill(struct Unit * unit, const u16 sid)
{
    if (sid >= MAX_EQUIP_SKILL_NUM)
        return -1;

    LearnSkill(unit, sid);

    if (GetSkillSlot(unit, sid) != -1)
        return 0;

    int slot = GetFreeSkillSlot(unit);
    if (slot == -1)
        return -1;

    SET_SKILL(unit, slot, sid);
    ResetSkillLists();
    return 0;
}

static inline void load_skill_ext(struct Unit * unit, u16 sid)
{
    if (EQUIP_SKILL_VALID(sid))
    {
        if (UNIT_FACTION(unit) == FACTION_BLUE)
            LearnSkill(unit, sid);

        AddSkill(unit, sid);
    }
}

void UnitAutoLoadSkills(struct Unit *unit)
{
	int i;
	int level_p, level_j;
	const struct SkillPreloadPConf *pConf = &gpSkillPreloadPData[UNIT_CHAR_ID(unit)];
	const struct SkillPreloadJConf *jConf = &gpSkillPreloadJData[UNIT_CLASS_ID(unit)];

	if (!UNIT_IS_VALID(unit))
		return;

	level_j = k_udiv(unit->level, 5) * 5;
	level_p = k_udiv(unit->level + GetUnitHiddenLevel(unit), 5) * 5;

	LIMIT_AREA(level_j, 0, CHAX_MAX_LEVEL);
	LIMIT_AREA(level_p, 0, CHAX_MAX_RECORD_LEVEL);

	while (level_p >= 0) {
		for (i = 0; i < 5; i++)
			AddSkill(unit, pConf->skills[level_p + i]);

		level_p = level_p - 5;
	}

	while (level_j >= 0) {
		for (i = 0; i < 5; i++)
			AddSkill(unit, jConf->skills[level_j + i]);

		level_j = level_j - 5;
	}

#ifdef CONFIG_FIT_OLD_SKILLSYS_LIST
	FebList_LoadUnitSkill(unit);
#endif /* FIT_OLD_SKILLSYS_LIST */

	/* For debug, we enable unit learn all of skills */
	if (gpKernelDesigerConfig->debug_autoload_skills) {
#ifdef CONFIG_TURN_ON_ALL_SKILLS
		int allSkills = 1023;
#else
		int allSkills = 254;
#endif
		for (i = 1; i < allSkills; i++)
			LearnSkill(unit, i);
	}
}

/**
 * Lvup
 */
STATIC_DECLAR void TryAddSkillLvupPConf(struct Unit *unit, int level)
{
	int i;
	u16 sid;

	const struct SkillPreloadPConf *pConf = &gpSkillPreloadPData[UNIT_CHAR_ID(unit)];
	int _level = k_udiv(level, 5) * 5;

	for (i = 0; i < 5; i++) {
		sid = pConf->skills[_level + i];

		if (EQUIP_SKILL_VALID(sid)) {
			AddSkill(unit, sid);
			PushSkillListStack(sid);
			
			SetPopupItem(sid);
			ProcPtr * phaseToBlock = Proc_Find(gProcScr_PlayerPhase);
			switch (gPlaySt.faction) {
				case FACTION_BLUE:
					phaseToBlock = Proc_Find(gProcScr_PlayerPhase);
					SetPopupUnit(gActiveUnit);
					break;
				case FACTION_RED:
					phaseToBlock = Proc_Find(gProcScr_CpDecide);
					SetPopupUnit(GetUnit(gActionData.targetIndex));
					break;
				case FACTION_GREEN:
					phaseToBlock = Proc_Find(gProcScr_CpDecide);
					SetPopupUnit(GetUnit(gActionData.targetIndex));
					break;
				case FACTION_PURPLE:
					phaseToBlock = Proc_Find(gProcScr_CpDecide);
					SetPopupUnit(GetUnit(gActionData.targetIndex));
					break;		
			};		

    		NewPopup_Simple(PopupScr_LearnSkill, SONG_SE_UPDATE, 0x00, phaseToBlock);
		}
	}
}

STATIC_DECLAR void TryAddSkillLvupJConf(struct Unit *unit, int level)
{
	int i;
	u16 sid;

	const struct SkillPreloadJConf *jConf = &gpSkillPreloadJData[UNIT_CLASS_ID(unit)];
	int _level = k_udiv(level, 5) * 5;

	for (i = 0; i < 5; i++) {
		sid = jConf->skills[_level + i];

		if (EQUIP_SKILL_VALID(sid)) {
			AddSkill(unit, sid);
			PushSkillListStack(sid);
		}
	}
}

void TryAddSkillLvup(struct Unit *unit, int level)
{
	int _level;

	if (!UNIT_IS_VALID(unit))
		return;

#ifdef CONFIG_FIT_OLD_SKILLSYS_LIST
	FebList_LvupAddSkill(unit, level);
#endif

	_level = level;
	if (k_umod(_level, 5) == 0)
		TryAddSkillLvupJConf(unit, _level);

	_level = level + GetUnitHiddenLevel(unit);
	if (k_umod(_level, 5) == 0)
		TryAddSkillLvupPConf(unit, _level);
	
	TryAddGaidenMagicPConf(unit, level);
}

/**
 * Promotion
 */
void TryAddSkillPromotion(struct Unit *unit, int jid)
{
	int i;
	u16 sid;

	const struct SkillPreloadJConf *jConf = &gpSkillPreloadJData[jid];

	ResetPopupSkillStack();

	if (!UNIT_IS_VALID(unit))
		return;

#ifdef CONFIG_FIT_OLD_SKILLSYS_LIST
	FebList_PromotionAddSkill(unit);
#endif

	for (i = 0; i < 5; i++) {
		sid = jConf->skills[0 + i];

		if (EQUIP_SKILL_VALID(sid)) {
			AddSkill(unit, sid);
			PushSkillListStack(sid);
			
			SetPopupItem(sid);
			ProcPtr * phaseToBlock = Proc_Find(ProcScr_ClassChgReal);

			switch (gPlaySt.faction) {
				case FACTION_BLUE:
					SetPopupUnit(gActiveUnit);
					break;
				case FACTION_RED:
					SetPopupUnit(GetUnit(gActionData.targetIndex));
					break;
				case FACTION_GREEN:
					SetPopupUnit(GetUnit(gActionData.targetIndex));
					break;
				case FACTION_PURPLE:
					SetPopupUnit(GetUnit(gActionData.targetIndex));
					break;		
			};		

    		NewPopup_Simple(PopupScr_LearnSkill, SONG_SE_UPDATE, 0x00, phaseToBlock);
		}
	}

	/**
	 * Append const list
	 */
	sid = gpConstSkillTable_Job[jid * 2 + 0];
	if (COMMON_SKILL_VALID(sid))
		PushSkillListStack(sid);

	sid = gpConstSkillTable_Job[jid * 2 + 1];
	if (COMMON_SKILL_VALID(sid))
		PushSkillListStack(sid);
}
