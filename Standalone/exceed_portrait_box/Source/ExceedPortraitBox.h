#pragma once

/*
 * Exceed the portrait hackbox for FE8U: two extra 16x16 pieces for portraits
 * whose portrait table entry sets byte 0x1A to 1.  See ExceedPortraitBox.c.
 *
 * Standalone/text_engine builds the same source with EXCEED_TEXT_ENGINE and
 * calls these from its face code.
 */

#include "gbafe.h"

#define EXCEED_PORTRAIT_MAX_PIECES 2

struct ExceedPortraitPiece {
	s16 x; /* left edge, from the face's xPos */
	s16 y; /* top edge, from the face's yPos */
	u16 chr; /* first tile, from the face's oam2 */
	u8 hflip;
};

bool ExceedPortrait_IsExceedFace(const struct FaceData *info);
int ExceedPortrait_GetFacePieces(struct FaceProc *proc, struct ExceedPortraitPiece *pieces);
void ExceedPortrait_PutFaceSprites(struct FaceProc *proc, int oam0);
void ExceedPortrait_ClearFaceSideColumns(u16 *tm, int tileref, const struct FaceData *info, bool raised);
