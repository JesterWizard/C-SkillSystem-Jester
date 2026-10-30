#include "gbafe.h"
#include "TextEngine.h"

/*
 * No half-body or 32-colour portraits, installed when Installer.event leaves
 * TEXT_ENGINE_HALF_BODY_PORTRAITS undefined.  The text engine calls these at
 * the same places it calls Standalone/portrait_formats' Source/Portraits.c.
 */

/*
 * Replaces Face_OnIdle (0x080055BC): vanilla, but drawn through
 * TextEngine_PutFaceSprite so the screen tear, static and earthquake move the
 * portrait along with its mouth and eyes, then its exceed pieces.
 */
void Face_OnIdle_Portraits(struct FaceProc *proc)
{
	int oam0;

	if (GetFaceDisplayBits(proc) & FACE_DISP_HIDDEN)
		return;

	oam0 = (GetFaceDisplayBits(proc) & FACE_DISP_BLEND) ? OAM0_BLEND : 0;

	TextEngine_PutFaceSprite(proc->spriteLayer, 0x1FF & proc->xPos, oam0 + OAM0_Y(proc->yPos), proc->sprite, proc->oam2);
	ExceedPortrait_PutFaceSprites(proc, oam0);
}

bool Portraits_IsHalfBodyFace(struct FaceProc *face)
{
	(void)face;
	return false;
}

bool Portraits_IsHalfBodyData(const struct FaceData *info)
{
	(void)info;
	return false;
}

bool Portraits_TalkFaceWillBeHalfBody(int fid, int disp)
{
	(void)fid;
	(void)disp;
	return false;
}

void HalfBody_OnTalkFaceClear(struct FaceProc *face)
{
	(void)face;
}

s32 Portrait32_GetOverlayOam2(struct FaceProc *face)
{
	(void)face;
	return -1;
}
