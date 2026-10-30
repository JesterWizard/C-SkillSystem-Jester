#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "bmlib.h"
#include "bmmind.h"
#include "face.h"
#include "hardware.h"
#include "ctc.h"

extern const u8 NewStatScreenPortraitTsa[];

/*
 * Exceed the portrait hackbox (tiki for FE7, FE8 port by aera, side-column
 * fix by laqieer), in C.
 *
 * A face whose FaceData byte 0x1A is 1 gets two extra 16x16 pieces from the
 * spare tiles of its sheet: 0x5C-0x5D/0x7C-0x7D (left) and 0x5E-0x5F/0x7E-0x7F
 * (right). Their positions, in tiles, replace the imgCard pointer (bytes
 * 0x10-0x13). Such a face always has an img, so imgCard is unused.
 */
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
	if (!gpKernelDesignerConfig->exceed_portrait_box)
		return false;

	if (info == NULL || info->img == NULL)
		return false;

	/* Halfbody faces use tiles 0x5C-0x5F for their own body pieces. */
	if (HalfBody_UsesTwoPalettes(info))
		return false;

	return GetFaceDataExceed(info)->exceed == 1;
}

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
 * Vanilla blanks the side columns of the top five rows. An exceed face can
 * have a piece there, so for it only the filler tile is blanked, over six
 * rows.
 */
static void ClearFaceSideColumns(u16 *tm, int tileref, bool exceed)
{
	int i;
	int rows = exceed ? 6 : 5;
	int filler = tileref + FACE_SIDE_FILLER_TILE;

	for (i = 0; i < rows; i++) {
		if (!exceed || tm[TILEMAP_INDEX(0, i)] == filler)
			tm[TILEMAP_INDEX(0, i)] = 0;

		if (!exceed || tm[TILEMAP_INDEX(9, i)] == filler)
			tm[TILEMAP_INDEX(9, i)] = 0;
	}
}

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

void ExceedPortrait_PutFaceSprites(struct FaceProc *proc, int oam0)
{
	struct ExceedPortraitPiece pieces[EXCEED_PORTRAIT_MAX_PIECES];
	int count = ExceedPortrait_GetFacePieces(proc, pieces);
	int i;

	for (i = 0; i < count; i++) {
		int xOam1 = OAM1_X(proc->xPos + pieces[i].x);

		if (pieces[i].hflip)
			xOam1 |= OAM1_HFLIP;

		/*
		 * The text engine's screen tear slips each row by its own
		 * offset. Slip the pieces by the face's row, as its mouth and
		 * eyes are, so they stay attached to the face.
		 */
		PutSpriteExt(
			proc->spriteLayer,
			TextEngine_ApplyStaticOam1(xOam1, proc->yPos),
			TextEngine_ApplyFxOam0(oam0 + OAM0_Y(proc->yPos + pieces[i].y)),
			gObject_16x16,
			proc->oam2 + pieces[i].chr);
	}
}

static void PutFaceMouthTiles(u16 *tm, int tileref, const struct FaceData *info)
{
	int x = info->xMouth - 1;
	int y = info->yMouth;

	tm[TILEMAP_INDEX(x, y) + 0x00 + 0] = tileref + 0x00 + 0x1C;
	tm[TILEMAP_INDEX(x, y) + 0x00 + 1] = tileref + 0x00 + 0x1D;
	tm[TILEMAP_INDEX(x, y) + 0x00 + 2] = tileref + 0x00 + 0x1E;
	tm[TILEMAP_INDEX(x, y) + 0x00 + 3] = tileref + 0x00 + 0x1F;

	tm[TILEMAP_INDEX(x, y) + 0x20 + 0] = tileref + 0x20 + 0x1C;
	tm[TILEMAP_INDEX(x, y) + 0x20 + 1] = tileref + 0x20 + 0x1D;
	tm[TILEMAP_INDEX(x, y) + 0x20 + 2] = tileref + 0x20 + 0x1E;
	tm[TILEMAP_INDEX(x, y) + 0x20 + 3] = tileref + 0x20 + 0x1F;
}

//! FE8U = 0x08005B78
LYN_REPLACE_CHECK(PutFace80x72_Standard);
void PutFace80x72_Standard(u16 *tm, int tileref, const struct FaceData *info)
{
	CallARM_FillTileRect(
		tm,
		HalfBody_UsesTwoPalettes(info) ? NewStatScreenPortraitTsa : gUnknown_085A0838,
		(u16)tileref);

#if defined(SID_EmergencyExitPlus) && (COMMON_SKILL_VALID(SID_EmergencyExitPlus))
	if (gActionData.unk08 == SID_EmergencyExitPlus)
		return;
#endif

	PutFaceMouthTiles(tm, tileref, info);

	if (ExceedPortrait_IsExceedFace(info))
		PutExceedFaceTiles(tm, tileref, info);
}

//! FE8U = 0x08005CA4
LYN_REPLACE_CHECK(PutFace80x72_Core);
void PutFace80x72_Core(u16 *tm, int fid, int chr, int pal)
{
	const struct FaceData *info;
	int tileref;
	bool raised;

	if (fid == 0)
		return;

	info = GetPortraitData(fid);

	/* Original halfbody ORG 0x886A2: statscreen CHR 0x9C -> 0x90 (0x4E0 -> 0x480). */
	if (HalfBody_UsesTwoPalettes(info) && chr == 0x4E0)
		chr = 0x480;

	ApplyPalette(info->pal, pal);

	if (info->img == NULL) {
		Decompress(info->imgCard, (void *)(chr * CHR_SIZE + VRAM));
		PutAppliedBitmap(tm, (pal << 12) + (0x3FF & chr), 10, 9);
		return;
	}

	Decompress(info->img, (void *)(chr * 0x20 + VRAM));
	ApplyPalette(info->pal, pal);

	tileref = (pal << 12) + (0x3FF & chr);
	raised = ShouldFaceBeRaised(fid) != 0;

#if defined(SID_EmergencyExitPlus) && (COMMON_SKILL_VALID(SID_EmergencyExitPlus))
	if (gActionData.unk08 == SID_EmergencyExitPlus)
		raised = false;
#endif

	if (raised)
		PutFace80x72_Raised(tm, tileref, info);
	else
		PutFace80x72_Standard(tm, tileref, info);

	/* Original halfbody ORG 0x5D22: skip the side-column clear for taller TSA. */
	if (HalfBody_UsesTwoPalettes(info))
		return;

	/* Exceed pieces are only drawn by PutFace80x72_Standard. */
	ClearFaceSideColumns(tm, tileref, !raised && ExceedPortrait_IsExceedFace(info));
}
