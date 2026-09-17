#include "common-chax.h"
#include "skill-system.h"
#include "kernel-lib.h"
#include "weapon-slots.h"
#include "popup.h"

static void EquipWRankSkill(struct Unit *unit, u16 sid, u16 replaces)
{
	int slot;

	if (GetSkillSlot(unit, sid) != -1)
		return;

	if (EQUIP_SKILL_VALID(replaces)) {
		slot = GetSkillSlot(unit, replaces);
		if (slot != -1) {
			ForgetSkill(unit, replaces);
			SET_SKILL(unit, slot, sid);
			LearnSkill(unit, sid);
			ResetSkillLists();
			return;
		}
	}

	AddSkill(unit, sid);
}

void TryAddSkillWRankRange(struct Unit *unit, int wtype, int fromLevel, int toLevel, bool popup)
{
	int rank, slot;

	if (!UNIT_IS_VALID(unit))
		return;

	if (!gpKernelDesignerConfig->weapon_rank_skills)
		return;

	if (wtype < 0 || wtype >= SKILL_WRANK_TYPE_COUNT)
		return;

	if (fromLevel < WPN_LEVEL_E)
		fromLevel = WPN_LEVEL_E;

	if (toLevel > WPN_LEVEL_S)
		toLevel = WPN_LEVEL_S;

	if (fromLevel > toLevel)
		return;

	for (rank = fromLevel; rank <= toLevel; rank++) {
		for (slot = 0; slot < SKILL_WRANK_SLOTS; slot++) {
			const struct SkillWRankEntry *ent = &gpSkillWRankTable->skills[wtype][rank][slot];
			u16 sid = ent->sid;
			u16 replaces = ent->replaces;

			if (!EQUIP_SKILL_VALID(sid))
				continue;

			if (GetSkillSlot(unit, sid) != -1)
				continue;

			EquipWRankSkill(unit, sid, replaces);

			if (popup && GetSkillSlot(unit, sid) != -1) {
				if (EQUIP_SKILL_VALID(replaces))
					PushWRankSkillPopup(replaces, rank - WPN_LEVEL_E);
				else
					PushSkillListStack(sid);
			}
		}
	}
}

void TryAddSkillWRank(struct Unit *unit, bool popup)
{
	u8 wtypes[UNIT_WEAPON_SLOT_COUNT];
	int i, amt, wtype, level;

	if (!UNIT_IS_VALID(unit))
		return;

	if (!gpKernelDesignerConfig->weapon_rank_skills)
		return;

	amt = ListUnitMappedWeaponTypes(unit, wtypes, UNIT_WEAPON_SLOT_COUNT);
	for (i = 0; i < amt; i++) {
		wtype = wtypes[i];
		level = GetWeaponLevelFromExp(UNIT_WRANK(unit, wtype));
		TryAddSkillWRankRange(unit, wtype, WPN_LEVEL_E, level, popup);
	}
}

void TryAddSkillWRankFromBattleUnit(struct BattleUnit *bu, bool popup)
{
	struct Unit *unit;
	int newExp, oldLv, newLv;

	if (!bu)
		return;

	newExp = GetBattleUnitUpdatedWeaponExp(bu);
	if (newExp <= 0)
		return;

	unit = GetUnit(bu->unit.index);
	if (!UNIT_IS_VALID(unit))
		return;

	oldLv = GetWeaponLevelFromExp(UNIT_WRANK(&bu->unit, bu->weaponType));
	newLv = GetWeaponLevelFromExp(newExp);
	if (newLv <= oldLv)
		return;

	TryAddSkillWRankRange(unit, bu->weaponType, oldLv + 1, newLv, popup);
	if (popup)
		SetPopupUnit(unit);
}
