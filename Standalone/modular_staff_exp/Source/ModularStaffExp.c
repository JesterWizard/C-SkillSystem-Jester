#include "global.h"
#include "bmitem.h"
#include "bmunit.h"
#include "bmbattle.h"

/*
 * Pairs of (staff item ID, EXP), ending with a pair whose item is 0.
 * Defined in Installer.event (edit there, no recompile).
 */
struct StaffExpEntry {
	u8 item;
	u8 exp;
};

extern const struct StaffExpEntry ModularStaffExpTable[];

/**
 * Replaces vanilla GetBattleUnitStaffExp with the C Skill System
 * modular_staff_exp rule: a staff listed in the table always gives its
 * fixed EXP (no promoted halving, no 1 EXP on a miss). A staff that is not
 * listed keeps the vanilla formula. The result is capped at 100.
 */
int GetBattleUnitStaffExp_Modular(struct BattleUnit *bu)
{
	const struct StaffExpEntry *it;
	int item = ITEM_INDEX(bu->weapon);
	int result;

	if (!CanBattleUnitGainLevels(bu))
		return 0;

	for (it = ModularStaffExpTable; it->item != 0; it++) {
		if (it->item == item) {
			result = it->exp;

			if (result > 100)
				result = 100;

			return result;
		}
	}

	/* Vanilla formula for staves missing from the table */
	if (gBattleHitArray[0].attributes & BATTLE_HIT_ATTR_MISS)
		return 1;

	result = 10 + GetItemCostPerUse(bu->weapon) / 20;

	if (UNIT_CATTRIBUTES(&bu->unit) & CA_PROMOTED)
		result = result / 2;

	if (result > 100)
		result = 100;

	return result;
}
