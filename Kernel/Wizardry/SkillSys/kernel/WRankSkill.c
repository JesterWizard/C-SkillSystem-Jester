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

static bool WRankSkillHeld(struct Unit *unit, u16 sid)
{
	return GetSkillSlot(unit, sid) != -1 || IsSkillLearned(unit, sid);
}

/*
 * Whether the unit holds a skill that replaces sid at a higher rank of wtype, following the chain (Prowess Lv1 is
 * replaced by Lv2 at C, Lv2 by Lv3 at B...). Walking the ranks from E again on a unit that holds Lv2 must not give
 * Lv1 back.
 */
static bool WRankSkillSuperseded(struct Unit *unit, int wtype, int rank, u16 sid)
{
	int r, slot;
	u16 cur = sid;

	for (r = rank + 1; r <= WPN_LEVEL_S; r++) {
		for (slot = 0; slot < SKILL_WRANK_SLOTS; slot++) {
			const struct SkillWRankEntry *ent = &gpSkillWRankTable->skills[wtype][r][slot];

			if (!EQUIP_SKILL_VALID(ent->sid) || ent->replaces != cur)
				continue;

			cur = ent->sid;
			if (WRankSkillHeld(unit, cur))
				return true;
		}
	}
	return false;
}

/* A lower level a unit still holds next to a higher one (saves from before the check): unequipped and forgotten */
static void DropWRankSkill(struct Unit *unit, u16 sid)
{
	int slot = GetSkillSlot(unit, sid);

	if (slot != -1)
		SET_SKILL(unit, slot, 0);

	if (IsSkillLearned(unit, sid))
		ForgetSkill(unit, sid);

	if (slot != -1)
		ResetSkillLists();
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

			if (WRankSkillSuperseded(unit, wtype, rank, sid)) {
				if (WRankSkillHeld(unit, sid))
					DropWRankSkill(unit, sid);
				continue;
			}

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
