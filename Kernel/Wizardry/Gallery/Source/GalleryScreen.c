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
 *  - The banner on top (the Sound Room's song banner, BG2): the name of the CG under the cursor, or "???" (BG0),
 *    centred on it.
 *  - On the left, in a window (BG1, BG2, BG0): which CG of how many, centred at the top; at the foot, the Sound Room's
 *    round A button with "View", and a B made from it (vanilla has no round B) with "Back".
 *  - The grid on the right, in a window (DrawUiFrame, the player's window colour, BG1), over the scrolling mural (BG3):
 *    three CGs a row, three rows on screen. Each is a 48x32 thumbnail (a sprite) in a frame (a sprite); the frame of
 *    the CG under the cursor is gold and glows. A locked CG is dimmed, with a padlock.
 *  - Moving off the rows on screen scrolls the grid a row (5 frames); window 0 is the window's inside, so the sprites
 *    never show outside it.
 *  - A shows an unlocked CG full size (BG3, the mural stopped); left and right go through the other unlocked ones;
 *    B goes back.
 *
 * The thumbnails are build output (Tools/gallery/thumbs.py, gGalleryThumbs): 4bpp, 16 colours each. The rows drawn
 * (three, four while scrolling) have their thumbnails in OBJ VRAM and their palettes in OBJ palettes 0-11, a slot for
 * each place on the grid, loaded again whenever the rows are drawn.
 */

#define GALLERY_COLS   3
#define VISIBLE_ROWS   3
#define DRAWN_ROWS     4 /* one more while scrolling */
#define SLOTS          (GALLERY_COLS * DRAWN_ROWS)
#define FRAME_W        52 /* a thumbnail and its 2-pixel frame */
#define FRAME_H        36
#define COL_PX         (FRAME_W + 2)
#define ROW_PX         (FRAME_H + 2)
#define SCROLL_STEP    8

/* The panel's window (tiles) */
#define PANEL_X        0
#define PANEL_Y        4
#define PANEL_W        8
#define PANEL_H        16
#define KEY_A_Y        (PANEL_Y + PANEL_H - 7)
#define KEY_B_Y        (PANEL_Y + PANEL_H - 4)

/* The window round the grid (tiles), and its inside (pixels): window 0 */
#define BOX_X          8
#define BOX_Y          4
#define BOX_W          22
#define BOX_H          16
#define IN_LEFT        (BOX_X * 8 + 8)
#define IN_TOP         (BOX_Y * 8 + 8)
#define IN_RIGHT       (240 - 8)
#define IN_BOTTOM      (160 - 8)
#define GRID_LEFT      (IN_LEFT + (IN_RIGHT - IN_LEFT - (GALLERY_COLS * COL_PX - 2)) / 2)
#define GRID_TOP       IN_TOP

/* The banner (tiles) */
#define BANNER_X       3
#define BANNER_W       24
#define BANNER_H       4

#define THUMB_BYTES    768 /* 6x4 4bpp tiles */
#define THUMB_SIZE     (THUMB_BYTES + 0x20) /* and the palette */
#define THUMB_UNITS    (THUMB_BYTES / 0x20)
#define LOCK_TILE      (SLOTS * THUMB_UNITS)
#define FRAME_OBJ_TILE (LOCK_TILE + 4)
#define OBJ_TEMP       ((u8 *)0x06014000) /* OBJ VRAM nothing uses here: vanilla sheets are unpacked here */

#define BANNER_TILE    0x220 /* BG VRAM 0x06004400, after the text's tiles */
#define ICON_A_TILE    (BANNER_TILE + BANNER_W * BANNER_H)
#define ICON_B_TILE    (ICON_A_TILE + 4)
#define PAL_ICON       10
#define PAL_BANNER     11
#define PAL_MURAL      14 /* and 15 */
#define PAL_OBJ_CURSOR 12
#define PAL_OBJ_FRAME  13
#define PAL_OBJ_LOCK   15

#define SE_MOVE  SONG_SE_SYS_CURSOR_UD1
#define SE_PAGE  SONG_SE_SYS_CURSOR_LR1
#define SE_OK    SONG_SE_SYS_WINDOW_SELECT1
#define SE_BAD   SONG_6C
#define SE_LEAVE SONG_SE_SYS_WINDOW_CANSEL1

struct GalleryScreen {
	/* 00 */ u8 count;
	/* 01 */ u8 cursor;
	/* 02 */ u8 top;	/* the first row on screen */
	/* 03 */ u8 view;	/* the CG the full view shows */
	/* 04 */ u8 drawTop;	/* the row drawn at the grid's top */
	/* 05 */ u8 drawRows;
	/* 06 */ u8 scrollPx;	/* the grid's offset */
	/* 07 */ s8 scrollDir;	/* 1: scrolling down, -1: up, 0: still */
	/* 08 */ u8 sprites;	/* the grid's thumbnails are on screen (they stay through its fades) */
	/* 09 */ u8 pad;
	/* 0A */ u16 timer;
	/* 0C */ u8 bits[GALLERY_MAX / 8];
	/* 18 */ struct Text title;
	/* 20 */ struct Text hintA;
	/* 28 */ struct Text hintB;
	/* 30 */ struct Text position;
};

extern struct GalleryScreen gGalleryScreen;
#define GS (&gGalleryScreen)

_Static_assert(sizeof(struct GalleryScreen) <= 0xA0, "gGalleryScreen is 0xA0 bytes (config-memmap.s)");

extern const u8 gGalleryThumbs[];
extern const u8 Img_GalleryLock[];
extern const u16 Pal_GalleryLock[];
extern const u8 Img_GalleryCursor[];
extern const u16 Pal_GalleryCursor[];

/* The Sound Room's sprite sheet (its A button) */
extern const u8 Img_SoundRoomUiElements[];
extern const u16 Pal_SoundRoomUiElements[];

struct ProcGallery {
	PROC_HEADER;
	struct DispCnt dispBak;
};

enum {
	L_GALLERY_GRID = 1,
	L_GALLERY_VIEW,
	L_GALLERY_BACK,
	L_GALLERY_EXIT,
};

/* BG0 text, BG1 the window, BG2 the banner and the buttons, BG3 the mural (or the CG) */
static const u16 sGalleryBgConfig[12] = {
	0x0000, 0x6000, 0,
	0x0000, 0x6800, 0,
	0x0000, 0x7000, 0,
	0x8000, 0x7800, 0,
};

static const u16 sFrameObjPal[16] = { 0, RGB(3, 4, 8), RGB(13, 15, 21) };

/* A 48x32 thumbnail: a 32x32 and a 16x32 sprite */
static const u16 sObjThumb[] = {
	2,
	0x0000, 0x8000, 0x0400,
	0x8000, 0x8000 + 32, 0x0400 + 16,
};

static const u16 sObjLock[] = {
	1,
	0x0000, 0x4000, 0x0000 | (PAL_OBJ_LOCK << 12) | LOCK_TILE,
};

/* A frame: 64x64, the thumbnail at (8, 8); grey, or gold for the cursor's (the palette is added) */
static const u16 sObjFrame[] = {
	1,
	0x0000, 0xC000, 0x0400 | FRAME_OBJ_TILE,
};

/* No % here: it is broken in this build (CLAUDE.md, Pitfalls) */
static int Gallery_Row(int index)
{
	return Div(index, GALLERY_COLS);
}

static int Gallery_RowCount(void)
{
	return Div(GS->count + GALLERY_COLS - 1, GALLERY_COLS);
}

static int Gallery_MaxTop(void)
{
	int top = Gallery_RowCount() - VISIBLE_ROWS;

	return top < 0 ? 0 : top;
}

static bool Gallery_Unlocked(int index)
{
	return Gallery_IsUnlocked(GS->bits, index);
}

/* ---- The banner and the panel ---- */

static void Gallery_DrawTitle(void)
{
	struct Text *t = &GS->title;
	const char *str;
	int w;

	if (GS->count == 0)
		return;

	str = GetStringFromIndex(Gallery_Unlocked(GS->cursor) ? gGalleryEntries[GS->cursor].name : MSG_GALLERY_LOCKED);
	w = GetStringTextLen(str);

	/* 20 tiles from tile 5: centred on the banner (and the screen) */
	ClearText(t);
	Text_InsertDrawString(t, w < 160 ? (160 - w) / 2 : 0, TEXT_COLOR_SYSTEM_WHITE, str);
	PutText(t, gBG0TilemapBuffer + TILEMAP_INDEX(5, 1));
}

static void Gallery_DrawPosition(void)
{
	struct Text *t = &GS->position;
	char buf[12];
	int i = 0, n = GS->cursor + 1, w;

	if (n >= 10)
		buf[i++] = '0' + Div(n, 10);
	buf[i++] = '0' + DivRem(n, 10);
	buf[i++] = '/';
	if (GS->count >= 10)
		buf[i++] = '0' + Div(GS->count, 10);
	buf[i++] = '0' + DivRem(GS->count, 10);
	buf[i] = '\0';

	/* Centred in the panel's inside (6 tiles) */
	w = GetStringTextLen(buf);
	ClearText(t);
	Text_InsertDrawString(t, w < 48 ? (48 - w) / 2 : 0, TEXT_COLOR_SYSTEM_WHITE, buf);
	PutText(t, gBG0TilemapBuffer + TILEMAP_INDEX(PANEL_X + 1, PANEL_Y + 2));
}

/*
 * A pixel of a 16x16 icon's 4 tiles (4bpp, top left, top right, bottom left, bottom right), in RAM: VRAM takes no byte
 * writes (a byte written there lands in both bytes of its halfword).
 */
static u8 *Gallery_IconByte(u8 *tiles, int x, int y)
{
	return tiles + ((y >> 3) * 2 + (x >> 3)) * 0x20 + (y & 7) * 4 + ((x & 7) >> 1);
}

static int Gallery_IconPixel(u8 *tiles, int x, int y)
{
	return (*Gallery_IconByte(tiles, x, y) >> ((x & 1) * 4)) & 0xF;
}

static void Gallery_SetIconPixel(u8 *tiles, int x, int y, int color)
{
	u8 *b = Gallery_IconByte(tiles, x, y);

	*b = (x & 1) ? ((*b & 0x0F) | (color << 4)) : ((*b & 0xF0) | color);
}

/* A 16x16 icon (its 4 BG tiles) as a 2x2 block on BG2 */
static void Gallery_PutIcon(int bgTile, int x, int y)
{
	int r, c;

	for (r = 0; r < 2; r++)
		for (c = 0; c < 2; c++)
			gBG2TilemapBuffer[TILEMAP_INDEX(x + c, y + r)] = (PAL_ICON << 12) | (bgTile + r * 2 + c);
}

/*
 * The B button: vanilla has the round A only (the Sound Room's), so the B is that button with its letter redrawn, the
 * same strokes as the A's (2 pixels of ink on the gold, a softer shade on the round corners) over the A's letter
 * (x 4-9, y 5-11). '#' ink, '+' the soft shade, '.' gold.
 */
static const char sLetterB[7][7] = {
	"####+.",
	"##..##",
	"##..##",
	"####+.",
	"##..##",
	"##..##",
	"####+.",
};

static void Gallery_DrawPanel(void)
{
	u32 icon[4 * 0x20 / 4];
	int r, ink, soft, gold, x, y;

	/* The Sound Room's A (its sheet's tiles 0x50-0x51, 0x70-0x71; its third palette) */
	Decompress(Img_SoundRoomUiElements, OBJ_TEMP);
	for (r = 0; r < 2; r++)
		CpuFastCopy(OBJ_TEMP + (0x50 + r * 32) * 0x20, (u8 *)VRAM + (ICON_A_TILE + r * 2) * 0x20, 2 * 0x20);
	ApplyPalette(Pal_SoundRoomUiElements + 0x20, PAL_ICON);

	/* The B: the A's tiles, the letter redrawn in the A's ink on its gold */
	CpuFastCopy((u8 *)VRAM + ICON_A_TILE * 0x20, icon, sizeof(icon));
	ink = Gallery_IconPixel((u8 *)icon, 6, 6);
	soft = Gallery_IconPixel((u8 *)icon, 5, 5);
	gold = Gallery_IconPixel((u8 *)icon, 6, 7);
	for (y = 0; y < 7; y++)
		for (x = 0; x < 6; x++)
			Gallery_SetIconPixel((u8 *)icon, 4 + x, 5 + y,
					     sLetterB[y][x] == '#' ? ink : sLetterB[y][x] == '+' ? soft : gold);
	CpuFastCopy(icon, (u8 *)VRAM + ICON_B_TILE * 0x20, sizeof(icon));

	/* At the panel's foot, a key: the buttons, and what they do */
	Gallery_PutIcon(ICON_A_TILE, PANEL_X + 1, KEY_A_Y);
	Gallery_PutIcon(ICON_B_TILE, PANEL_X + 1, KEY_B_Y);

	ClearText(&GS->hintA);
	Text_InsertDrawString(&GS->hintA, 2, TEXT_COLOR_SYSTEM_WHITE, GetStringFromIndex(MSG_GALLERY_HINT));
	PutText(&GS->hintA, gBG0TilemapBuffer + TILEMAP_INDEX(PANEL_X + 3, KEY_A_Y));

	ClearText(&GS->hintB);
	Text_InsertDrawString(&GS->hintB, 2, TEXT_COLOR_SYSTEM_WHITE, GetStringFromIndex(MSG_GALLERY_HINT_BACK));
	PutText(&GS->hintB, gBG0TilemapBuffer + TILEMAP_INDEX(PANEL_X + 3, KEY_B_Y));
}

/* The Sound Room's song banner: its sheet into spare OBJ VRAM, the 24x4 tiles of the banner on to BG VRAM */
static void Gallery_LoadBanner(void)
{
	int x, y;

	Decompress(Img_PlayStatusSprites, OBJ_TEMP);

	for (y = 0; y < BANNER_H; y++)
		CpuFastCopy(OBJ_TEMP + y * 32 * 0x20, (u8 *)VRAM + (BANNER_TILE + y * BANNER_W) * 0x20, BANNER_W * 0x20);

	ApplyPalette(Pal_PlayStatusSprites, PAL_BANNER);

	for (y = 0; y < BANNER_H; y++)
		for (x = 0; x < BANNER_W; x++)
			gBG2TilemapBuffer[TILEMAP_INDEX(BANNER_X + x, y)] = (PAL_BANNER << 12) | (BANNER_TILE + y * BANNER_W + x);
}

/* ---- The grid's rows ---- */

/* A grid place's thumbnail and palette (dimmed when locked) */
static void Gallery_LoadThumb(int slot, int index)
{
	const u8 *src = gGalleryThumbs + index * THUMB_SIZE;
	const u16 *pal = (const u16 *)(src + THUMB_BYTES);
	u16 *dst = gPaletteBuffer + 0x100 + slot * 16;
	int i;

	CpuFastCopy(src, (u8 *)0x06010000 + slot * THUMB_BYTES, THUMB_BYTES);

	for (i = 0; i < 16; i++) {
		u16 c = pal[i];

		if (!Gallery_Unlocked(index))
			c = RGB(((c & 0x1F) * 5) >> 4, (((c >> 5) & 0x1F) * 5) >> 4, (((c >> 10) & 0x1F) * 5) >> 4);
		dst[i] = c;
	}
}

/* Rows first..first+n-1 from the grid's top, their thumbnails in the slots; scrolled by px */
static void Gallery_DrawRows(int first, int n, int px)
{
	int r, c;

	for (r = 0; r < n; r++) {
		for (c = 0; c < GALLERY_COLS; c++) {
			int index = (first + r) * GALLERY_COLS + c;

			if (index < GS->count)
				Gallery_LoadThumb(r * GALLERY_COLS + c, index);
		}
	}

	GS->drawTop = first;
	GS->drawRows = n;
	GS->scrollPx = px;

	Gallery_DrawTitle();
	Gallery_DrawPosition();
	BG_EnableSyncByMask(BG0_SYNC_BIT);
	EnablePaletteSync();
}

static void Gallery_PutSprites(void)
{
	int r, c;

	if (!GS->sprites)
		return;

	for (r = 0; r < GS->drawRows; r++) {
		int y = GRID_TOP + r * ROW_PX - GS->scrollPx;

		if (y <= GRID_TOP - FRAME_H || y >= IN_BOTTOM)
			continue;

		for (c = 0; c < GALLERY_COLS; c++) {
			int index = (GS->drawTop + r) * GALLERY_COLS + c, slot = r * GALLERY_COLS + c;
			int x = GRID_LEFT + c * COL_PX;

			if (index >= GS->count)
				continue;

			/* The frame's sprite has the thumbnail at (8, 8); the frame line is 2 pixels outside it */
			PutSpriteExt(4, (x - 6) & 0x1FF, (y - 6) & 0xFF, sObjFrame,
				     (index == GS->cursor ? PAL_OBJ_CURSOR : PAL_OBJ_FRAME) << 12);
			PutSpriteExt(4, x + 2, (y + 2) & 0xFF, sObjThumb, slot * THUMB_UNITS + (slot << 12));

			if (!Gallery_Unlocked(index))
				PutSpriteExt(4, x + 18, (y + 10) & 0xFF, sObjLock, 0);
		}
	}
}

/* The cursor's frame glows: gold to near white and back */
static void Gallery_Glow(void)
{
	int t = GS->timer & 0x3F, k = t < 0x20 ? t : 0x3F - t;
	u16 *pal = gPaletteBuffer + 0x100 + PAL_OBJ_CURSOR * 16;

	GS->timer++;
	pal[1] = RGB(14 + (k >> 2), 9 + (k >> 2), 2);
	pal[2] = RGB(31, 24 + (k >> 2), 6 + (k >> 1) + (k >> 2));
	EnablePaletteSync();
}

/* The grid's screen: BGs, text, the banner, the buttons, the window, the mural, window 0; the rows from top */
static void Gallery_SetupGrid(struct ProcGallery *proc)
{
	int i;

	SetupBackgrounds((u16 *)sGalleryBgConfig);
	gLCDControlBuffer.bg3cnt.colorMode = 0;
	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_Fill(gBG3TilemapBuffer, 0);
	gLCDControlBuffer.bg0cnt.priority = 0;
	gLCDControlBuffer.bg1cnt.priority = 2;
	gLCDControlBuffer.bg2cnt.priority = 1;
	gLCDControlBuffer.bg3cnt.priority = 3;
	for (i = 0; i < 4; i++)
		BG_SetPosition(i, 0, 0);

	ResetText();
	SetTextFont(0);
	InitSystemTextFont();
	LoadUiFrameGraphics();

	InitText(&GS->title, 20);
	InitText(&GS->hintA, 4);
	InitText(&GS->hintB, 4);
	InitText(&GS->position, 6);

	CpuFastCopy(Img_GalleryLock, (u8 *)0x06010000 + LOCK_TILE * 0x20, 4 * 0x20);
	ApplyPalette(Pal_GalleryLock, 0x10 + PAL_OBJ_LOCK);
	CpuFastCopy(Img_GalleryCursor, (u8 *)0x06010000 + FRAME_OBJ_TILE * 0x20, 64 * 0x20);
	ApplyPalette(Pal_GalleryCursor, 0x10 + PAL_OBJ_CURSOR);
	ApplyPalette(sFrameObjPal, 0x10 + PAL_OBJ_FRAME);

	Gallery_LoadBanner();
	Gallery_DrawPanel();
	DrawUiFrame(gBG1TilemapBuffer, PANEL_X, PANEL_Y, PANEL_W, PANEL_H, 0, 0);
	DrawUiFrame(gBG1TilemapBuffer, BOX_X, BOX_Y, BOX_W, BOX_H, 0, 0);

	StartMuralBackground(proc, NULL, PAL_MURAL);

	gLCDControlBuffer.dispcnt.obj1dMap = 1;
	SetDispEnable(1, 1, 1, 1, 1);

	/* Window 0 is the window's inside: the grid's sprites show only there */
	gLCDControlBuffer.dispcnt.win0_on = 1;
	gLCDControlBuffer.win0_left = IN_LEFT;
	gLCDControlBuffer.win0_right = IN_RIGHT;
	gLCDControlBuffer.win0_top = IN_TOP;
	gLCDControlBuffer.win0_bottom = IN_BOTTOM;
	gLCDControlBuffer.wincnt.win0_enableBg0 = 1;
	gLCDControlBuffer.wincnt.win0_enableBg1 = 1;
	gLCDControlBuffer.wincnt.win0_enableBg2 = 1;
	gLCDControlBuffer.wincnt.win0_enableBg3 = 1;
	gLCDControlBuffer.wincnt.win0_enableObj = 1;
	gLCDControlBuffer.wincnt.win0_enableBlend = 1;
	gLCDControlBuffer.wincnt.wout_enableBg0 = 1;
	gLCDControlBuffer.wincnt.wout_enableBg1 = 1;
	gLCDControlBuffer.wincnt.wout_enableBg2 = 1;
	gLCDControlBuffer.wincnt.wout_enableBg3 = 1;
	gLCDControlBuffer.wincnt.wout_enableObj = 0;
	gLCDControlBuffer.wincnt.wout_enableBlend = 1;

	SetDefaultColorEffects();

	/* The cursor's row on screen */
	if (Gallery_Row(GS->cursor) < GS->top)
		GS->top = Gallery_Row(GS->cursor);
	if (Gallery_Row(GS->cursor) > GS->top + VISIBLE_ROWS - 1)
		GS->top = Gallery_Row(GS->cursor) - (VISIBLE_ROWS - 1);
	if (GS->top > Gallery_MaxTop())
		GS->top = Gallery_MaxTop();

	GS->scrollDir = 0;
	Gallery_DrawRows(GS->top, VISIBLE_ROWS, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT | BG3_SYNC_BIT);
	GS->sprites = true;
}

/* ---- The grid ---- */

static void Gallery_Init(struct ProcGallery *proc)
{
	proc->dispBak = gLCDControlBuffer.dispcnt;

	CpuFill16(0, GS, sizeof(*GS));
	GS->count = Gallery_Count();
	Gallery_ReadUnlocks(GS->bits);

	Gallery_SetupGrid(proc);
	Gallery_PutSprites();
}

/* Scroll a row toward the cursor's, if it is off screen */
static bool Gallery_StartScroll(void)
{
	int row = Gallery_Row(GS->cursor);

	if (row > GS->top + VISIBLE_ROWS - 1) {
		Gallery_DrawRows(GS->top, DRAWN_ROWS, 0);
		GS->scrollDir = 1;
		return true;
	}

	if (row < GS->top) {
		GS->top--;
		Gallery_DrawRows(GS->top, DRAWN_ROWS, ROW_PX);
		GS->scrollDir = -1;
		return true;
	}

	return false;
}

static void Gallery_StepScroll(void)
{
	int px = GS->scrollPx + GS->scrollDir * SCROLL_STEP;

	if (px > 0 && px < ROW_PX) {
		GS->scrollPx = px;
		return;
	}

	/* A row further: the three rows on screen, still; then on toward the cursor */
	if (GS->scrollDir > 0)
		GS->top++;
	GS->scrollDir = 0;
	Gallery_DrawRows(GS->top, VISIBLE_ROWS, 0);
	Gallery_StartScroll();
}

static void Gallery_Move(int to)
{
	if (GS->count == 0)
		return;
	if (to < 0)
		to = 0;
	if (to >= GS->count)
		to = GS->count - 1;
	if (to == GS->cursor)
		return;

	GS->cursor = to;
	PlaySoundEffect(SE_MOVE);

	if (!Gallery_StartScroll())
		Gallery_DrawRows(GS->top, VISIBLE_ROWS, 0);
}

static void Gallery_GridLoop(struct ProcGallery *proc)
{
	u16 press = gKeyStatusPtr->newKeys, repeat = gKeyStatusPtr->repeatedKeys;
	int old = GS->cursor;

	Gallery_Glow();

	if (GS->scrollDir != 0) {
		Gallery_StepScroll();
		Gallery_PutSprites();
		return;
	}

	Gallery_PutSprites();

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

	if (repeat & DPAD_RIGHT)
		Gallery_Move(old + 1);
	else if (repeat & DPAD_LEFT)
		Gallery_Move(old - 1);
	else if (repeat & DPAD_DOWN)
		Gallery_Move(old + GALLERY_COLS);
	else if (repeat & DPAD_UP)
		Gallery_Move(old - GALLERY_COLS);
	else if (repeat & R_BUTTON)
		Gallery_Move(old + GALLERY_COLS * VISIBLE_ROWS);
	else if (repeat & L_BUTTON)
		Gallery_Move(old - GALLERY_COLS * VISIBLE_ROWS);
}

/* A fade, with the grid's thumbnails kept on screen */
static void Gallery_FadeWait(struct ProcGallery *proc)
{
	Gallery_PutSprites();

	if (!FadeInExists() && !FadeOutExists())
		Proc_Break(proc);
}

/* ---- The full view ---- */

static void Gallery_ShowFull(struct ProcGallery *proc)
{
	GS->sprites = false;
	EndMuralBackground();

	SetupBackgrounds((u16 *)sGalleryBgConfig);
	gLCDControlBuffer.bg3cnt.colorMode = 0;
	gLCDControlBuffer.dispcnt.win0_on = 0;
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

static void Gallery_OnEnd(struct ProcGallery *proc)
{
	EndMuralBackground();
	gLCDControlBuffer.bg3cnt.colorMode = 0;
	gLCDControlBuffer.dispcnt = proc->dispBak;
	SetDefaultColorEffects();
}

static const struct ProcCmd ProcScr_Gallery[] = {
	PROC_NAME("Gallery"),
	PROC_SET_END_CB(Gallery_OnEnd),
	PROC_YIELD,
	PROC_CALL(Gallery_Init),
	PROC_CALL_ARG(NewFadeIn, 8),
	PROC_REPEAT(Gallery_FadeWait),

	PROC_LABEL(L_GALLERY_GRID),
	PROC_REPEAT(Gallery_GridLoop),

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
	PROC_CALL(Gallery_SetupGrid),
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
