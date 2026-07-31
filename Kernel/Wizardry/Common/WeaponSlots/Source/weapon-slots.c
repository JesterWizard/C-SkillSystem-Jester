#include "common-chax.h"
#include "weapon-slots.h"
#include "kernel-lib.h"

STATIC_DECLAR bool IsRankBearingWeaponType(int wtype)
{
	/* Ballista / item / dragonstone are not rank slots. */
	if (wtype < 0)
		return false;

	if (wtype == ITYPE_BLLST || wtype == ITYPE_ITEM || wtype == ITYPE_DRAGN)
		return false;

	return true;
}

STATIC_DECLAR bool DynamicWeaponSlotsEnabled(void)
{
	return gpKernelDesignerConfig && gpKernelDesignerConfig->dynamic_weapon_slots;
}

const struct ClassWeaponSlotConf *GetClassWeaponSlotConf(int jid)
{
	const struct ClassWeaponSlotConf *it;

	if (!DynamicWeaponSlotsEnabled())
		return NULL;

	if (!gpClassWeaponSlotConf || jid <= 0)
		return NULL;

	for (it = gpClassWeaponSlotConf; it->jid != 0; it++) {
		if (it->jid == jid)
			return it;
	}

	return NULL;
}

int GetClassWeaponSlotType(int jid, int slot)
{
	const struct ClassWeaponSlotConf *conf;

	if (slot < 0 || slot >= UNIT_WEAPON_SLOT_COUNT)
		return WEAPON_SLOT_NONE;

	conf = GetClassWeaponSlotConf(jid);
	if (conf)
		return conf->wtypes[slot];

	/* Identity mapping when disabled or for unlisted classes. */
	return slot;
}

int GetClassWeaponRankSlot(int jid, int wtype)
{
	int slot;

	if (!IsRankBearingWeaponType(wtype))
		return -1;

	for (slot = 0; slot < UNIT_WEAPON_SLOT_COUNT; slot++) {
		if (GetClassWeaponSlotType(jid, slot) == wtype)
			return slot;
	}

	return -1;
}

int GetClassWeaponRank(int jid, int wtype)
{
	const struct ClassData *jinfo;
	int slot = GetClassWeaponRankSlot(jid, wtype);

	if (slot < 0)
		return 0;

	jinfo = GetClassData(jid);
	if (!jinfo)
		return 0;

	return jinfo->baseRanks[slot];
}

bool ClassHasWeaponType(int jid, int wtype)
{
	return GetClassWeaponRankSlot(jid, wtype) >= 0;
}

int GetUnitWeaponRankSlot(struct Unit *unit, int wtype)
{
	if (!unit || !unit->pClassData)
		return -1;

	return GetClassWeaponRankSlot(UNIT_CLASS_ID(unit), wtype);
}

int GetUnitWeaponSlotType(struct Unit *unit, int slot)
{
	if (!unit || !unit->pClassData)
		return WEAPON_SLOT_NONE;

	return GetClassWeaponSlotType(UNIT_CLASS_ID(unit), slot);
}

int GetUnitWeaponExpBySlot(struct Unit *unit, int slot)
{
	if (!unit || slot < 0 || slot >= UNIT_WEAPON_SLOT_COUNT)
		return 0;

	return unit->ranks[slot];
}

void SetUnitWeaponExpBySlot(struct Unit *unit, int slot, int exp)
{
	if (!unit || slot < 0 || slot >= UNIT_WEAPON_SLOT_COUNT)
		return;

	if (exp < 0)
		exp = 0;

	if (exp > WPN_EXP_S)
		exp = WPN_EXP_S;

	unit->ranks[slot] = exp;
}

int GetUnitWeaponExp(struct Unit *unit, int wtype)
{
	int slot = GetUnitWeaponRankSlot(unit, wtype);

	if (slot < 0)
		return 0;

	return unit->ranks[slot];
}

void SetUnitWeaponExp(struct Unit *unit, int wtype, int exp)
{
	int slot = GetUnitWeaponRankSlot(unit, wtype);

	if (slot < 0)
		return;

	SetUnitWeaponExpBySlot(unit, slot, exp);
}

void InitUnitWeaponRanks(struct Unit *unit, const struct CharacterData *character)
{
	int slot;
	int jid;

	if (!unit || !unit->pClassData)
		return;

	jid = UNIT_CLASS_ID(unit);

	for (slot = 0; slot < UNIT_WEAPON_SLOT_COUNT; slot++) {
		int wtype = GetClassWeaponSlotType(jid, slot);
		int exp;

		if (wtype == WEAPON_SLOT_NONE) {
			unit->ranks[slot] = 0;
			continue;
		}

		exp = unit->pClassData->baseRanks[slot];

		/**
		 * Character baseRanks remain type-indexed for the classic 0-7 set.
		 * Custom types (>= 8 that are rank-bearing) only take class slot bases.
		 */
		if (character && wtype < UNIT_WEAPON_SLOT_COUNT && character->baseRanks[wtype] != 0)
			exp = character->baseRanks[wtype];

		unit->ranks[slot] = exp;
	}
}

void RemapUnitWeaponRanksOnClassChange(struct Unit *unit, const struct ClassData *oldClass, const struct ClassData *newClass, bool zeroUnmapped)
{
	u8 earned[0x100];
	u8 present[0x100];
	int slot;
	int oldJid;
	int newJid;

	if (!unit || !oldClass || !newClass)
		return;

	oldJid = oldClass->number;
	newJid = newClass->number;

	memset(earned, 0, sizeof(earned));
	memset(present, 0, sizeof(present));

	/* Snapshot earned EXP by weapon type using the old mapping. */
	for (slot = 0; slot < UNIT_WEAPON_SLOT_COUNT; slot++) {
		int wtype = GetClassWeaponSlotType(oldJid, slot);
		int exp;

		if (wtype == WEAPON_SLOT_NONE)
			continue;

		exp = unit->ranks[slot] - oldClass->baseRanks[slot];
		if (exp < 0)
			exp = 0;

		earned[wtype] = exp;
		present[wtype] = 1;
	}

	unit->pClassData = newClass;

	for (slot = 0; slot < UNIT_WEAPON_SLOT_COUNT; slot++) {
		int wtype = GetClassWeaponSlotType(newJid, slot);
		int exp;

		if (wtype == WEAPON_SLOT_NONE) {
			unit->ranks[slot] = 0;
			continue;
		}

		/**
		 * Reclass zeroes slots the new class does not actually use
		 * (baseRanks[slot] == 0). Promotion keeps earned EXP even when
		 * the new base is 0, matching vanilla subtract/add behaviour.
		 * In both cases EXP follows weapon type into the new slot map.
		 */
		if (zeroUnmapped && newClass->baseRanks[slot] == 0) {
			unit->ranks[slot] = 0;
			continue;
		}

		exp = newClass->baseRanks[slot];

		if (present[wtype])
			exp += earned[wtype];

		if (exp > WPN_EXP_S)
			exp = WPN_EXP_S;

		unit->ranks[slot] = exp;
	}
}

int ListUnitMappedWeaponTypes(struct Unit *unit, u8 *out, int max)
{
	int slot;
	int count = 0;

	if (!unit || !out || max <= 0)
		return 0;

	for (slot = 0; slot < UNIT_WEAPON_SLOT_COUNT && count < max; slot++) {
		int wtype = GetUnitWeaponSlotType(unit, slot);

		if (wtype == WEAPON_SLOT_NONE)
			continue;

		out[count++] = wtype;
	}

	return count;
}

LYN_REPLACE_CHECK(CanClassWieldWeaponType);
bool CanClassWieldWeaponType(u8 classId, u8 wpnType)
{
	return GetClassWeaponRank(classId, wpnType) != 0 ? TRUE : FALSE;
}

LYN_REPLACE_CHECK(GetUnitBestWRankType);
int GetUnitBestWRankType(struct Unit *unit)
{
	int slot;
	int bestSlot = -1;
	int bestExp = 0;

	if (!unit)
		return -1;

	for (slot = 0; slot < UNIT_WEAPON_SLOT_COUNT; slot++) {
		int wtype = GetUnitWeaponSlotType(unit, slot);
		int exp;

		if (wtype == WEAPON_SLOT_NONE)
			continue;

		/* Match vanilla: ignore staff when picking "best weapon" type. */
		if (wtype == ITYPE_STAFF)
			continue;

		exp = unit->ranks[slot];
		if (exp > bestExp) {
			bestExp = exp;
			bestSlot = slot;
		}
	}

	if (bestSlot < 0)
		return -1;

	return GetUnitWeaponSlotType(unit, bestSlot);
}

void GameInit_ValidateWeaponSlots(void)
{
	const struct ClassWeaponSlotConf *it;
	struct Unit scratch;
	const struct ClassData *jinfo;
	int slot;

	if (!DynamicWeaponSlotsEnabled())
		return;

	if (gpClassWeaponSlotConf) {
		for (it = gpClassWeaponSlotConf; it->jid != 0; it++) {
			u8 seen[0x100];

			memset(seen, 0, sizeof(seen));

			for (slot = 0; slot < UNIT_WEAPON_SLOT_COUNT; slot++) {
				int wtype = it->wtypes[slot];

				if (wtype == WEAPON_SLOT_NONE)
					continue;

				if (!IsRankBearingWeaponType(wtype)) {
					Errorf("WeaponSlots: class 0x%02X slot %d has non-rank type %d", it->jid, slot, wtype);
					continue;
				}

				if (seen[wtype])
					Errorf("WeaponSlots: class 0x%02X duplicates weapon type %d", it->jid, wtype);

				seen[wtype] = 1;
			}
		}
	}

	/**
	 * Identity-mapping smoke checks: existing classes/saves keep slot N
	 * for weapon type N, and auxiliary types never claim a rank slot.
	 */
	jinfo = GetClassData(CLASS_MERCENARY);
	if (!jinfo)
		return;

	for (slot = 0; slot < UNIT_WEAPON_SLOT_COUNT; slot++) {
		if (GetClassWeaponSlotType(CLASS_MERCENARY, slot) != slot) {
			Errorf("WeaponSlots compat: identity map broken for slot %d", slot);
			return;
		}
	}

	memset(&scratch, 0, sizeof(scratch));
	scratch.pClassData = jinfo;
	scratch.pCharacterData = GetCharacterData(CHARACTER_GILLIAM);
	InitUnitWeaponRanks(&scratch, scratch.pCharacterData);

	if (GetUnitWeaponRankSlot(&scratch, ITYPE_SWORD) != ITYPE_SWORD)
		Error("WeaponSlots compat: sword slot lookup failed");

	if (GetUnitWeaponRankSlot(&scratch, ITYPE_KNIFE) >= 0)
		Error("WeaponSlots compat: knife unexpectedly mapped on mercenary");

	if (GetUnitWeaponRankSlot(&scratch, ITYPE_BLLST) >= 0)
		Error("WeaponSlots compat: ballista treated as rank-bearing");
}
