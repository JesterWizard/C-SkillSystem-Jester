#include "ExceedPortraitBox.h"

#include "bmlib.h"
#include "ctc.h"
#include "face.h"
#include "hardware.h"
#include "variables.h"

/*
 * Exceed the portrait hackbox (tiki for FE7, FE8 port by aera, side-column
 * fix by laqieer) for vanilla FE8U, without the C Skill System kernel.  The
 * kernel's copy is Kernel/Wizardry/ExceedPortraitBox/ExceedPortraitBox.c.
 *
 * A face whose FaceData byte 0x1A is 1 gets two extra 16x16 pieces from the
 * spare tiles of its sheet: 0x5C-0x5D/0x7C-0x7D (left) and 0x5E-0x5F/0x7E-0x7F
 * (right). Their positions, in tiles, replace the imgCard pointer (bytes
 * 0x10-0x13). Such a face always has an img, so imgCard is unused.
 *
 * Built twice: on its own for the exceed_portrait_box package, and with
 * EXCEED_TEXT_ENGINE by Standalone/text_engine, whose screen effects shift
 * the pieces with their face and whose half-body faces never get pieces.
 */

#ifdef EXCEED_TEXT_ENGINE
int TextEngine_ApplyStaticOam1(int xOam1, int screenY);
int TextEngine_ApplyFxOam0(int yOam0);
bool Portraits_IsHalfBodyData(const struct FaceData *info);
#endif

struct FaceDataExceed {
	/* 00 */ const u8 *img;
	/* 04 */ const u8 *imgChibi;
	/* 08 */ const u16 *pal;
	/* 0C */ const u8 *imgMouth;
	/* 10 */ u8 xLeft, yLeft;
	/* 12 */ u8 xRight, yRight;
	/* 14 */ u8 xMouth, yMouth;
	/* 16 */ u8 xEyes, yEyes;
	/* 18 */ u8 blinkKind;
	/* 19 */ u8 _pad_19;
	/* 1A */ u8 exceed;
	/* 1B */ u8 _pad_1B;
};

#define EXCEED_TILE_LEFT 0x5C
#define EXCEED_TILE_RIGHT 0x5E
#define FACE_SIDE_FILLER_TILE 0x07

static const struct FaceDataExceed *GetFaceDataExceed(const struct FaceData *info)
{
	return (const struct FaceDataExceed *)info;
}

bool ExceedPortrait_IsExceedFace(const struct FaceData *info)
{
	if (info == NULL || info->img == NULL)
		return false;

#ifdef EXCEED_TEXT_ENGINE
	/* Half-body faces use tiles 0x5C-0x5F for their own body pieces. */
	if (Portraits_IsHalfBodyData(info))
		return false;
#endif

	return GetFaceDataExceed(info)->exceed == 1;
}

/* --------------------------------------------------------------------------
 * 80x72 BG faces (stat screen, unit and item menus)
 * ------------------------------------------------------------------------ */

static void PutExceedTile(u16 *tm, int x, int y, int tile)
{
	/* The 80x72 face box is 10x9 tiles. */
	if ((unsigned int)x < 10 && (unsigned int)y < 9)
		tm[TILEMAP_INDEX(x, y)] = tile;
}

static void PutExceedPiece(u16 *tm, int x, int y, int tile)
{
	PutExceedTile(tm, x - 1, y + 0, tile + 0x00);
	PutExceedTile(tm, x + 0, y + 0, tile + 0x01);
	PutExceedTile(tm, x - 1, y + 1, tile + 0x20);
	PutExceedTile(tm, x + 0, y + 1, tile + 0x21);
}

static void PutExceedFaceTiles(u16 *tm, int tileref, const struct FaceData *info)
{
	const struct FaceDataExceed *exceed = GetFaceDataExceed(info);

	PutExceedPiece(tm, exceed->xLeft, exceed->yLeft, tileref + EXCEED_TILE_LEFT);
	PutExceedPiece(tm, exceed->xRight, exceed->yRight, tileref + EXCEED_TILE_RIGHT);
}

/*
 * Vanilla blanks the side columns of the top five rows. An exceed face drawn
 * by PutFace80x72_Standard can have a piece there, so for it only the filler
 * tile is blanked, over six rows.
 */
void ExceedPortrait_ClearFaceSideColumns(u16 *tm, int tileref, const struct FaceData *info, bool raised)
{
	bool exceed = !raised && ExceedPortrait_IsExceedFace(info);
	int rows = exceed ? 6 : 5;
	int filler = tileref + FACE_SIDE_FILLER_TILE;
	int i;

	for (i = 0; i < rows; i++) {
		if (!exceed || tm[TILEMAP_INDEX(0, i)] == filler)
			tm[TILEMAP_INDEX(0, i)] = 0;

		if (!exceed || tm[TILEMAP_INDEX(9, i)] == filler)
			tm[TILEMAP_INDEX(9, i)] = 0;
	}
}

/* Replaces PutFace80x72_Standard (0x08005B78): vanilla, then the pieces. */
void PutFace80x72_Standard_Exceed(u16 *tm, int tileref, const struct FaceData *info)
{
	int x = info->xMouth - 1;
	int y = info->yMouth;

	CallARM_FillTileRect(tm, gUnknown_085A0838, (u16)tileref);

	tm[TILEMAP_INDEX(x, y) + 0x00 + 0] = tileref + 0x00 + 0x1C;
	tm[TILEMAP_INDEX(x, y) + 0x00 + 1] = tileref + 0x00 + 0x1D;
	tm[TILEMAP_INDEX(x, y) + 0x00 + 2] = tileref + 0x00 + 0x1E;
	tm[TILEMAP_INDEX(x, y) + 0x00 + 3] = tileref + 0x00 + 0x1F;

	tm[TILEMAP_INDEX(x, y) + 0x20 + 0] = tileref + 0x20 + 0x1C;
	tm[TILEMAP_INDEX(x, y) + 0x20 + 1] = tileref + 0x20 + 0x1D;
	tm[TILEMAP_INDEX(x, y) + 0x20 + 2] = tileref + 0x20 + 0x1E;
	tm[TILEMAP_INDEX(x, y) + 0x20 + 3] = tileref + 0x20 + 0x1F;

	if (ExceedPortrait_IsExceedFace(info))
		PutExceedFaceTiles(tm, tileref, info);
}

/* Replaces PutFace80x72_Core (0x08005CA4): vanilla, with the side columns above. */
void PutFace80x72_Core_Exceed(u16 *tm, int fid, int chr, int pal)
{
	const struct FaceData *info;
	int tileref;
	bool raised;

	if (fid == 0)
		return;

	info = GetPortraitData(fid);

	ApplyPalette(info->pal, pal);

	if (info->img == NULL) {
		Decompress(info->imgCard, (void *)(chr * CHR_SIZE + VRAM));
		PutAppliedBitmap(tm, (pal << 12) + (0x3FF & chr), 10, 9);
		return;
	}

	Decompress(info->img, (void *)(chr * CHR_SIZE + VRAM));
	ApplyPalette(info->pal, pal);

	tileref = (pal << 12) + (0x3FF & chr);
	raised = ShouldFaceBeRaised(fid) != 0;

	if (raised)
		PutFace80x72_Raised(tm, tileref, info);
	else
		PutFace80x72_Standard(tm, tileref, info);

	ExceedPortrait_ClearFaceSideColumns(tm, tileref, info, raised);
}

/* --------------------------------------------------------------------------
 * Face sprites (dialogue and other StartFace users)
 * ------------------------------------------------------------------------ */

static void GetExceedPiece(
	struct FaceProc *proc,
	int x,
	int y,
	int tile,
	struct ExceedPortraitPiece *piece)
{
	/* Offsets from the original hack, relative to the face's centre. */
	if (GetFaceDisplayBits(proc) & FACE_DISP_FLIPPED) {
		piece->x = (4 - x) * 8;
		piece->hflip = true;
	} else {
		piece->x = (x - 4) * 8 - 16;
		piece->hflip = false;
	}

	piece->y = y * 8;
	piece->chr = tile;
}

/* Fills up to EXCEED_PORTRAIT_MAX_PIECES pieces; returns how many. */
int ExceedPortrait_GetFacePieces(struct FaceProc *proc, struct ExceedPortraitPiece *pieces)
{
	const struct FaceDataExceed *exceed;

	if (proc == NULL || !ExceedPortrait_IsExceedFace(proc->pFaceInfo))
		return 0;

	exceed = GetFaceDataExceed(proc->pFaceInfo);
	GetExceedPiece(proc, exceed->xLeft, exceed->yLeft, EXCEED_TILE_LEFT, &pieces[0]);
	GetExceedPiece(proc, exceed->xRight, exceed->yRight, EXCEED_TILE_RIGHT, &pieces[1]);
	return EXCEED_PORTRAIT_MAX_PIECES;
}

/* oam0 holds the face's OAM0 flags (blend), not its Y. */
void ExceedPortrait_PutFaceSprites(struct FaceProc *proc, int oam0)
{
	struct ExceedPortraitPiece pieces[EXCEED_PORTRAIT_MAX_PIECES];
	int count = ExceedPortrait_GetFacePieces(proc, pieces);
	int i;

	for (i = 0; i < count; i++) {
		int xOam1 = OAM1_X(proc->xPos + pieces[i].x);
		int yOam0 = oam0 + OAM0_Y(proc->yPos + pieces[i].y);

		if (pieces[i].hflip)
			xOam1 |= OAM1_HFLIP;

#ifdef EXCEED_TEXT_ENGINE
		/*
		 * The screen tear slips each row by its own offset. Slip the
		 * pieces by the face's row, as its mouth and eyes are, so they
		 * stay attached to the face.
		 */
		xOam1 = TextEngine_ApplyStaticOam1(xOam1, proc->yPos);
		yOam0 = TextEngine_ApplyFxOam0(yOam0);
#endif

		PutSpriteExt(proc->spriteLayer, xOam1, yOam0, gObject_16x16, proc->oam2 + pieces[i].chr);
	}
}

#ifndef EXCEED_TEXT_ENGINE
/* Replaces Face_OnIdle (0x080055BC): vanilla, then the pieces. */
void Face_OnIdle_Exceed(struct FaceProc *proc)
{
	int oam0;

	if (GetFaceDisplayBits(proc) & FACE_DISP_HIDDEN)
		return;

	oam0 = (GetFaceDisplayBits(proc) & FACE_DISP_BLEND) ? OAM0_BLEND : 0;

	PutSpriteExt(proc->spriteLayer, 0x1FF & proc->xPos, oam0 + OAM0_Y(proc->yPos), proc->sprite, proc->oam2);
	ExceedPortrait_PutFaceSprites(proc, oam0);
}
#endif
