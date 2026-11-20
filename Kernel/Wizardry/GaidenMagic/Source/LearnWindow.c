#include "common-chax.h"
#include "kernel-lib.h"
#include "gaiden-magic.h"
#include "weapon-range.h"
#include "constants/texts.h"

/*
 * The New Spell window (learning by level, gaiden_magic_learn_by_level). A unit that levels up past a spell of its
 * level list (gGaidenPinfoConfigList) in a battle has the spell queued (TryAddGaidenMagicPConf). Once the action is
 * over, PostAction_GaidenMagicLearnWindow shows one window per queued spell, A or B for the next:
 *
 *               [ New Spell! ]
 *   +--------+---------------------------+
 *   |  (())  | Hono learned              |
 *   | (icon) | Arcfire                   |
 *   |  (())  | Uses 5  Mt 14  Hit 85  Rng 1-2 |
 *   +--------+---------------------------+
 *
 * The window is a UI frame on BG1 and the text is on BG0. The tab is a sprite, a copy of the UI frame's tiles, so it
 * sits over the window's top edge without replacing it (priority 1: over BG1, under BG0, which has its text). The icon
 * is the item's icon at twice its size, in its own colours (icon palette 0), in a ring of twinkling sparkles. The spell's name is purple: its tiles use BG palette 5, the text's palette
 * with its white made purple, while the window is up (the text's own colours are rewritten every frame by other code,
 * so they can't be borrowed). OBJ tiles: the map popups' (0x240 on) for the icon and sparkles, and rows 28-31, columns
 * 19-28 for the tab: tiles the map leaves alone while the window is up (rows 4-11, empty as they look, are the unit
 * sprites' and are rewritten). OBJ palettes: 4 (the map popups') for the icon, 5 for the sparkles
 * and 6 for the tab, both empty on the map (whatever uses them loads its own).
 */

#define LW_X        2   /* the window, in tiles */
#define LW_Y        6
#define LW_W        26
#define LW_H        10
#define LW_PANEL_W  7   /* the icon's panel, on the window's left */
/* the "New Spell!" tab, centred over the window and overlapping its top edge */
#define LW_TAB_X    (LW_X + (LW_W - LW_TAB_W) / 2) /* in tiles */
#define LW_TAB_Y    4   /* in tiles */
#define LW_TAB_W    10
#define LW_TAB_H    4
#define LW_TEXT_X   (LW_X + LW_PANEL_W + 1)

#define LW_ICON_CHR     0x240 /* the map popups' OBJ tiles: the icon (2x2, 2D mapping) */
#define LW_SPARKLE_CHR  0x242 /* two 8x8 sparkle frames, 0x242 and 0x243 */
#define LW_TAB_CHR      0x393 /* the tab: LW_TAB_W x LW_TAB_H tiles, 2D mapping (rows 28-31, columns 19-28) */
#define LW_OBJ_PAL      4     /* the map popups' OBJ palette: icon palette 0 */
#define LW_SPARKLE_PAL  5     /* the sparkles' colours in 13-15 */
#define LW_TAB_PAL      6     /* the UI frame's palette */
#define LW_AFFINE       30
#define LW_SPARKLES     10
#define LW_RING_R       22    /* the sparkles' ring, in pixels */
#define LW_MIN_FRAMES   24    /* frames before A or B closes it */

/* The spell's name: BG palette 5, the text's palette (0) with its white (entries 1-3: shade, main, outline) purple */
#define LW_NAME_PAL     5
static const u16 sPurple[3] = { RGB(15, 8, 22), RGB(25, 15, 31), RGB(6, 2, 10) };

enum { LW_TEXT_TITLE, LW_TEXT_LEARNED, LW_TEXT_SPELL, LW_TEXT_STATS, LW_TEXT_COUNT };

/* A proc has 0x6C bytes: the texts are LW_Draw's locals, and OBJ palettes 5 and 6 (empty on the map) are not kept */
struct ProcLearnWindow {
	PROC_HEADER;

	u16 timer;
	u16 chr_counter; /* the font's tile allocator: each window's texts start from it, and it is put back at the end */
	u8 uid, iid;
	u16 saved_pal[16]; /* BG palette 5, put back on the way out */
};

_Static_assert(sizeof(struct ProcLearnWindow) <= 0x6C, "a proc has 0x6C bytes");

/* A twinkle: a 4-point star, two frames (bright and small); colours 13 (glow), 14 (core) and 15 (tip) of the palette */
static const u32 sSparkleTiles[2][8] = {
	{ 0x00000000, 0x0000F000, 0x0000E000, 0x00FEDEF0, 0x0000E000, 0x0000F000, 0x00000000, 0x00000000 },
	{ 0x00000000, 0x00000000, 0x0000D000, 0x000DED00, 0x0000D000, 0x00000000, 0x00000000, 0x00000000 },
};

static const u16 sSparklePal[3] = { RGB(22, 18, 31), RGB(31, 31, 31), RGB(27, 24, 31) };

/* sin(i * 360 / 64) * 256, a quarter wave */
static const s16 sSin64[17] = { 0, 25, 50, 74, 98, 121, 142, 162, 181, 198, 213, 226, 237, 245, 251, 255, 256 };

static int LW_Sin(int a)
{
	a &= 63;
	if (a < 16)
		return sSin64[a];
	if (a < 32)
		return sSin64[32 - a];
	if (a < 48)
		return -sSin64[a - 32];
	return -sSin64[64 - a];
}

static u16 *LW_Bg(int bg, int x, int y)
{
	return BG_GetMapBuffer(bg) + TILEMAP_INDEX(x, y);
}

static void LW_AppendNumber(char *buf, int *i, int n)
{
	char tmp[6];
	int len = 0;

	do {
		tmp[len++] = '0' + DivRem(n, 10);
		n = Div(n, 10);
	} while (n && len < 5);
	while (len)
		buf[(*i)++] = tmp[--len];
	buf[*i] = 0;
}

static void LW_DrawLabel(struct Text *th, int msg, const char *value, int *cursor)
{
	Text_SetCursor(th, *cursor);
	Text_SetColor(th, TEXT_COLOR_SYSTEM_GOLD);
	Text_DrawString(th, GetStringFromIndex(msg));
	*cursor += GetStringTextLen(GetStringFromIndex(msg)) + 3;
	Text_SetCursor(th, *cursor);
	Text_SetColor(th, TEXT_COLOR_SYSTEM_WHITE);
	Text_DrawString(th, value);
	*cursor += GetStringTextLen(value) + 7;
}

/* "Uses 5  Mt 14  Hit 85  Rng 1-2" (a staff: its uses and range) */
static void LW_DrawStats(struct Text *th, struct Unit *unit, int item)
{
	char buf[12];
	int cursor = 0, i;
	bool staff = GetItemType(item) == ITYPE_STAFF;

	if (IsGaidenMagicUsesSystem()) {
		int uses = GetGaidenMagicMaxUses(unit, item);

		i = 0;
		if (uses >= GAIDEN_MAGIC_USES_INFINITE) {
			buf[0] = '-';
			buf[1] = '-';
			buf[2] = 0;
		} else {
			LW_AppendNumber(buf, &i, uses);
		}
		LW_DrawLabel(th, MSG_NEW_SPELL_USES, buf, &cursor);
	} else {
		i = 0;
		LW_AppendNumber(buf, &i, GetGaidenWeaponHpCost(unit, item));
		LW_DrawLabel(th, MSG_NEW_SPELL_COST, buf, &cursor);
	}

	if (!staff) {
		i = 0;
		LW_AppendNumber(buf, &i, GetItemMight(item));
		LW_DrawLabel(th, MSG_NEW_SPELL_MT, buf, &cursor);

		i = 0;
		LW_AppendNumber(buf, &i, GetItemHit(item));
		LW_DrawLabel(th, MSG_NEW_SPELL_HIT, buf, &cursor);
	}

	i = 0;
	LW_AppendNumber(buf, &i, GetItemMinRangeRework(item, unit));
	if (GetItemMaxRangeRework(item, unit) != GetItemMinRangeRework(item, unit)) {
		buf[i++] = '-';
		LW_AppendNumber(buf, &i, GetItemMaxRangeRework(item, unit));
	}
	LW_DrawLabel(th, MSG_NEW_SPELL_RNG, buf, &cursor);
}

/* The spell's name: the text's palette with a purple white, in BG palette 5 (again if something else wrote it) */
static void LW_ApplyNamePalette(void)
{
	u16 *pal = gPaletteBuffer + LW_NAME_PAL * 16;
	int i;

	if (pal[2] == sPurple[1] && pal[1] == sPurple[0] && pal[3] == sPurple[2])
		return;

	for (i = 0; i < 16; i++)
		pal[i] = gPaletteBuffer[i];
	for (i = 0; i < 3; i++)
		pal[1 + i] = sPurple[i];
	EnablePaletteSync();
}

static void LW_ClearWindow(void)
{
	TileMap_FillRect(LW_Bg(0, LW_X, LW_TAB_Y), LW_W - 1, LW_Y + LW_H - LW_TAB_Y - 1, 0);
	TileMap_FillRect(LW_Bg(1, LW_X, LW_TAB_Y), LW_W - 1, LW_Y + LW_H - LW_TAB_Y - 1, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

/* A BG tile, with a map entry's flips, into an OBJ tile */
static void LW_CopyTile(const u8 *src, u8 *dst, u16 entry)
{
	int x, y;

	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x += 2) {
			int px[2], k;

			for (k = 0; k < 2; k++) {
				int sx = (entry & 0x400) ? 7 - (x + k) : x + k;
				int sy = (entry & 0x800) ? 7 - y : y;

				px[k] = (src[sy * 4 + sx / 2] >> ((sx & 1) * 4)) & 0xF;
			}
			dst[y * 4 + x / 2] = px[0] | (px[1] << 4);
		}
	}
}

/*
 * The tab: the UI frame as DrawUiFrame2 draws it (the style's own tiles: VeslyUI replaces the frame drawing, so
 * DrawUiFrame alone gives other tiles), drawn on BG1's rows 22-25, which are off the screen, copied tile by tile into
 * OBJ tiles, and cleared again. Its palette too.
 */
#define LW_TAB_SCRATCH_Y 22

static void LW_BuildTab(void)
{
	const u8 *chr = (const u8 *)VRAM + GetBackgroundTileDataOffset(1);
	u16 *tm = LW_Bg(1, 0, LW_TAB_SCRATCH_Y);
	int x, y;

	DrawUiFrame2(0, LW_TAB_SCRATCH_Y, LW_TAB_W, LW_TAB_H, 0);
	for (y = 0; y < LW_TAB_H; y++) {
		for (x = 0; x < LW_TAB_W; x++) {
			u16 entry = tm[TILEMAP_INDEX(x, y)];
			u16 tile[0x10]; /* built here: VRAM ignores byte writes */

			LW_CopyTile(chr + (entry & 0x3FF) * 0x20, (u8 *)tile, entry);
			CpuCopy16(tile, (u8 *)OBJ_VRAM0 + (LW_TAB_CHR + y * 32 + x) * 0x20, sizeof(tile));
		}
	}
	TileMap_FillRect(tm, LW_TAB_W - 1, LW_TAB_H - 1, 0);
	CpuCopy16(gPaletteBuffer + 0x10 * 1, gPaletteBuffer + 0x100 + LW_TAB_PAL * 0x10, 0x20);
	EnablePaletteSync();
}

static void LW_Draw(struct ProcLearnWindow *proc)
{
	struct Unit *unit = GetUnit(proc->uid);
	int item = proc->iid;
	struct Text text[LW_TEXT_COUNT];
	int i;

	LW_ClearWindow();

	/* the window and the icon's panel (a frame of its own); the tab is a sprite (LW_BuildTab) */
	DrawUiFrame2(LW_X, LW_Y, LW_W, LW_H, 0);
	DrawUiFrame2(LW_X, LW_Y, LW_PANEL_W, LW_H, 1);
	LW_BuildTab();

	gActiveFont->chr_counter = proc->chr_counter;
	InitText(&text[LW_TEXT_TITLE], LW_TAB_W - 2);
	InitText(&text[LW_TEXT_LEARNED], LW_W - LW_PANEL_W - 2);
	InitText(&text[LW_TEXT_SPELL], LW_W - LW_PANEL_W - 2);
	InitText(&text[LW_TEXT_STATS], LW_W - LW_PANEL_W - 2);
	for (i = 0; i < LW_TEXT_COUNT; i++)
		ClearText(&text[i]); /* the tiles are the last window's */

	Text_SetColor(&text[LW_TEXT_TITLE], TEXT_COLOR_SYSTEM_WHITE);
	Text_SetCursor(&text[LW_TEXT_TITLE],
		GetStringTextCenteredPos((LW_TAB_W - 2) * 8, GetStringFromIndex(MSG_NEW_SPELL_TITLE)));
	Text_DrawString(&text[LW_TEXT_TITLE], GetStringFromIndex(MSG_NEW_SPELL_TITLE));
	PutText(&text[LW_TEXT_TITLE], LW_Bg(0, LW_TAB_X + 1, LW_TAB_Y + 1));

	Text_SetColor(&text[LW_TEXT_LEARNED], TEXT_COLOR_SYSTEM_WHITE);
	Text_DrawString(&text[LW_TEXT_LEARNED], GetStringFromIndex(unit->pCharacterData->nameTextId));
	Text_DrawString(&text[LW_TEXT_LEARNED], " ");
	Text_DrawString(&text[LW_TEXT_LEARNED], GetStringFromIndex(MSG_NEW_SPELL_LEARNED));
	PutText(&text[LW_TEXT_LEARNED], LW_Bg(0, LW_TEXT_X, LW_Y + 2));

	Text_SetColor(&text[LW_TEXT_SPELL], TEXT_COLOR_SYSTEM_WHITE);
	Text_DrawString(&text[LW_TEXT_SPELL], GetItemName(item));
	PutText(&text[LW_TEXT_SPELL], LW_Bg(0, LW_TEXT_X, LW_Y + 4));
	for (i = 0; i < 2 * 32; i++) {
		u16 *t = LW_Bg(0, LW_TEXT_X, LW_Y + 4) + i;

		if ((i & 31) < LW_W - LW_PANEL_W - 2)
			*t = (*t & 0x0FFF) | (LW_NAME_PAL << 12);
	}

	LW_DrawStats(&text[LW_TEXT_STATS], unit, item);
	PutText(&text[LW_TEXT_STATS], LW_Bg(0, LW_TEXT_X, LW_Y + 6));

	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

	/* the icon and the sparkles */
	LoadIconPalette(0, 0x10 + LW_OBJ_PAL); /* item icons use icon palette 0 */
	LoadIconObjectGraphics(GetItemIconId(item), LW_ICON_CHR);
	CpuCopy16(sSparkleTiles, OBJ_VRAM0 + LW_SPARKLE_CHR * 0x20, sizeof(sSparkleTiles));
	CpuCopy16(sSparklePal, gPaletteBuffer + 0x100 + LW_SPARKLE_PAL * 0x10 + 13, sizeof(sSparklePal));
	EnablePaletteSync();

	PlaySoundEffect(SONG_SE_UPDATE);
	proc->timer = 0;
}

static void LW_Next(struct ProcLearnWindow *proc)
{
	for (;;) {
		struct Unit *unit;

		if (!PopGaidenMagicLearnWindow(&proc->uid, &proc->iid)) {
			Proc_Break(proc);
			return;
		}

		unit = GetUnit(proc->uid);
		if (UNIT_IS_VALID(unit) && !(unit->state & (US_DEAD | US_NOT_DEPLOYED)))
			break;
	}

	LW_Draw(proc);
}

static void LW_Init(struct ProcLearnWindow *proc)
{
	int i;

	proc->chr_counter = gActiveFont->chr_counter;

	for (i = 0; i < 16; i++)
		proc->saved_pal[i] = gPaletteBuffer[LW_NAME_PAL * 16 + i];

	LW_Next(proc);
}

static void LW_DrawSprites(struct ProcLearnWindow *proc)
{
	int cx = (LW_X + 1) * 8 + (LW_PANEL_W - 2) * 4;   /* the panel's middle */
	int cy = (LW_Y + 1) * 8 + (LW_H - 2) * 4;
	int scale = proc->timer < 8 ? 0x100 - proc->timer * 0x10 : 0x80; /* 1x to 2x as it opens */
	int i;

	/* the icon at twice its size: a double-size affine 16x16, its 32x32 box centred */
	SetObjAffine(LW_AFFINE, scale, 0, 0, scale);
	PutSpriteExt(0, OAM1_X(cx - 16) | OAM1_AFFINE_ID(LW_AFFINE), OAM0_Y(cy - 16) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE,
		gObject_16x16, OAM2_CHR(LW_ICON_CHR) | OAM2_PAL(LW_OBJ_PAL) | OAM2_LAYER(0));

	/* the ring: sparkles turning slowly round it, each twinkling in turn */
	for (i = 0; i < LW_SPARKLES; i++) {
		int a = (proc->timer >> 2) + i * 64 / LW_SPARKLES;
		int x = cx + (LW_Sin(a + 16) * LW_RING_R >> 8) - 4;
		int y = cy + (LW_Sin(a) * LW_RING_R >> 8) - 4;
		int frame = ((proc->timer >> 3) + i) & 1;

		PutSpriteExt(0, OAM1_X(x), OAM0_Y(y), gObject_8x8,
			OAM2_CHR(LW_SPARKLE_CHR + frame) | OAM2_PAL(LW_SPARKLE_PAL) | OAM2_LAYER(0));
	}

	/* the tab: 32x32, 32x32 and 16x32 over its 80x32, under BG0's text */
	PutSpriteExt(0, OAM1_X(LW_TAB_X * 8), OAM0_Y(LW_TAB_Y * 8), gObject_32x32,
		OAM2_CHR(LW_TAB_CHR) | OAM2_PAL(LW_TAB_PAL) | OAM2_LAYER(1));
	PutSpriteExt(0, OAM1_X(LW_TAB_X * 8 + 32), OAM0_Y(LW_TAB_Y * 8), gObject_32x32,
		OAM2_CHR(LW_TAB_CHR + 4) | OAM2_PAL(LW_TAB_PAL) | OAM2_LAYER(1));
	PutSpriteExt(0, OAM1_X(LW_TAB_X * 8 + 64), OAM0_Y(LW_TAB_Y * 8), gObject_16x32,
		OAM2_CHR(LW_TAB_CHR + 8) | OAM2_PAL(LW_TAB_PAL) | OAM2_LAYER(1));
}

static void LW_Loop(struct ProcLearnWindow *proc)
{
	proc->timer++;
	LW_ApplyNamePalette();
	LW_DrawSprites(proc);

	if (proc->timer < LW_MIN_FRAMES)
		return;

	if (gKeyStatusPtr->newKeys & (A_BUTTON | B_BUTTON)) {
		PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
		LW_Next(proc);
	}
}

static void LW_End(struct ProcLearnWindow *proc)
{
	int i;

	LW_ClearWindow();
	for (i = 0; i < 16; i++)
		gPaletteBuffer[LW_NAME_PAL * 16 + i] = proc->saved_pal[i];
	EnablePaletteSync();
	gActiveFont->chr_counter = proc->chr_counter;
}

static const struct ProcCmd ProcScr_GaidenMagicLearnWindow[] = {
	PROC_NAME("GaidenMagicLearnWindow"),
	PROC_YIELD,
	PROC_CALL(LW_Init),
	PROC_REPEAT(LW_Loop),
	PROC_CALL(LW_End),
	PROC_END,
};

/* gPostActionFuncs: the New Spell windows for the spells the action taught */
bool PostAction_GaidenMagicLearnWindow(ProcPtr parent)
{
	if (!IsGaidenMagicLearnByLevel() || !HasGaidenMagicLearnWindow())
		return false;

	Proc_StartBlocking(ProcScr_GaidenMagicLearnWindow, parent);
	return true;
}
