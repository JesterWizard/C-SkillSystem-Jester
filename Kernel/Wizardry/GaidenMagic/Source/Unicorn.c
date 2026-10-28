/*
 * Unicorn (W.Mag): restores an ally's MP by what a Mend staff would heal and
 * cures its status, for 30 MP.
 */

#include <common-chax.h>
#include <kernel-lib.h>
#include <item-sys.h>
#include <weapon-range.h>
#include <gaiden-magic.h>
#include "bwl.h"
#include "jester_headers/custom-functions.h"

static void TryAddUnitToUnicornTargetList(struct Unit *unit)
{
	if (!AreUnitsAllied(gSubjectUnit->index, unit->index))
		return;

	if (unit->state & US_RESCUED)
		return;

	if (unit->statusIndex != UNIT_STATUS_NONE ||
	    (CheckHasBwl(UNIT_CHAR_ID(unit)) && GetUnitCurrentMP(unit) < GetUnitMaxMP(unit)))
		AddTarget(unit->xPos, unit->yPos, unit->index, 0);
}

void MakeTargetListForUnicorn(struct Unit *unit)
{
	gSubjectUnit = unit;
	InitTargets(unit->xPos, unit->yPos);

	BmMapFill(gBmMapRange, 0);
	AddMapForItem(unit, ITEM_STAFF_UNICORN);
	ForEachUnit(TryAddUnitToUnicornTargetList, gBmMapRange, 0);
}

bool IER_Usability_Unicorn(struct Unit *unit, int item)
{
	return HasSelectTarget(unit, MakeTargetListForUnicorn);
}

void IER_Effect_Unicorn(struct Unit *unit, int item)
{
	DoUseRestoreStaff(unit, MakeTargetListForUnicorn);
}

void IER_Action_Unicorn(ProcPtr proc, struct Unit *unit, int item)
{
	struct Unit *target = GetUnit(gActionData.targetIndex);

	/*
	 * Pay the cost before the restore, so that a self-target ends on
	 * (current - cost + heal), capped at max.
	 */
	SpendGaidenMagicMp(unit, ITEM_STAFF_UNICORN);

	if (target && CheckHasBwl(UNIT_CHAR_ID(target))) {
		struct NewBwl *bwl = GetNewBwl(UNIT_CHAR_ID(target));
		int maxMP = GetUnitMaxMP(target);
		int mp = GetUnitCurrentMP(target) + GetUnitItemHealAmount(unit, MakeNewItem(ITEM_STAFF_MEND));

		if (mp > maxMP)
			mp = maxMP;

		bwl->currentMP = mp;
	}

	/* ExecRestore cures the target's status and plays the staff animation */
	ExecRestore(proc);
}
