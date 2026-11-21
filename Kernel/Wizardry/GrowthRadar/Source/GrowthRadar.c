#include "common-chax.h"
#include "kernel-lib.h"
#include "stat-screen.h"
#include "skill-system.h"
#include "lvup.h"
#include "strmag.h"
#include "growth-radar.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"

/*
 * Growth Radar: the stat screen's "Potential" page. An octagonal radar chart with one axis per stat (HP at the top,
 * then clockwise Str, Mag, Skl, Spd, Lck, Def, Res) and two layers: the unit's growth rates (green, the same values a
 * level-up rolls against, so Class Growths count when that option is on) and its base stats as a share of their caps
 * (blue, the caps page 0's bars use). Where the two overlap the fill is teal.
 *
 * The chart is drawn like a bitmap into 4bpp tiles in gGenericBuffer, then copied to BG2 tiles the stat screen leaves
 * free (see GrowthRadar_Chr) and mapped through the page's BG2 scratch tilemap, so it slides with the page. The grid
 * (rings and spokes) is a fixed image, Img_GrowthRadarGrid (Gfx/make-grid.py), copied in first; the layers are filled
 * a row at a time on whole words of 8 pixels, and recolour the grid lines they cover. The chart grows from the
 * centre over GR_ANIM_FRAMES frames: a child proc of the stat screen draws one step a frame and stops on the last one
 * or as soon as the page is left.
 */

#define GR_CELLS        12                      /* the chart is 12x12 tiles... */
#define GR_SIZE         (GR_CELLS * 8)          /* ...96x96 pixels */
#define GR_RADIUS       44                      /* pixels from the centre to a 100% vertex */
#define GR_CENTER       ((47 << 8) + 0x80)      /* 47.5 pixels, Q8 */
#define GR_TM_X         3                       /* page tile of the chart's top left */
#define GR_TM_Y         2
#define GR_PAL          10                      /* BG palette bank: unused on every stat screen page */
#define GR_CELL_AMT     (GR_CELLS * GR_CELLS)   /* 144 chart tiles */
#define GR_TILE_AMT     (GR_CELL_AMT + 4)       /* and two 1x2 legend swatches (then FillLayer's rows) */
#define GR_ANIM_FRAMES  16
#define GR_AXES         8

/* BG2's character base is 0x06008000 on the stat screen */
#define GR_BG2_CHR(chr) ((void *)(0x06008000 + (chr) * 0x20))

enum {
	GR_COL_NONE,
	GR_COL_GRID,        /* rings and spokes over the window */
	GR_COL_RING,        /* the 100% ring */
	GR_COL_G_FILL,      /* growths */
	GR_COL_G_GRID,      /* a grid line seen through the growth fill */
	GR_COL_S_FILL,      /* stats against caps */
	GR_COL_S_GRID,
	GR_COL_O_FILL,      /* both */
	GR_COL_O_GRID,
	GR_COL_G_LINE,
	GR_COL_S_LINE,
};

#define GR_RGB(r, g, b) ((r) | ((g) << 5) | ((b) << 10))

static const u16 sGrowthRadarPal[16] = {
	[GR_COL_NONE]   = 0,
	[GR_COL_GRID]   = GR_RGB(9, 10, 15),
	[GR_COL_RING]   = GR_RGB(18, 19, 24),
	[GR_COL_G_FILL] = GR_RGB(5, 13, 6),
	[GR_COL_G_GRID] = GR_RGB(9, 18, 10),
	[GR_COL_S_FILL] = GR_RGB(5, 8, 18),
	[GR_COL_S_GRID] = GR_RGB(9, 12, 23),
	[GR_COL_O_FILL] = GR_RGB(4, 12, 14),
	[GR_COL_O_GRID] = GR_RGB(8, 17, 19),
	[GR_COL_G_LINE] = GR_RGB(21, 30, 18),
	[GR_COL_S_LINE] = GR_RGB(18, 25, 31),
};

/* GrowthRadar_Installer.event: the grid, 144 tiles in chart order (Gfx/make-grid.py) */
extern const u32 Img_GrowthRadarGrid[];

/* Axis directions, clockwise from the top, Q8 (181 = 256 / sqrt 2) */
static const s16 sDirX[GR_AXES] = { 0, 181, 256, 181, 0, -181, -256, -181 };
static const s16 sDirY[GR_AXES] = { -256, -181, 0, 181, 256, 181, 0, -181 };

struct GrowthRadarLabel {
	u8 text;        /* gStatScreen.text slot */
	u8 x, y;        /* page tile */
	u8 width;       /* pixels the line may use */
	u8 align;       /* GR_ALIGN_* */
	u8 stacked;     /* the number goes on a second text two rows down (num_text) */
	u8 num_text;
	u16 msg;        /* the stat's name */
};

enum { GR_ALIGN_LEFT, GR_ALIGN_CENTER, GR_ALIGN_RIGHT };

/* Text slots: those the other pages clear and reuse (14-20, 29-34) */
static const struct GrowthRadarLabel sGrowthRadarLabels[GR_AXES] = {
	{ 15,  6,  0, 48, GR_ALIGN_CENTER, false,  0, 0x4E9 }, /* HP */
	{ 16, 13,  2, 40, GR_ALIGN_LEFT,   false,  0, 0x4FE }, /* Str */
	{ 29, 15,  6, 24, GR_ALIGN_LEFT,   true,  30, 0x4FF }, /* Mag */
	{ 17, 13, 12, 40, GR_ALIGN_LEFT,   false,  0, 0x4EC }, /* Skill */
	{ 18,  6, 14, 48, GR_ALIGN_CENTER, false,  0, 0x4ED }, /* Spd */
	{ 19,  0, 12, 40, GR_ALIGN_RIGHT,  false,  0, 0x4EE }, /* Lck */
	{ 31,  0,  6, 24, GR_ALIGN_RIGHT,  true,  32, 0x4EF }, /* Def */
	{ 20,  0,  2, 40, GR_ALIGN_RIGHT,  false,  0, 0x4F0 }, /* Res */
};

#define GR_LEGEND_TEXT  34
#define GR_LEGEND_X     2
#define GR_LEGEND_Y     16
#define GR_LEGEND_X2    10  /* page tile of the second legend entry's text */

struct ProcGrowthRadar {
	PROC_HEADER;

	u16 growth[GR_AXES];  /* 2C: Q8 share of the chart's radius */
	u16 stat[GR_AXES];    /* 3C */
	s16 timer;            /* 4C */
};

/* Chart tile i (cell y * 12 + x, then the swatches) to a BG2 tile the stat screen leaves free on every page */
static int GrowthRadar_Chr(int i)
{
	/* 0x82-0x9F: blank end of the 0x40 sheet (gUnknown_08A01F24); 0xA0-0xDF: left over from the map */
	if (i < 94)
		return 0x82 + i;

	/* 0x4F-0x7F: blank middle of the same sheet */
	if (i < 94 + 49)
		return 0x4F + (i - 94);

	/* 0x37-0x3F: after the stat bars (0x01-0x36), left over from the map */
	return 0x37 + (i - 94 - 49);
}

void GrowthRadar_RestoreVram(void)
{
	if (!IsStatScreenPageAvailable(PAGE_GROWTH_RADAR))
		return;

	/* The sheet holds 0x40-0x9F; the other tiles the chart takes are not shown by any other page */
	Decompress(gUnknown_08A01F24, GR_BG2_CHR(0x40));
}

static inline u32 *GrowthRadar_Word(u32 *buf, int x, int y)
{
	return buf + ((((y >> 3) * GR_CELLS + (x >> 3)) << 3) | (y & 7));
}

/* A layer's edge: every point is inside the chart (the vertices are at most GR_RADIUS from its centre) */
static void GrowthRadar_Line(u32 *buf, int x0, int y0, int x1, int y1, int color)
{
	int dx = ABS(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -ABS(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy;

	for (;;) {
		u32 *word = GrowthRadar_Word(buf, x0, y0);
		int shift = (x0 & 7) * 4;
		int e2;

		*word = (*word & ~(0xFu << shift)) | ((u32)color << shift);

		if (x0 == x1 && y0 == y1)
			break;

		e2 = 2 * err;

		if (e2 >= dy) {
			err += dy;
			x0 += sx;
		}

		if (e2 <= dx) {
			err += dx;
			y0 += sy;
		}
	}
}

#define GR_REP(color) ((u32)(color) * 0x11111111)

/* 0xF in each nibble of word that holds color */
static inline u32 GrowthRadar_NibbleEq(u32 word, int color)
{
	u32 x = word ^ GR_REP(color);

	x |= x >> 1;
	x |= x >> 2;
	x = ~x & 0x11111111;
	return x * 0xF;
}

/*
 * Fill pixels xa..xb of row y. Growths: the grid under them turns green. Stats: teal over growths, and the grid under
 * them turns blue (or light teal over growths). The 100% ring stays.
 */
static void GrowthRadar_Span(u32 *buf, int y, int xa, int xb, bool stat)
{
	if (xa < 0)
		xa = 0;

	if (xb > GR_SIZE - 1)
		xb = GR_SIZE - 1;

	while (xa <= xb) {
		int end = MIN(xa | 7, xb);
		int lo = (xa & 7) * 4, hi = ((end & 7) + 1) * 4;
		u32 mask = (hi == 32 ? 0xFFFFFFFF : ((1u << hi) - 1)) & ~((1u << lo) - 1);
		u32 *word = GrowthRadar_Word(buf, xa, y);
		u32 old = *word;
		u32 grid, ring, fill;

		/* Most words are all empty, or all growth under the stats */
		if (old == 0) {
			fill = GR_REP(stat ? GR_COL_S_FILL : GR_COL_G_FILL);
			*word = (old & ~mask) | (fill & mask);
			xa = end + 1;
			continue;
		}

		if (stat && old == GR_REP(GR_COL_G_FILL)) {
			*word = (old & ~mask) | (GR_REP(GR_COL_O_FILL) & mask);
			xa = end + 1;
			continue;
		}

		grid = GrowthRadar_NibbleEq(old, GR_COL_GRID);
		ring = GrowthRadar_NibbleEq(old, GR_COL_RING);

		if (stat) {
			u32 g_fill = GrowthRadar_NibbleEq(old, GR_COL_G_FILL);
			u32 g_grid = GrowthRadar_NibbleEq(old, GR_COL_G_GRID);

			fill = (GR_REP(GR_COL_O_FILL) & g_fill) | (GR_REP(GR_COL_O_GRID) & g_grid) |
				(GR_REP(GR_COL_S_GRID) & grid) | (GR_REP(GR_COL_RING) & ring) |
				(GR_REP(GR_COL_S_FILL) & ~(g_fill | g_grid | grid | ring));
		} else {
			fill = (GR_REP(GR_COL_G_GRID) & grid) | (GR_REP(GR_COL_RING) & ring) |
				(GR_REP(GR_COL_G_FILL) & ~(grid | ring));
		}

		*word = (old & ~mask) | (fill & mask);
		xa = end + 1;
	}
}

/* Vertices of a layer, Q8 pixels: share[k] is Q8 of the radius, scale the animation's Q8 step */
static void GrowthRadar_Vertices(const u16 *share, int scale, s32 *vx, s32 *vy)
{
	int k;

	for (k = 0; k < GR_AXES; k++) {
		s32 r = (GR_RADIUS * share[k] * scale) >> 8;

		vx[k] = GR_CENTER + ((sDirX[k] * r) >> 8);
		vy[k] = GR_CENTER + ((sDirY[k] * r) >> 8);
	}
}

/*
 * Even-odd scanline fill of a layer (star-shaped from the centre). Each edge first adds its crossing to every row it
 * spans (stepping x by its slope), in a table after the tiles in gGenericBuffer; then each row's crossings are sorted
 * and filled in pairs. Pixel x is in when its centre x + 0.5 is.
 */
static void GrowthRadar_FillLayer(u32 *buf, const s32 *vx, const s32 *vy, bool stat)
{
	u8 *count = (u8 *)(buf + GR_TILE_AMT * 8);
	s16 *cross = (s16 *)(count + GR_SIZE);
	int ymin = GR_SIZE, ymax = 0;
	int k, y;

	CpuFill32(0, count, GR_SIZE);

	for (k = 0; k < GR_AXES; k++) {
		int n = (k + 1) & 7;
		s32 x0 = vx[k], y0 = vy[k], x1 = vx[n], y1 = vy[n];
		s32 slope, x;
		int ra, rb;

		if (y0 == y1)
			continue;

		if (y0 > y1) {
			s32 t;

			t = x0, x0 = x1, x1 = t;
			t = y0, y0 = y1, y1 = t;
		}

		/* rows whose centre is in [y0, y1) */
		ra = (y0 - 0x80 + 0xFF) >> 8;
		rb = (y1 - 0x80 + 0xFF) >> 8;

		if (ra < 0)
			ra = 0;

		if (rb > GR_SIZE)
			rb = GR_SIZE;

		if (ra >= rb)
			continue;

		ymin = MIN(ymin, ra);
		ymax = MAX(ymax, rb);

		slope = Div((x1 - x0) << 8, y1 - y0);
		x = x0 + ((((ra << 8) + 0x80 - y0) * slope) >> 8);

		for (y = ra; y < rb; y++) {
			if (count[y] < GR_AXES)
				cross[y * GR_AXES + count[y]++] = x;

			x += slope;
		}
	}

	for (y = ymin; y < ymax; y++) {
		s16 *xs = cross + y * GR_AXES;
		int cnt = count[y], i, j;

		for (i = 1; i < cnt; i++) {
			s16 v = xs[i];

			for (j = i; j > 0 && xs[j - 1] > v; j--)
				xs[j] = xs[j - 1];

			xs[j] = v;
		}

		for (i = 0; i + 1 < cnt; i += 2)
			GrowthRadar_Span(buf, y, (xs[i] - 0x80 + 0xFF) >> 8, ((xs[i + 1] - 0x80 + 0xFF) >> 8) - 1, stat);
	}
}

static void GrowthRadar_Outline(u32 *buf, const s32 *vx, const s32 *vy, int color)
{
	int k;

	for (k = 0; k < GR_AXES; k++) {
		int n = (k + 1) & 7;

		GrowthRadar_Line(buf,
			vx[k] >> 8, vy[k] >> 8,
			vx[n] >> 8, vy[n] >> 8,
			color);
	}
}

/* A legend swatch: two tiles (one above the other), a 7x7 square from 4 pixels down */
static void GrowthRadar_Swatch(u32 *tiles, int fill, int line)
{
	int y, x;

	for (y = 0; y < 7; y++) {
		int row = y + 4;
		u32 *word = tiles + ((row >> 3) << 3) + (row & 7);

		for (x = 0; x < 7; x++) {
			int color = (y == 0 || y == 6 || x == 0 || x == 6) ? line : fill;

			*word |= (u32)color << (x * 4 + 4);
		}
	}
}

static void GrowthRadar_Render(const u16 *growth, const u16 *stat, int scale)
{
	u32 *buf = (u32 *)gGenericBuffer;
	s32 gx[GR_AXES], gy[GR_AXES], sx[GR_AXES], sy[GR_AXES];

	CpuFastCopy(Img_GrowthRadarGrid, buf, GR_CELL_AMT * 0x20);
	CpuFastFill(0, buf + GR_CELL_AMT * 8, (GR_TILE_AMT - GR_CELL_AMT) * 0x20);

	/* The layers, then their edges */
	GrowthRadar_Vertices(growth, scale, gx, gy);
	GrowthRadar_Vertices(stat, scale, sx, sy);
	GrowthRadar_FillLayer(buf, gx, gy, false);
	GrowthRadar_FillLayer(buf, sx, sy, true);
	GrowthRadar_Outline(buf, gx, gy, GR_COL_G_LINE);
	GrowthRadar_Outline(buf, sx, sy, GR_COL_S_LINE);

	/* The legend's swatches */
	GrowthRadar_Swatch(buf + GR_CELL_AMT * 8, GR_COL_G_FILL, GR_COL_G_LINE);
	GrowthRadar_Swatch(buf + (GR_CELL_AMT + 2) * 8, GR_COL_S_FILL, GR_COL_S_LINE);

	CpuFastCopy(buf, GR_BG2_CHR(GrowthRadar_Chr(0)), 94 * 0x20);
	CpuFastCopy(buf + 94 * 8, GR_BG2_CHR(GrowthRadar_Chr(94)), 49 * 0x20);
	CpuFastCopy(buf + (94 + 49) * 8, GR_BG2_CHR(GrowthRadar_Chr(94 + 49)), (GR_TILE_AMT - 94 - 49) * 0x20);
}

/* Q8 of the radius for value out of max, at most the full radius */
static u16 GrowthRadar_Share(int value, int max)
{
	if (value <= 0 || max <= 0)
		return 0;

	if (value >= max)
		return 0x100;

	return Div(value << 8, max);
}

NOINLINE static int GrowthRadar_LimitBreaker(struct Unit *unit)
{
	int bonus = 0;

#if defined(SID_LimitBreaker) && (COMMON_SKILL_VALID(SID_LimitBreaker))
	if (SkillTester(unit, SID_LimitBreaker))
		bonus = SKILL_EFF0(SID_LimitBreaker);
#endif

#if defined(SID_LimitBreakerPlus) && (COMMON_SKILL_VALID(SID_LimitBreakerPlus))
	if (SkillTesterPlus(unit, SID_LimitBreakerPlus))
		bonus = SKILL_EFF0(SID_LimitBreakerPlus);
#endif

	return bonus;
}

/* The stats and caps page 0's bars show (HP against its class cap) */
static void GrowthRadar_GetStats(struct Unit *unit, int *stat, int *cap)
{
	int lb = GrowthRadar_LimitBreaker(unit);

	stat[CLASS_GROWTH_HP]  = unit->maxHP;
	cap[CLASS_GROWTH_HP]   = UNIT_MHP_MAX(unit) + lb;
	stat[CLASS_GROWTH_POW] = unit->pow;
	cap[CLASS_GROWTH_POW]  = UNIT_POW_MAX(unit) + lb;
	stat[CLASS_GROWTH_MAG] = UNIT_MAG(unit);
	cap[CLASS_GROWTH_MAG]  = GetUnitMaxMagic(unit) + lb;
	stat[CLASS_GROWTH_SKL] = unit->skl;
	cap[CLASS_GROWTH_SKL]  = UNIT_SKL_MAX(unit) + lb;
	stat[CLASS_GROWTH_SPD] = unit->spd;
	cap[CLASS_GROWTH_SPD]  = UNIT_SPD_MAX(unit) + lb;
	stat[CLASS_GROWTH_LCK] = unit->lck;
	cap[CLASS_GROWTH_LCK]  = UNIT_LCK_MAX(unit) + lb;
	stat[CLASS_GROWTH_DEF] = unit->def;
	cap[CLASS_GROWTH_DEF]  = UNIT_DEF_MAX(unit) + lb;
	stat[CLASS_GROWTH_RES] = unit->res;
	cap[CLASS_GROWTH_RES]  = UNIT_RES_MAX(unit) + lb;
}

/* Ease out: fast at first, slowing into place */
static int GrowthRadar_Scale(int timer)
{
	int left = GR_ANIM_FRAMES - timer;

	if (left <= 0)
		return 0x100;

	return 0x100 - Div(0x100 * left * left, GR_ANIM_FRAMES * GR_ANIM_FRAMES);
}

static void GrowthRadar_Loop(struct ProcGrowthRadar *proc)
{
	if (TranslateStatPageId(gStatScreen.page) != PAGE_GROWTH_RADAR) {
		Proc_End(proc);
		return;
	}

	proc->timer++;
	GrowthRadar_Render(proc->growth, proc->stat, GrowthRadar_Scale(proc->timer));

	if (proc->timer >= GR_ANIM_FRAMES)
		Proc_End(proc);
}

static const struct ProcCmd ProcScr_GrowthRadar[] = {
	PROC_NAME("GrowthRadar"),
	PROC_YIELD, /* Proc_Start runs a script at once: start drawing from the next frame */
	PROC_REPEAT(GrowthRadar_Loop),
	PROC_END,
};

/* "123" in buf */
static void GrowthRadar_NumStr(char *buf, int n)
{
	char tmp[6];
	int len = 0;

	if (n < 0)
		n = 0;

	do {
		int q = Div(n, 10);

		tmp[len++] = '0' + (n - q * 10);
		n = q;
	} while (n > 0 && len < 5);

	while (len > 0)
		*buf++ = tmp[--len];

	*buf = '\0';
}

static void GrowthRadar_DrawLabel(const struct GrowthRadarLabel *label, int growth)
{
	struct Text *text = &gStatScreen.text[label->text];
	const char *name = GetStringFromIndex(label->msg);
	char num[8];
	int name_w, num_w, x;

	GrowthRadar_NumStr(num, growth);
	name_w = GetStringTextLen(name);
	num_w = GetStringTextLen(num);

	if (label->stacked) {
		struct Text *text2 = &gStatScreen.text[label->num_text];

		ClearText(text);
		ClearText(text2);

		x = label->align == GR_ALIGN_RIGHT ? label->width - name_w - 1 : 1;
		Text_SetCursor(text, x);
		Text_SetColor(text, TEXT_COLOR_SYSTEM_GOLD);
		Text_DrawString(text, name);

		x = label->align == GR_ALIGN_RIGHT ? label->width - num_w - 1 : 1;
		Text_SetCursor(text2, x);
		Text_SetColor(text2, TEXT_COLOR_SYSTEM_GREEN);
		Text_DrawString(text2, num);

		PutText(text, gUiTmScratchA + TILEMAP_INDEX(label->x, label->y));
		PutText(text2, gUiTmScratchA + TILEMAP_INDEX(label->x, label->y + 2));
		return;
	}

	/* Name, a gap, the growth */
	switch (label->align) {
	case GR_ALIGN_RIGHT:
		x = label->width - (name_w + 4 + num_w) - 1;
		break;

	case GR_ALIGN_CENTER:
		x = (label->width - (name_w + 4 + num_w)) / 2;
		break;

	default:
		x = 1;
		break;
	}

	if (x < 0)
		x = 0;

	ClearText(text);
	Text_SetCursor(text, x);
	Text_SetColor(text, TEXT_COLOR_SYSTEM_GOLD);
	Text_DrawString(text, name);
	Text_SetCursor(text, x + name_w + 4);
	Text_SetColor(text, TEXT_COLOR_SYSTEM_GREEN);
	Text_DrawString(text, num);
	PutText(text, gUiTmScratchA + TILEMAP_INDEX(label->x, label->y));
}

static void GrowthRadar_DrawLegend(void)
{
	struct Text *text = &gStatScreen.text[GR_LEGEND_TEXT];
	int chr;

	ClearText(text);
	Text_SetCursor(text, 0);
	Text_SetColor(text, TEXT_COLOR_SYSTEM_GREEN);
	Text_DrawString(text, GetStringFromIndex(MSG_GROWTH_RADAR_LEGEND_GROWTH));
	Text_SetCursor(text, (GR_LEGEND_X2 - GR_LEGEND_X) * 8);
	Text_SetColor(text, TEXT_COLOR_SYSTEM_BLUE);
	Text_DrawString(text, GetStringFromIndex(MSG_GROWTH_RADAR_LEGEND_STATS));
	PutText(text, gUiTmScratchA + TILEMAP_INDEX(GR_LEGEND_X, GR_LEGEND_Y));

	/* The swatches, one tile left of each word */
	chr = GrowthRadar_Chr(GR_CELL_AMT);
	gUiTmScratchC[TILEMAP_INDEX(GR_LEGEND_X - 1, GR_LEGEND_Y)]      = TILEREF(chr + 0, GR_PAL);
	gUiTmScratchC[TILEMAP_INDEX(GR_LEGEND_X - 1, GR_LEGEND_Y + 1)]  = TILEREF(chr + 1, GR_PAL);
	gUiTmScratchC[TILEMAP_INDEX(GR_LEGEND_X2 - 1, GR_LEGEND_Y)]     = TILEREF(chr + 2, GR_PAL);
	gUiTmScratchC[TILEMAP_INDEX(GR_LEGEND_X2 - 1, GR_LEGEND_Y + 1)] = TILEREF(chr + 3, GR_PAL);
}

void DrawPageGrowthRadar(void)
{
	struct Unit *unit = gStatScreen.unit;
	struct ProcGrowthRadar *proc;
	int stat[GR_AXES], cap[GR_AXES];
	int i;

	for (i = 14; i < 21; i++)
		ClearText(&gStatScreen.text[i]);

	for (i = 29; i < 35; i++)
		ClearText(&gStatScreen.text[i]);

	ApplyPalette(sGrowthRadarPal, GR_PAL);

	/* The chart's tiles, in GrowthRadar_Chr order */
	for (i = 0; i < GR_CELL_AMT; i++) {
		int y = i / GR_CELLS;
		int x = i - y * GR_CELLS;

		gUiTmScratchC[TILEMAP_INDEX(GR_TM_X + x, GR_TM_Y + y)] = TILEREF(GrowthRadar_Chr(i), GR_PAL);
	}

	Proc_EndEach(ProcScr_GrowthRadar);
	proc = Proc_Start(ProcScr_GrowthRadar, Proc_Find(gProcScr_StatScreen) ?: PROC_TREE_3);

	GrowthRadar_GetStats(unit, stat, cap);

	for (i = 0; i < GR_AXES; i++) {
		int growth = GetUnitGrowthByStat(unit, i);

		proc->growth[i] = GrowthRadar_Share(growth, 100);
		proc->stat[i] = GrowthRadar_Share(stat[i], cap[i]);

		GrowthRadar_DrawLabel(&sGrowthRadarLabels[i], growth);
	}

	proc->timer = 0;
	GrowthRadar_DrawLegend();

	/* The grid at once; the layers grow in from the next frame */
	GrowthRadar_Render(proc->growth, proc->stat, 0);
}
