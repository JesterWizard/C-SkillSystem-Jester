#include "common-chax.h"
#include "kernel-lib.h"
#include "event.h"
#include "eventscript.h"
#include "recap.h"
#include "chapter-intro-styles.h"

/*
 * Alternative chapter intro animations, chosen on the Options screen's "Chapter Intro" row (gPlaySt.config.chapter_intro,
 * 0 = vanilla). The designer config's chapter_intro_style is the default for a new game.
 * The gate replaces the first step of vanilla's gProcScr_ChapterIntro (ChapterIntroStyles_Installer.event). Each style
 * shows the chapter title on a black screen (it ends black), then reveals the map. A, B or START skips the title.
 * The "Intro Backdrop" row picks what is behind the title: black, vanilla's scrolling runes, or the previous chapter's
 * closing background (the recap's record; vanilla's runes until there is one). The designer config's chapter_intro_bg
 * is the default for a new game. With the title band on, a see-through black bar sits behind the title on the Black and
 * Last Scene backgrounds (semi-transparent sprites; see CIS_LoadBar).
 */

enum chapter_intro_style {
	CHAPTER_INTRO_VANILLA = 0,
	CHAPTER_INTRO_SLIDE,
	CHAPTER_INTRO_CURTAIN,
	CHAPTER_INTRO_MOSAIC,
	CHAPTER_INTRO_LETTERBOX,
	CHAPTER_INTRO_STYLE_COUNT,
};

enum chapter_intro_bg {
	CHAPTER_INTRO_BG_BLACK = 0,
	CHAPTER_INTRO_BG_VANILLA,   /* vanilla's scrolling runes */
	CHAPTER_INTRO_BG_SCENE,     /* the previous chapter's closing background (the recap's record), else vanilla's */
	CHAPTER_INTRO_BG_COUNT,
};

struct ChapterIntroChoice {
	const char *name;
	const char *desc;
};

static const struct ChapterIntroChoice sStyleChoices[CHAPTER_INTRO_STYLE_COUNT] = {
	[CHAPTER_INTRO_VANILLA]   = { "Vanilla",   "The original light burst and title." },
	[CHAPTER_INTRO_SLIDE]     = { "Slide",     "The title slides in and out." },
	[CHAPTER_INTRO_CURTAIN]   = { "Curtain",   "The title fades in and out." },
	[CHAPTER_INTRO_MOSAIC]    = { "Mosaic",    "The title forms out of blocks." },
	[CHAPTER_INTRO_LETTERBOX] = { "Letterbox", "The title is wiped in and out." },
};

static const struct ChapterIntroChoice sBgChoices[CHAPTER_INTRO_BG_COUNT] = {
	[CHAPTER_INTRO_BG_BLACK]   = { "Black",      "A black screen behind the title." },
	[CHAPTER_INTRO_BG_VANILLA] = { "Vanilla",    "The original scrolling runes." },
	[CHAPTER_INTRO_BG_SCENE]   = { "Last Scene", "Where the last chapter ended." },
};

int ChapterIntro_StyleCount(void)
{
	return CHAPTER_INTRO_STYLE_COUNT;
}

int ChapterIntro_BgCount(void)
{
	return CHAPTER_INTRO_BG_COUNT;
}

const char *ChapterIntro_StyleName(int style)
{
	return sStyleChoices[style].name;
}

const char *ChapterIntro_StyleDesc(int style)
{
	return sStyleChoices[style].desc;
}

const char *ChapterIntro_BgName(int bg)
{
	return sBgChoices[bg].name;
}

const char *ChapterIntro_BgDesc(int bg)
{
	return sBgChoices[bg].desc;
}

/* style + 5 * background in gPlaySt.config.chapter_intro; anything out of range reads as the vanilla intro */
int ChapterIntro_GetStyle(void)
{
	int value = gPlaySt.config.chapter_intro;

	while (value >= CHAPTER_INTRO_STYLE_COUNT)
		value -= CHAPTER_INTRO_STYLE_COUNT;

	return value;
}

int ChapterIntro_GetBg(void)
{
	int bg = gPlaySt.config.chapter_intro / CHAPTER_INTRO_STYLE_COUNT;

	return bg < CHAPTER_INTRO_BG_COUNT ? bg : CHAPTER_INTRO_BG_BLACK;
}

bool ChapterIntro_GetBand(void)
{
	return gChapterIntroExtra.band;
}

void ChapterIntro_SetBand(bool on)
{
	gChapterIntroExtra.magic = CHAPTER_INTRO_EXTRA_MAGIC;
	gChapterIntroExtra.band = on ? 1 : 0;
}

void ChapterIntro_ResetExtra(void)
{
	ChapterIntro_SetBand(gpKernelDesignerConfig->chapter_intro_band);
}

/* Written and read as Recap_SaveTail / Recap_LoadTail do: the tail is SRAM */
void ChapterIntro_SaveTail(u8 *dst, const u32 size)
{
	if (size < sizeof(gChapterIntroExtra))
		return;

	gChapterIntroExtra.magic = CHAPTER_INTRO_EXTRA_MAGIC;
	WriteAndVerifySramFast(&gChapterIntroExtra, dst, sizeof(gChapterIntroExtra));
}

/* A save from before this share (or with no room for it) gets the designer config's defaults */
void ChapterIntro_LoadTail(u8 *src, const u32 size)
{
	struct ChapterIntroExtra buf;

	if (src && size >= sizeof(buf)) {
		ReadSramFast(src, &buf, sizeof(buf));
		if (buf.magic == CHAPTER_INTRO_EXTRA_MAGIC) {
			ChapterIntro_SetBand(buf.band);
			return;
		}
	}

	ChapterIntro_ResetExtra();
}

static void ChapterIntro_Set(int style, int bg)
{
	if (style < 0 || style >= CHAPTER_INTRO_STYLE_COUNT)
		style = CHAPTER_INTRO_VANILLA;

	if (bg < 0 || bg >= CHAPTER_INTRO_BG_COUNT)
		bg = CHAPTER_INTRO_BG_BLACK;

	gPlaySt.config.chapter_intro = style + CHAPTER_INTRO_STYLE_COUNT * bg;
}

void ChapterIntro_SetStyle(int style)
{
	ChapterIntro_Set(style, ChapterIntro_GetBg());
}

void ChapterIntro_SetBg(int bg)
{
	ChapterIntro_Set(ChapterIntro_GetStyle(), bg);
}

int ChapterIntro_DefaultOption(void)
{
	int style = gpKernelDesignerConfig->chapter_intro_style;
	int bg = gpKernelDesignerConfig->chapter_intro_bg;

	if (style >= CHAPTER_INTRO_STYLE_COUNT)
		style = CHAPTER_INTRO_VANILLA;

	if (bg >= CHAPTER_INTRO_BG_COUNT)
		bg = CHAPTER_INTRO_BG_BLACK;

	return style + CHAPTER_INTRO_STYLE_COUNT * bg;
}

/* 0x08020010: the runes on BG1 (palettes pal, pal + 1) and their scroll proc, ProcScr_ChapterIntroRuneScroll */
#define ChapterIntro_DrawRunes ((void (*)(ProcPtr proc, void *vram, int pal))0x08020011)
#define ProcScr_ChapterIntroRuneScroll ((const struct ProcCmd *)0x0859B108)

struct ChapterIntroStyleProc {
	/* 00 */ PROC_HEADER;
	/* 2C */ int style;
	/* 30 */ int timer;
	/* 34 */ int reveal;
	/* 38 */ int bg;
	/* 3C */ int titlePal;
	/* 40 */ int titleChr;
	/* 44 */ int bar;
	/* 48 */ u8 _pad[0x6C - 0x48]; /* ChapterIntro_Init_PlaySound316 writes 0x4C-0x68 */
};

#define CIS_BAR_CHR 0x380 /* OBJ tiles 0x380-0x3FF (0x06017000), all colour 1 */
#define CIS_BAR_PAL 15    /* OBJ palette 15, colour 1 black */

/* The see-through black bar behind the title: four 64x32 semi-transparent sprites */
static const u16 sObj_ChapterIntroBar[] = {
	4,
	OAM0_SHAPE_64x32 | OAM0_BLEND, OAM1_SIZE_64x32 + 0,   0,
	OAM0_SHAPE_64x32 | OAM0_BLEND, OAM1_SIZE_64x32 + 64,  0,
	OAM0_SHAPE_64x32 | OAM0_BLEND, OAM1_SIZE_64x32 + 128, 0,
	OAM0_SHAPE_64x32 | OAM0_BLEND, OAM1_SIZE_64x32 + 192, 0,
};

static void CIS_LoadBar(void)
{
	u32 *dst = (u32 *)(0x06010000 + CIS_BAR_CHR * 0x20);
	int i;

	for (i = 0; i < 0x1000 / 4; i++)
		dst[i] = 0x11111111;

	gPaletteBuffer[0x100 + CIS_BAR_PAL * 16 + 1] = 0;
	EnablePaletteSync();
}

/* PutSprite masks x to 9 bits, so -8 is 0x1F8 (the pieces then cover -8 to 247) */
static void CIS_PutBar(struct ChapterIntroStyleProc *proc)
{
	if (proc->bar)
		PutSprite(0, -8, 64, sObj_ChapterIntroBar,
			  OAM2_CHR(CIS_BAR_CHR) | OAM2_PAL(CIS_BAR_PAL) | OAM2_LAYER(1));
}

typedef bool (*ChapterIntroFrameFunc)(struct ChapterIntroStyleProc *proc, int t);

/* Quadratic easing from `from` to `to` over `len` frames, at frame t */
static int CIS_EaseOut(int from, int to, int t, int len)
{
	int r;

	if (t >= len)
		return to;

	r = len - t;
	return to + (from - to) * r * r / (len * len);
}

static int CIS_EaseIn(int from, int to, int t, int len)
{
	if (t >= len)
		return to;

	return from + (to - from) * t * t / (len * len);
}

/* Darkens every layer and the backdrop by y (0 = none, 16 = black) */
static void CIS_Darken(int y)
{
	SetBlendConfig(BLEND_EFFECT_DARKEN, 16, 8, y);
	SetBlendTargetA(1, 1, 1, 1, 1);
	SetBlendBackdropA(1);
	/*
	 * the bar's semi-transparent sprites blend with what is under them at EVA 16 / EVB 8 (half as bright),
	 * whatever the darken level
	 */
	SetBlendTargetB(0, 1, 1, 1, 0);
	SetBlendBackdropB(1);
}

/* The chapter title banner on BG0, as vanilla's intro draws it (0x08020778), with its 64 tiles from chr on */
static void CIS_DrawTitle(int pal, int chr)
{
	int idx, i;
	u16 *tm = gBG0TilemapBuffer + 0x123;

	BG_Fill(gBG0TilemapBuffer, 0x1000 | chr); /* the title's first tile is blank */
	sub_80895B4(8, pal);
	idx = GetChapterTitleWM(&gPlaySt);
	_PutChapterTitleGfx(chr, idx);

	/* as vanilla's sub_80896FC, which always uses tile 0x280 */
	for (i = 0; i < 64; i++)
		tm[i] = (pal << 12) + chr + i;

	BG_EnableSyncByMask(1);
}

/* The previous chapter's closing background, from the recap's record, or RECAP_NONE */
static int CIS_LastSceneBg(void)
{
	if (gRecap.data.magic != RECAP_MAGIC)
		return RECAP_NONE;

	if (gRecap.data.endBg != RECAP_NONE)
		return gRecap.data.endBg;

	return gRecap.data.lastBg;
}

/* The background behind the title: the previous chapter's closing background on BG3, or vanilla's runes on BG1 */
static void CIS_DrawBackground(struct ChapterIntroStyleProc *proc)
{
	int scene = RECAP_NONE;

	proc->titlePal = 5;
	proc->titleChr = 0x280;

	/* BG1-3 still hold what was on screen before (e.g. the world map's text box) */
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_Fill(gBG3TilemapBuffer, 0);
	BG_EnableSyncByMask(0xE);

	if (proc->bg == CHAPTER_INTRO_BG_SCENE) {
		scene = CIS_LastSceneBg();
		if (scene == RECAP_NONE)
			proc->bg = CHAPTER_INTRO_BG_VANILLA; /* no record yet (the prologue, an old save): vanilla's */
	}

	if (proc->bg == CHAPTER_INTRO_BG_VANILLA) {
		ChapterIntro_DrawRunes(proc, NULL, 14);
		BG_EnableSyncByMask(2);
		return;
	}

	if (proc->bg == CHAPTER_INTRO_BG_SCENE) {
		/*
		 * a 224-colour CG leaves BG palettes 2 and 3 free: the title takes 2. The CG's tiles cover
		 * vanilla's title tiles at 0x280, so the title's tiles move below them (0x80-0xBF)
		 */
		gLCDControlBuffer.bg3cnt.colorMode = 0;
		EventShowTextBgDirect(EVSUBCMD_REMOVEPORTRAITS, scene);
		proc->titlePal = 2;
		proc->titleChr = 0x80; /* a CG's tiles cover 0x280 (0x06005000) */
	}
}

/* Slide: the banner slides in from the right as it brightens, holds, and slides out to the left as it darkens */
#define SLIDE_IN   20
#define SLIDE_HOLD 100
#define SLIDE_OUT  20
#define SLIDE_DIST 48

static bool Slide_Title(struct ChapterIntroStyleProc *proc, int t)
{
	if (t < SLIDE_IN) {
		BG_SetPosition(0, -CIS_EaseOut(SLIDE_DIST, 0, t, SLIDE_IN), 0);
		CIS_Darken(CIS_EaseOut(16, 0, t, SLIDE_IN));
		return false;
	}

	t -= SLIDE_IN;
	if (t < SLIDE_HOLD) {
		BG_SetPosition(0, 0, 0);
		CIS_Darken(0);
		return false;
	}

	t -= SLIDE_HOLD;
	BG_SetPosition(0, CIS_EaseIn(0, SLIDE_DIST, t, SLIDE_OUT), 0);
	CIS_Darken(CIS_EaseIn(0, 16, t, SLIDE_OUT));
	return t >= SLIDE_OUT;
}

static bool Slide_Map(struct ChapterIntroStyleProc *proc, int t)
{
	CIS_Darken(16 - t);
	return t >= 16;
}

/* Curtain: the title fades in and out; then the map opens from the centre line like a curtain */
#define CURTAIN_IN   32
#define CURTAIN_HOLD 100
#define CURTAIN_OUT  24
#define CURTAIN_OPEN 28

static bool Curtain_Title(struct ChapterIntroStyleProc *proc, int t)
{
	if (t < CURTAIN_IN) {
		CIS_Darken(16 - t / 2);
		return false;
	}

	t -= CURTAIN_IN;
	if (t < CURTAIN_HOLD) {
		CIS_Darken(0);
		return false;
	}

	t -= CURTAIN_HOLD;
	CIS_Darken(CIS_EaseIn(0, 16, t, CURTAIN_OUT));
	return t >= CURTAIN_OUT;
}

static bool Curtain_Map(struct ChapterIntroStyleProc *proc, int t)
{
	int h = CIS_EaseOut(0, 80, t, CURTAIN_OPEN);

	if (t == 0) {
		/* inside window 0: the map as it is; outside: darkened to black */
		SetWin0Layers(1, 1, 1, 1, 1);
		gLCDControlBuffer.wincnt.win0_enableBlend = 0;
		SetWOutLayers(1, 1, 1, 1, 1);
		gLCDControlBuffer.wincnt.wout_enableBlend = 1;
		CIS_Darken(16);
		SetWinEnable(1, 0, 0);
	}

	SetWin0Box(0, 80 - h, 240, 80 + h);
	return t >= CURTAIN_OPEN;
}

/* Mosaic: the title resolves out of a coarse mosaic and dissolves back into it; then the map does the same */
#define MOSAIC_IN   32
#define MOSAIC_HOLD 100
#define MOSAIC_OUT  32
#define MOSAIC_MAP  32

/* Mosaic size m (0 = none, 15 = 16x16 blocks) for the BGs whose mosaic bit is set */
static void CIS_Mosaic(int m)
{
	gLCDControlBuffer.mosaic = m | (m << 4);
}

static bool Mosaic_Title(struct ChapterIntroStyleProc *proc, int t)
{
	gLCDControlBuffer.bg0cnt.mosaic = 1;
	gLCDControlBuffer.bg1cnt.mosaic = 1;
	gLCDControlBuffer.bg2cnt.mosaic = 1;
	gLCDControlBuffer.bg3cnt.mosaic = 1;

	if (t < MOSAIC_IN) {
		CIS_Mosaic(15 - t / 2);
		CIS_Darken(16 - t / 2);
		return false;
	}

	t -= MOSAIC_IN;
	if (t < MOSAIC_HOLD) {
		CIS_Mosaic(0);
		CIS_Darken(0);
		return false;
	}

	t -= MOSAIC_HOLD;
	if (t < MOSAIC_OUT) {
		CIS_Mosaic(t / 2);
		CIS_Darken(t / 2);
		return false;
	}

	CIS_Darken(16);
	return true;
}

static bool Mosaic_Map(struct ChapterIntroStyleProc *proc, int t)
{
	gLCDControlBuffer.bg1cnt.mosaic = 1;
	gLCDControlBuffer.bg2cnt.mosaic = 1;
	gLCDControlBuffer.bg3cnt.mosaic = 1;

	if (t < MOSAIC_MAP) {
		CIS_Mosaic(15 - t / 2);
		CIS_Darken(16 - t / 2);
		return false;
	}

	CIS_Mosaic(0);
	CIS_Darken(0);
	return true;
}

/* Letterbox: the title is wiped in from the left and out to the right; the map fades in between black bars, which pull back */
#define LETTERBOX_WIPE  40
#define LETTERBOX_HOLD  100
#define LETTERBOX_CLOSE 24
#define LETTERBOX_FADE  16
#define LETTERBOX_BARS  48
#define LETTERBOX_OPEN  24
#define LETTERBOX_BAR_H 32

static bool Letterbox_Title(struct ChapterIntroStyleProc *proc, int t)
{
	if (t == 0) {
		/*
		 * inside window 0: the title, its background and the bar (OBJ, blended); outside: nothing (the black backdrop).
		 * Effects are on inside the window so the bar blends; darken is 0 during this title, so nothing else changes
		 */
		SetWin0Layers(1, 1, 1, 1, 1);
		gLCDControlBuffer.wincnt.win0_enableBlend = 1;
		SetWOutLayers(0, 0, 0, 0, 0);
		gLCDControlBuffer.wincnt.wout_enableBlend = 0;
		CIS_Darken(0);
		SetWin0Box(0, 0, 0, 160);
		SetWinEnable(1, 0, 0);
	}

	if (t < LETTERBOX_WIPE) {
		SetWin0Box(0, 0, CIS_EaseOut(0, 240, t, LETTERBOX_WIPE), 160);
		return false;
	}

	t -= LETTERBOX_WIPE;
	if (t < LETTERBOX_HOLD) {
		SetWin0Box(0, 0, 240, 160);
		return false;
	}

	t -= LETTERBOX_HOLD;
	if (t < LETTERBOX_CLOSE) {
		SetWin0Box(CIS_EaseIn(0, 240, t, LETTERBOX_CLOSE), 0, 240, 160);
		return false;
	}

	SetWinEnable(0, 0, 0);
	CIS_Darken(16);
	return true;
}

static bool Letterbox_Map(struct ChapterIntroStyleProc *proc, int t)
{
	int h;

	if (t == 0) {
		/* inside window 0: the map, fading in; outside: nothing (the backdrop, made black) */
		gPaletteBuffer[0] = 0;
		EnablePaletteSync();
		SetWin0Layers(1, 1, 1, 1, 1);
		gLCDControlBuffer.wincnt.win0_enableBlend = 1;
		SetWOutLayers(0, 0, 0, 0, 0);
		gLCDControlBuffer.wincnt.wout_enableBlend = 0;
		SetWin0Box(0, LETTERBOX_BAR_H, 240, 160 - LETTERBOX_BAR_H);
		SetWinEnable(1, 0, 0);
	}

	if (t < LETTERBOX_BARS) {
		CIS_Darken(t < LETTERBOX_FADE ? 16 - t : 0);
		return false;
	}

	t -= LETTERBOX_BARS;
	h = CIS_EaseIn(LETTERBOX_BAR_H, 0, t, LETTERBOX_OPEN);
	SetWin0Box(0, h, 240, 160 - h);
	return t >= LETTERBOX_OPEN;
}

static const ChapterIntroFrameFunc sTitleFrames[CHAPTER_INTRO_STYLE_COUNT] = {
	[CHAPTER_INTRO_SLIDE] = Slide_Title,
	[CHAPTER_INTRO_CURTAIN] = Curtain_Title,
	[CHAPTER_INTRO_MOSAIC] = Mosaic_Title,
	[CHAPTER_INTRO_LETTERBOX] = Letterbox_Title,
};

static const ChapterIntroFrameFunc sMapFrames[CHAPTER_INTRO_STYLE_COUNT] = {
	[CHAPTER_INTRO_SLIDE] = Slide_Map,
	[CHAPTER_INTRO_CURTAIN] = Curtain_Map,
	[CHAPTER_INTRO_MOSAIC] = Mosaic_Map,
	[CHAPTER_INTRO_LETTERBOX] = Letterbox_Map,
};

static void CIS_ResetFx(void)
{
	SetWinEnable(0, 0, 0);
	gLCDControlBuffer.mosaic = 0;
	gLCDControlBuffer.bg0cnt.mosaic = 0;
	gLCDControlBuffer.bg1cnt.mosaic = 0;
	gLCDControlBuffer.bg2cnt.mosaic = 0;
	gLCDControlBuffer.bg3cnt.mosaic = 0;
	BG_SetPosition(0, 0, 0);
}

static void CIS_Init(struct ChapterIntroStyleProc *proc)
{
	proc->style = ChapterIntro_GetStyle();
	proc->timer = 0;
	proc->bg = ChapterIntro_GetBg();

	SetupBackgrounds(NULL);
	CIS_ResetFx();
	CIS_DrawBackground(proc);
	/* after the background: it may fall back to vanilla's runes, which have their own band */
	proc->bar = ChapterIntro_GetBand() && proc->bg != CHAPTER_INTRO_BG_VANILLA;
	if (proc->bar)
		CIS_LoadBar();
	SetDispEnable(1, 1, 0, 1, proc->bar ? 1 : 0);
	gPaletteBuffer[0] = 0;
	EnablePaletteSync();
	CIS_Darken(16);
	CIS_DrawTitle(proc->titlePal, proc->titleChr);
	ChapterIntro_Init_PlaySound316((void *)proc);
}

static void CIS_TitleLoop(struct ChapterIntroStyleProc *proc)
{
	CIS_PutBar(proc);

	if (gKeyStatusPtr->newKeys & (A_BUTTON | B_BUTTON | START_BUTTON)) {
		CIS_Darken(16);
		Proc_Break(proc);
		return;
	}

	if (sTitleFrames[proc->style](proc, proc->timer++))
		Proc_Break(proc);
}

/* The chapter's map music, as vanilla's ChapterIntro_LoopFadeToMap starts it */
static void CIS_StartMapBgm(void)
{
	const struct ROMChapterData *chapter = GetROMChapterStruct(gPlaySt.chapterIndex);

	if (chapter->mapBgmIds[9] != 0xFFFF)
		StartBgm(chapter->mapBgmIds[9], NULL);
}

/* Vanilla shows the map at the end of the intro only for these; for the others the opening event fades it in */
static bool CIS_ShouldRevealMap(void)
{
	if (GetBattleMapKind() == BATTLEMAP_KIND_SKIRMISH)
		return false;

	return !GetROMChapterStruct(gPlaySt.chapterIndex)->fadeToBlack;
}

static void CIS_PrepareMap(struct ChapterIntroStyleProc *proc)
{
	Proc_EndEach(ProcScr_ChapterIntroRuneScroll);
	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_Fill(gBG3TilemapBuffer, 0);
	BG_SetPosition(1, 0, 0);
	BG_SetPosition(2, 0, 0);
	BG_EnableSyncByMask(0xF);
	if (proc->bg == CHAPTER_INTRO_BG_SCENE)
		SetupBackgrounds(NULL); /* the conversation background changed BG3's setup */
	CIS_ResetFx();

	ChapterIntro_8020B20();
	BMapDispResume();
	ChapterIntro_InitMapDisplay();

	if (GetROMChapterStruct(gPlaySt.chapterIndex)->initialWeather == 5)
		WfxFlamesInitGradientPublic();

	CIS_StartMapBgm();
	proc->reveal = CIS_ShouldRevealMap();
	if (!proc->reveal)
		SetDispEnable(1, 1, 1, 0, 0); /* as vanilla: the map (BG3) and the units (OBJ) stay off */

	CIS_Darken(16);
	proc->timer = 0;
}

static void CIS_MapLoop(struct ChapterIntroStyleProc *proc)
{
	int t = proc->timer++;

	if (proc->reveal ? sMapFrames[proc->style](proc, t) : t >= 30)
		Proc_Break(proc);
}

static void CIS_End(struct ChapterIntroStyleProc *proc)
{
	CIS_ResetFx();
	ChapterIntro_80210C8();
	/* the blend vanilla's ChapterIntro_InitMapDisplay sets up, which the map expects after the intro */
	SetBlendConfig(BLEND_EFFECT_ALPHA, 16, 0, 0);
	SetBlendTargetA(0, 1, 0, 0, 0);
	SetBlendBackdropA(0);
	SetBlendTargetB(0, 0, 0, 1, 1);
	SetBlendBackdropB(1);
}

static const struct ProcCmd ProcScr_ChapterIntroStyle[] = {
	PROC_NAME("ChapterIntroStyle"),
	PROC_CALL(CIS_Init),
	PROC_YIELD,
	PROC_REPEAT(CIS_TitleLoop),
	PROC_CALL(CIS_PrepareMap),
	PROC_REPEAT(CIS_MapLoop),
	PROC_CALL(CIS_End),
	PROC_END,
};

/* gProcScr_ChapterIntro's first step (was BMapDispSuspend): with a style set, our proc takes the vanilla one's place */
void ChapterIntroStyles_Gate(ProcPtr proc)
{
	int style = ChapterIntro_GetStyle();

	BMapDispSuspend();

	if (style <= CHAPTER_INTRO_VANILLA || style >= CHAPTER_INTRO_STYLE_COUNT || !sTitleFrames[style])
		return;

	Proc_StartBlocking(ProcScr_ChapterIntroStyle, ((struct Proc *)proc)->proc_parent);
	Proc_End(proc);
}
