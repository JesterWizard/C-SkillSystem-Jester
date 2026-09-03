#include "global.h"
#include "bmunit.h"
#include "variables.h"

/**
 * Vanilla UnitDrop with Death Dance enabled:
 * rescued allies stay selectable when the rescuer is dead (curHP == 0).
 */
void UnitDrop_DeathDance(struct Unit* actor, int xTarget, int yTarget)
{
	struct Unit* target = GetUnit(actor->rescue);

	actor->state = actor->state & ~(US_RESCUING | US_RESCUED);
	target->state = target->state & ~(US_RESCUING | US_RESCUED | US_HIDDEN);

	if (UNIT_FACTION(target) == gPlaySt.faction && actor->curHP != 0)
		target->state |= US_UNSELECTABLE;

	actor->rescue = 0;
	target->rescue = 0;

	target->xPos = xTarget;
	target->yPos = yTarget;
}
