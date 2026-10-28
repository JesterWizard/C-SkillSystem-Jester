#include "common-chax.h"
#include "kernel-lib.h"
#include "map-anims.h"
#include "map-movement.h"
#include "kernel/traps.h"

/*
 * Collapsing floor (TRAP_COLLAPSING_FLOOR), a weak floor tile:
 * - Intact: the first grounded unit to end its action on it cracks it.
 * - Cracked: a grounded unit whose move enters it stops there
 *   (CutMovementScriptAtCaltrops) and falls through to its death. A unit
 *   already standing on it when it cracked is safe until it moves off.
 * - Hole: what is left. Grounded units cannot enter it.
 * The AI keeps off cracked floors. Fliers ignore the trap in every state.
 */

#define SE_FLOOR_CRACK 0xBD

struct Trap *AddCollapsingFloor(int x, int y)
{
	struct Trap *trap = AddTrap(x, y, TRAP_COLLAPSING_FLOOR, 0);

	trap->data[TRAP_EXTDATA_COLLAPSING_FLOOR_STATE] = COLLAPSING_FLOOR_INTACT;
	return trap;
}

/* A grounded unit that enters a cracked floor stops on it */
bool CollapsingFloorStopsMoveAt(int x, int y)
{
	struct Trap *trap = GetTypedTrapAt(x, y, TRAP_COLLAPSING_FLOOR);

	return trap && trap->data[TRAP_EXTDATA_COLLAPSING_FLOOR_STATE] == COLLAPSING_FLOOR_CRACKED;
}

/* Called from PreGenerateMovementMap */
void MarkCollapsingFloorBarriers(struct Unit *unit)
{
	struct Trap *trap;
	int state;

	if (UNIT_CATTRIBUTES(unit) & CA_FLYER)
		return;

	for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap) {
		if (trap->type != TRAP_COLLAPSING_FLOOR || !IsPositionValid(trap->xPos, trap->yPos))
			continue;

		state = trap->data[TRAP_EXTDATA_COLLAPSING_FLOOR_STATE];

		if (state == COLLAPSING_FLOOR_HOLE ||
		    (state == COLLAPSING_FLOOR_CRACKED && UNIT_FACTION(unit) != FACTION_BLUE)) {
			KernelExtMoveBarrierMap[trap->yPos][trap->xPos] = -1;
			KernelMoveMapFlags |= FMOVSTRE_BARRIER;
		}
	}
}

static void CollapsingFloor_FallCallback(ProcPtr proc)
{
	UnitKill(gActiveUnit);
	RefreshUnitSprites();
}

/* Cracks an intact floor, or drops the unit through a cracked one */
bool PostAction_CollapsingFloor(ProcPtr proc)
{
	struct Trap *trap;

	if (!UNIT_IS_VALID(gActiveUnit) || gActiveUnit->curHP <= 0)
		return false;

	if (UNIT_CATTRIBUTES(gActiveUnit) & CA_FLYER)
		return false;

	trap = GetTypedTrapAt(gActiveUnit->xPos, gActiveUnit->yPos, TRAP_COLLAPSING_FLOOR);
	if (!trap)
		return false;

	switch (trap->data[TRAP_EXTDATA_COLLAPSING_FLOOR_STATE]) {
	case COLLAPSING_FLOOR_INTACT:
		trap->data[TRAP_EXTDATA_COLLAPSING_FLOOR_STATE] = COLLAPSING_FLOOR_CRACKED;
		PlaySoundEffect(SE_FLOOR_CRACK);
		RefreshUnitSprites();
		return false;

	case COLLAPSING_FLOOR_CRACKED:
		/* The unit that cracked it may act from it later without falling */
		if (gActiveUnitMoveOrigin.x == gActiveUnit->xPos && gActiveUnitMoveOrigin.y == gActiveUnit->yPos)
			return false;

		trap->data[TRAP_EXTDATA_COLLAPSING_FLOOR_STATE] = COLLAPSING_FLOOR_HOLE;
		PlaySoundEffect(SE_FLOOR_CRACK);
		RefreshUnitSprites();
		CallMapAnim_HurtExt(proc, gActiveUnit, gActiveUnit->curHP, NULL, CollapsingFloor_FallCallback);
		return true;
	}

	return false;
}
