#include "gbafe.h"

/*
 * No half-body or 32-colour portraits, installed when Installer.event leaves
 * TEXT_ENGINE_HALF_BODY_PORTRAITS undefined.  The text engine calls these at
 * the same places it calls Standalone/portrait_formats' Source/Portraits.c.
 */

bool Portraits_IsHalfBodyFace(struct FaceProc *face)
{
	(void)face;
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
