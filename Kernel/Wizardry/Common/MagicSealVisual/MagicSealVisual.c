#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/video-global.h"

#include "bmmap.h"
#include "bmunit.h"
#include "bm.h"
#include "hardware.h"
#include "proc.h"
#include "event.h"

/**
 * Magic Seal visual.
 *
 * Fog palettes (banks 11-15) are never modified, so real fog stays standard.
 * Sealed tiles keep normal map coloring; a soft BG2 blend wash adds red only
 * on those tiles (avoids fog-dither spots showing through the wash).
 *
 * Cost: full redraw only when dirty or the camera crosses a map tile.
 * Sub-tile cursor scroll only updates BG2 position (same as vanilla BG3).
 */

enum {
	MAGIC_SEAL_FOG_BANK0 = BGPAL_TILESET + 5,
	MAGIC_SEAL_OVERLAY_PAL = BGPAL_LIMITVIEW + 1, /* bank 5 */
	/* Past move/attack range squares at 0x280-0x287 — do not share those CHRs. */
	MAGIC_SEAL_OVERLAY_CHR = BGCHR_LIMITVIEW + 8, /* 0x288 */
	MAGIC_SEAL_RED = 0x319D,
	MAGIC_SEAL_RANGE = 10,
	MAGIC_SEAL_MAX_SOURCES = 8,
};

struct MagicSealOverlayProc {
	/* 00 */ PROC_HEADER;
	/* 29 */ u8 active;
	/* 2A */ u8 dirty;
	/* 2C */ s16 lastCamTileX;
	/* 2E */ s16 lastCamTileY;
	/* 30 */ s16 lastOriginX;
	/* 32 */ s16 lastOriginY;
};

extern u16 sTilesetConfig[];

static void MagicSealOverlay_OnLoop(struct MagicSealOverlayProc *proc);

static const struct ProcCmd ProcScr_MagicSealOverlay[] = {
	PROC_NAME("MagicSealOverlay"),
	PROC_MARK(PROC_MARK_DISP),
	PROC_REPEAT(MagicSealOverlay_OnLoop),
	PROC_END,
};

static void EnsureSolidOverlayTiles(void)
{
	u32 solid[8];
	int i;

	for (i = 0; i < 8; i++)
		solid[i] = 0x11111111;

	for (i = 0; i < 4; i++)
		CpuFastCopy(solid, (void *)(VRAM + (MAGIC_SEAL_OVERLAY_CHR + i) * CHR_SIZE), sizeof(solid));
}

static void ApplySealOverlayPalette(void)
{
	u16 pal[16];
	u16 red = MAGIC_SEAL_RED;
	int i;

	if (gPaletteBuffer[MAGIC_SEAL_FOG_BANK0 * 0x10 + 5] != 0)
		red = gPaletteBuffer[MAGIC_SEAL_FOG_BANK0 * 0x10 + 5];

	pal[0] = 0;
	for (i = 1; i < 16; i++)
		pal[i] = red;

	ApplyPalette(pal, MAGIC_SEAL_OVERLAY_PAL);
	EnablePaletteSync();
}

static void ApplySealOverlayBlend(void)
{
	SetBackgroundTileDataOffset(BG_2, 0);
	SetBlendAlpha(8, 8);
	SetBlendTargetA(0, 0, 1, 0, 0);
	SetBlendBackdropA(0);
	SetBlendTargetB(0, 0, 0, 1, 1);
	SetBlendBackdropB(1);
}

static void SyncSealOverlayScroll(void)
{
	BG_SetPosition(BG_2,
		gBmSt.camera.x - gBmSt.mapRenderOrigin.x * 16,
		gBmSt.camera.y - gBmSt.mapRenderOrigin.y * 16);
}

static void ClearSealOverlay(struct MagicSealOverlayProc *proc)
{
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);
	SetBlendNone();
	proc->active = false;
	proc->dirty = false;
}

/* Gather seal sources once per redraw — avoids SkillTester × tile spam. */
static int CollectMagicSealSources(s8 *xs, s8 *ys)
{
	int i;
	int n = 0;

	if (EventEngineExists())
		return 0;

	for (i = 1; i < 0xC0; ++i) {
		struct Unit *unit = GetUnit(i);

		if (!UNIT_IS_VALID(unit) || (unit->state & US_HIDDEN))
			continue;

		if (!(UNIT_CATTRIBUTES(unit) & CA_MAGICSEAL)) {
#if defined(SID_MagicSeal) && (COMMON_SKILL_VALID(SID_MagicSeal))
			if (!SkillTester(unit, SID_MagicSeal))
				continue;
#else
			continue;
#endif
		}

		xs[n] = unit->xPos;
		ys[n] = unit->yPos;
		n++;

		if (n >= MAGIC_SEAL_MAX_SOURCES)
			break;
	}

	return n;
}

static bool IsSealedBySources(int x, int y, const s8 *xs, const s8 *ys, int n)
{
	int i;

	for (i = 0; i < n; ++i) {
		if (RECT_DISTANCE(xs[i], ys[i], x, y) <= MAGIC_SEAL_RANGE)
			return true;
	}

	return false;
}

static void DrawSealOverlayTiles(const s8 *xs, const s8 *ys, int n)
{
	int ix, iy;
	int xBmBase = gBmSt.camera.x >> 4;
	int yBmBase = gBmSt.camera.y >> 4;
	int xTileBase = (xBmBase - gBmSt.mapRenderOrigin.x) & 0xF;
	int yTileBase = (yBmBase - gBmSt.mapRenderOrigin.y) & 0xF;

	for (iy = 10; iy >= 0; --iy) {
		for (ix = 15; ix >= 0; --ix) {
			int xMap = xBmBase + ix;
			int yMap = yBmBase + iy;
			int xTile = (xTileBase + ix) & 0xF;
			int yTile = (yTileBase + iy) & 0xF;
			u16 *bg = gBG2TilemapBuffer + yTile * 0x40 + xTile * 2;

			if (xMap >= 0 && yMap >= 0 && xMap < gBmMapSize.x && yMap < gBmMapSize.y
				&& IsSealedBySources(xMap, yMap, xs, ys, n)) {
				bg[0x00 + 0] = TILEREF(MAGIC_SEAL_OVERLAY_CHR + 0, MAGIC_SEAL_OVERLAY_PAL);
				bg[0x00 + 1] = TILEREF(MAGIC_SEAL_OVERLAY_CHR + 1, MAGIC_SEAL_OVERLAY_PAL);
				bg[0x20 + 0] = TILEREF(MAGIC_SEAL_OVERLAY_CHR + 2, MAGIC_SEAL_OVERLAY_PAL);
				bg[0x20 + 1] = TILEREF(MAGIC_SEAL_OVERLAY_CHR + 3, MAGIC_SEAL_OVERLAY_PAL);
			} else {
				bg[0x00 + 0] = 0;
				bg[0x00 + 1] = 0;
				bg[0x20 + 0] = 0;
				bg[0x20 + 1] = 0;
			}
		}
	}

	BG_EnableSyncByMask(BG2_SYNC_BIT);
	SyncSealOverlayScroll();
}

static void MagicSealOverlay_OnLoop(struct MagicSealOverlayProc *proc)
{
	int camTileX;
	int camTileY;
	s8 xs[MAGIC_SEAL_MAX_SOURCES];
	s8 ys[MAGIC_SEAL_MAX_SOURCES];
	int n;

	/*
	 * Move/attack range owns BG2 (same tree). Do not BG_Fill / SetBlendNone
	 * here — that runs after MoveLimitView_OnInit and erases the blue tiles.
	 */
	if (gBmSt.gameStateBits & BM_FLAG_0) {
		proc->active = false;
		proc->dirty = true;
		return;
	}

	if (EventEngineExists()) {
		if (proc->active)
			ClearSealOverlay(proc);
		return;
	}

	camTileX = gBmSt.camera.x >> 4;
	camTileY = gBmSt.camera.y >> 4;

	/*
	 * Fast path: overlay already up and camera stayed on the same map tile —
	 * only nudge BG2 scroll (vanilla does the same for BG3).
	 */
	if (proc->active
		&& !proc->dirty
		&& proc->lastCamTileX == camTileX
		&& proc->lastCamTileY == camTileY
		&& proc->lastOriginX == gBmSt.mapRenderOrigin.x
		&& proc->lastOriginY == gBmSt.mapRenderOrigin.y) {
		ApplySealOverlayBlend();
		SyncSealOverlayScroll();
		return;
	}

	n = CollectMagicSealSources(xs, ys);
	if (n <= 0) {
		if (proc->active)
			ClearSealOverlay(proc);
		return;
	}

	if (!proc->active) {
		EnsureSolidOverlayTiles();
		ApplySealOverlayPalette();
		proc->active = true;
	} else if (proc->dirty) {
		EnsureSolidOverlayTiles();
		ApplySealOverlayPalette();
	}

	ApplySealOverlayBlend();
	DrawSealOverlayTiles(xs, ys, n);

	proc->lastCamTileX = camTileX;
	proc->lastCamTileY = camTileY;
	proc->lastOriginX = gBmSt.mapRenderOrigin.x;
	proc->lastOriginY = gBmSt.mapRenderOrigin.y;
	proc->dirty = false;
}

void UpdateMagicSealVisualPalette(void)
{
	struct MagicSealOverlayProc *proc;

	UnpackChapterMapPalette();

	if (!Proc_Find(ProcScr_MagicSealOverlay))
		Proc_Start(ProcScr_MagicSealOverlay, PROC_TREE_4);

	proc = Proc_Find(ProcScr_MagicSealOverlay);
	if (proc)
		proc->dirty = true;
}

LYN_REPLACE_CHECK(DisplayBmTile);
void DisplayBmTile(u16 *bg, int xTileMap, int yTileMap, int xBmMap, int yBmMap)
{
	u16 *out = bg + yTileMap * 0x40 + xTileMap * 2;
	u16 *tile = sTilesetConfig + gBmMapBaseTiles[yBmMap][xBmMap];
	/* Vanilla path only — seal red is the BG2 wash, so fog dither does not show through. */
	u16 base = gBmMapFog[yBmMap][xBmMap] ? (BGPAL_TILESET << 12) : (MAGIC_SEAL_FOG_BANK0 << 12);

	out[0x00 + 0] = base + *tile++;
	out[0x00 + 1] = base + *tile++;
	out[0x20 + 0] = base + *tile++;
	out[0x20 + 1] = base + *tile++;
}
