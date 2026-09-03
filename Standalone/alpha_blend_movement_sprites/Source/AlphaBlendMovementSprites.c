#include "global.h"
#include "variables.h"
#include "hardware.h"
#include "ctc.h"
#include "mu.h"
#include "ap.h"
#include "bmpatharrowdisp.h"
#include "gba/defines.h"

extern u16 sPathfindingGhostObjBuf[];

/**
 * Lex Talionis-style translucent MU ghost at the pathfinding cursor tip.
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

	/* Only while the cursor sits on the path tip */
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

	x = gBmSt.playerCursor.x * 16 - gBmSt.camera.x + 8;
	y = gBmSt.playerCursor.y * 16 - gBmSt.camera.y + 16;

	if (x < -16 || x > DISPLAY_WIDTH + 16)
		return;

	if (y < -32 || y > DISPLAY_HEIGHT + 32)
		return;

	DisplayBlendedMuAp(mu->sprite_anim, x, y);
}

void DrawUpdatedPathArrow_AlphaBlendMovementSprites(void)
{
	UpdatePathArrowWithCursor();
	DrawPathArrow();
	DrawPathfindingUnitGhost();
}
