#include "common-chax.h"
#include "kernel-lib.h"
#include "face.h"
#include "bmlib.h"
#include "scene.h"
#include "bmmenu.h"
#include "menu_def.h"
#include "menuitempanel.h"

#define sFaceConfig ((struct FaceVramEntry *)0x0202A68C)
#define sTalkState (*(struct TalkState **)0x0859133C)

extern struct ProcCmd gProcScr_08591204[];

extern u16 LeftMugOAM[];
extern u16 RightMugOAM[];
extern u16 LeftMugCutOAM[];
extern u16 RightMugCutOAM[];
extern const u8 NewStatScreenPortraitTsa[];

static bool HalfBodyPortraitsEnabled(void)
{
	return gpKernelDesignerConfig->half_body_portraits != false;
}

/*
 * HalfbodyFormatter embeds chibi/pal/mouth as fixed offsets into the same
 * mug blob (see setMugEntry_Halfbody). Vanilla mugs use separate labels.
 */
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

static bool UseHalfBodyGeometry(const struct FaceData *info)
{
	return HalfBodyPortraitsEnabled() && IsHalfBodyFaceData(info);
}

/* Halfbody needs two palettes + extra CHR; slot 1 reuses slot 2's banks. */
static int HalfBodyVramSlot(int slot)
{
	if (HalfBodyPortraitsEnabled() && slot == 1)
		return 2;

	return slot;
}

static void HalfBodyStopFaceOverlays(struct FaceProc *proc)
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

static void StartFaceChange_HalfBody(struct FaceProc *parent, int fid)
{
	sub_80066E0(parent, fid);
}

static int ItemMenuFaceDisplay(void)
{
	return HalfBodyPortraitsEnabled()
		? FACE_DISP_KIND(FACE_64x72)
		: FACE_DISP_KIND(FACE_96x80);
}

static int ItemMenuPanelX(void)
{
	return HalfBodyPortraitsEnabled() ? 0x0D : 0x0F;
}

LYN_REPLACE_CHECK(ItemCommandEffect);
u8 ItemCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem)
{
	struct MenuProc *proc;

	if (menuItem->availability != MENU_ENABLED)
		return 0;

	ResetIconGraphics();
	LoadIconPalettes(4);
	ResetTextFont();

	proc = StartOrphanMenu(&gItemSelectMenuDef);
	StartFace(0, GetUnitPortraitId(gActiveUnit), 0xB0, 0x0C, ItemMenuFaceDisplay());
	SetFaceBlinkControlById(0, 5);
	ForceMenuItemPanel(proc, gActiveUnit, ItemMenuPanelX(), 0x0B);

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

LYN_REPLACE_CHECK(sub_8023550);
u8 sub_8023550(struct MenuProc *menu)
{
	struct MenuProc *proc;

	sub_8023538(menu);
	MenuCommand_SelectNo(menu);

	proc = StartOrphanMenu(&gItemSelectMenuDef);
	StartFace(0, GetUnitPortraitId(gActiveUnit), 0xB0, 0x0C, ItemMenuFaceDisplay());
	SetFaceBlinkControlById(0, 5);
	ForceMenuItemPanel(proc, gActiveUnit, ItemMenuPanelX(), 0x0B);

	return MENU_ENABLED;
}

LYN_REPLACE_CHECK(sub_80235A8);
u8 sub_80235A8(struct MenuProc *menu)
{
	struct MenuProc *proc;

	sub_8023538(menu);

	if (GetUnitItemCount(gActiveUnit) == 0) {
		ClearBg0Bg1();
		EndFaceById(0);
		StartSemiCenteredOrphanMenu(
			&gUnitActionMenuDef,
			gBmSt.cursorTarget.x - gBmSt.camera.x,
			1,
			0x16
		);
		return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6B | MENU_ACT_CLEAR;
	}

	TileMap_CopyRect(gUiTmScratchA, gBG0TilemapBuffer + 0x2B, 9, 0x13);
	TileMap_CopyRect(gUiTmScratchB, gBG1TilemapBuffer + 0x2B, 9, 0x13);
	TileMap_FillRect(gBG0TilemapBuffer + 0x2B - 0xA, 0xE, 0xC, 0);
	TileMap_FillRect(gBG1TilemapBuffer + 0x2B - 0xA, 0xD, 0xC, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

	proc = StartOrphanMenu(&gItemSelectMenuDef);
	StartFace(0, GetUnitPortraitId(gActiveUnit), 0xB0, 0x0C, ItemMenuFaceDisplay());
	SetFaceBlinkControlById(0, 5);
	ForceMenuItemPanel(proc, gActiveUnit, ItemMenuPanelX(), 0x0B);

	return MENU_ACT_SKIPCURSOR;
}

/*
 * Original halfbody ORG 0x5584/0x5586: walk face slots by 2 so talk only
 * uses slots 0 and 2 (palette 6+7 and 8+9) and never the odd banks.
 */
LYN_REPLACE_CHECK(FindFreeFaceSlot);
int FindFreeFaceSlot(void)
{
	int i;
	int step = HalfBodyPortraitsEnabled() ? 2 : 1;

	for (i = 0; i < FACE_SLOT_COUNT; i += step) {
		if (gFaces[i] == NULL)
			return i;
	}

	return -1;
}

LYN_REPLACE_CHECK(StartFace);
struct FaceProc *StartFace(int slot, int fid, int x, int y, int disp)
{
	struct FaceProc *proc;
	const struct FaceData *info;
	int vramSlot = HalfBodyVramSlot(slot);
	int paletteSize;
	bool halfbody;

	if (gFaces[slot] != NULL)
		return NULL;

	proc = Proc_Start(gProcScr_E_FACE, PROC_TREE_5);
	gFaces[slot] = proc;

	info = GetPortraitData(fid);
	halfbody = UseHalfBodyGeometry(info);
	/*
	 * Halfbodies are authored as 32 colours across two consecutive OBJ
	 * banks (original ORG 0x56C2 BYTE 0x40). Even-slot allocation keeps
	 * pal+1 reserved for that second bank.
	 */
	paletteSize = halfbody ? 0x40 : 0x20;

	if (disp & FACE_DISP_BIT_13) {
		CpuFastFill(0, PAL_OBJ(0) + PAL_OFFSET(sFaceConfig[vramSlot].paletteId), paletteSize);
		EnablePaletteSync();
	} else {
		/* CopyToPaletteBuffer takes a byte offset (0x20 per palette), not PAL_OFFSET. */
		CopyToPaletteBuffer(
			info->pal,
			0x20 * (sFaceConfig[vramSlot].paletteId + 0x10),
			paletteSize
		);
	}

	proc->pFaceInfo = info;
	proc->faceSlot = slot;
	proc->faceId = fid;
	proc->spriteLayer = 5;
	proc->xPos = x;
	proc->yPos = y;

	if (disp & FACE_DISP_BIT_12) {
		proc->unk_44 = NULL;
		proc->pBlinkProc = NULL;
	} else {
		/*
		 * Mouth/eye frames are baked into the halfbody sheet. Vanilla
		 * FaceMouth/FaceBlink still overlay CHR every frame and ignore
		 * HIDDEN, which flashes garbage colours on fade-out.
		 */
		if (halfbody) {
			proc->unk_44 = NULL;
			proc->pBlinkProc = NULL;
		} else {
			proc->unk_44 = Proc_Start(gProcScr_08591204, proc);
			proc->pBlinkProc = Proc_Start(gProcScr_FaceBlink, proc);
		}
	}

	proc->displayBits = ~disp;
	SetFaceDisplayBits(proc, disp);

	return proc;
}

LYN_REPLACE_CHECK(Face_OnInit);
void Face_OnInit(struct FaceProc *proc)
{
	int vramSlot = HalfBodyVramSlot(proc->faceSlot);

	Decompress(
		proc->pFaceInfo->img,
		(void *)(sFaceConfig[vramSlot].tileOffset + 0x06010000)
	);
}

LYN_REPLACE_CHECK(FaceRefreshSprite);
void FaceRefreshSprite(struct FaceProc *proc)
{
	int oam2Layer;
	int vramSlot = HalfBodyVramSlot(proc->faceSlot);
	bool halfbody = UseHalfBodyGeometry(proc->pFaceInfo);

	switch (proc->displayBits & 0x807) {
	case 0:
		proc->sprite = gSprite_Face64x96;
		break;

	case 1:
		proc->sprite = halfbody
			? LeftMugOAM
			: gSprite_Face64x96_Flipped;
		break;

	case 2:
		proc->sprite = halfbody
			? RightMugOAM
			: gSprite_Face96x96;
		break;

	case 3:
		proc->sprite = halfbody
			? LeftMugOAM
			: gSprite_Face96x96_Flipped;
		break;

	case 4:
		proc->sprite = halfbody
			? RightMugCutOAM
			: gSprite_Face80x72;
		break;

	case 5:
		proc->sprite = halfbody
			? LeftMugCutOAM
			: gSprite_Face80x72_Flipped;
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

	proc->oam2 =
		(sFaceConfig[vramSlot].tileOffset / CHR_SIZE)
		+ ((sFaceConfig[vramSlot].paletteId & 0xF) * 0x1000)
		+ oam2Layer;
}

LYN_REPLACE_CHECK(StartFaceFadeIn);
void StartFaceFadeIn(struct FaceProc *proc)
{
	const struct FaceData *info = GetPortraitData(proc->faceId);
	int pal = ((proc->oam2 >> 12) & 0xF) + 0x10;

	SetBlackPal(pal);
	StartPalFade(info->pal, pal, 12, proc);

	if (UseHalfBodyGeometry(info)) {
		SetBlackPal(pal + 1);
		StartPalFade(info->pal + 0x10, pal + 1, 12, proc);
	}
}

struct HbFaceEndProc {
	/* 00 */ PROC_HEADER;
	/* 2C */ struct FaceProc *face;
	/* 30 */ u8 clock;
};

static void HbFaceEnd_OnLoop(struct HbFaceEndProc *aproc)
{
	struct FaceProc *face = aproc->face;

	if (face == NULL || gFaces[face->faceSlot] != face) {
		Proc_Break(aproc);
		return;
	}

	if (aproc->clock >= 8) {
		EndFace(face);
		Proc_Break(aproc);
		return;
	}

	aproc->clock++;
}

static struct ProcCmd const sProcScr_HbFaceEndAfterFade[] = {
	PROC_REPEAT(HbFaceEnd_OnLoop),
	PROC_END,
};

/*
 * Hide halfbody faces before talk teardown touches the screen.
 * The neon flash happens on the first palette-fade frame; skipping
 * StartPalFade and hiding immediately avoids it.
 */
void HalfBody_OnTalkFaceClear(struct FaceProc *proc)
{
	const struct FaceData *info;

	if (proc == NULL)
		return;

	info = GetPortraitData(proc->faceId);
	if (!UseHalfBodyGeometry(info))
		return;

	HalfBodyStopFaceOverlays(proc);
	/* Do not call SetFaceDisplayBits — that refreshes OAM. */
	proc->displayBits |= FACE_DISP_HIDDEN;
}

LYN_REPLACE_CHECK(StartFaceFadeOut);
void StartFaceFadeOut(struct FaceProc *proc)
{
	const struct FaceData *info = GetPortraitData(proc->faceId);
	int pal = ((proc->oam2 >> 12) & 0xF) + 0x10;

	if (UseHalfBodyGeometry(info)) {
		struct HbFaceEndProc *end;

		HalfBody_OnTalkFaceClear(proc);

		/*
		 * No StartPalFade: writing the OBJ palette at fade-start is what
		 * flashes black/neon. Keep the face hidden and tear it down on the
		 * usual EndFaceIn8Frames schedule.
		 */
		end = Proc_Start(sProcScr_HbFaceEndAfterFade, PROC_TREE_3);
		end->face = proc;
		end->clock = 0;
		return;
	}

	StartPalFadeToBlack(pal, 12, proc);
	EndFaceIn8Frames(proc);
}

/* FaceChange_LoadGfx */
LYN_REPLACE_CHECK(sub_8006650);
void sub_8006650(struct UnkFaceProc *proc)
{
	struct FaceProc *faceProc;
	int vramSlot;
	int paletteSize;

	proc->pFaceInfo = GetPortraitData(proc->faceId);
	vramSlot = HalfBodyVramSlot(proc->pFaceProc->faceSlot);
	paletteSize = UseHalfBodyGeometry(proc->pFaceInfo) ? 0x40 : 0x20;

	Decompress(
		proc->pFaceInfo->img,
		(void *)(sFaceConfig[vramSlot].tileOffset + 0x06010000)
	);

	CopyToPaletteBuffer(
		proc->pFaceInfo->pal,
		0x20 * (sFaceConfig[vramSlot].paletteId + 0x10),
		paletteSize
	);

	faceProc = proc->pFaceProc;
	faceProc->pFaceInfo = proc->pFaceInfo;
	faceProc->faceId = proc->faceId;
}

LYN_REPLACE_CHECK(TalkLoadFace);
void TalkLoadFace(ProcPtr proc)
{
	int faceDisp = 0;
	int faceId;
	int y = HalfBodyPortraitsEnabled() ? 0x20 : 0x50;

	if (sTalkState->activeFaceSlot == 0xFF)
		SetActiveTalkFace(1);

	if ((s8)IsBattleDeamonActive())
		SetupFaceGfxDataInBanim();
	else
		faceDisp |= FACE_DISP_KIND(FACE_96x80);

	if (GetTalkFaceHPos(sTalkState->activeFaceSlot) <= 14)
		faceDisp |= FACE_DISP_FLIPPED;

	faceId = sTalkState->str[0];
	faceId = (sTalkState->str[1] * 0x100) + faceId;

	if (faceId == 0xFFFF)
		faceId = GetUnitPortraitId(gActiveUnit);
	else
		faceId -= 0x100;

	if (sTalkState->faces[sTalkState->activeFaceSlot] != NULL) {
		StartFaceChange_HalfBody(sTalkState->faces[sTalkState->activeFaceSlot], faceId);
		return;
	}

	sTalkState->faces[sTalkState->activeFaceSlot] = StartFaceAuto(
		faceId,
		GetTalkFaceHPos(sTalkState->activeFaceSlot) * 8,
		y,
		faceDisp
	);

	StartFaceFadeIn(sTalkState->faces[sTalkState->activeFaceSlot]);
	SetTalkFaceLayer(sTalkState->activeFaceSlot, CheckTalkFlag(TALK_FLAG_4));
	StartTemporaryLock(proc, 8);
}

#define TILEMAP_INDEX_UNK(x, y) ((x) + ((y) << 5))

LYN_REPLACE_CHECK(PutTalkBubble);
void PutTalkBubble(int xAnchor, int yAnchor, int width, int height)
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
		int tailY = yAnchor;

		/* Original halfbody ORG 0x83AE: force bubble-tail Y to 0x0C. */
		if (HalfBodyPortraitsEnabled() && kind < 2)
			tailY = 0x0C;

		PutTalkBubbleTail(BG_1, xTail, tailY, kind);
	}

	InitTalkTextWin(x, y, width, height);
	StartOpenTalkBubble();
	TalkBgSync(2);
}

/* Vanilla bubble-tail tile refs. */
static const u16 sTalkBubbleTailTilesVanilla[6][4] = {
	{
		TILEREF(0x14, 3),
		TILEREF(0x14, 3) + 0x400,
		TILEREF(0x16, 3) + 0x400,
		TILEREF(0x15, 3) + 0x400,
	},
	{
		TILEREF(0x14, 3),
		TILEREF(0x14, 3) + 0x400,
		TILEREF(0x15, 3),
		TILEREF(0x16, 3),
	},
	{
		TILEREF(0x18, 3) + 0x400,
		TILEREF(0x19, 3) + 0x400,
		TILEREF(0x17, 3) + 0x400,
		TILEREF(0x17, 3) + 0x400 + 0x800,
	},
	{
		TILEREF(0x17, 3),
		TILEREF(0x17, 3) + 0x800,
		TILEREF(0x18, 3),
		TILEREF(0x19, 3),
	},
	{
		TILEREF(0x19, 3) + 0x400 + 0x800,
		TILEREF(0x18, 3) + 0x400 + 0x800,
		TILEREF(0x17, 3) + 0x400,
		TILEREF(0x17, 3) + 0x400 + 0x800,
	},
	{
		TILEREF(0x17, 3),
		TILEREF(0x17, 3) + 0x800,
		TILEREF(0x19, 3) + 0x800,
		TILEREF(0x18, 3) + 0x800,
	},
};

/*
 * Halfbody remaps from the original ORG 0x8540 / 0x8544 / 0x8578 patches.
 * Kind 2-5 keep vanilla (battle-demo tails were not patched).
 */
static const u16 sTalkBubbleTailTilesHalfBody[6][4] = {
	{
		0x3C16,
		0x3C15,
		0x3814,
		0x3C14,
	},
	{
		0x3815,
		0x3816,
		0x3814,
		0x3C14,
	},
	{
		TILEREF(0x18, 3) + 0x400,
		TILEREF(0x19, 3) + 0x400,
		TILEREF(0x17, 3) + 0x400,
		TILEREF(0x17, 3) + 0x400 + 0x800,
	},
	{
		TILEREF(0x17, 3),
		TILEREF(0x17, 3) + 0x800,
		TILEREF(0x18, 3),
		TILEREF(0x19, 3),
	},
	{
		TILEREF(0x19, 3) + 0x400 + 0x800,
		TILEREF(0x18, 3) + 0x400 + 0x800,
		TILEREF(0x17, 3) + 0x400,
		TILEREF(0x17, 3) + 0x400 + 0x800,
	},
	{
		TILEREF(0x17, 3),
		TILEREF(0x17, 3) + 0x800,
		TILEREF(0x19, 3) + 0x800,
		TILEREF(0x18, 3) + 0x800,
	},
};

LYN_REPLACE_CHECK(PutTalkBubbleTail);
void PutTalkBubbleTail(int bg, int x, int y, int kind)
{
	u16 *buf = BG_GetMapBuffer(bg);
	const u16 (*table)[4] = HalfBodyPortraitsEnabled()
		? sTalkBubbleTailTilesHalfBody
		: sTalkBubbleTailTilesVanilla;
	const u16 *tiles = table[kind];

	buf[TILEMAP_INDEX_UNK(x, y)] = tiles[0];
	buf[TILEMAP_INDEX_UNK(x + 1, y)] = tiles[1];
	buf[TILEMAP_INDEX_UNK(x, y + 1)] = tiles[2];
	buf[TILEMAP_INDEX_UNK(x + 1, y + 1)] = tiles[3];
}

LYN_REPLACE_CHECK(PutFace80x72_Raised);
void PutFace80x72_Raised(u16 *tm, int tileref, const struct FaceData *info)
{
	int x = info->xMouth - 1;
	int y = info->yMouth - 1;
	const u8 *tsa = UseHalfBodyGeometry(info)
		? NewStatScreenPortraitTsa
		: gUnknown_085A08F0;

	CallARM_FillTileRect(tm, tsa, (u16)tileref);

	tm[TILEMAP_INDEX(x, y) + 0x00] = tileref + 0x1C;
	tm[TILEMAP_INDEX(x, y) + 0x01] = tileref + 0x1D;
	tm[TILEMAP_INDEX(x, y) + 0x02] = tileref + 0x1E;
	tm[TILEMAP_INDEX(x, y) + 0x03] = tileref + 0x1F;
	tm[TILEMAP_INDEX(x, y) + 0x20] = tileref + 0x20 + 0x1C;
	tm[TILEMAP_INDEX(x, y) + 0x21] = tileref + 0x20 + 0x1D;
	tm[TILEMAP_INDEX(x, y) + 0x22] = tileref + 0x20 + 0x1E;
	tm[TILEMAP_INDEX(x, y) + 0x23] = tileref + 0x20 + 0x1F;
}
