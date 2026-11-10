#include "common-chax.h"
#include "kernel-lib.h"
#include "mu.h"
#include "big-unit.h"

/*
 * 2x2 units on the map: the block is drawn once, the map sprite of its shown corner (the first one standing) at twice
 * its size with an OBJ affine matrix, bottom-centred on the block. A 32x32 map sprite already fills the block and is
 * drawn as it is. The block shows only while every corner's sprite does, so a corner walking (its MU) or hidden by an
 * event hides it.
 */

#define BIG_AFFINE 26 /* 26: facing left, 27: mirrored */

extern int GetDangerBonesShakeOffset(int shaking);
extern const u16 gObject_32x32_HFlipped[];

int BigUnit_SmsTag(struct Unit *unit)
{
	struct BigUnitGroup *group;
	int corner, c;

	group = BigUnit_GetGroup(unit, &corner);
	if (!group)
		return 0;

	for (c = 0; c < corner; c++)
		if (BigUnit_GetCorner(group, c))
			return BIG_SMS_CORNER | ((group - gBigUnitGroups) << 4);

	return BIG_SMS_CORNER | BIG_SMS_SHOWN | ((group - gBigUnitGroups) << 4);
}

bool BigUnit_HandleShown(struct SMSHandle *handle)
{
	struct BigUnitGroup *group = &gBigUnitGroups[BIG_SMS_GROUP(handle->_u0A)];
	struct Unit *unit;
	int c;

	for (c = 0; c < BIG_CORNERS; c++) {
		unit = BigUnit_GetCorner(group, c);
		/* A corner at 0 HP is breaking: the rest of the block stays */
		if (!unit || unit->curHP == 0)
			continue;
		if (!unit->pMapSpriteHandle || (unit->pMapSpriteHandle->config & 0x80))
			return false;
	}
	return true;
}

void BigUnit_PutBlockSprite(struct SMSHandle *handle)
{
	struct BigUnitGroup *group = &gBigUnitGroups[BIG_SMS_GROUP(handle->_u0A)];
	int x, y, size, layer, flip, shake = 0;

	if (!BigUnit_HandleShown(handle))
		return;

	x = group->x * 16 - gBmSt.camera.x;
	y = group->y * 16 - gBmSt.camera.y;
	if (x < -32 || x > DISPLAY_WIDTH || y < -32 || y > DISPLAY_HEIGHT + 32)
		return;

	x += TextEngine_GetStaticOffsetAtY(y);
	y += TextEngine_GetFxOffsetY();

	if (handle->config & 0x40)
		shake = GetDangerBonesShakeOffset(true);
	x += shake;

	size = handle->config & 0xF;
	layer = OAM2_LAYER(size >= 3 ? 3 : 2);
	if (size >= 3)
		size -= 3;
	flip = handle->_u0A & 1;

	if (size == 2) {
		/* 32x32: already the block's size */
		CallARM_PushToSecondaryOAM(OAM1_X(x + 0x200), OAM0_Y(0x100 + y),
					   flip ? gObject_32x32_HFlipped : gObject_32x32, handle->oam2Base + layer);
		return;
	}

	SetObjAffine(BIG_AFFINE + flip, flip ? -0x80 : 0x80, 0, 0, 0x80);

	/* Double size: a 16x16 fills a 32x32 box, a 16x32 a 32x64 one; the box's bottom is the block's */
	if (size == 0)
		CallARM_PushToSecondaryOAM(OAM1_X(x + 0x200) | OAM1_AFFINE_ID(BIG_AFFINE + flip),
					   OAM0_Y(0x100 + y) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE,
					   gObject_16x16, handle->oam2Base + layer);
	else
		CallARM_PushToSecondaryOAM(OAM1_X(x + 0x200) | OAM1_AFFINE_ID(BIG_AFFINE + flip),
					   OAM0_Y(0x100 + y - 32) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE,
					   gObject_16x32, handle->oam2Base + layer);
}

/* OBJ sizes in pixels, by shape (attribute 0) and size (attribute 1) */
static const u8 sObjWidth[3][4] = { { 8, 16, 32, 64 }, { 16, 32, 32, 64 }, { 8, 8, 16, 32 } };
static const u8 sObjHeight[3][4] = { { 8, 16, 32, 64 }, { 8, 8, 16, 32 }, { 16, 32, 32, 64 } };

/* The objects of an MU frame, twice the size and twice as far from (x, y), the block's bottom centre */
static void BigUnit_PutMuFrame(struct APHandle *ap, int x, int y, int oam0, bool ghost)
{
	const u16 *obj = ap->pCurrentObjData;
	u16 data[4];
	int n, a0, a1, shape, size, flip, ox, oy, ix, iy;

	if (!obj)
		return;

	for (n = *obj++; n > 0; n--, obj += 3) {
		a0 = obj[0];
		a1 = obj[1];
		shape = (a0 >> 14) & 3;
		size = (a1 >> 14) & 3;
		if (shape == 3)
			continue;

		flip = (a1 >> 12) & 1;
		ox = ((s16)(a1 << 7)) >> 7;
		oy = (s8)(a0 & 0xFF);
		ix = x + 2 * ox;
		iy = y + 2 * oy;
		if (ix + 2 * sObjWidth[shape][size] < 0 || ix > DISPLAY_WIDTH ||
		    iy + 2 * sObjHeight[shape][size] < 0 || iy > DISPLAY_HEIGHT)
			continue;

		data[0] = 1;
		/* A ghost is a semi-transparent OBJ */
		data[1] = ghost ? ((a0 & 0xF000) | OAM0_BLEND) : (a0 & 0xFC00);
		data[2] = a1 & 0xC000;
		data[3] = obj[2];
		CallARM_PushToSecondaryOAM(OAM1_X(ix + 0x200) | OAM1_AFFINE_ID(BIG_AFFINE + flip),
					   OAM0_Y(iy + 0x100) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE | oam0, data, ap->tileBase);
	}
}

/*
 * The translucent unit sprite (alpha_blend_movement_sprites) for a 2x2 enemy: while the AI walks it, its frame is
 * also drawn faded on the block it walks to. The faded OBJ blends with the map (BG2 and BG3) at 8/8; no BG is a first
 * target, so nothing else on the map changes.
 */
static void BigUnit_PutMuGhost(struct MuProc *proc, int corner, int x, int y)
{
	struct Unit *unit = proc->unit;
	int gx, gy;

	if (!gpKernelDesignerConfig->alpha_blend_movement_sprites || proc->state != MU_STATE_MOVEMENT)
		return;
	if (gPlaySt.faction == FACTION_BLUE || unit != gActiveUnit || gAiDecision.unitId != (u8)unit->index)
		return;

	gx = gAiDecision.xMove * 16 - gBmSt.camera.x + 16 - 16 * gBigUnitCornerOffsets[corner][0];
	gy = gAiDecision.yMove * 16 - gBmSt.camera.y + 32 - 16 * gBigUnitCornerOffsets[corner][1];

	/* There already */
	if (gx == x && gy == y)
		return;

	SetBlendConfig(BLEND_EFFECT_ALPHA, 8, 8, 0);
	SetBlendTargetA(0, 0, 0, 0, 0);
	SetBlendTargetB(0, 0, 1, 1, 0);
	SetBlendBackdropB(1);

	BigUnit_PutMuFrame(proc->sprite_anim, gx, gy, 0, true);
}

/*
 * A corner's MU (MU_DisplayAsMMS, at x, y: the bottom centre of its tile) drawn as the whole block: the animation runs
 * as usual off screen, and each object of its frame is drawn again, twice the size and twice as far from the block's
 * bottom centre, with an OBJ affine matrix (mirrored for a mirrored object). oam0 carries the death fade's blend bit.
 */
bool BigUnit_PutMu(struct MuProc *proc, int x, int y, int oam0)
{
	struct APHandle *ap = proc->sprite_anim;
	int corner;

	if (!proc->unit || !ap || !BigUnit_GetGroup(proc->unit, &corner))
		return false;

	/* A breaking corner just goes: the block stays drawn by its other corners */
	if (proc->state == MU_STATE_DEATHFADE)
		return true;

	/* Runs the animation (and loads its frame's tiles), drawn below the screen */
	AP_Update(ap, OAM1_X(0), OAM0_Y(0xD0));

	x += 8 - 16 * gBigUnitCornerOffsets[corner][0];
	y += 16 - 16 * gBigUnitCornerOffsets[corner][1];

	SetObjAffine(BIG_AFFINE, 0x80, 0, 0, 0x80);
	SetObjAffine(BIG_AFFINE + 1, -0x80, 0, 0, 0x80);

	BigUnit_PutMuFrame(ap, x, y, oam0, false);
	BigUnit_PutMuGhost(proc, corner, x, y);
	return true;
}
