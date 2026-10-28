#include "common-chax.h"
#include "kernel-lib.h"
#include "kernel/traps.h"

/*
 * Hidden caltrops (TRAP_HIDDEN_CALTROPS): an invisible trap. A grounded unit
 * whose move crosses it stops on it with the fog-ambush "!" and its turn ends
 * (UNIT_ACTION_TRAPPED, as for a fog ambush). PostAction_HiddenCaltrops then
 * reveals it. A revealed trap is drawn on the map (RefreshUnitSprites) and
 * still stops grounded units, without the "!".
 */

/* Vanilla (FE8U 0x08039FAC); struct CpPerformProc is in vanilla.h */
void CpPerform_BeginUnitMovement(struct CpPerformProc *proc);

struct Trap *AddHiddenCaltrops(int x, int y)
{
	struct Trap *trap = AddTrap(x, y, TRAP_HIDDEN_CALTROPS, 0);

	trap->data[TRAP_EXTDATA_HIDDEN_CALTROPS_REVEALED] = false;
	return trap;
}

/*
 * Cuts gWorkingMovementScript on the first caltrops tile the move enters.
 * Called right after UnitApplyWorkingMovementScript, which has already cut
 * the script for fog ambushes, mines and spin tiles.
 */
void CutMovementScriptAtCaltrops(struct Unit *unit)
{
	u8 *it = gWorkingMovementScript;
	int x = unit->xPos;
	int y = unit->yPos;
	struct Trap *trap;

	if (UNIT_CATTRIBUTES(unit) & CA_FLYER)
		return;

	for (;; it++) {
		switch (*it) {
		case MOVE_CMD_MOVE_UP:
			y--;
			break;

		case MOVE_CMD_MOVE_DOWN:
			y++;
			break;

		case MOVE_CMD_MOVE_LEFT:
			x--;
			break;

		case MOVE_CMD_MOVE_RIGHT:
			x++;
			break;

		default:
			return;
		}

		/* Passing through an ally standing on it */
		if (gBmMapUnit[y][x])
			continue;

		trap = GetTypedTrapAt(x, y, TRAP_HIDDEN_CALTROPS);
		if (!trap)
			continue;

		if (!trap->data[TRAP_EXTDATA_HIDDEN_CALTROPS_REVEALED])
			*++it = MOVE_CMD_BUMP;

		*++it = MOVE_CMD_HALT;

		gActionData.unitActionType = UNIT_ACTION_TRAPPED;
		gActionData.xMove = x;
		gActionData.yMove = y;
		return;
	}
}

/* Vanilla, plus the caltrops cut, so AI units are caught too */
LYN_REPLACE_CHECK(CpPerform_BeginUnitMovement);
void CpPerform_BeginUnitMovement(struct CpPerformProc *proc)
{
	UnitBeginAction(gActiveUnit);

	HideUnitSprite(gActiveUnit);

	GenerateUnitMovementMap(gActiveUnit);
	SetWorkingBmMap(gBmMapMovement);

	GenerateBestMovementScript(gAiDecision.xMove, gAiDecision.yMove, gWorkingMovementScript);

	UnitApplyWorkingMovementScript(gActiveUnit, gActiveUnit->xPos, gActiveUnit->yPos);
	CutMovementScriptAtCaltrops(gActiveUnit);

	gAiDecision.xMove = gActionData.xMove;
	gAiDecision.yMove = gActionData.yMove;

	if (proc->isUnitVisible) {
		StartMu(gActiveUnit);
		SetAutoMuDefaultFacing();
		SetAutoMuMoveScript(gWorkingMovementScript);
	}
}

/* Reveals hidden caltrops under a grounded unit that ends its action on them */
bool PostAction_HiddenCaltrops(ProcPtr proc)
{
	struct Trap *trap;

	if (!UNIT_IS_VALID(gActiveUnit) || (UNIT_CATTRIBUTES(gActiveUnit) & CA_FLYER))
		return false;

	trap = GetTypedTrapAt(gActiveUnit->xPos, gActiveUnit->yPos, TRAP_HIDDEN_CALTROPS);
	if (!trap || trap->data[TRAP_EXTDATA_HIDDEN_CALTROPS_REVEALED])
		return false;

	trap->data[TRAP_EXTDATA_HIDDEN_CALTROPS_REVEALED] = true;
	RefreshUnitSprites();
	return false;
}
