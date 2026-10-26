#include "Portraits.h"

#include "bmlib.h"
#include "ctc.h"
#include "ekrbattle.h"
#include "face.h"
#include "hardware.h"
#include "proc.h"
#include "scene.h"
#include "variables.h"

/*
 * Half-body portraits (the C Skill System's half_body_portraits, by
 * Ryrumeli and Kirb) and 32-colour portraits (portrait_32_color) for
 * vanilla FE8U, without the kernel.
 *
 * The C Skill System switches every portrait to the new layout when its flag
 * is on.  Here each face is looked at on its own: a portrait installed with
 * setMugEntry_Halfbody / setMugEntry_32Color uses the new format, every other
 * portrait stays vanilla, so a ROM can convert portraits one at a time.
 *
 * Built twice: on its own for the portrait_formats package, and with
 * PORTRAITS_TEXT_ENGINE by Standalone/text_engine, whose face sprites go
 * through TextEngine_PutFaceSprite so the screen effects move them.
 */

#define sFaceConfig ((struct FaceVramEntry *)0x0202A68C)
#define sTalkState (*(struct TalkState **)0x0859133C)

#ifdef PORTRAITS_TEXT_ENGINE
void TextEngine_PutFaceSprite(int layer, int xOam1, int yOam0, const u16 *object, int oam2);
#define PutFaceSprite TextEngine_PutFaceSprite
#else
#define PutFaceSprite PutSpriteExt
#endif

extern struct ProcCmd CONST_DATA gProcScr_0859124C[];

enum {
	FACE_VRAM_BLOCK = 0x1000,
	OBJ_VRAM_FACE_END = 0x8000,
	HALFBODY_FACE_HEIGHT_EXTRA = 48, /* 128-pixel half-body vs 80-pixel face */
	STAT_SCREEN_FACE_CHR = 0x4E0,
	STAT_SCREEN_HALFBODY_CHR = 0x480,
};

/* --------------------------------------------------------------------------
 * Half-body sprite layouts (from the C Skill System's
 * HalfBodyPortraits_Installer.event).  The sheet is a 32x8-tile block of
 * OBJ VRAM; the top 32x4 tiles hold the same pieces as a vanilla mug, so
 * vanilla sprite layouts still show a half-body's face correctly.
 * ------------------------------------------------------------------------ */

static const u16 sHalfBodyRight[] = {
	22,
	0x4000, 0xC1E0, 0x0000, /* 64x32 x=-32 y=0   tile=0x00 */
	0x4020, 0xC1E0, 0x0008, /* 64x32 x=-32 y=32  tile=0x08 */
	0x0000, 0x41D0, 0x009A, /* 16x16 x=-48 y=0   tile=0x9A */
	0x0000, 0x4020, 0x00DA, /* 16x16 x=32  y=0   tile=0xDA */
	0x4040, 0x81E0, 0x0010, /* 32x16 x=-32 y=64  tile=0x10 */
	0x4040, 0x8000, 0x0050, /* 32x16 x=0   y=64  tile=0x50 */
	0x8030, 0x81D0, 0x0014, /* 16x32 x=-48 y=48  tile=0x14 */
	0x8030, 0x8020, 0x0016, /* 16x32 x=32  y=48  tile=0x16 */
	0x8030, 0x8030, 0x0096, /* 16x32 x=48  y=48  tile=0x96 */
	0x8030, 0x81C0, 0x0098, /* 16x32 x=-64 y=48  tile=0x98 */
	0x8010, 0x81D0, 0x0092, /* 16x32 x=-48 y=16  tile=0x92 */
	0x8010, 0x8020, 0x0094, /* 16x32 x=32  y=16  tile=0x94 */
	0x8050, 0x81C0, 0x0080, /* 16x32 x=-64 y=80  tile=0x80 */
	0x0050, 0x81D0, 0x0082, /* 32x32 x=-48 y=80  tile=0x82 */
	0x0050, 0x81F0, 0x0086, /* 32x32 x=-16 y=80  tile=0x86 */
	0x0050, 0x8010, 0x008A, /* 32x32 x=16  y=80  tile=0x8A */
	0x8050, 0x8030, 0x008E, /* 16x32 x=48  y=80  tile=0x8E */
	0x0070, 0x41C0, 0x0090, /* 16x16 x=-64 y=112 tile=0x90 */
	0x4070, 0x81D0, 0x005C, /* 32x16 x=-48 y=112 tile=0x5C */
	0x4070, 0x81F0, 0x009C, /* 32x16 x=-16 y=112 tile=0x9C */
	0x4070, 0x8010, 0x00DC, /* 32x16 x=16  y=112 tile=0xDC */
	0x0070, 0x4030, 0x00D0, /* 16x16 x=48  y=112 tile=0xD0 */
};

static const u16 sHalfBodyLeft[] = {
	22,
	0x4000, 0xD1E0, 0x0000,
	0x4020, 0xD1E0, 0x0008,
	0x0000, 0x5020, 0x009A,
	0x0000, 0x51D0, 0x00DA,
	0x4040, 0x9000, 0x0010,
	0x4040, 0x91E0, 0x0050,
	0x8030, 0x9020, 0x0014,
	0x8030, 0x91D0, 0x0016,
	0x8030, 0x91C0, 0x0096,
	0x8030, 0x9030, 0x0098,
	0x8010, 0x9020, 0x0092,
	0x8010, 0x91D0, 0x0094,
	0x8050, 0x9030, 0x0080,
	0x0050, 0x9010, 0x0082,
	0x0050, 0x91F0, 0x0086,
	0x0050, 0x91D0, 0x008A,
	0x8050, 0x91C0, 0x008E,
	0x0070, 0x5030, 0x0090,
	0x4070, 0x9010, 0x005C,
	0x4070, 0x91F0, 0x009C,
	0x4070, 0x91D0, 0x00DC,
	0x0070, 0x51C0, 0x00D0,
};

/* Top 80 rows only (face, chest and shoulders): menus and level-ups. */
static const u16 sHalfBodyRightCut[] = {
	12,
	0x4000, 0xC1E0, 0x0000,
	0x4020, 0xC1E0, 0x0008,
	0x0000, 0x41D0, 0x009A,
	0x0000, 0x4020, 0x00DA,
	0x4040, 0x81E0, 0x0010,
	0x4040, 0x8000, 0x0050,
	0x8030, 0x81D0, 0x0014,
	0x8030, 0x8020, 0x0016,
	0x8030, 0x8030, 0x0096,
	0x8030, 0x81C0, 0x0098,
	0x8010, 0x81D0, 0x0092,
	0x8010, 0x8020, 0x0094,
};

static const u16 sHalfBodyLeftCut[] = {
	12,
	0x4000, 0xD1E0, 0x0000,
	0x4020, 0xD1E0, 0x0008,
	0x0000, 0x5020, 0x009A,
	0x0000, 0x51D0, 0x00DA,
	0x4040, 0x9000, 0x0010,
	0x4040, 0x91E0, 0x0050,
	0x8030, 0x9020, 0x0014,
	0x8030, 0x91D0, 0x0016,
	0x8030, 0x91C0, 0x0096,
	0x8030, 0x9030, 0x0098,
	0x8010, 0x9020, 0x0092,
	0x8010, 0x91D0, 0x0094,
};

/*
 * 80x72 BG face for the stat screen (CallARM_FillTileRect format: width-1,
 * height-1, then rows from the bottom up).  Same tiles as vanilla's
 * gUnknown_085A0838 plus the half-body's wider head columns.
 */
static const u16 sHalfBodyStatTsa[] = {
	0x0809,
	0x055, 0x010, 0x011, 0x012, 0x013, 0x050, 0x051, 0x052, 0x053, 0x056,
	0x035, 0x068, 0x069, 0x06A, 0x06B, 0x06C, 0x06D, 0x06E, 0x06F, 0x036,
	0x015, 0x048, 0x049, 0x04A, 0x04B, 0x04C, 0x04D, 0x04E, 0x04F, 0x016,
	0x0F3, 0x028, 0x029, 0x02A, 0x02B, 0x02C, 0x02D, 0x02E, 0x02F, 0x0F4,
	0x0D3, 0x008, 0x009, 0x00A, 0x00B, 0x00C, 0x00D, 0x00E, 0x00F, 0x0D4,
	0x0B3, 0x060, 0x061, 0x062, 0x063, 0x064, 0x065, 0x066, 0x067, 0x0B4,
	0x093, 0x040, 0x041, 0x042, 0x043, 0x044, 0x045, 0x046, 0x047, 0x094,
	0x0BB, 0x020, 0x021, 0x022, 0x023, 0x024, 0x025, 0x026, 0x027, 0x0FA,
	0x09B, 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007, 0x0DA,
};

/* The same, one tile row lower in the sheet (vanilla's raised faces). */
static const u16 sHalfBodyStatTsaRaised[] = {
	0x0809,
	0x075, 0x030, 0x031, 0x032, 0x033, 0x070, 0x071, 0x072, 0x073, 0x076,
	0x055, 0x010, 0x011, 0x012, 0x013, 0x050, 0x051, 0x052, 0x053, 0x056,
	0x035, 0x068, 0x069, 0x06A, 0x06B, 0x06C, 0x06D, 0x06E, 0x06F, 0x036,
	0x015, 0x048, 0x049, 0x04A, 0x04B, 0x04C, 0x04D, 0x04E, 0x04F, 0x016,
	0x0F3, 0x028, 0x029, 0x02A, 0x02B, 0x02C, 0x02D, 0x02E, 0x02F, 0x0F4,
	0x0D3, 0x008, 0x009, 0x00A, 0x00B, 0x00C, 0x00D, 0x00E, 0x00F, 0x0D4,
	0x0B3, 0x060, 0x061, 0x062, 0x063, 0x064, 0x065, 0x066, 0x067, 0x0B4,
	0x093, 0x040, 0x041, 0x042, 0x043, 0x044, 0x045, 0x046, 0x047, 0x094,
	0x0BB, 0x020, 0x021, 0x022, 0x023, 0x024, 0x025, 0x026, 0x027, 0x0FA,
};

/* --------------------------------------------------------------------------
 * Portrait formats
 * ------------------------------------------------------------------------ */

/* HalfbodyFormatter puts the chibi at a fixed offset in the same blob. */
static bool IsHalfBodyData(const struct FaceData *info)
{
	const u8 *img;
	const u8 *chibi;

	if (info == NULL || info->img == NULL)
		return false;

	img = info->img;
	chibi = info->imgChibi;

	return chibi == img + 0x2644 || chibi == img + 0x2648;
}

/* setMugEntry_32Color points imgCard at the second palette. */
static bool Is32ColorData(const struct FaceData *info)
{
	if (info == NULL || info->pal == NULL || info->img == NULL)
		return false;

	if (IsHalfBodyData(info))
		return false;

	return info->imgCard == (const u8 *)(info->pal + 16);
}

/* Kinds 2-7 draw tiles below the top 32x4 block for a half-body. */
static bool KindUsesHalfBodyLayout(int disp)
{
	if (disp & 0x800)
		return false;

	return (disp & 6) != 0;
}

static bool KindIsFullHeight(int disp)
{
	return (disp & 0x806) == 2;
}

static int WantedMode(const struct FaceData *info, int disp)
{
	if (IsHalfBodyData(info))
		return KindUsesHalfBodyLayout(disp) ? PORTRAIT_MODE_HALFBODY : PORTRAIT_MODE_NORMAL;

	if (Is32ColorData(info))
		return PORTRAIT_MODE_COLOR32;

	return PORTRAIT_MODE_NORMAL;
}

/* --------------------------------------------------------------------------
 * Face VRAM and palette bookkeeping
 * ------------------------------------------------------------------------ */

enum {
	FIT_FREE,
	FIT_EVICT,    /* only clashes with another face's extra block/palette */
	FIT_CLASH,
};

static bool IsConfigEntryUsable(int r)
{
	return sFaceConfig[r].tileOffset != 0 || sFaceConfig[r].paletteId != 0;
}

static bool RangesOverlap(u32 a, u32 aEnd, u32 b, u32 bEnd)
{
	return a < bEnd && b < aEnd;
}

static bool IsFaceActive(int slot)
{
	return gFaces[slot] != NULL;
}

/*
 * Can face slot `slot` draw from config entry `r` in `mode`?  Compares its
 * VRAM and palettes with the faces already on screen.  `evictMask` gets the
 * faces whose extra block/palette would have to be given up.
 */
static int CheckFit(int slot, int r, int mode, u32 *evictMask)
{
	u32 base = sFaceConfig[r].tileOffset;
	u32 end = base + (mode == PORTRAIT_MODE_NORMAL ? FACE_VRAM_BLOCK : 2 * FACE_VRAM_BLOCK);
	int pal = sFaceConfig[r].paletteId & 0xF;
	int palExt = mode == PORTRAIT_MODE_COLOR32 ? pal + 1 : -1;
	int result = FIT_FREE;
	int i;

	if (mode != PORTRAIT_MODE_NORMAL && end > OBJ_VRAM_FACE_END)
		return FIT_CLASH;

	if (palExt > 15)
		return FIT_CLASH;

	if (evictMask)
		*evictMask = 0;

	for (i = 0; i < FACE_SLOT_COUNT; i++) {
		const struct PortraitSlot *other = &sPortraitSlots[i];
		u32 oBase;
		int oPal;

		if (i == slot || !IsFaceActive(i))
			continue;

		oBase = sFaceConfig[other->resSlot].tileOffset;
		oPal = sFaceConfig[other->resSlot].paletteId & 0xF;

		/* Its own block and palette can't be taken away. */
		if (RangesOverlap(base, end, oBase, oBase + FACE_VRAM_BLOCK)
				|| pal == oPal || palExt == oPal)
			return FIT_CLASH;

		/* Its extra block / palette can. */
		if ((other->mode != PORTRAIT_MODE_NORMAL
				&& RangesOverlap(base, end, oBase + FACE_VRAM_BLOCK, oBase + 2 * FACE_VRAM_BLOCK))
				|| (other->mode == PORTRAIT_MODE_COLOR32
				&& (pal == other->extPal || palExt == other->extPal))) {
			result = FIT_EVICT;
			if (evictMask)
				*evictMask |= 1 << i;
		}
	}

	return result;
}

static int CountColor32Faces(int exceptSlot)
{
	int i;
	int n = 0;

	for (i = 0; i < FACE_SLOT_COUNT; i++) {
		if (i != exceptSlot && IsFaceActive(i)
				&& sPortraitSlots[i].mode == PORTRAIT_MODE_COLOR32)
			n++;
	}

	return n;
}

/* As the C Skill System: at most two overlays at once (OAM budget). */
static int LimitMode(int slot, int mode)
{
	if (mode == PORTRAIT_MODE_COLOR32 && CountColor32Faces(slot) >= 2)
		return PORTRAIT_MODE_NORMAL;

	return mode;
}

/* Probe order for a face slot's config entry: its own first. */
static const u8 sResOrder[FACE_SLOT_COUNT] = { 0, 2, 1, 3 };

/*
 * Pick the config entry and mode for a face starting in `slot`:
 *  1. its own entry in the wanted mode, 2. another free entry in that mode,
 *  3. same two in the normal mode, 4. its own entry, taking the extra
 *  block/palette back from the face that holds it, 5. its own entry anyway
 *  (what vanilla does).
 */
static void ChooseResources(int slot, int wanted, int *outRes, int *outMode, u32 *outEvict)
{
	int pass;
	int k;
	u32 evict;

	*outEvict = 0;
	wanted = LimitMode(slot, wanted);

	for (pass = 0; pass < 2; pass++) {
		int mode = pass == 0 ? wanted : PORTRAIT_MODE_NORMAL;

		if (pass == 0 && wanted == PORTRAIT_MODE_NORMAL)
			continue;

		if (CheckFit(slot, slot, mode, NULL) == FIT_FREE) {
			*outRes = slot;
			*outMode = mode;
			return;
		}

		for (k = 0; k < FACE_SLOT_COUNT; k++) {
			int r = sResOrder[k];

			if (r == slot || !IsConfigEntryUsable(r))
				continue;

			if (CheckFit(slot, r, mode, NULL) == FIT_FREE) {
				*outRes = r;
				*outMode = mode;
				return;
			}
		}
	}

	*outRes = slot;
	*outMode = PORTRAIT_MODE_NORMAL;

	if (CheckFit(slot, slot, PORTRAIT_MODE_NORMAL, &evict) == FIT_EVICT)
		*outEvict = evict;
}

static void RestoreExtPalette(struct PortraitSlot *st)
{
	if (st->mode != PORTRAIT_MODE_COLOR32 || st->extPal > 15)
		return;

	CopyToPaletteBuffer(st->savedPal, (st->extPal + 0x10) * 0x20, 0x20);
	st->extPal = 0xFF;
}

static void LoadExtPalette(struct PortraitSlot *st, const struct FaceData *info, bool black)
{
	int r = st->resSlot;
	int pal = (sFaceConfig[r].paletteId & 0xF) + 1;

	CpuCopy16(gPaletteBuffer + (pal + 0x10) * 16, st->savedPal, 0x20);

	if (black) {
		CpuFastFill(0, PAL_OBJ(0) + PAL_OFFSET(pal), 0x20);
		EnablePaletteSync();
	} else {
		CopyToPaletteBuffer(info->pal + 16, (pal + 0x10) * 0x20, 0x20);
	}

	st->extPal = pal;
	st->extChr = (sFaceConfig[r].tileOffset + FACE_VRAM_BLOCK) / CHR_SIZE;
}

/* Copy the face tiles (and the 32-colour overlay) into OBJ VRAM. */
static void LoadFaceGfx(struct FaceProc *proc)
{
	const struct FaceData *info = proc->pFaceInfo;
	struct PortraitSlot *st = &sPortraitSlots[proc->faceSlot];
	u8 *dst = (u8 *)(sFaceConfig[st->resSlot].tileOffset + 0x06010000);

	if (IsHalfBodyData(info) && st->mode != PORTRAIT_MODE_HALFBODY && info->img[0] == 0) {
		/* No room for the lower half: only the top 32x4 tiles. */
		CpuFastCopy(info->img + 4, dst, FACE_VRAM_BLOCK);
		return;
	}

	Decompress(info->img, dst);

	if (st->mode == PORTRAIT_MODE_COLOR32)
		CpuFastCopy(info->pal + 32, dst + FACE_VRAM_BLOCK, FACE_VRAM_BLOCK);
}

static void StopMouthAndBlink(struct FaceProc *proc)
{
	if (proc->unk_44 != NULL) {
		Proc_End(proc->unk_44);
		proc->unk_44 = NULL;
	}

	if (proc->pBlinkProc != NULL) {
		Proc_End(proc->pBlinkProc);
		proc->pBlinkProc = NULL;
	}
}

/* Keep a portrait's feet where they were when it changes height. */
static void ShiftForHeight(struct FaceProc *proc, int oldMode, int newMode)
{
	if (!KindIsFullHeight(proc->displayBits))
		return;

	if (oldMode == PORTRAIT_MODE_HALFBODY && newMode != PORTRAIT_MODE_HALFBODY)
		proc->yPos += HALFBODY_FACE_HEIGHT_EXTRA;
	else if (oldMode != PORTRAIT_MODE_HALFBODY && newMode == PORTRAIT_MODE_HALFBODY)
		proc->yPos -= HALFBODY_FACE_HEIGHT_EXTRA;
}

/* Another face needs this one's extra block or palette: drop to normal. */
static void DemoteFace(int slot)
{
	struct FaceProc *proc = gFaces[slot];
	struct PortraitSlot *st = &sPortraitSlots[slot];
	int oldMode = st->mode;

	if (proc == NULL || oldMode == PORTRAIT_MODE_NORMAL)
		return;

	RestoreExtPalette(st);
	st->mode = PORTRAIT_MODE_NORMAL;
	ShiftForHeight(proc, oldMode, PORTRAIT_MODE_NORMAL);
	FaceRefreshSprite(proc);
}

static void ApplyEvictions(u32 mask)
{
	int i;

	for (i = 0; i < FACE_SLOT_COUNT; i++) {
		if (mask & (1 << i))
			DemoteFace(i);
	}
}

/* --------------------------------------------------------------------------
 * Face procs
 * ------------------------------------------------------------------------ */

static struct FaceProc *StartFaceCommon(int slot, int fid, int x, int y, int disp, bool extraFrame)
{
	struct FaceProc *proc;
	struct PortraitSlot *st;
	const struct FaceData *info;
	int res;
	int mode;
	int pal;
	u32 evict;

	if (gFaces[slot] != NULL)
		return NULL;

	info = GetPortraitData(fid);
	ChooseResources(slot, WantedMode(info, disp), &res, &mode, &evict);
	ApplyEvictions(evict);

	proc = Proc_Start(extraFrame ? gProcScr_E_FACE_ExtraFrame : gProcScr_E_FACE, PROC_TREE_5);
	gFaces[slot] = proc;

	st = &sPortraitSlots[slot];
	st->mode = mode;
	st->resSlot = res;
	st->extPal = 0xFF;
	st->extChr = 0;
	st->hasMouth = false;

	pal = sFaceConfig[res].paletteId;

	if (disp & FACE_DISP_BIT_13) {
		CpuFastFill(0, PAL_OBJ(0) + PAL_OFFSET(pal), 0x20);
		EnablePaletteSync();
	} else {
		ApplyPalette(info->pal, pal + 0x10);
	}

	if (mode == PORTRAIT_MODE_COLOR32)
		LoadExtPalette(st, info, (disp & FACE_DISP_BIT_13) != 0);

	proc->pFaceInfo = info;
	proc->faceSlot = slot;
	proc->faceId = fid;
	proc->spriteLayer = 5;
	proc->xPos = x;
	proc->yPos = y;

	/*
	 * Half-body sheets have no usable mouth/eye frames (the C Skill System
	 * doesn't animate them either).
	 */
	if (extraFrame || (disp & FACE_DISP_BIT_12) || IsHalfBodyData(info)) {
		proc->unk_44 = NULL;
		proc->pBlinkProc = NULL;
	} else {
		proc->unk_44 = Proc_Start(gProcScr_0859124C, proc);
		proc->pBlinkProc = Proc_Start(gProcScr_FaceBlink, proc);
		st->hasMouth = true;
	}

	if (extraFrame) {
		proc->displayBits = disp;
		FaceRefreshSprite(proc);
	} else {
		proc->displayBits = ~disp;
		SetFaceDisplayBits(proc, disp);
	}

	return proc;
}

/* Replaces StartFace (0x0800563C). */
struct FaceProc *StartFace_Portraits(int slot, int fid, int x, int y, int disp)
{
	return StartFaceCommon(slot, fid, x, y, disp, false);
}

/* Replaces StartFace2 (0x080064F4): prep screens, world map. */
struct FaceProc *StartFace2_Portraits(int slot, int fid, int x, int y, int disp)
{
	return StartFaceCommon(slot, fid, x, y, disp, true);
}

/*
 * The face slot StartFaceAuto (dialogue) will use.  Vanilla takes the first
 * free slot; a new-format portrait first looks for a slot with room for its
 * extra block.
 */
static int PickAutoSlot(int fid, int disp)
{
	const struct FaceData *info = GetPortraitData(fid);
	int wanted = LimitMode(-1, WantedMode(info, disp));
	int i;

	if (wanted != PORTRAIT_MODE_NORMAL) {
		for (i = 0; i < FACE_SLOT_COUNT; i++) {
			int slot = sResOrder[i];

			if (!IsFaceActive(slot) && CheckFit(slot, slot, wanted, NULL) == FIT_FREE)
				return slot;
		}
	}

	for (i = 0; i < FACE_SLOT_COUNT; i++) {
		if (!IsFaceActive(i) && CheckFit(i, i, PORTRAIT_MODE_NORMAL, NULL) == FIT_FREE)
			return i;
	}

	for (i = 0; i < FACE_SLOT_COUNT; i++) {
		if (!IsFaceActive(i))
			return i;
	}

	return -1;
}

/* Replaces StartFaceAuto (0x08005610). */
struct FaceProc *StartFaceAuto_Portraits(int fid, int x, int y, int disp)
{
	int slot = PickAutoSlot(fid, disp);

	if (slot < 0)
		return NULL;

	return StartFaceCommon(slot, fid, x, y, disp, false);
}

/* Would StartFaceAuto(fid, ..., disp) show this face as a half-body? */
bool Portraits_TalkFaceWillBeHalfBody(int fid, int disp)
{
	const struct FaceData *info = GetPortraitData(fid);
	int slot;
	int res;
	int mode;
	u32 evict;

	if (WantedMode(info, disp) != PORTRAIT_MODE_HALFBODY)
		return false;

	slot = PickAutoSlot(fid, disp);
	if (slot < 0)
		return false;

	ChooseResources(slot, PORTRAIT_MODE_HALFBODY, &res, &mode, &evict);
	return mode == PORTRAIT_MODE_HALFBODY;
}

bool Portraits_IsHalfBodyFace(struct FaceProc *face)
{
	if (face == NULL || face->faceSlot >= FACE_SLOT_COUNT || gFaces[face->faceSlot] != face)
		return false;

	return sPortraitSlots[face->faceSlot].mode == PORTRAIT_MODE_HALFBODY;
}

/* Replaces Face_OnInit (0x08005594). */
void Face_OnInit_Portraits(struct FaceProc *proc)
{
	LoadFaceGfx(proc);
}

/* Replaces Face_OnIdle (0x080055BC): adds the 32-colour overlay sprite. */
void Face_OnIdle_Portraits(struct FaceProc *proc)
{
	const struct PortraitSlot *st;
	int oam0;

	if (GetFaceDisplayBits(proc) & FACE_DISP_HIDDEN)
		return;

	oam0 = (GetFaceDisplayBits(proc) & FACE_DISP_BLEND) ? OAM0_BLEND : 0;
	oam0 += OAM0_Y(proc->yPos);

	PutFaceSprite(proc->spriteLayer, 0x1FF & proc->xPos, oam0, proc->sprite, proc->oam2);

	st = &sPortraitSlots[proc->faceSlot];
	if (st->mode != PORTRAIT_MODE_COLOR32 || st->extPal > 15)
		return;

	PutFaceSprite(
		proc->spriteLayer,
		0x1FF & proc->xPos,
		oam0,
		proc->sprite,
		st->extChr + st->extPal * 0x1000 + (proc->oam2 & 0x0C00)
	);
}

/* Replaces EndFace (0x08005738). */
void EndFace_Portraits(struct FaceProc *proc)
{
	struct PortraitSlot *st;

	/* ResetFaces ends every slot, empty ones included. */
	if (proc == NULL)
		return;

	st = &sPortraitSlots[proc->faceSlot];
	RestoreExtPalette(st);
	st->mode = PORTRAIT_MODE_NORMAL;

	gFaces[proc->faceSlot] = NULL;
	Proc_End(proc);
}

/* Replaces FaceRefreshSprite (0x080057C0). */
void FaceRefreshSprite_Portraits(struct FaceProc *proc)
{
	const struct PortraitSlot *st = &sPortraitSlots[proc->faceSlot];
	bool halfbody = st->mode == PORTRAIT_MODE_HALFBODY;
	int r = st->resSlot;
	int oam2Layer;

	switch (proc->displayBits & 0x807) {
	case 0:
		proc->sprite = gSprite_Face64x96;
		break;

	case 1:
		proc->sprite = gSprite_Face64x96_Flipped;
		break;

	case 2:
		proc->sprite = halfbody ? (void *)sHalfBodyRight : gSprite_Face96x96;
		break;

	case 3:
		proc->sprite = halfbody ? (void *)sHalfBodyLeft : gSprite_Face96x96_Flipped;
		break;

	case 4:
		proc->sprite = halfbody ? (void *)sHalfBodyRightCut : gSprite_Face80x72;
		break;

	case 5:
		proc->sprite = halfbody ? (void *)sHalfBodyLeftCut : gSprite_Face80x72_Flipped;
		break;

	/* Map menus and level-up screens (patched in Installer.event). */
	case PORTRAITS_KIND_MENU:
		proc->sprite = halfbody ? (void *)sHalfBodyRightCut : gSprite_Face96x96;
		break;

	case PORTRAITS_KIND_MENU | 1:
		proc->sprite = halfbody ? (void *)sHalfBodyLeftCut : gSprite_Face96x96_Flipped;
		break;

	case 0x800:
		proc->sprite = gSprite_Face96x72;
		break;

	case 0x801:
		proc->sprite = gSprite_Face96x72_Flipped;
		break;
	}

	switch (proc->displayBits & FACE_DISP_HLAYER_MASK) {
	case FACE_DISP_HLAYER(FACE_HLAYER_0):
		oam2Layer = OAM2_LAYER(0);
		break;

	case FACE_DISP_HLAYER(FACE_HLAYER_1):
		oam2Layer = OAM2_LAYER(1);
		break;

	case FACE_DISP_HLAYER(FACE_HLAYER_3):
		oam2Layer = OAM2_LAYER(3);
		break;

	default:
		oam2Layer = OAM2_LAYER(2);
		break;
	}

	proc->oam2 = (sFaceConfig[r].tileOffset / CHR_SIZE)
		+ ((sFaceConfig[r].paletteId & 0xF) * 0x1000)
		+ oam2Layer;
}

/* Replaces StartFaceFadeIn (0x08005EF0). */
void StartFaceFadeIn_Portraits(struct FaceProc *proc)
{
	const struct FaceData *info;
	const struct PortraitSlot *st;
	int pal;

	/* TalkLoadFace fades in whatever StartFaceAuto returned. */
	if (proc == NULL)
		return;

	info = GetPortraitData(proc->faceId);
	st = &sPortraitSlots[proc->faceSlot];
	pal = ((proc->oam2 >> 12) & 0xF) + 0x10;

	SetBlackPal(pal);
	StartPalFade(info->pal, pal, 12, proc);

	if (st->mode == PORTRAIT_MODE_COLOR32 && st->extPal <= 15) {
		SetBlackPal(st->extPal + 0x10);
		StartPalFade(info->pal + 16, st->extPal + 0x10, 12, proc);
	}
}

/*
 * Hide a half-body face at once (the C Skill System does this before the
 * text engine tears a face down, to avoid a palette flash).
 */
void HalfBody_OnTalkFaceClear(struct FaceProc *proc)
{
	if (!Portraits_IsHalfBodyFace(proc))
		return;

	StopMouthAndBlink(proc);
	proc->displayBits |= FACE_DISP_HIDDEN;
}

/* Replaces StartFaceFadeOut (0x08005F38). */
void StartFaceFadeOut_Portraits(struct FaceProc *proc)
{
	const struct PortraitSlot *st;

	if (proc == NULL)
		return;

	st = &sPortraitSlots[proc->faceSlot];

	if (st->mode == PORTRAIT_MODE_HALFBODY) {
		HalfBody_OnTalkFaceClear(proc);
		EndFaceIn8Frames(proc);
		return;
	}

	StartPalFadeToBlack(((proc->oam2 >> 12) & 0xF) + 0x10, 12, proc);

	if (st->mode == PORTRAIT_MODE_COLOR32 && st->extPal <= 15)
		StartPalFadeToBlack(st->extPal + 0x10, 12, proc);

	EndFaceIn8Frames(proc);
}

s32 Portrait32_GetOverlayOam2(struct FaceProc *proc)
{
	const struct PortraitSlot *st;

	if (proc == NULL || proc->faceSlot >= FACE_SLOT_COUNT || gFaces[proc->faceSlot] != proc)
		return -1;

	st = &sPortraitSlots[proc->faceSlot];
	if (st->mode != PORTRAIT_MODE_COLOR32 || st->extPal > 15)
		return -1;

	return st->extChr + st->extPal * 0x1000 + (proc->oam2 & 0x0C00);
}

/* Replaces sub_8006650 (FaceChange_LoadGfx, 0x08006650): [LoadFace] on a face already on screen. */
void FaceChange_LoadGfx_Portraits(struct UnkFaceProc *proc)
{
	struct FaceProc *face = proc->pFaceProc;
	struct PortraitSlot *st = &sPortraitSlots[face->faceSlot];
	int oldMode = st->mode;
	int mode;

	proc->pFaceInfo = GetPortraitData(proc->faceId);

	RestoreExtPalette(st);

	mode = LimitMode(face->faceSlot, WantedMode(proc->pFaceInfo, face->displayBits));
	if (mode != PORTRAIT_MODE_NORMAL
			&& CheckFit(face->faceSlot, st->resSlot, mode, NULL) != FIT_FREE)
		mode = PORTRAIT_MODE_NORMAL;

	st->mode = mode;
	face->pFaceInfo = proc->pFaceInfo;
	face->faceId = proc->faceId;

	LoadFaceGfx(face);
	ApplyPalette(proc->pFaceInfo->pal, (sFaceConfig[st->resSlot].paletteId & 0xF) + 0x10);

	if (mode == PORTRAIT_MODE_COLOR32)
		LoadExtPalette(st, proc->pFaceInfo, false);

	if (IsHalfBodyData(proc->pFaceInfo))
		StopMouthAndBlink(face);

	ShiftForHeight(face, oldMode, mode);
	FaceRefreshSprite(face);
}

/* --------------------------------------------------------------------------
 * Stat screen (and other 80x72 BG faces)
 * ------------------------------------------------------------------------ */

static void PutBgMouth(u16 *tm, int tileref, int x, int y)
{
	tm[TILEMAP_INDEX(x, y) + 0x00] = tileref + 0x1C;
	tm[TILEMAP_INDEX(x, y) + 0x01] = tileref + 0x1D;
	tm[TILEMAP_INDEX(x, y) + 0x02] = tileref + 0x1E;
	tm[TILEMAP_INDEX(x, y) + 0x03] = tileref + 0x1F;
	tm[TILEMAP_INDEX(x, y) + 0x20] = tileref + 0x20 + 0x1C;
	tm[TILEMAP_INDEX(x, y) + 0x21] = tileref + 0x20 + 0x1D;
	tm[TILEMAP_INDEX(x, y) + 0x22] = tileref + 0x20 + 0x1E;
	tm[TILEMAP_INDEX(x, y) + 0x23] = tileref + 0x20 + 0x1F;
}

/* PutFace80x72_Standard / _Raised with the half-body's wider head. */
static void PutHalfBodyBgFace(u16 *tm, int tileref, const struct FaceData *info, bool raised)
{
	CallARM_FillTileRect(tm, raised ? sHalfBodyStatTsaRaised : sHalfBodyStatTsa, (u16)tileref);
	PutBgMouth(tm, tileref, info->xMouth - 1, raised ? info->yMouth - 1 : info->yMouth);
}

/* Replaces PutFace80x72_Core (0x08005CA4). */
void PutFace80x72_Core_Portraits(u16 *tm, int fid, int chr, int pal)
{
	const struct FaceData *info;
	bool halfbody;
	int i;

	if (fid == 0)
		return;

	info = GetPortraitData(fid);
	halfbody = IsHalfBodyData(info);

	ApplyPalette(info->pal, pal);

	if (info->img == NULL) {
		Decompress(info->imgCard, (void *)(chr * CHR_SIZE + VRAM));
		PutAppliedBitmap(tm, (pal << 12) + (0x3FF & chr), 10, 9);
		return;
	}

	/*
	 * The stat screen puts its face at CHR 0x4E0.  A whole half-body sheet
	 * (0x100 tiles) goes at 0x480 instead, as in the C Skill System; other
	 * screens only get the top 32x4 tiles and the vanilla layout.
	 */
	if (halfbody && chr == STAT_SCREEN_FACE_CHR && info->img[0] == 0) {
		chr = STAT_SCREEN_HALFBODY_CHR;
		Decompress(info->img, (void *)(chr * CHR_SIZE + VRAM));
	} else if (halfbody && info->img[0] == 0) {
		CpuFastCopy(info->img + 4, (void *)(chr * CHR_SIZE + VRAM), FACE_VRAM_BLOCK);
	} else {
		Decompress(info->img, (void *)(chr * CHR_SIZE + VRAM));
	}

	if (halfbody && chr == STAT_SCREEN_HALFBODY_CHR) {
		PutHalfBodyBgFace(tm, (pal << 12) + (0x3FF & chr), info, ShouldFaceBeRaised(fid));
		return;
	}

	if (ShouldFaceBeRaised(fid))
		PutFace80x72_Raised(tm, (pal << 12) + (0x3FF & chr), info);
	else
		PutFace80x72_Standard(tm, (pal << 12) + (0x3FF & chr), info);

	for (i = 0; i < 5; i++) {
		tm[i * 0x20 + 0] = 0;
		tm[i * 0x20 + 9] = 0;
	}
}

/* --------------------------------------------------------------------------
 * Speech bubble tail
 * ------------------------------------------------------------------------ */

static bool IsSpeakerHalfBody(void)
{
	int slot = sTalkState->speakingFaceSlot;

	if (slot < 0 || slot >= 8)
		return false;

	return Portraits_IsHalfBodyFace(sTalkState->faces[slot]);
}

/*
 * A half-body speaker's bubble sits low on the screen (see StartTalkOpen),
 * so its tail points up at the face from just above the bubble.
 */
static const u16 sTalkBubbleTailUp[2][4] = {
	{ 0x3C16, 0x3C15, 0x3814, 0x3C14 },
	{ 0x3815, 0x3816, 0x3814, 0x3C14 },
};

static void PutTalkBubbleTailUp(int x, int y, int kind)
{
	u16 *buf = BG_GetMapBuffer(BG_1);
	const u16 *tiles = sTalkBubbleTailUp[kind];

	buf[TILEMAP_INDEX(x, y)] = tiles[0];
	buf[TILEMAP_INDEX(x + 1, y)] = tiles[1];
	buf[TILEMAP_INDEX(x, y + 1)] = tiles[2];
	buf[TILEMAP_INDEX(x + 1, y + 1)] = tiles[3];
}

/* Replaces PutTalkBubble (0x080082A4). */
void PutTalkBubble_Portraits(int xAnchor, int yAnchor, int width, int height)
{
	int y;
	int kind;
	int xTail = 0;
	int x = 0;

	BG_Fill(gBG1TilemapBuffer, 0);

	kind = xAnchor < 16 ? 0 : 1;

	if ((s8)IsBattleDeamonActive())
		kind += 2;

	y = (yAnchor - height) + 1;

	switch (kind) {
	case 0:
		xTail = xAnchor + 3;
		x = xTail - width / 2;
		if (x < 1)
			x = 1;
		break;

	case 1:
		xTail = xAnchor - 5;
		if ((width + 1) / 2 + xTail >= 30)
			x = 0x1D - width;
		else
			x = xTail - width / 2;
		break;

	case 2:
		x = 9;
		y = 14;
		width = 20;
		xTail = 8;
		yAnchor = 16;
		break;

	case 3:
		x = 1;
		y = 14;
		width = 20;
		xTail = 20;
		yAnchor = 16;
		break;
	}

	sTalkState->xText = x + 1;
	sTalkState->yText = y + 1;

	PutTalkBubbleTm(BG_1, x, y, width, height);

	if (sTalkState->invertedFlags & 2) {
		TalkToggleInvertedPalette(sTalkState->invertedFlags & 1);
		sTalkState->invertedFlags ^= 2;
	}

	if (!(sTalkState->invertedFlags & 1)) {
		if (kind < 2 && IsSpeakerHalfBody())
			PutTalkBubbleTailUp(xTail, y - 1, kind);
		else
			PutTalkBubbleTail(BG_1, xTail, yAnchor, kind);
	}

	InitTalkTextWin(x, y, width, height);
	StartOpenTalkBubble();
	TalkBgSync(2);
}
