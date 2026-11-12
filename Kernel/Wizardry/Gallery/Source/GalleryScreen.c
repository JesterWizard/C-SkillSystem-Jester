#include "common-chax.h"
#include "kernel-lib.h"
#include "event.h"
#include "eventscript.h"
#include "gallery.h"
#include "constants/texts.h"
#include "constants/songs.h"

/*
 * The Gallery screen (Documentation/Features/Gallery.md), started from Extras > Gallery.
 *
 * The grid: 9 CGs to a page, 3x3. Each is a 48x32 thumbnail (a fifth of the CG) with a frame, its name above; the
 * frame of the one under the cursor is gold and glows. A locked CG is dimmed, with a padlock, and "???" above.
 * A on an unlocked CG shows it full size; left and right go through the other unlocked ones; B goes back.
 *
 * The thumbnails are made by drawing each CG with the game's own loader (EventShowTextBgDirect, which also takes the
 * 224- and 256-colour CGs), with the screen dark, then reading its pixels back from VRAM and the palette, four to a
 * thumbnail pixel. They are 256-colour sprites: the OBJ palette holds a 6x6x6 colour cube (colours 16-231), and each
 * pixel is put on the cube with a 2x2 ordered dither. The loader only writes the BG side, so the sprites outlive the
 * full view; only a new page makes new thumbnails.
 */

#define GALLERY_COLS     3
#define GALLERY_PER_PAGE 9
#define THUMB_W          48
#define THUMB_H          32
#define THUMB_UNITS      48 /* OBJ VRAM 32-byte units a thumbnail takes: 6x4 tiles of 64 bytes */
#define LOCK_TILE        (GALLERY_PER_PAGE * THUMB_UNITS)
#define FRAME_TILE       0x200 /* BG VRAM 0x06004000, after the text's tiles */
#define PAL_FRAME        13
#define PAL_FRAME_HI     14
#define PAL_OBJ_LOCK     15
#define CUBE_BASE        16

#define SE_MOVE  SONG_SE_SYS_CURSOR_UD1
#define SE_PAGE  SONG_SE_SYS_CURSOR_LR1
#define SE_OK    SONG_SE_SYS_WINDOW_SELECT1
#define SE_BAD   SONG_6C
#define SE_LEAVE SONG_SE_SYS_WINDOW_CANSEL1

struct GalleryScreen {
	/* 00 */ u8 count;
	/* 01 */ u8 cursor;
	/* 02 */ u8 page;	/* the page whose thumbnails are in OBJ VRAM */
	/* 03 */ u8 view;	/* the CG the full view shows */
	/* 04 */ u8 built;	/* thumbnails made so far for this page */
	/* 05 */ u8 sprites;	/* the grid's thumbnails are on screen (they stay through its fades) */
	/* 06 */ u16 timer;
	/* 08 */ u8 bits[GALLERY_MAX / 8];
	/* 14 */ struct Text names[GALLERY_PER_PAGE];
	/* 5C */ struct Text footer;
};

extern struct GalleryScreen gGalleryScreen;
#define GS (&gGalleryScreen)

_Static_assert(sizeof(struct GalleryScreen) <= 0x80, "gGalleryScreen is 0x80 bytes (config-memmap.s)");

extern const u8 Img_GalleryFrame[];
extern const u8 Img_GalleryLock[];
extern const u16 Pal_GalleryLock[];

struct ProcGallery {
	PROC_HEADER;
	struct DispCnt dispBak;
};

enum {
	L_GALLERY_PAGE = 1,
	L_GALLERY_GRID,
	L_GALLERY_TURN,
	L_GALLERY_VIEW,
	L_GALLERY_BACK,
	L_GALLERY_EXIT,
};

/* BG3 takes the CGs; BG0 text, BG1 the frames */
static const u16 sGalleryBgConfig[12] = {
	0x0000, 0x6000, 0,
	0x0000, 0x6800, 0,
	0x0000, 0x7000, 0,
	0x8000, 0x7800, 0,
};

static const u16 sFramePal[16] = { 0, RGB(3, 4, 8), RGB(13, 15, 21) };
static const u8 sCubeLevel[6] = { 0, 6, 12, 19, 25, 31 };

/* A 48x32 thumbnail: a 32x32 and a 16x32 sprite, 256 colours */
static const u16 sObjThumb[] = {
	2,
	0x2000, 0x8000, 0x0800,
	0xA000, 0x8000 + 32, 0x0800 + 32,
};

static const u16 sObjLock[] = {
	1,
	0x0000, 0x4000, 0x0400 | (PAL_OBJ_LOCK << 12) | LOCK_TILE,
};

/* A slot's column and row on the page (no %: it is broken in this build, see CLAUDE.md) */
static int Gallery_Col(int slot)
{
	return DivRem(slot, GALLERY_COLS);
}

static int Gallery_Row(int slot)
{
	return Div(slot, GALLERY_COLS);
}

static int Gallery_PageCount(void)
{
	return GS->count == 0 ? 1 : (GS->count + GALLERY_PER_PAGE - 1) / GALLERY_PER_PAGE;
}

static int Gallery_OnPage(int page)
{
	int n = GS->count - page * GALLERY_PER_PAGE;

	return n > GALLERY_PER_PAGE ? GALLERY_PER_PAGE : (n < 0 ? 0 : n);
}

static bool Gallery_Unlocked(int index)
{
	return Gallery_IsUnlocked(GS->bits, index);
}

/* ---- Thumbnails ---- */

/* A pixel of the CG the loader just drew on BG3: its tile map (RAM), tiles (VRAM) and palette */
static u16 Gallery_CgPixel(int x, int y, const u8 *chr, bool c256)
{
	u16 entry = gBG3TilemapBuffer[(y >> 3) * 32 + (x >> 3)];
	int tile = entry & 0x3FF;
	int px = x & 7, py = y & 7, ci;

	if (entry & 0x400)
		px = 7 - px;
	if (entry & 0x800)
		py = 7 - py;

	if (c256)
		return gPaletteBuffer[chr[tile * 64 + py * 8 + px]];

	ci = (chr[tile * 32 + py * 4 + (px >> 1)] >> ((px & 1) * 4)) & 0xF;
	return ci ? gPaletteBuffer[(entry >> 12) * 16 + ci] : gPaletteBuffer[0];
}

/* A channel (0-31, x4 for four samples) on the cube's 6 levels, with a 2x2 ordered dither: (c * 5 + d) / 31 */
static int Gallery_Level(int sum4, int dither)
{
	return ((sum4 * 5 + dither * 31) * 529) >> 16;
}

static u8 *Gallery_ThumbPixelAddr(int slot, int x, int y)
{
	u8 *base = (u8 *)0x06010000 + slot * THUMB_UNITS * 32;

	if (x < 32)
		return base + ((y >> 3) * 4 + (x >> 3)) * 64 + (y & 7) * 8 + (x & 7);

	x -= 32;
	return base + 1024 + ((y >> 3) * 2 + (x >> 3)) * 64 + (y & 7) * 8 + (x & 7);
}

NOINLINE static void Gallery_MakeThumb(int slot, int index)
{
	static const u8 dither[2][2] = { { 0, 2 }, { 3, 1 } };
	const u8 *chr;
	bool c256, dim = !Gallery_Unlocked(index);
	int x, y, k;

	SetupBackgrounds((u16 *)sGalleryBgConfig);
	gLCDControlBuffer.bg3cnt.colorMode = 0;
	SetDispEnable(0, 0, 0, 0, 0);

	EventShowTextBgDirect(EVSUBCMD_REMOVEPORTRAITS, gGalleryEntries[index].bg);

	c256 = gLCDControlBuffer.bg3cnt.colorMode;
	chr = (const u8 *)VRAM + GetBackgroundTileDataOffset(3);

	for (y = 0; y < THUMB_H; y++) {
		for (x = 0; x < THUMB_W; x += 2) {
			u8 out[2];

			for (k = 0; k < 2; k++) {
				int sx = (x + k) * 5 + 1, sy = y * 5 + 1;
				int r = 0, g = 0, b = 0, i;

				for (i = 0; i < 4; i++) {
					u16 c = Gallery_CgPixel(sx + (i & 1) * 2, sy + (i >> 1) * 2, chr, c256);

					r += c & 0x1F;
					g += (c >> 5) & 0x1F;
					b += (c >> 10) & 0x1F;
				}

				if (dim) {
					r = (r * 5) >> 4;
					g = (g * 5) >> 4;
					b = (b * 5) >> 4;
				}

				i = dither[y & 1][(x + k) & 1];
				out[k] = CUBE_BASE + Gallery_Level(r, i) * 36 + Gallery_Level(g, i) * 6 + Gallery_Level(b, i);
			}

			/* VRAM takes halfwords */
			*(u16 *)Gallery_ThumbPixelAddr(slot, x, y) = out[0] | (out[1] << 8);
		}
	}

	/* The CG is on BG, which stays off; the backdrop shows */
	gPaletteBuffer[0] = 0;
	EnablePaletteSync();
}

/* ---- The grid ---- */

static void Gallery_DrawFrame(int slot, bool selected)
{
	u16 *map = gBG1TilemapBuffer;
	int x0 = 2 + Gallery_Col(slot) * 10, y0 = 2 + Gallery_Row(slot) * 6;
	int pal = (selected ? PAL_FRAME_HI : PAL_FRAME) << 12;
	int i;

	map[TILEMAP_INDEX(x0 - 1, y0 - 1)] = pal | (FRAME_TILE + 0);
	map[TILEMAP_INDEX(x0 + 6, y0 - 1)] = pal | (FRAME_TILE + 2);
	map[TILEMAP_INDEX(x0 - 1, y0 + 4)] = pal | (FRAME_TILE + 5);
	map[TILEMAP_INDEX(x0 + 6, y0 + 4)] = pal | (FRAME_TILE + 7);

	for (i = 0; i < 6; i++) {
		map[TILEMAP_INDEX(x0 + i, y0 - 1)] = pal | (FRAME_TILE + 1);
		map[TILEMAP_INDEX(x0 + i, y0 + 4)] = pal | (FRAME_TILE + 6);
	}

	for (i = 0; i < 4; i++) {
		map[TILEMAP_INDEX(x0 - 1, y0 + i)] = pal | (FRAME_TILE + 3);
		map[TILEMAP_INDEX(x0 + 6, y0 + i)] = pal | (FRAME_TILE + 4);
	}
}

static void Gallery_DrawName(int slot, int index, bool selected)
{
	struct Text *t = &GS->names[slot];
	const char *str;
	int color, w;

	if (Gallery_Unlocked(index)) {
		str = GetStringFromIndex(gGalleryEntries[index].name);
		color = selected ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_WHITE;
	} else {
		str = GetStringFromIndex(MSG_GALLERY_LOCKED);
		color = selected ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY;
	}

	w = GetStringTextLen(str);
	ClearText(t);
	Text_InsertDrawString(t, w < 80 ? (80 - w) / 2 : 0, color, str);
	PutText(t, gBG0TilemapBuffer + TILEMAP_INDEX(Gallery_Col(slot) * 10, Gallery_Row(slot) * 6));
}

static void Gallery_DrawFooter(void)
{
	struct Text *t = &GS->footer;
	char buf[24];
	const char *str = GetStringFromIndex(MSG_GALLERY_PAGE);
	int i = 0, w, pages = Gallery_PageCount(), page = GS->cursor / GALLERY_PER_PAGE + 1;

	while (*str && i < 12)
		buf[i++] = *str++;
	buf[i++] = ' ';
	if (page >= 10)
		buf[i++] = '0' + Div(page, 10);
	buf[i++] = '0' + DivRem(page, 10);
	buf[i++] = '/';
	if (pages >= 10)
		buf[i++] = '0' + Div(pages, 10);
	buf[i++] = '0' + DivRem(pages, 10);
	buf[i] = '\0';

	ClearText(t);
	Text_InsertDrawString(t, 8, TEXT_COLOR_SYSTEM_GRAY, GetStringFromIndex(MSG_GALLERY_HINT));
	w = GetStringTextLen(buf);
	Text_InsertDrawString(t, 232 - w, TEXT_COLOR_SYSTEM_WHITE, buf);
	PutText(t, gBG0TilemapBuffer + TILEMAP_INDEX(0, 18));
}

static void Gallery_DrawGrid(void)
{
	int slot, n = Gallery_OnPage(GS->page);

	for (slot = 0; slot < n; slot++) {
		int index = GS->page * GALLERY_PER_PAGE + slot;

		Gallery_DrawFrame(slot, index == GS->cursor);
		Gallery_DrawName(slot, index, index == GS->cursor);
	}

	Gallery_DrawFooter();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

/* The grid's BGs, text, frames and palettes; the thumbnails are already in OBJ VRAM */
static void Gallery_SetupGrid(void)
{
	int i, r, g, b;

	SetupBackgrounds((u16 *)sGalleryBgConfig);
	gLCDControlBuffer.bg3cnt.colorMode = 0;
	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_Fill(gBG3TilemapBuffer, 0);
	gLCDControlBuffer.bg0cnt.priority = 0;
	gLCDControlBuffer.bg1cnt.priority = 1;
	for (i = 0; i < 4; i++)
		BG_SetPosition(i, 0, 0);

	ResetText();
	SetTextFont(0);
	InitSystemTextFont();
	LoadUiFrameGraphics();

	for (i = 0; i < GALLERY_PER_PAGE; i++)
		InitText(&GS->names[i], 10);
	InitText(&GS->footer, 30);

	CpuFastCopy(Img_GalleryFrame, (u8 *)VRAM + FRAME_TILE * 0x20, 8 * 0x20);
	ApplyPalette(sFramePal, PAL_FRAME);
	ApplyPalette(sFramePal, PAL_FRAME_HI);

	/* The OBJ palette: the colour cube, and the padlock's */
	for (r = 0; r < 6; r++)
		for (g = 0; g < 6; g++)
			for (b = 0; b < 6; b++)
				gPaletteBuffer[0x100 + CUBE_BASE + r * 36 + g * 6 + b] = RGB(sCubeLevel[r], sCubeLevel[g], sCubeLevel[b]);
	CpuFastCopy(Img_GalleryLock, (u8 *)0x06010000 + LOCK_TILE * 0x20, 4 * 0x20);
	ApplyPalette(Pal_GalleryLock, 0x10 + PAL_OBJ_LOCK);

	gPaletteBuffer[0] = RGB(2, 3, 7);

	gLCDControlBuffer.dispcnt.obj1dMap = 1;
	SetDispEnable(1, 1, 0, 0, 1);
	SetDefaultColorEffects();

	Gallery_DrawGrid();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT | BG3_SYNC_BIT);
	EnablePaletteSync();
	GS->sprites = true;
}

static void Gallery_PutSprites(void)
{
	int slot, n = Gallery_OnPage(GS->page);

	if (!GS->sprites)
		return;

	for (slot = 0; slot < n; slot++) {
		int x = 16 + Gallery_Col(slot) * 80, y = 16 + Gallery_Row(slot) * 48;

		PutSpriteExt(4, x, y, sObjThumb, slot * THUMB_UNITS);

		if (!Gallery_Unlocked(GS->page * GALLERY_PER_PAGE + slot))
			PutSpriteExt(4, x + 16, y + 8, sObjLock, 0);
	}
}

/* The cursor's frame glows: gold to near white and back */
static void Gallery_Glow(void)
{
	int t = GS->timer & 0x3F, k = t < 0x20 ? t : 0x3F - t;

	GS->timer++;
	gPaletteBuffer[PAL_FRAME_HI * 16 + 1] = RGB(14 + (k >> 2), 9 + (k >> 2), 2);
	gPaletteBuffer[PAL_FRAME_HI * 16 + 2] = RGB(31, 24 + (k >> 2), 6 + (k >> 1) + (k >> 2));
	EnablePaletteSync();
}

/* ---- The proc ---- */

static void Gallery_Init(struct ProcGallery *proc)
{
	proc->dispBak = gLCDControlBuffer.dispcnt;

	CpuFill16(0, GS, sizeof(*GS));
	GS->count = Gallery_Count();
	Gallery_ReadUnlocks(GS->bits);
}

static void Gallery_StartPage(struct ProcGallery *proc)
{
	GS->page = GS->cursor / GALLERY_PER_PAGE;
	GS->built = 0;
	GS->sprites = false;

	SetDispEnable(0, 0, 0, 0, 0);
	gLCDControlBuffer.dispcnt.obj1dMap = 1;
	gPaletteBuffer[0] = 0;
	EnablePaletteSync();
}

/* One thumbnail a frame, then the grid */
static void Gallery_BuildLoop(struct ProcGallery *proc)
{
	if (GS->built < Gallery_OnPage(GS->page)) {
		Gallery_MakeThumb(GS->built, GS->page * GALLERY_PER_PAGE + GS->built);
		GS->built++;
		return;
	}

	Gallery_SetupGrid();
	Gallery_PutSprites();
	Proc_Break(proc);
}

/* A fade, with the grid's thumbnails kept on screen */
static void Gallery_FadeWait(struct ProcGallery *proc)
{
	Gallery_PutSprites();

	if (!FadeInExists() && !FadeOutExists())
		Proc_Break(proc);
}

static void Gallery_Move(int to)
{
	int old = GS->cursor;

	if (to < 0)
		to = 0;
	if (to >= GS->count)
		to = GS->count - 1;
	if (to == old)
		return;

	GS->cursor = to;
	PlaySoundEffect(SE_MOVE);
}

static void Gallery_GridLoop(struct ProcGallery *proc)
{
	u16 press = gKeyStatusPtr->newKeys, repeat = gKeyStatusPtr->repeatedKeys;
	int old = GS->cursor;

	Gallery_PutSprites();
	Gallery_Glow();

	if (press & B_BUTTON) {
		PlaySoundEffect(SE_LEAVE);
		Proc_Goto(proc, L_GALLERY_EXIT);
		return;
	}

	if (press & A_BUTTON) {
		if (GS->count != 0 && Gallery_Unlocked(GS->cursor)) {
			PlaySoundEffect(SE_OK);
			GS->view = GS->cursor;
			Proc_Goto(proc, L_GALLERY_VIEW);
		} else {
			PlaySoundEffect(SE_BAD);
		}
		return;
	}

	if (GS->count == 0)
		return;

	if (repeat & DPAD_RIGHT)
		Gallery_Move(old + 1);
	else if (repeat & DPAD_LEFT)
		Gallery_Move(old - 1);
	else if (repeat & DPAD_DOWN)
		Gallery_Move(old + GALLERY_COLS);
	else if (repeat & DPAD_UP)
		Gallery_Move(old - GALLERY_COLS);
	else if (press & R_BUTTON)
		Gallery_Move(old + GALLERY_PER_PAGE);
	else if (press & L_BUTTON)
		Gallery_Move(old - GALLERY_PER_PAGE);

	if (GS->cursor == old)
		return;

	if (GS->cursor / GALLERY_PER_PAGE != GS->page) {
		Proc_Goto(proc, L_GALLERY_TURN);
		return;
	}

	Gallery_DrawGrid();
}

/* ---- The full view ---- */

static void Gallery_ShowFull(struct ProcGallery *proc)
{
	GS->sprites = false;
	SetupBackgrounds((u16 *)sGalleryBgConfig);
	gLCDControlBuffer.bg3cnt.colorMode = 0;
	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_Fill(gBG2TilemapBuffer, 0);

	EventShowTextBgDirect(EVSUBCMD_REMOVEPORTRAITS, gGalleryEntries[GS->view].bg);

	SetDispEnable(0, 0, 0, 1, 0);
	SetDefaultColorEffects();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT | BG3_SYNC_BIT);
	EnablePaletteSync();
}

/* The next unlocked CG that way, around the end; the same one if it is the only one */
static int Gallery_NextUnlocked(int from, int step)
{
	int i, index = from;

	for (i = 0; i < GS->count; i++) {
		index += step;
		if (index < 0)
			index = GS->count - 1;
		if (index >= GS->count)
			index = 0;
		if (Gallery_Unlocked(index))
			return index;
	}
	return from;
}

static void Gallery_ViewLoop(struct ProcGallery *proc)
{
	u16 press = gKeyStatusPtr->newKeys, repeat = gKeyStatusPtr->repeatedKeys;
	int next;

	if (press & B_BUTTON) {
		PlaySoundEffect(SE_LEAVE);
		GS->cursor = GS->view;
		Proc_Goto(proc, L_GALLERY_BACK);
		return;
	}

	if (!(repeat & (DPAD_LEFT | DPAD_RIGHT)))
		return;

	next = Gallery_NextUnlocked(GS->view, (repeat & DPAD_RIGHT) ? 1 : -1);
	if (next == GS->view) {
		PlaySoundEffect(SE_BAD);
		return;
	}

	PlaySoundEffect(SE_PAGE);
	GS->view = next;
	Proc_Goto(proc, L_GALLERY_VIEW);
}

/* Back from the full view: the thumbnails are still there unless the CG is on another page */
static void Gallery_Back(struct ProcGallery *proc)
{
	if (GS->cursor / GALLERY_PER_PAGE != GS->page) {
		Proc_Goto(proc, L_GALLERY_PAGE);
		return;
	}

	Gallery_SetupGrid();
	Gallery_PutSprites();
}

static void Gallery_OnEnd(struct ProcGallery *proc)
{
	gLCDControlBuffer.bg3cnt.colorMode = 0;
	gLCDControlBuffer.dispcnt = proc->dispBak;
	SetDefaultColorEffects();
}

static const struct ProcCmd ProcScr_Gallery[] = {
	PROC_NAME("Gallery"),
	PROC_SET_END_CB(Gallery_OnEnd),
	PROC_YIELD,
	PROC_CALL(Gallery_Init),

	PROC_LABEL(L_GALLERY_PAGE),
	PROC_CALL(Gallery_StartPage),
	PROC_REPEAT(Gallery_BuildLoop),
	PROC_CALL_ARG(NewFadeIn, 8),
	PROC_REPEAT(Gallery_FadeWait),

	PROC_LABEL(L_GALLERY_GRID),
	PROC_REPEAT(Gallery_GridLoop),

	PROC_LABEL(L_GALLERY_TURN),
	PROC_CALL_ARG(NewFadeOut, 8),
	PROC_REPEAT(Gallery_FadeWait),
	PROC_GOTO(L_GALLERY_PAGE),

	PROC_LABEL(L_GALLERY_VIEW),
	PROC_CALL_ARG(NewFadeOut, 8),
	PROC_REPEAT(Gallery_FadeWait),
	PROC_CALL(Gallery_ShowFull),
	PROC_CALL_ARG(NewFadeIn, 8),
	PROC_REPEAT(Gallery_FadeWait),
	PROC_REPEAT(Gallery_ViewLoop),

	PROC_LABEL(L_GALLERY_BACK),
	PROC_CALL_ARG(NewFadeOut, 8),
	PROC_REPEAT(Gallery_FadeWait),
	PROC_CALL(Gallery_Back),
	PROC_CALL_ARG(NewFadeIn, 8),
	PROC_REPEAT(Gallery_FadeWait),
	PROC_GOTO(L_GALLERY_GRID),

	PROC_LABEL(L_GALLERY_EXIT),
	PROC_CALL_ARG(NewFadeOut, 8),
	PROC_REPEAT(Gallery_FadeWait),
	PROC_END,
};

ProcPtr StartGalleryScreen(ProcPtr parent)
{
	return Proc_StartBlocking(ProcScr_Gallery, parent);
}
