#include "global.h"
#include "bmunit.h"

/**
 * Vanilla UnitDrop, except that a dropped unit of the active faction is only
 * greyed out if it is in the middle of a Canto move (US_HAS_MOVED). A unit
 * that had not acted yet before it was rescued can act again after the drop;
 * a unit that had already acted keeps its own grey (US_UNSELECTABLE) state.
 */
void UnitDrop_MoveAgain(struct Unit *actor, int xTarget, int yTarget)
{
	struct Unit *target = GetUnit(actor->rescue);

	actor->state = actor->state & ~(US_RESCUING | US_RESCUED);
	target->state = target->state & ~(US_RESCUING | US_RESCUED | US_HIDDEN);

	if (target->state & US_HAS_MOVED) {
		if (UNIT_FACTION(target) == gPlaySt.faction)
			target->state |= US_UNSELECTABLE;
	}

	actor->rescue = 0;
	target->rescue = 0;

	target->xPos = xTarget;
	target->yPos = yTarget;
}
