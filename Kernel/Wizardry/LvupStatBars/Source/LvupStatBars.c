#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "lvup.h"
#include "strmag.h"
#include "constants/skills.h"
#include "jester_headers/custom-functions.h"

/*
 * Animated level-up stat bars (Documentation/Features/LvupStatBars.md). Both level-up windows, the map's
 * (ManimLevelUp, Lvupfx/MapLvup.c) and the battle animation's (EkrLvup, Lvupfx/EfxLvup.c), draw a stat as a 3-tile
 * gold label and a 2-tile blue value, each in its own text tiles. The glyphs end on the lower tile's row 4, so its rows
 * 6 and 7 are blank: the bar is drawn there, across the label's and the value's five lower tiles, in the text
 * palette's colours (bank 0 of both screens: the value's blue, the gain's green, the grey text's grey). Being part of
 * the text, it scrolls, slides and shifts with the window, and needs no OBJ tiles, palette or OAM entries.
 *
 * The bar is the stat against its cap, the caps the stat screen's bars use (class caps plus Limit Breaker). When a
 * stat goes up, its bar fills from the old value to the new one, a step every LVUP_BARS_STEP frames with a tick; the
 * gained part is green. A stat at its cap is grey (bar and label), before the level-up or once its fill reaches it.
 *
 * A text redraw (a new value, a grey label) clears or replaces the tiles, so each change draws the bar again, finding
 * the tiles through the tilemap buffer. On the map the level-up quote moves the window's tilemap rows: the fills end
 * first (LvupBars_Finish) and nothing is drawn after.
 */

enum {
	LVUP_BARS_STATS = 8,
	LVUP_BARS_TILES = 5,  /* the label's three tiles and the value's two */
	LVUP_BARS_W     = 38, /* the bar's pixels, from the first tile's x 1 */
	LVUP_BARS_STEP  = 4,  /* frames per point of a fill */
	LVUP_BARS_SE    = 0x66, /* the menu cursor's tick */
};

/* Text palette indices (bank 0, the system text palette, on both screens): the pixel row 6 shade and row 7 shade */
enum {
	BAR_TRACK = 9,              /* the value digits' navy outline */
	BAR_FILL_HI = 8, BAR_FILL_LO = 7, /* the value's cyan and blue */
	BAR_GAIN_HI = 14, BAR_GAIN_LO = 13, /* green text */
	BAR_CAP_HI = 5, BAR_CAP_LO = 4,   /* grey text */
};

/* The indices above, as a mask: a pixel of the bar's rows in one of them is the bar's, any other is a glyph's */
#define LVUP_BARS_COLORS ((1 << BAR_TRACK) | (1 << BAR_FILL_HI) | (1 << BAR_FILL_LO) | (1 << BAR_GAIN_HI) | \
	(1 << BAR_GAIN_LO) | (1 << BAR_CAP_HI) | (1 << BAR_CAP_LO))

enum lvup_bars_screen {
	LVUP_BARS_MANIM,
	LVUP_BARS_EKR,
};

enum {
	BAR_SHOWN  = 1 << 0, /* this stat has a bar (0-99, with a cap) */
	BAR_CAPPED = 1 << 1, /* grey */
	BAR_FILL   = 1 << 2, /* filling */
};

struct LvupBarsProc {
	PROC_HEADER;

	u8 screen;
	u8 frozen;
	u8 x, y; /* the map window's origin (PutManimLevelUpFrame) */
	u8 base[LVUP_BARS_STATS];
	u8 target[LVUP_BARS_STATS];
	u8 cur[LVUP_BARS_STATS];
	u8 sub[LVUP_BARS_STATS];
	u8 cap[LVUP_BARS_STATS];
	u8 flags[LVUP_BARS_STATS];
};

_Static_assert(sizeof(struct LvupBarsProc) <= 0x6C, "a proc is 0x6C bytes");

/* The Options row's bits share PlaySt with the rest: the struct keeps vanilla's size and offsets (types.h) */
_Static_assert(offsetof(struct PlaySt, config2) == 0x21, "PlaySt config2 offset");
_Static_assert(sizeof(struct PlaySt_OptionBits2) == 10, "PlaySt config2 size");
_Static_assert(offsetof(struct PlaySt, unk_30) == 0x30, "PlaySt unk_30 offset");
_Static_assert(offsetof(struct PlaySt, config) == 0x40, "PlaySt config offset");
_Static_assert(sizeof(struct PlaySt) == 0x4C, "PlaySt size");

static void LvupBars_Loop(struct LvupBarsProc *proc);

static const struct ProcCmd ProcScr_LvupBars[] = {
	PROC_NAME("LvupStatBars"),
	PROC_REPEAT(LvupBars_Loop),
	PROC_END,
};

static bool LvupBars_Enabled(void)
{
	return gpKernelDesignerConfig->lvup_stat_bars && !gPlaySt.config2.lvup_stat_bars_off;
}

NOINLINE static int LvupBars_LimitBreaker(struct Unit *unit)
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

/* The caps the stat screen's bars use (Page1WithBwl.c, GrowthRadar.c): HP, Str, Mag, Skl, Spd, Lck, Def, Res */
static void LvupBars_GetCaps(struct Unit *unit, int *cap)
{
	int lb = LvupBars_LimitBreaker(unit);

	cap[0] = UNIT_MHP_MAX(unit) + lb;
	cap[1] = UNIT_POW_MAX(unit) + lb;
	cap[2] = GetUnitMaxMagic(unit) + lb;
	cap[3] = UNIT_SKL_MAX(unit) + lb;
	cap[4] = UNIT_SPD_MAX(unit) + lb;
	cap[5] = UNIT_LCK_MAX(unit) + lb;
	cap[6] = UNIT_DEF_MAX(unit) + lb;
	cap[7] = UNIT_RES_MAX(unit) + lb;
}

/* The bar's first (leftmost) lower-row cell in the tilemap buffer */
static u16 *LvupBars_Cells(struct LvupBarsProc *proc, int i)
{
	if (proc->screen == LVUP_BARS_MANIM)
		return GetManimLevelUpLabelTm(proc->x, proc->y, i + 1) + 0x20;

	return gBG2TilemapBuffer + sEfxLvupPartsPos[i] + 0x20;
}

static int LvupBars_FillPx(int value, int cap)
{
	if (value >= cap)
		return LVUP_BARS_W;
	if (value <= 0)
		return 0;

	return Div(value * LVUP_BARS_W, cap);
}

/* A row of a text tile's 8 pixels with the bar's colours taken out: what is left is a glyph's descender */
static u32 LvupBars_KeepGlyph(u32 row)
{
	u32 out = 0;
	int x;

	for (x = 0; x < 8; x++) {
		int c = (row >> (4 * x)) & 0xF;

		if (c == 0 || (LVUP_BARS_COLORS >> c) & 1)
			continue;

		out |= c << (4 * x);
	}

	return out;
}

/* The two pixel rows (6 and 7) of the bar's five lower tiles */
static void LvupBars_Draw(struct LvupBarsProc *proc, int i)
{
	u16 *tm;
	u8 *chr;
	int t, x, px, basePx, curPx;
	int value = proc->cur[i];

	if (!(proc->flags[i] & BAR_SHOWN))
		return;

	tm = LvupBars_Cells(proc, i);
	chr = (u8 *)VRAM + GetBackgroundTileDataOffset(proc->screen == LVUP_BARS_MANIM ? BG_0 : BG_2);

	basePx = LvupBars_FillPx(proc->base[i], proc->cap[i]);
	curPx = LvupBars_FillPx(value, proc->cap[i]);

	/* Between two points of a fill: the part of the next point's pixels its frames have reached */
	if ((proc->flags[i] & BAR_FILL) && proc->sub[i] != 0)
		curPx += Div((LvupBars_FillPx(value + 1, proc->cap[i]) - curPx) * proc->sub[i], LVUP_BARS_STEP);

	for (t = 0; t < LVUP_BARS_TILES; t++) {
		u32 *dst;
		u32 hi, lo;

		/* Tile 0 is the BG's blank tile, shared by every empty cell: never draw into it */
		if ((tm[t] & 0x3FF) == 0)
			continue;

		dst = (u32 *)(chr + (tm[t] & 0x3FF) * CHR_SIZE);
		hi = LvupBars_KeepGlyph(dst[6]);
		lo = LvupBars_KeepGlyph(dst[7]);

		for (x = 0; x < 8; x++) {
			int a, b;

			px = t * 8 + x - 1;
			if (px < 0 || px >= LVUP_BARS_W)
				continue;

			/* A descender ("p" of Spd, "g" of Mag) stays in front of the bar */
			if (((hi | lo) >> (4 * x)) & 0xF)
				continue;

			if (proc->flags[i] & BAR_CAPPED) {
				a = BAR_CAP_HI;
				b = BAR_CAP_LO;
			} else if (px < basePx) {
				a = BAR_FILL_HI;
				b = BAR_FILL_LO;
			} else if (px < curPx) {
				a = BAR_GAIN_HI;
				b = BAR_GAIN_LO;
			} else {
				a = BAR_TRACK;
				b = BAR_TRACK;
			}

			hi |= a << (4 * x);
			lo |= b << (4 * x);
		}

		dst[6] = hi;
		dst[7] = lo;
	}
}

static void LvupBars_PutLabel(struct LvupBarsProc *proc, int i, int color)
{
	if (proc->screen == LVUP_BARS_MANIM) {
		PutManimLevelUpLabel(proc->x, proc->y, i + 1, color);
		BG_EnableSyncByMask(BG0_SYNC_BIT);
	} else {
		PutEkrLvupLabel(i, color);
		BG_EnableSyncByMask(BG2_SYNC_BIT);
	}
}

/* A stat that reached its cap: grey bar, grey label (whose new or cleared tiles get the bar again) */
static void LvupBars_Cap(struct LvupBarsProc *proc, int i)
{
	proc->flags[i] |= BAR_CAPPED;
	LvupBars_PutLabel(proc, i, TEXT_COLOR_SYSTEM_GRAY);
	LvupBars_Draw(proc, i);
}

static void LvupBars_Step(struct LvupBarsProc *proc, int i)
{
	if (++proc->sub[i] < LVUP_BARS_STEP) {
		LvupBars_Draw(proc, i);
		return;
	}

	proc->sub[i] = 0;
	proc->cur[i]++;
	PlaySoundEffect(LVUP_BARS_SE);

	if (proc->cur[i] >= proc->target[i]) {
		proc->cur[i] = proc->target[i];
		proc->flags[i] &= ~BAR_FILL;

		if (proc->cur[i] >= proc->cap[i]) {
			LvupBars_Cap(proc, i);
			return;
		}
	}

	LvupBars_Draw(proc, i);
}

static void LvupBars_Loop(struct LvupBarsProc *proc)
{
	int i;

	if (proc->frozen)
		return;

	for (i = 0; i < LVUP_BARS_STATS; i++)
		if (proc->flags[i] & BAR_FILL)
			LvupBars_Step(proc, i);
}

/* The bars of a window whose labels and values are drawn: the stats before the level-up, grey ones at their cap */
static void LvupBars_Setup(struct LvupBarsProc *proc, struct Unit *unit, const int *base, const int *target)
{
	int i, cap[LVUP_BARS_STATS];

	LvupBars_GetCaps(unit, cap);

	for (i = 0; i < LVUP_BARS_STATS; i++) {
		proc->flags[i] = 0;
		proc->sub[i] = 0;

		/* The map window draws a value of 100 or more with shared UI digit tiles: no bar there */
		if (cap[i] <= 0 || cap[i] > 255 || base[i] < 0 || base[i] > 99 || target[i] < 0 || target[i] > 99)
			continue;

		proc->base[i] = base[i];
		proc->cur[i] = base[i];
		proc->target[i] = target[i] > base[i] ? target[i] : base[i];
		proc->cap[i] = cap[i];
		proc->flags[i] = BAR_SHOWN;

		if (base[i] >= cap[i])
			LvupBars_Cap(proc, i);
		else
			LvupBars_Draw(proc, i);
	}
}

void LvupBars_StartManim(ProcPtr parent, int actor_id, int x, int y)
{
	struct LvupBarsProc *proc;
	int i, base[LVUP_BARS_STATS], target[LVUP_BARS_STATS];

	if (!LvupBars_Enabled())
		return;

	Proc_EndEach(ProcScr_LvupBars);
	proc = Proc_Start(ProcScr_LvupBars, parent);
	proc->screen = LVUP_BARS_MANIM;
	proc->frozen = false;
	proc->x = x;
	proc->y = y;

	for (i = 0; i < LVUP_BARS_STATS; i++) {
		base[i] = GetManimLevelUpBaseStat(actor_id, i + 1);
		target[i] = base[i] + GetManimLevelUpStatGain(actor_id, i + 1);
	}

	LvupBars_Setup(proc, GetUnit(gManimSt.actor[actor_id].unit->index), base, target);
}

void LvupBars_StartEkr(ProcPtr parent, struct Unit *unit)
{
	struct LvupBarsProc *proc;
	int i, base[LVUP_BARS_STATS], target[LVUP_BARS_STATS];

	if (!LvupBars_Enabled() || unit == NULL)
		return;

	Proc_EndEach(ProcScr_LvupBars);
	proc = Proc_Start(ProcScr_LvupBars, parent);
	proc->screen = LVUP_BARS_EKR;
	proc->frozen = false;

	for (i = 0; i < LVUP_BARS_STATS; i++) {
		base[i] = gEkrLvupBaseStatus[i];
		target[i] = gEkrLvupPostStatus[i];
	}

	LvupBars_Setup(proc, unit, base, target);
}

/* Stat i went up (its value is drawn again): its bar fills from the old value to the new one */
void LvupBars_Gain(int i)
{
	struct LvupBarsProc *proc = Proc_Find(ProcScr_LvupBars);

	if (proc == NULL || proc->frozen || i < 0 || i >= LVUP_BARS_STATS)
		return;

	if (!(proc->flags[i] & BAR_SHOWN))
		return;

	proc->cur[i] = proc->base[i];
	proc->sub[i] = 0;

	if (proc->target[i] > proc->base[i] && !(proc->flags[i] & BAR_CAPPED))
		proc->flags[i] |= BAR_FILL;

	LvupBars_Draw(proc, i);
}

/* Before the level-up quote: every fill ends at its new value, and the bars stay as they are */
void LvupBars_Finish(void)
{
	struct LvupBarsProc *proc = Proc_Find(ProcScr_LvupBars);
	int i;

	if (proc == NULL || proc->frozen)
		return;

	for (i = 0; i < LVUP_BARS_STATS; i++) {
		if (!(proc->flags[i] & BAR_FILL))
			continue;

		proc->flags[i] &= ~BAR_FILL;
		proc->cur[i] = proc->target[i];
		proc->sub[i] = 0;

		if (proc->cur[i] >= proc->cap[i])
			LvupBars_Cap(proc, i);
		else
			LvupBars_Draw(proc, i);
	}

	proc->frozen = true;
}

/* The battle animation's level-up: vanilla's redraw of a stat's new value, then its bar */
LYN_REPLACE_CHECK(EkrLvup_DrawUpdatedStatus);
void EkrLvup_DrawUpdatedStatus(struct ProcEkrLevelup *proc, int index)
{
	struct Text *th = gBanimText + EKRLVUP_STAT_MAX + index;

	(void)proc;

	ClearText(th);
	Text_SetCursor(th, 8);
	Text_SetColor(th, TEXT_COLOR_SYSTEM_BLUE);
	Text_DrawNumber(th, gEkrLvupBaseStatus[index]);
	PutText(th, gBG2TilemapBuffer + 3 + sEfxLvupPartsPos[index]);

	LvupBars_Gain(index);
}
