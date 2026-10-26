#include "global.h"
#include "bmidoten.h"
#include "bmunit.h"
#include "mu.h"
#include "variables.h"

/**
 * Remove move path:
 * The path arrow is never drawn (DrawUpdatedPathArrow is turned into a
 * no-op by Installer.event), so the unit cannot follow the arrow the
 * player traced. Instead it walks the best (shortest) route to the tile
 * under the cursor, like AI units do.
 *
 * Vanilla PlayerPhase_DisplayUnitMovement calls GetMovementScriptFromPath()
 * here; everything else is unchanged.
 */
void PlayerPhase_DisplayUnitMovement_RemoveMovePath(void)
{
	GenerateBestMovementScript(
		gBmSt.playerCursor.x,
		gBmSt.playerCursor.y,
		gWorkingMovementScript);

	UnitApplyWorkingMovementScript(gActiveUnit, gActiveUnit->xPos, gActiveUnit->yPos);
	SetAutoMuMoveScript(gWorkingMovementScript);
}
