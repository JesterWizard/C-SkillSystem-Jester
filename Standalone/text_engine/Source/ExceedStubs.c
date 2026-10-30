#include "TextEngine.h"

/*
 * No exceed portraits, installed when Installer.event leaves
 * TEXT_ENGINE_EXCEED_PORTRAITS undefined.  The text engine and the portrait
 * code call these at the same places they call Standalone/exceed_portrait_box's
 * Source/ExceedPortraitBox.c.
 */

int ExceedPortrait_GetFacePieces(struct FaceProc *proc, struct ExceedPortraitPiece *pieces)
{
	(void)proc;
	(void)pieces;
	return 0;
}

void ExceedPortrait_PutFaceSprites(struct FaceProc *proc, int oam0)
{
	(void)proc;
	(void)oam0;
}

/* Vanilla: blank the side columns of the top five rows. */
void ExceedPortrait_ClearFaceSideColumns(u16 *tm, int tileref, const struct FaceData *info, bool raised)
{
	int i;

	(void)tileref;
	(void)info;
	(void)raised;

	for (i = 0; i < 5; i++) {
		tm[TILEMAP_INDEX(0, i)] = 0;
		tm[TILEMAP_INDEX(9, i)] = 0;
	}
}
