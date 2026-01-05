#include "common-chax.h"
#include "skill-system.h"
#include "status-getter.h"
#include "weapon-range.h"
#include "kernel-lib.h"
#include "gaiden-magic.h"
#include "constants/skills.h"
#include "playst-expa.h"

#define LOCAL_TRACE 0

typedef int (*WeaponRangeGetterFunc_t)(int old, struct Unit *unit, u16 item);
// extern const WeaponRangeGetterFunc_t gWeaponRangeGetters[];
extern WeaponRangeGetterFunc_t const *const gpWeaponRangeGetters;

int GetItemMinRangeRework(u16 item, struct Unit *unit)
{
	int status = GetItemMinRange(item);

	if (IsUnitStruct(unit)) {
#if defined(SID_PointBlank) && (COMMON_SKILL_VALID(SID_PointBlank))
		if (SkillTester(unit, SID_PointBlank) && GetItemType(item) == ITYPE_BOW)
			status = 1;
#endif
	}
	return status;
}

int GetItemMaxRangeRework(u16 item, struct Unit *unit)
{
	const WeaponRangeGetterFunc_t *it;
	int status = GetItemMaxRange(item);

	if (status == 0)
		status = GetUnitMagBy2Range(unit);

	if (IsUnitStruct(unit)) {
		for (it = gpWeaponRangeGetters; *it; it++)
			status = (*it)(status, unit, item);
	}

	return status;
}

int WeaponRangeGetterSkills(int range, struct Unit *unit, u16 item)
{

#if defined(SID_Counterattack) && (COMMON_SKILL_VALID(SID_Counterattack))
    if (SkillTester(unit, SID_Counterattack))
        range = (gBmMapSize.x > gBmMapSize.y) ? gBmMapSize.x : gBmMapSize.y;
#endif

#if defined(SID_Thrust) && (COMMON_SKILL_VALID(SID_Thrust))
    if (SkillTester(unit, SID_Thrust) && PlayStExpa_CheckBit(PLAYSTEXPA_BIT_Thrust_InForce) && !PlayStExpa_CheckBit(PLAYSTEXPA_BIT_Thrust_Used))
        range = range < SKILL_EFF0(SID_Thrust) ? SKILL_EFF0(SID_Thrust)  : range;
#endif

	switch (GetItemType(item)) {
	case ITYPE_STAFF:
#if defined(SID_StaffSavant) && (COMMON_SKILL_VALID(SID_StaffSavant))
		if (SkillTester(unit, SID_StaffSavant))
			range = range + SKILL_EFF0(SID_StaffSavant);
#endif
		break;

    case ITYPE_SWORD:
#if defined(SID_SwordSaint) && (COMMON_SKILL_VALID(SID_SwordSaint))
        if (SkillTester(unit, SID_SwordSaint) && range < 2)
            range = range + 1;
#endif
        break;

	case ITYPE_BOW:
#if defined(SID_BowRangePlus1) && (COMMON_SKILL_VALID(SID_BowRangePlus1))
		if (SkillTester(unit, SID_BowRangePlus1))
			range = range + SKILL_EFF0(SID_BowRangePlus1);
#endif

#if defined(SID_BowRangePlus2) && (COMMON_SKILL_VALID(SID_BowRangePlus2))
		if (SkillTester(unit, SID_BowRangePlus2))
			range = range + SKILL_EFF0(SID_BowRangePlus2);
#endif

#if defined(SID_RunningStart) && (COMMON_SKILL_VALID(SID_RunningStart))
		if (SkillTester(unit, SID_RunningStart))
			range = range + gActionData.moveCount / 2;
#endif

#if defined(SID_HighGround) && (COMMON_SKILL_VALID(SID_HighGround))
		if (SkillTester(unit, SID_HighGround) && gBmMapTerrain[unit->yPos][unit->xPos] == TERRAIN_FORT)
			range = range + SKILL_EFF0(SID_HighGround);
#endif
		break;

	case ITYPE_ANIMA:
	case ITYPE_LIGHT:
	case ITYPE_DARK:
#if defined(SID_MagicRangePlus1) && (COMMON_SKILL_VALID(SID_MagicRangePlus1))
		if (SkillTester(unit, SID_MagicRangePlus1))
			range = range + SKILL_EFF0(SID_MagicRangePlus1);
#endif

#if defined(SID_MagicRangePlus2) && (COMMON_SKILL_VALID(SID_MagicRangePlus2))
		if (SkillTester(unit, SID_MagicRangePlus2))
			range = range + SKILL_EFF0(SID_MagicRangePlus2);
#endif

#if defined(SID_HighGround) && (COMMON_SKILL_VALID(SID_HighGround))
		if (SkillTester(unit, SID_HighGround) && gBmMapTerrain[unit->yPos][unit->xPos] == TERRAIN_FORT)
			range = range + SKILL_EFF0(SID_HighGround);
#endif
		break;
	}
	return range;
}

int GetUnitMinRange(struct Unit *unit)
{
	int i, item, _ret, ret = 1;

	for (i = 0; i < UNIT_ITEM_COUNT; i++) {
		item = unit->items[i];

		if (ITEM_INDEX(item) > 0 && CanUnitUseWeapon(unit, item)) {
			_ret = GetItemMinRangeRework(item, unit);

			LTRACEF("item=%#x, rng=%d, min=%d", item, _ret, ret);

			if (_ret < ret)
				ret = _ret;
		}
	}

	if (gpKernelDesignerConfig->gaiden_magic_en) {
		struct GaidenMagicList *list = GetGaidenMagicList(unit);

		for (i = 0; i < GAIDEN_MAGIC_LIST_LEN; i++) {
			item = list->bmags[i];

			if (item == ITEM_NONE)
				break;

			if (CanUnitUseGaidenMagic(unit, item) && GetItemAttributes(item) & IA_WEAPON) {
				_ret = GetItemMinRangeRework(item, unit);
				if (_ret < ret)
					ret = _ret;
			}
		}

		for (i = 0; i < GAIDEN_MAGIC_LIST_LEN; i++) {
			item = list->wmags[i];

			if (item == ITEM_NONE)
				break;

			if (CanUnitUseGaidenMagic(unit, item) && GetItemAttributes(item) & IA_WEAPON) {
				_ret = GetItemMinRangeRework(item, unit);
				if (_ret < ret)
					ret = _ret;
			}
		}
	}
	return ret;
}

int GetUnitMaxRange(struct Unit *unit)
{
	int i, item, _ret, ret = 1;

	for (i = 0; i < UNIT_ITEM_COUNT; i++) {
		item = unit->items[i];

		if (ITEM_INDEX(item) > 0 && CanUnitUseWeapon(unit, item)) {
			_ret = GetItemMaxRangeRework(item, unit);

			LTRACEF("item=%#x, rng=%d, max=%d", item, _ret, ret);

			if (_ret > ret)
				ret = _ret;
		}
	}

	if (gpKernelDesignerConfig->gaiden_magic_en) {
		struct GaidenMagicList *list = GetGaidenMagicList(unit);

		for (i = 0; i < GAIDEN_MAGIC_LIST_LEN; i++) {
			item = list->bmags[i];

			if (item == ITEM_NONE)
				break;

			if (CanUnitUseGaidenMagic(unit, item) && GetItemAttributes(item) & IA_WEAPON) {
				_ret = GetItemMaxRangeRework(item, unit);
				if (_ret > ret)
					ret = _ret;
			}
		}

		for (i = 0; i < GAIDEN_MAGIC_LIST_LEN; i++) {
			item = list->wmags[i];

			if (item == ITEM_NONE)
				break;

			if (CanUnitUseGaidenMagic(unit, item) && GetItemAttributes(item) & IA_WEAPON) {
				_ret = GetItemMaxRangeRework(item, unit);
				if (_ret > ret)
					ret = _ret;
			}
		}
	}
	return ret;
}
