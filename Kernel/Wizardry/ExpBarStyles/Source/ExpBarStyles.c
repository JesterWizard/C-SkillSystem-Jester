#include "common-chax.h"
#include "kernel-lib.h"
#include "mapanim.h"
#include "ekrbattle.h"
#include "efxbattle.h"
#include "exp-bar-styles.h"

/*
 * EXP bar styles (Documentation/Features/ExpBarStyles.md). The Options screen's "UI Style" row picks a UI style
 * (gPlaySt.config.exp_bar_style holds the row's value, sUiStyles below); each UI style has a matching EXP bar style.
 * Both EXP bars use it: the map one (battle animations off) and the battle-animation one. A style has the same tiles in
 * the same order as vanilla's:
 *   img     the window: Img_EkrExpBar tiles 0-12 (frame and label)
 *   steps   the 24 fill steps of the bar (Img_EkrExpBarChange)
 *   digits  0-9 and a blank (Img_BarNumfx)
 * and its window tilemaps, so a style can arrange its window tiles differently.
 * Each new style's art is one PNG in Gfx/, drawn by Tools/expbar/expbar.py.
 */
struct ExpBarStyle {
	const char *name;
	const char *desc;
	const u8 *img;
	const u8 *steps;
	const u8 *digits;
	const u16 *pal;
	const u16 *tsa_map;	/* 17x3, CallARM_FillTileRect format */
	const u16 *tsa_battle;	/* 18x3, EfxTmCpyBG format */
};

/* The style PNGs (ExpBarStyles_Installer.event): window tiles, then the fill steps at 0x380, the digits at 0x680 */
#define EXP_BAR_STYLE_IMG(n)	extern const u8 Img_ExpBarStyle_##n[]; extern const u16 Pal_ExpBarStyle_##n[]
#define EXP_BAR_STYLE(n, d)	{ #n, d, Img_ExpBarStyle_##n, Img_ExpBarStyle_##n + 0x380, Img_ExpBarStyle_##n + 0x680, \
				  Pal_ExpBarStyle_##n, gUnknown_089AD498, Tsa_EkrExpBar }

EXP_BAR_STYLE_IMG(Blade);
EXP_BAR_STYLE_IMG(Gems);
EXP_BAR_STYLE_IMG(Pearls);
EXP_BAR_STYLE_IMG(Tech);
EXP_BAR_STYLE_IMG(Scroll);
EXP_BAR_STYLE_IMG(Vial);
EXP_BAR_STYLE_IMG(Chain);
EXP_BAR_STYLE_IMG(Stars);
EXP_BAR_STYLE_IMG(Arrow);
EXP_BAR_STYLE_IMG(Minimal);

static const struct ExpBarStyle sExpBarStyles[] = {
	{ "Vanilla", "The original EXP bar.", Img_EkrExpBar, (const u8 *)Img_EkrExpBarChange, (const u8 *)Img_BarNumfx,
	  Pal_ExpBar, gUnknown_089AD498, Tsa_EkrExpBar },
	EXP_BAR_STYLE(Blade, "A sword that charges up."),
	EXP_BAR_STYLE(Gems, "Rubies in a gold jewel box."),
	EXP_BAR_STYLE(Pearls, "Pearls on a thread."),
	EXP_BAR_STYLE(Tech, "A sci-fi display of LED segments."),
	EXP_BAR_STYLE(Scroll, "A red ribbon on a parchment scroll."),
	EXP_BAR_STYLE(Vial, "A potion vial filling up."),
	EXP_BAR_STYLE(Chain, "Iron links that turn to gold."),
	EXP_BAR_STYLE(Stars, "Stars lighting up a night sky."),
	EXP_BAR_STYLE(Arrow, "An arrow that glows to its point."),
	EXP_BAR_STYLE(Minimal, "A slim bar in a dark window."),
};

/* sExpBarStyles' rows, for the UI styles' table */
enum {
	EXP_BAR_VANILLA, EXP_BAR_BLADE, EXP_BAR_GEMS, EXP_BAR_PEARLS, EXP_BAR_TECH, EXP_BAR_SCROLL, EXP_BAR_VIAL,
	EXP_BAR_CHAIN, EXP_BAR_STARS, EXP_BAR_ARROW, EXP_BAR_MINIMAL,
};

/*
 * The UI styles the Options screen's "UI Style" row offers, in its order (Documentation/Features/VeslyUIStyles.md).
 * Each is a VeslyUI style ID (GetUI_id, Kernel/Wizardry/VeslyUI/C_Code.c) and the EXP bar style closest to its look.
 * The row's value is a save option of 4 bits: at most 16 rows.
 */
struct UiStyleOption {
	const char *name;
	const char *desc;
	u8 ui_id;
	u8 exp_bar;
};

static const struct UiStyleOption sUiStyles[] = {
	{ "Vanilla",       "The original windows and EXP bar.",   0,  EXP_BAR_VANILLA },
	{ "Wood",          "Wooden planks. EXP: a potion vial.",  4,  EXP_BAR_VIAL },
	{ "Tech",          "Neon glass panels. EXP: LED bars.",   5,  EXP_BAR_TECH },
	{ "Royal",         "Velvet and gold. EXP: rubies.",       6,  EXP_BAR_GEMS },
	{ "Stained Glass", "Leaded glass. EXP: a starry sky.",    7,  EXP_BAR_STARS },
	{ "Paper",         "Paper cutouts. EXP: a slim bar.",     8,  EXP_BAR_MINIMAL },
	{ "Ocean",         "Rope and waves. EXP: pearls.",        9,  EXP_BAR_PEARLS },
	{ "Forest",        "Woven vines. EXP: a glowing arrow.",  10, EXP_BAR_ARROW },
	{ "Parchment",     "Inked parchment. EXP: a scroll.",     11, EXP_BAR_SCROLL },
	{ "Stephano",      "Stephano's menus. EXP: a sword.",     1,  EXP_BAR_BLADE },
	{ "Gamma",         "Gamma's menus. EXP: a chain.",        2,  EXP_BAR_CHAIN },
	{ "Pikmin",        "Pikmin's menus. EXP: a starry sky.",  3,  EXP_BAR_STARS },
};

_Static_assert(ARRAY_COUNT(sUiStyles) <= 16, "the option has 4 bits");

static const struct UiStyleOption *GetUiStyleOption(int id)
{
	return &sUiStyles[(unsigned int)id < ARRAY_COUNT(sUiStyles) ? id : 0];
}

int UiStyle_Count(void)
{
	return ARRAY_COUNT(sUiStyles);
}

const char *UiStyle_Name(int id)
{
	return GetUiStyleOption(id)->name;
}

const char *UiStyle_Desc(int id)
{
	return GetUiStyleOption(id)->desc;
}

/* The VeslyUI style ID the player picked */
int UiStyle_GetUiId(void)
{
	return GetUiStyleOption(gPlaySt.config.exp_bar_style)->ui_id;
}

static const struct ExpBarStyle *GetExpBarStyle(void)
{
	return &sExpBarStyles[GetUiStyleOption(gPlaySt.config.exp_bar_style)->exp_bar];
}

/* The map EXP bar (battle animations off): vanilla's, with the style's tiles, palette and window */
LYN_REPLACE_CHECK(ProcMAExpBar_OnDraw);
void ProcMAExpBar_OnDraw(struct MAExpBarProc *proc)
{
	const struct ExpBarStyle *st = GetExpBarStyle();
	u8 *chr = (u8 *)VRAM + GetBackgroundTileDataOffset(BG_0);

	BG_SetPosition(BG_0, 0, 0);
	BG_SetPosition(BG_1, 0, 0);

	RegisterDataMove(st->img, chr + 0x4000, 13 * 0x20);
	RegisterDataMove(st->steps, chr + 0x41A0, 24 * 0x20);
	RegisterDataMove(st->digits, chr + 0x44A0, 11 * 0x20);
	CopyToPaletteBuffer(st->pal, 0x20 * 5, 0x20);

	CallARM_FillTileRect(gBG0TilemapBuffer + TILEMAP_INDEX(6, 8), st->tsa_map, TILEREF(0x200, 5));
	DrawMAExpBar(6, 8, proc->expFrom);
}

/*
 * The battle-animation EXP bar. Vanilla's ekrBattleExecExpGain and ekrBattleWaitExpBarIdle draw it; these run after
 * them (ExpBarStyles_Installer.event points the battle proc at them) and draw the same frame again with the style's
 * graphics, before the screen updates. The 13 bar tiles and two digits go to a buffer that vanilla copies to VRAM.
 */
#define EKR_EXP_BAR_CHR	((void *)(VRAM + 0x2000))
#define EKR_EXP_BAR_BUF	((u8 *)gEkrBarfxBuf + 0x100)

static void ExpBarStyle_DrawBattleBar(const struct ExpBarStyle *st, int exp)
{
	int i, rem, tens, ones;
	u8 *buf = EKR_EXP_BAR_BUF;

	/* gEkrBarfxBuf holds each bar tile's fill step (EkrModifyBarfx, called by vanilla for this exp) */
	for (i = 0; i < 13; i++)
		CpuFastSet(st->steps + gEkrBarfxBuf[i] * 0x20, buf + i * 0x20, 8);

	rem = DivRem(exp, 100);
	tens = Div(rem, 10);
	ones = rem - tens * 10;
	if (tens == 0)
		tens = 10;

	CpuFastSet(st->digits + tens * 0x20, buf + 13 * 0x20, 8);
	CpuFastSet(st->digits + ones * 0x20, buf + 14 * 0x20, 8);
	RegisterDataMove(buf, (u8 *)EKR_EXP_BAR_CHR + 13 * 0x20, 15 * 0x20);
}

void ExpBarStyle_ExecExpGain(struct ProcEkrBattle *proc)
{
	const struct ExpBarStyle *st;

	ekrBattleExecExpGain(proc);

	st = GetExpBarStyle();
	if (st == &sExpBarStyles[0])
		return;

	RegisterDataMove(st->img, EKR_EXP_BAR_CHR, 13 * 0x20);
	EfxTmCpyBG(st->tsa_battle, gBG1TilemapBuffer + TILEMAP_INDEX(6, 17), 18, 3, 1, 0x100);
	CpuFastSet(st->pal, gPaletteBuffer + 0x10, 8);
	EnablePaletteSync();
	BG_EnableSyncByMask(BG1_SYNC_BIT);

	ExpBarStyle_DrawBattleBar(st, gBanimExpGain[0] ? gBanimExpPrevious[0] : gBanimExpPrevious[1]);
}

void ExpBarStyle_WaitExpBarIdle(struct ProcEkrBattle *proc)
{
	int exp = proc->timer;	/* vanilla draws this value, then counts up */
	const struct ExpBarStyle *st;

	ekrBattleWaitExpBarIdle(proc);

	st = GetExpBarStyle();
	if (st != &sExpBarStyles[0])
		ExpBarStyle_DrawBattleBar(st, exp);
}
