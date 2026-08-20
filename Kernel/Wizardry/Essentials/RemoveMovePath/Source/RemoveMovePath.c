#include "common-chax.h"
#include "kernel-lib.h"
#include "map-movement.h"

/**
 * Lex Talionis "Translucent Unit Sprite":
 * While pathfinding, face the live MU toward the last path step and blit a
 * faded moving-map-sprite ghost at the cursor tip.
 *
 * Blend setup (must satisfy tiles AND semi-transparent OBJ at once):
 *   TargetA = BG2  → range squares blend over the map
 *   TargetB = BG2 + BG3 (no OBJ)
 *     - Range (A) finds BG3 beneath → translucent tiles (vanilla look)
 *     - Ghost OBJ mode-1 finds BG2 beneath → blends with the blue/red
 *       square (visible sprite fade). BG3-only TargetB fails on mGBA when
 *       an opaque range pixel covers the tile, so no 2nd target is found.
 *   OBJ is omitted from TargetB so normal sprites are not pulled into the
 *   alpha pass (avoids the global flicker we hit earlier).
 *
 * Ghost obj list is copied to EWRAM (sPathfindingGhostObjBuf) because
 * PutSpriteExt only stores a pointer and OAM is flushed later — a stack
 * buffer would dangle and corrupt every sprite.
 */
static void ApplyPathfindingBlend(void)
{
	SetBlendConfig(BLEND_EFFECT_ALPHA, 8, 8, 0);
	SetBlendTargetA(0, 0, 1, 0, 0); /* BG2 = move/attack range */
	SetBlendBackdropA(0);
	SetBlendTargetB(0, 0, 1, 1, 0); /* BG2 + BG3; not OBJ */
	SetBlendBackdropB(1);
}

static void DisplayBlendedMuAp(struct APHandle *ap, int x, int y)
{
	const u16 *src;
	int count;
	int i;

	src = ap->pCurrentObjData;
	if (!src)
		return;

	count = src[0];
	if (count <= 0 || count > 8)
		return;

	sPathfindingGhostObjBuf[0] = count;
	for (i = 0; i < count; i++) {
		const u16 *s = src + 1 + i * 3;
		u16 *d = sPathfindingGhostObjBuf + 1 + i * 3;

		/* Force semi-transparent OBJ mode in ATTR0 */
		d[0] = (s[0] & ~0x0C00) | OAM0_BLEND;
		d[1] = s[1];
		d[2] = s[2];
	}

	PutSpriteExt(
		ap->objLayer,
		OAM1_X(x),
		OAM0_Y(y) | OAM0_BLEND,
		sPathfindingGhostObjBuf,
		ap->tileBase);
}

static void DrawPathfindingUnitGhost(void)
{
	struct MuProc *mu;
	s8 pathLen;
	s8 facing;
	s8 dx, dy;
	int x, y;

	if (!gpKernelDesignerConfig->translucent_unit_sprite)
		return;

	if (!gActiveUnit || !MuExists())
		return;

	mu = GetUnitMu(gActiveUnit);
	if (!mu || !mu->sprite_anim || mu->hidden_b)
		return;

	if (mu->facing == MU_FACING_STANDING)
		return;

	pathLen = gpPathArrowProc->pathLen;

	/* Keep range + ghost blend coherent for the whole pathfinding state */
	ApplyPathfindingBlend();

	/* At the origin tile: restore the selected bounce, no ghost */
	if (pathLen < 1) {
		if (mu->facing != MU_FACING_SELECTED)
			SetMuFacing(mu, MU_FACING_SELECTED);
		return;
	}

	/* Only while the cursor sits on the path tip (LT draw_arrows behaviour) */
	if (gBmSt.playerCursor.x != gpPathArrowProc->pathX[pathLen] ||
	    gBmSt.playerCursor.y != gpPathArrowProc->pathY[pathLen])
		return;

	/* Last path step == last d-pad direction that extended the arrow */
	dx = gpPathArrowProc->pathX[pathLen] - gpPathArrowProc->pathX[pathLen - 1];
	dy = gpPathArrowProc->pathY[pathLen] - gpPathArrowProc->pathY[pathLen - 1];

	if (dx > 0)
		facing = MU_FACING_RIGHT;
	else if (dx < 0)
		facing = MU_FACING_LEFT;
	else if (dy < 0)
		facing = MU_FACING_UP;
	else
		facing = MU_FACING_DOWN;

	if (mu->facing != facing)
		SetMuFacing(mu, facing);

	/* Same screen-space origin convention as GetMuDisplayPosition */
	x = gBmSt.playerCursor.x * 16 - gBmSt.camera.x + 8;
	y = gBmSt.playerCursor.y * 16 - gBmSt.camera.y + 16;

	if (x < -16 || x > DISPLAY_WIDTH + 16)
		return;

	if (y < -32 || y > DISPLAY_HEIGHT + 32)
		return;

	DisplayBlendedMuAp(mu->sprite_anim, x, y);
}

LYN_REPLACE_CHECK(PlayerPhase_DisplayUnitMovement);
void PlayerPhase_DisplayUnitMovement(void)
{
	if (gpKernelDesignerConfig->remove_move_path == false)
		GetMovementScriptFromPath();
	else
		GenerateBestMovementScript(
			gBmSt.playerCursor.x,
			gBmSt.playerCursor.y,
			gWorkingMovementScript);

	UnitApplyWorkingMovementScript(gActiveUnit, gActiveUnit->xPos, gActiveUnit->yPos);
	SetAutoMuMoveScript(gWorkingMovementScript);
}

LYN_REPLACE_CHECK(DrawUpdatedPathArrow);
void DrawUpdatedPathArrow(void)
{
	if (gpKernelDesignerConfig->remove_move_path == false) {
		UpdatePathArrowWithCursor();
		DrawPathArrow();
		DrawPathfindingUnitGhost();
	}
}
