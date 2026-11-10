#include "common-chax.h"
#include "portrait-expressions.h"

/*
 * Frame sets (include/kernel/portrait-expressions.h): an expression drawn with the base portrait's graphics and its own
 * eyes and mouth. Per face slot, the set it shows; the eyes are written over the face's OBJ graphics, where the face
 * image has them, and over its two blink frames; the mouth proc (sub_8005FE0, TextEngineRework.c) draws from the set.
 */

struct PortraitFrameSlots {
	const struct PortraitFrames *set[FACE_SLOT_COUNT];
	u8 applied[FACE_SLOT_COUNT];   /* the eyes are in VRAM */
};

extern struct PortraitFrameSlots gPortraitFrameSlots;

typedef char PortraitFrameSlots_SizeCheck[(sizeof(struct PortraitFrameSlots) <= 0x14) ? 1 : -1];

/*
 * Where the face's tiles go in the 256x32 face image (32x4 tiles) the game loads into OBJ VRAM, as PortraitFormatter
 * lays the sheet out: { sheet tile x, y, image tile x, y, w, h } (Tools/portraits/portrait_tool.py MUG_MAP).
 */
static const u8 sFaceTileMap[6][6] = {
	{ 2, 0, 0, 0, 8, 4 },
	{ 2, 4, 8, 0, 8, 4 },
	{ 2, 8, 16, 0, 4, 2 },
	{ 6, 8, 16, 2, 4, 2 },
	{ 0, 6, 20, 0, 2, 4 },
	{ 10, 6, 22, 0, 2, 4 },
};

/* The blink frames in the face image: half shut at tile 24, shut at tile 88 (row 2, column 24) */
#define FACE_TILE_EYES_HALF 24
#define FACE_TILE_EYES_SHUT 88

/* The tile of the face image that shows sheet tile (x, y) of the face, or -1 */
static int PortraitFrames_FaceTile(int x, int y)
{
	const u8 *m;
	int i;

	for (i = 0; i < 6; i++) {
		m = sFaceTileMap[i];
		if (x >= m[0] && x < m[0] + m[4] && y >= m[1] && y < m[1] + m[5])
			return (m[3] + y - m[1]) * 32 + m[2] + x - m[0];
	}
	return -1;
}

static u8 *PortraitFrames_Vram(struct FaceProc *face, int tile)
{
	return (u8 *)(((face->oam2 + tile) & 0x3FF) * 0x20 + 0x06010000);
}

/* A 32x16 frame (8 tiles: top row, bottom row) into the face image at an image tile */
static void PortraitFrames_PutBlock(struct FaceProc *face, const u8 *frame, int tile)
{
	CpuFastCopy(frame, PortraitFrames_Vram(face, tile), 4 * 0x20);
	CpuFastCopy(frame + 4 * 0x20, PortraitFrames_Vram(face, tile + 32), 4 * 0x20);
}

static void PortraitFrames_PutEyes(struct FaceProc *face, const struct PortraitFrames *set)
{
	int x, y, tile;

	for (y = 0; y < 2; y++) {
		for (x = 0; x < 4; x++) {
			tile = PortraitFrames_FaceTile(face->pFaceInfo->xEyes + x, face->pFaceInfo->yEyes + y);
			if (tile >= 0)
				CpuFastCopy(set->frames + (y * 4 + x) * 0x20, PortraitFrames_Vram(face, tile), 0x20);
		}
	}
	PortraitFrames_PutBlock(face, set->frames + 1 * PORTRAIT_FRAME_SIZE, FACE_TILE_EYES_HALF);
	PortraitFrames_PutBlock(face, set->frames + 2 * PORTRAIT_FRAME_SIZE, FACE_TILE_EYES_SHUT);
}

const struct PortraitFrames *PortraitFrames_Find(int basePortrait, int expression)
{
	const struct PortraitFrames *it;

	for (it = gPortraitFrames; it->basePortrait; it++)
		if (it->basePortrait == basePortrait && it->expression == expression)
			return it;
	return NULL;
}

/* The face shows the set's base portrait, and has eye and mouth frames (not a halfbody) */
static bool PortraitFrames_Fits(struct FaceProc *face, const struct PortraitFrames *set)
{
	return set && face->faceId == set->basePortrait && face->pBlinkProc != NULL;
}

void PortraitFrames_Set(struct FaceProc *face, const struct PortraitFrames *set)
{
	int slot = face->faceSlot;

	if (slot >= FACE_SLOT_COUNT)
		return;
	if (gPortraitFrameSlots.applied[slot] && gPortraitFrameSlots.set[slot] != set) {
		/* the base portrait's eyes and blink frames back */
		Decompress(face->pFaceInfo->img, PortraitFrames_Vram(face, 0));
		gPortraitFrameSlots.applied[slot] = FALSE;
	}
	gPortraitFrameSlots.set[slot] = set;
	if (PortraitFrames_Fits(face, set) && !gPortraitFrameSlots.applied[slot]) {
		PortraitFrames_PutEyes(face, set);
		gPortraitFrameSlots.applied[slot] = TRUE;
	}
}

const u8 *PortraitFrames_Mouth(struct FaceProc *face)
{
	const struct PortraitFrames *set = face->faceSlot < FACE_SLOT_COUNT ? gPortraitFrameSlots.set[face->faceSlot] : NULL;

	if (set && gPortraitFrameSlots.applied[face->faceSlot] && face->faceId == set->basePortrait)
		return set->frames + PORTRAIT_FRAMES_EYES * PORTRAIT_FRAME_SIZE;
	return face->pFaceInfo->imgMouth;
}

void PortraitFrames_OnNewFace(struct FaceProc *face)
{
	if (face->faceSlot >= FACE_SLOT_COUNT)
		return;
	gPortraitFrameSlots.set[face->faceSlot] = NULL;
	gPortraitFrameSlots.applied[face->faceSlot] = FALSE;
}

/* The face's graphics were just loaded again (a change in place): its set shows if it is still the set's portrait */
void PortraitFrames_OnFaceLoaded(struct FaceProc *face)
{
	int slot = face->faceSlot;

	if (slot >= FACE_SLOT_COUNT)
		return;
	gPortraitFrameSlots.applied[slot] = FALSE;
	if (!PortraitFrames_Fits(face, gPortraitFrameSlots.set[slot])) {
		gPortraitFrameSlots.set[slot] = NULL;
		return;
	}
	PortraitFrames_PutEyes(face, gPortraitFrameSlots.set[slot]);
	gPortraitFrameSlots.applied[slot] = TRUE;
}
