#include "common-chax.h"
#include "kernel-lib.h"
#include "face.h"
#include "bmlib.h"
#include "hardware.h"
#include "ctc.h"
#include "proc.h"

#define FACE_SLOT_MAX 4
#define OVERLAY_ACTIVE_MAX 2
#define PALETTE_COLORS 16
#define PALETTE_BYTES 0x20
#define OVERLAY_BYTES 0x1000
#define STOLEN_PAL_NONE 0xFF
#define PORTRAIT32_MAGIC 0x50323343 /* P23C */
#define sFaceConfig ((struct FaceVramEntry *)0x0202A68C)

struct Portrait32Slot {
	const u16 *pal1;
	u16 savedPal[PALETTE_COLORS];
	u16 overlayChr;
	u8 overlayPal;
	u8 active;
};

struct Portrait32State {
	struct Portrait32Slot slots[FACE_SLOT_MAX];
	u32 magic;
};

extern struct Portrait32State sPortrait32State;

static bool Portrait32ColorEnabled(void)
{
	return gpKernelDesignerConfig->portrait_32_color != false;
}

static bool Is32ColorFaceData(const struct FaceData *info)
{
	if (info == NULL || info->pal == NULL || info->imgCard == NULL)
		return false;

	return info->imgCard == (const u8 *)(info->pal + PALETTE_COLORS);
}

static bool IsHalfBodyFaceData(const struct FaceData *info)
{
	const u8 *img;
	const u8 *chibi;

	if (info == NULL || info->img == NULL)
		return false;

	img = info->img;
	chibi = (const u8 *)info->imgChibi;

	return chibi == img + 0x2648 || chibi == img + 0x2644;
}

static int FaceVramSlot(int slot)
{
	if (gpKernelDesignerConfig->half_body_portraits != false && slot == 1)
		return 2;

	return slot;
}

/* Pair 0-1 and 2-3 so pal1 lives in the odd face bank (its own OBJ rows). */
static int OverlayVramSlot(int faceSlot)
{
	return faceSlot ^ 1;
}

static void Portrait32_EnsureInited(void)
{
	int i;

	if (sPortrait32State.magic == PORTRAIT32_MAGIC)
		return;

	CpuFill16(0, &sPortrait32State, sizeof(sPortrait32State));
	sPortrait32State.magic = PORTRAIT32_MAGIC;

	for (i = 0; i < FACE_SLOT_MAX; i++)
		sPortrait32State.slots[i].overlayPal = STOLEN_PAL_NONE;
}

static int Portrait32_ActiveCount(void)
{
	int i;
	int n = 0;

	for (i = 0; i < FACE_SLOT_MAX; i++) {
		if (sPortrait32State.slots[i].active == TRUE)
			n++;
	}

	return n;
}

static void Portrait32_RestoreOverlayPal(struct Portrait32Slot *slot)
{
	if (slot->overlayPal >= 0x10)
		return;

	CopyToPaletteBuffer(
		slot->savedPal,
		(slot->overlayPal + 0x10) * PALETTE_BYTES,
		PALETTE_BYTES
	);
	slot->overlayPal = STOLEN_PAL_NONE;
	slot->overlayChr = 0;
}

void Portrait32_LoadOverlayGfx(struct FaceProc *proc)
{
	struct Portrait32Slot *slot;
	const u8 *overlay;
	int overlaySlot;

	if (proc == NULL || proc->faceSlot >= FACE_SLOT_MAX)
		return;

	Portrait32_EnsureInited();
	slot = &sPortrait32State.slots[proc->faceSlot];

	if (slot->active != TRUE || slot->overlayPal >= 0x10 || slot->pal1 == NULL)
		return;

	overlaySlot = OverlayVramSlot(proc->faceSlot);
	overlay = (const u8 *)(slot->pal1 + PALETTE_COLORS);
	CpuCopy16(
		overlay,
		(void *)(sFaceConfig[overlaySlot].tileOffset + 0x06010000),
		OVERLAY_BYTES
	);
	slot->overlayChr = (u16)(sFaceConfig[overlaySlot].tileOffset / CHR_SIZE);
}

void Portrait32_BindFace(int faceSlot, const struct FaceData *info, int palIndex)
{
	struct Portrait32Slot *slot;
	int overlaySlot;
	int overlayPal;

	Portrait32_EnsureInited();

	if (faceSlot < 0 || faceSlot >= FACE_SLOT_MAX)
		return;

	Portrait32_UnbindFace(faceSlot);

	if (!Portrait32ColorEnabled() || !Is32ColorFaceData(info) || palIndex < 0 || palIndex >= 0x20)
		return;

	if (gpKernelDesignerConfig->half_body_portraits != false
			&& IsHalfBodyFaceData(info))
		return;

	if (Portrait32_ActiveCount() >= OVERLAY_ACTIVE_MAX)
		return;

	overlaySlot = OverlayVramSlot(faceSlot);
	if (overlaySlot == FaceVramSlot(faceSlot))
		return;

	if (gFaces[overlaySlot] != NULL)
		return;

	overlayPal = sFaceConfig[overlaySlot].paletteId & 0xF;
	slot = &sPortrait32State.slots[faceSlot];
	CpuCopy16(
		gPaletteBuffer + (overlayPal + 0x10) * PALETTE_COLORS,
		slot->savedPal,
		PALETTE_BYTES
	);
	slot->pal1 = info->pal + PALETTE_COLORS;
	CopyToPaletteBuffer(slot->pal1, (overlayPal + 0x10) * PALETTE_BYTES, PALETTE_BYTES);
	slot->overlayPal = (u8)overlayPal;
	slot->overlayChr = (u16)(sFaceConfig[overlaySlot].tileOffset / CHR_SIZE);
	slot->active = TRUE;
}

void Portrait32_UnbindFace(int faceSlot)
{
	struct Portrait32Slot *slot;

	Portrait32_EnsureInited();

	if (faceSlot < 0 || faceSlot >= FACE_SLOT_MAX)
		return;

	slot = &sPortrait32State.slots[faceSlot];
	Portrait32_RestoreOverlayPal(slot);
	slot->active = FALSE;
	slot->pal1 = NULL;
}

void Portrait32_OnFadeIn(struct FaceProc *proc)
{
	struct Portrait32Slot *slot;
	int pal;

	if (proc == NULL)
		return;

	Portrait32_EnsureInited();

	if (proc->faceSlot >= FACE_SLOT_MAX)
		return;

	slot = &sPortrait32State.slots[proc->faceSlot];
	if (slot->active != TRUE || slot->overlayPal >= 0x10 || slot->pal1 == NULL)
		return;

	pal = slot->overlayPal + 0x10;
	SetBlackPal(pal);
	StartPalFade(slot->pal1, pal, 12, proc);
}

LYN_REPLACE_CHECK(Face_OnIdle);
void Face_OnIdle(struct FaceProc *proc)
{
	int oam0;
	struct Portrait32Slot *slot;

	if (GetFaceDisplayBits(proc) & FACE_DISP_HIDDEN)
		return;

	if (GetFaceDisplayBits(proc) & FACE_DISP_BLEND)
		oam0 = OAM0_BLEND;
	else
		oam0 = 0;

	PutSpriteExt(
		proc->spriteLayer,
		0x1FF & proc->xPos,
		oam0 + OAM0_Y(proc->yPos),
		proc->sprite,
		proc->oam2
	);

	Portrait32_EnsureInited();

	if (proc->faceSlot >= FACE_SLOT_MAX)
		return;

	slot = &sPortrait32State.slots[proc->faceSlot];
	if (slot->active != TRUE || slot->overlayPal >= 0x10)
		return;

	PutSpriteExt(
		proc->spriteLayer,
		0x1FF & proc->xPos,
		oam0 + OAM0_Y(proc->yPos),
		proc->sprite,
		slot->overlayChr
			+ (slot->overlayPal * 0x1000)
			+ (proc->oam2 & 0x0C00)
	);
}

LYN_REPLACE_CHECK(EndFace);
void EndFace(struct FaceProc *proc)
{
	if (proc == NULL)
		return;

	Portrait32_UnbindFace(proc->faceSlot);
	gFaces[proc->faceSlot] = NULL;
	Proc_End(proc);
}
