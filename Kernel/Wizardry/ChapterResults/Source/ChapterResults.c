#include "common-chax.h"
#include "kernel-lib.h"
#include "chapter-awards.h"
#include "chapter-results.h"
#include "extras-ui.h"
#include "constants/texts.h"

#ifdef CONFIG_DISTRICT_GUILD

/*
 * Chapter results (include/kernel/chapter-results.h): the screen after a cleared story chapter, before the awards and
 * the save menu (GameControl_CallPostChapterSaveMenu, Kernel/Wizardry/Guild/Source/ChapterAwards.c, starts it).
 *
 * The title on the Extras screens' banner, then a window: on the left the chapter's numbers, counted up one line after
 * another with a tick; on the right the MVP (a chibi portrait, the name, what won it) and the rank, a stamp (an affine
 * OBJ) that drops onto the window with a sound. A or B shows it all at once, then leaves. Layers: BG0 the text and the
 * chibi, BG1 the window, BG2 the banner, BG3 the scrolling mural. Avoid `%` and unsigned `/` (CLAUDE.md).
 */

enum {
	LINE_TURNS,
	LINE_FIELDED,
	LINE_LOST,
	LINE_DEFEATED,
	LINE_DEALT,
	LINE_TAKEN,
	LINE_FUNDS,
	LINE_COUNT,
};

/* The steps after the lines' (0 to LINE_COUNT - 1: counting that line) */
enum {
	STEP_MVP = LINE_COUNT,
	STEP_STAMP,
	STEP_DONE,
};

#define ROW_TITLE 1
#define ROW_FRAME 4
#define ROW_LINE0 5
#define LINE_W 14          /* tiles */
#define LINE_NUM_RIGHT 104 /* px: the numbers' right edge in a line */

#define MVP_X 16
#define MVP_ROW 5
#define CHIBI_X 16
#define CHIBI_ROW 7
#define DETAIL_X 21
#define RANK_ROW 13
#define HINT_ROW 17

#define CHIBI_CHR (EXTRAS_UI_FREE_TILE) /* 16 tiles after the banner's */
#define CHIBI_PAL 4
#define PAL_MURAL 14 /* and 15 */

/* The stamp: a 64x64 OBJ (2D mapping) drawn at run time, double-size affine so it can drop from twice its size */
#define STAMP_CHR 0x300
#define STAMP_PAL 2
#define STAMP_AFFINE 1
#define STAMP_CX 212 /* its centre on screen */
#define STAMP_CY 116

#define TICK_FRAMES 4     /* frames between ticks */
#define TICKS_PER_LINE 8  /* at most */
#define LINE_GAP 12       /* frames between lines */
#define MVP_FRAMES 60
#define STAMP_FRAMES 10
#define STAMP_LANDS 5 /* the first frame below its size: the squash, and the sound */

#define SE_TICK SONG_SE_SYS_CURSOR_UD1
#define SE_MVP SONG_SE_SYS_WINDOW_SELECT1
#define SE_STAMP SONG_SE_UPDATE
#define SE_LEAVE SONG_SE_SYS_WINDOW_SELECT1

struct ChapterResultsScreen {
	/* 00 */ struct Text lines[LINE_COUNT];
	/* 38 */ s32 value[LINE_COUNT];
	/* 54 */ s32 shown;         /* the counting line's value so far */
	/* 58 */ u8 step;
	/* 59 */ u8 timer;
	/* 5A */ u8 rank;
	/* 5B */ s8 mvp;            /* an index in gChapterAwards.units, or -1 */
	/* 5C */ u8 stamp_frame;    /* frames since the stamp started */
	/* 5D */ u8 pad[3];
};

/* It shares the awards screen's RAM (gChapterAwardsScreen, include/link/config-memmap.s): the awards come after it */
extern struct ChapterResultsScreen gChapterResultsScreen;
#define CR (&gChapterResultsScreen)

typedef char ChapterResultsScreen_SizeCheck[(sizeof(struct ChapterResultsScreen) <= 0x60) ? 1 : -1];

struct ChapterResultsProc {
	PROC_HEADER;
	/* 29 */ u8 save; /* the save menu (and awards) follow: vanilla has none after chapter 0x38 */
};

static const u16 sLineMsgs[LINE_COUNT] = {
	[LINE_TURNS]    = MSG_CHAPTER_RESULTS_TURNS,
	[LINE_FIELDED]  = MSG_CHAPTER_RESULTS_FIELDED,
	[LINE_LOST]     = MSG_CHAPTER_RESULTS_LOST,
	[LINE_DEFEATED] = MSG_CHAPTER_RESULTS_DEFEATED,
	[LINE_DEALT]    = MSG_CHAPTER_RESULTS_DEALT,
	[LINE_TAKEN]    = MSG_CHAPTER_RESULTS_TAKEN,
	[LINE_FUNDS]    = MSG_CHAPTER_RESULTS_FUNDS,
};

/* The rank letters, 16x16, one bit per pixel (bit 15 the left) */
static const u16 sRankGlyphs[5][16] = {
	{ /* S */
		0x0000, 0x0FF0, 0x1FF8, 0x3C3C, 0x381C, 0x3C00, 0x1FE0, 0x0FF8,
		0x007C, 0x003C, 0x381C, 0x3C3C, 0x1FF8, 0x0FF0, 0x0000, 0x0000,
	},
	{ /* A */
		0x0000, 0x0180, 0x03C0, 0x03C0, 0x07E0, 0x0660, 0x0E70, 0x0E70,
		0x1E78, 0x1FF8, 0x3FFC, 0x381C, 0x781E, 0x781E, 0x0000, 0x0000,
	},
	{ /* B */
		0x0000, 0x3FE0, 0x3FF0, 0x3878, 0x3838, 0x3878, 0x3FF0, 0x3FF8,
		0x383C, 0x381C, 0x383C, 0x3FFC, 0x3FF8, 0x3FE0, 0x0000, 0x0000,
	},
	{ /* C */
		0x0000, 0x0000, 0x07F0, 0x1FF8, 0x3C3C, 0x781C, 0x7000, 0x7000,
		0x7000, 0x7000, 0x781C, 0x3C3C, 0x1FF8, 0x07F0, 0x0000, 0x0000,
	},
	{ /* D */
		0x0000, 0x0000, 0x3FC0, 0x3FF0, 0x3878, 0x383C, 0x381C, 0x381C,
		0x381C, 0x381C, 0x383C, 0x3878, 0x3FF0, 0x3FC0, 0x0000, 0x0000,
	},
};

/* The stamp's ink and its dark edge, per rank; the paper is the same for all */
static const u16 sRankInk[5][2] = {
	{ RGB(31, 23, 2), RGB(18, 10, 0) },  /* S gold */
	{ RGB(29, 5, 4), RGB(15, 1, 1) },    /* A red */
	{ RGB(5, 12, 29), RGB(1, 4, 15) },   /* B blue */
	{ RGB(6, 22, 6), RGB(1, 10, 1) },    /* C green */
	{ RGB(16, 16, 17), RGB(7, 7, 8) },   /* D grey */
};

#define STAMP_PAPER RGB(31, 30, 26)

/* The stamp's scale per frame (0x100 is its size), dropping from twice its size, with a small squash as it lands */
static const u16 sStampScale[STAMP_FRAMES] = { 0x200, 0x1C0, 0x180, 0x148, 0x118, 0xF0, 0xE6, 0xF0, 0xFA, 0x100 };

/* ---- The numbers ---- */

static int CR_CountDeadPlayers(void)
{
	int i, n = 0;

	for (i = FACTION_BLUE + 1; i < FACTION_GREEN; i++) {
		struct Unit *unit = GetUnit(i);

		if (UNIT_IS_VALID(unit) && (unit->state & US_DEAD))
			n++;
	}
	return n;
}

static bool CR_HasSnapshot(void)
{
	return gChapterAwards.magic == CHAPTER_AWARDS_MAGIC
		&& gChapterAwards.units[CHAPTER_RESULTS_SPARE_MARK].spare == CHAPTER_RESULTS_MARK;
}

/* The player units on the map (not dead, deployed) */
static int CR_CountFieldedPlayers(void)
{
	int i, n = 0;

	for (i = FACTION_BLUE + 1; i < FACTION_GREEN; i++) {
		struct Unit *unit = GetUnit(i);

		if (UNIT_IS_VALID(unit) && !(unit->state & (US_DEAD | US_NOT_DEPLOYED)))
			n++;
	}
	return n;
}

bool ChapterResults_PrePhase(ProcPtr proc)
{
	struct ChapterAwardsUnit *u = gChapterAwards.units;
	int fielded;
	u32 funds;

	(void)proc;
	if (gPlaySt.faction != FACTION_BLUE || gChapterAwards.magic != CHAPTER_AWARDS_MAGIC)
		return false;

	if (!CR_HasSnapshot()) {
		if (gPlaySt.chapterTurnNumber != 1)
			return false;
		funds = GetPartyGoldAmount();
		if (funds > 0xFFFFFF)
			funds = 0xFFFFFF;
		u[CHAPTER_RESULTS_SPARE_FUNDS + 0].spare = funds;
		u[CHAPTER_RESULTS_SPARE_FUNDS + 1].spare = funds >> 8;
		u[CHAPTER_RESULTS_SPARE_FUNDS + 2].spare = funds >> 16;
		u[CHAPTER_RESULTS_SPARE_DEAD].spare = CR_CountDeadPlayers();
		u[CHAPTER_RESULTS_SPARE_FIELDED].spare = 0;
		u[CHAPTER_RESULTS_SPARE_MARK].spare = CHAPTER_RESULTS_MARK;
	}

	/* the ones on the map and the ones fallen since the first phase: a unit that joins is counted from its next phase */
	fielded = CR_CountFieldedPlayers() + CR_CountDeadPlayers() - u[CHAPTER_RESULTS_SPARE_DEAD].spare;
	if (fielded > u[CHAPTER_RESULTS_SPARE_FIELDED].spare)
		u[CHAPTER_RESULTS_SPARE_FIELDED].spare = fielded > 0xFF ? 0xFF : fielded;
	return false;
}

int ChapterResults_Rank(int turns, int lost, int defeated)
{
	/* the pace: half a turn per foe defeated, plus two turns, at least three */
	int pace = 2 + ((defeated + 1) >> 1);
	int over, score;

	if (pace < 3)
		pace = 3;
	over = turns - pace;
	if (over < 0)
		over = 0;
	if (over > 8)
		over = 8;
	score = 85 + (defeated < 10 ? defeated : 10) - 25 * lost - 5 * over;

	if (score >= 90)
		return 0; /* S */
	if (score >= 80)
		return 1; /* A */
	if (score >= 70)
		return 2; /* B */
	if (score >= 55)
		return 3; /* C */
	return 4;         /* D */
}

static void CR_Compute(void)
{
	const struct ChapterAwardsUnit *it;
	int i, fielded, lost, defeated = 0, dealt = 0, taken = 0;
	int count = gChapterAwards.count < CHAPTER_AWARDS_UNITS ? gChapterAwards.count : CHAPTER_AWARDS_UNITS;
	struct ChapterAwardsUnit *u = gChapterAwards.units;

	for (i = 0; i < count; i++) {
		it = &gChapterAwards.units[i];
		defeated += it->kills;
		dealt += it->dealt;
		taken += it->taken;
	}

	/* the screen only shows with the snapshot (ChapterResults_Start) */
	lost = CR_CountDeadPlayers() - u[CHAPTER_RESULTS_SPARE_DEAD].spare;
	fielded = u[CHAPTER_RESULTS_SPARE_FIELDED].spare;
	CR->value[LINE_FUNDS] = (s32)GetPartyGoldAmount() - (s32)(u[CHAPTER_RESULTS_SPARE_FUNDS + 0].spare
		| (u[CHAPTER_RESULTS_SPARE_FUNDS + 1].spare << 8) | (u[CHAPTER_RESULTS_SPARE_FUNDS + 2].spare << 16));
	if (lost < 0)
		lost = 0;
	/* the units that fought were fielded, whenever they joined */
	if (fielded < count)
		fielded = count;
	if (fielded < lost)
		fielded = lost;

	CR->value[LINE_TURNS] = gPlaySt.chapterTurnNumber;
	CR->value[LINE_FIELDED] = fielded;
	CR->value[LINE_LOST] = lost;
	CR->value[LINE_DEFEATED] = defeated;
	CR->value[LINE_DEALT] = dealt;
	CR->value[LINE_TAKEN] = taken;

	CR->mvp = ChapterAwards_Winner(CHAPTER_AWARD_MVP);
	CR->rank = ChapterResults_Rank(CR->value[LINE_TURNS], lost, defeated);
}

/* ---- Text helpers ---- */

static char *CR_PutNumber(char *buf, s32 n)
{
	char digits[10];
	int d = 0, q;

	if (n < 0) {
		*buf++ = '-';
		n = -n;
	}
	do {
		q = Div(n, 10);
		digits[d++] = '0' + (n - q * 10);
		n = q;
	} while (n > 0 && d < 9);
	while (d > 0)
		*buf++ = digits[--d];
	*buf = 0;
	return buf;
}

static u16 *CR_Tile(int x, int y)
{
	return TILEMAP_LOCATED(gBG0TilemapBuffer, x, y);
}

/* A text drawn once: its tiles stay taken, its struct is not needed again */
static void CR_PutOnce(int x, int y, int width, int color, const char *str)
{
	struct Text t;

	InitText(&t, width);
	Text_InsertDrawString(&t, 0, color, str);
	PutText(&t, CR_Tile(x, y));
}

/* ---- Drawing ---- */

static void CR_DrawLine(int k, s32 n, bool counted)
{
	struct Text *t = &CR->lines[k];
	char buf[16], *p = buf;

	ClearText(t);
	Text_InsertDrawString(t, 0, TEXT_COLOR_SYSTEM_GOLD, GetStringFromIndex(sLineMsgs[k]));
	if (counted) {
		if (k == LINE_FUNDS && n > 0)
			*p++ = '+';
		CR_PutNumber(p, n);
		Text_InsertDrawString(t, LINE_NUM_RIGHT - GetStringTextLen(buf),
				      k == LINE_LOST && n > 0 ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_BLUE, buf);
	}
	PutText(t, CR_Tile(1, ROW_LINE0 + 2 * k));
	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void CR_DrawMvp(void)
{
	const struct ChapterAwardsUnit *it;
	char buf[24], *p;
	struct Text t;

	InitText(&t, 13);
	Text_InsertDrawString(&t, 0, TEXT_COLOR_SYSTEM_GOLD, GetStringFromIndex(MSG_AWARDS_MVP));
	if (CR->mvp < 0) {
		/* nobody fought: "No one this time." under the label */
		PutText(&t, CR_Tile(MVP_X, MVP_ROW));
		CR_PutOnce(MVP_X, CHIBI_ROW, 13, TEXT_COLOR_SYSTEM_GRAY, GetStringFromIndex(MSG_AWARDS_NOBODY));
		return;
	}
	it = &gChapterAwards.units[CR->mvp];
	Text_InsertDrawString(&t, 32, TEXT_COLOR_SYSTEM_WHITE, GetStringFromIndex(GetCharacterData(it->pid)->nameTextId));
	PutText(&t, CR_Tile(MVP_X, MVP_ROW));

	p = CR_PutNumber(buf, it->dealt);
	*p++ = ' ';
	ExtrasUi_Append(p, GetStringFromIndex(MSG_AWARDS_DAMAGE));
	CR_PutOnce(DETAIL_X, CHIBI_ROW, 8, TEXT_COLOR_SYSTEM_WHITE, buf);

	p = CR_PutNumber(buf, it->kills);
	*p++ = ' ';
	ExtrasUi_Append(p, GetStringFromIndex(MSG_AWARDS_DEFEATED));
	CR_PutOnce(DETAIL_X, CHIBI_ROW + 2, 8, TEXT_COLOR_SYSTEM_WHITE, buf);

	PutFaceChibi(GetUnitMiniPortraitId(GetUnitFromCharId(it->pid)), CR_Tile(CHIBI_X, CHIBI_ROW), CHIBI_CHR, CHIBI_PAL, 0);
	EnablePaletteSync();
}

/* A pixel of the stamp: a paper disc in an ink ring, the letter in ink at twice its size, a dark edge round the ring */
static int CR_StampPixel(int rank, int x, int y)
{
	/* twice the distance from the centre (31.5, 31.5), squared */
	int dx = 2 * x - 63, dy = 2 * y - 63, r2 = dx * dx + dy * dy;
	int gx = (x >> 1) - 8, gy = (y >> 1) - 8;

	if (r2 > 47 * 47)
		return 0;
	if (r2 > 45 * 45)
		return 2;
	if (r2 > 39 * 39)
		return 1;
	if (r2 > 37 * 37)
		return 2;
	if (gx >= 0 && gx < 16 && gy >= 0 && gy < 16 && (sRankGlyphs[rank][gy] & (0x8000 >> gx)))
		return 1;
	return 3;
}

/* The stamp's 64x64 pixels into OBJ VRAM (2D mapping: rows of 32 tiles), a word per tile row: VRAM takes no bytes */
static void CR_DrawStampGfx(int rank)
{
	u16 *pal = gPaletteBuffer + (0x10 + STAMP_PAL) * 0x10;
	int tx, ty, row, px;

	pal[0] = 0;
	pal[1] = sRankInk[rank][0];
	pal[2] = sRankInk[rank][1];
	pal[3] = STAMP_PAPER;
	EnablePaletteSync();

	for (ty = 0; ty < 8; ty++) {
		for (tx = 0; tx < 8; tx++) {
			u32 *tile = (u32 *)((u8 *)OBJ_VRAM0 + (STAMP_CHR + ty * 32 + tx) * 0x20);

			for (row = 0; row < 8; row++) {
				u32 word = 0;

				/* 4 bits a pixel, the left one lowest */
				for (px = 0; px < 8; px++)
					word |= CR_StampPixel(rank, tx * 8 + px, ty * 8 + row) << (px * 4);
				tile[row] = word;
			}
		}
	}
}

static void CR_PutStamp(void)
{
	int f = CR->stamp_frame, scale, m;

	if (CR->step < STEP_STAMP || (CR->step == STEP_STAMP && CR->timer > 0))
		return;
	scale = sStampScale[f < STAMP_FRAMES ? f : STAMP_FRAMES - 1];
	m = Div(0x10000, scale);
	SetObjAffine(STAMP_AFFINE, m, 0, 0, m);
	PutSpriteExt(0, OAM1_X(STAMP_CX - 64) | OAM1_AFFINE_ID(STAMP_AFFINE),
		     OAM0_Y(STAMP_CY - 64) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE,
		     gObject_64x64, OAM2_CHR(STAMP_CHR) | OAM2_PAL(STAMP_PAL) | OAM2_LAYER(0));
}

/* ---- Revealing ---- */

/* Starts line k's count, or the step after the lines, after a short gap */
static void CR_StartStep(int step)
{
	CR->step = step;
	CR->shown = 0;
	CR->timer = LINE_GAP;
}

/* One tick of the counting line: TICKS_PER_LINE at most, the last on its value */
static void CR_Tick(void)
{
	int k = CR->step;
	s32 target = CR->value[k], left, inc;

	if (target == CR->shown) {
		CR_DrawLine(k, target, TRUE);
		PlaySoundEffect(SE_TICK);
		CR_StartStep(k + 1);
		return;
	}

	left = target - CR->shown;
	inc = Div((left < 0 ? -left : left) + TICKS_PER_LINE - 1, TICKS_PER_LINE);
	if (inc < 1)
		inc = 1;
	if (left < 0)
		inc = -inc;
	if ((left < 0 && inc < left) || (left > 0 && inc > left))
		inc = left;

	CR->shown += inc;
	CR_DrawLine(k, CR->shown, TRUE);
	PlaySoundEffect(SE_TICK);
	CR->timer = TICK_FRAMES - 1;
	if (CR->shown == target)
		CR_StartStep(k + 1);
}

static void CR_DrawHint(void)
{
	CR_PutOnce(MVP_X, HINT_ROW, 8, TEXT_COLOR_SYSTEM_WHITE, GetStringFromIndex(MSG_AWARDS_HINT));
	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

/* Shows everything at once: the lines' values, the MVP, the stamp landed */
static void CR_ShowAll(void)
{
	int k;

	if (CR->step < LINE_COUNT)
		for (k = CR->step; k < LINE_COUNT; k++)
			CR_DrawLine(k, CR->value[k], TRUE);
	if (CR->step <= STEP_MVP)
		CR_DrawMvp();
	if (CR->step < STEP_DONE) {
		PlaySoundEffect(SE_STAMP);
		CR_DrawHint();
	}
	CR->step = STEP_DONE;
	CR->stamp_frame = STAMP_FRAMES - 1;
	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

/* ---- The proc ---- */

static void CR_Init(struct ChapterResultsProc *proc)
{
	int k;

	SetupBackgrounds(NULL);
	SetDefaultColorEffects();
	SetDispEnable(0, 0, 0, 0, 0);
	BG_Fill(BG_GetMapBuffer(0), 0);
	BG_Fill(BG_GetMapBuffer(1), 0);
	BG_Fill(BG_GetMapBuffer(2), 0);
	BG_Fill(BG_GetMapBuffer(3), 0);
	gLCDControlBuffer.bg0cnt.priority = 0;
	gLCDControlBuffer.bg1cnt.priority = 2;
	gLCDControlBuffer.bg2cnt.priority = 1;
	gLCDControlBuffer.bg3cnt.priority = 3;
	for (k = 0; k < 4; k++)
		BG_SetPosition(k, 0, 0);

	ResetText();
	SetTextFont(0);
	InitSystemTextFont();
	LoadUiFrameGraphics();

	CpuFill16(0, CR, sizeof(*CR));
	CR_Compute();

	ExtrasUi_LoadBanner();
	DrawUiFrame2(0, ROW_FRAME, 30, 16, 0);
	StartMuralBackground(proc, NULL, PAL_MURAL);

	{
		struct Text t;

		/* 20 tiles from tile 5: centred on the banner (and the screen), as the awards' title */
		InitText(&t, 20);
		ExtrasUi_PutCentred(&t, 160, TEXT_COLOR_SYSTEM_WHITE, GetStringFromIndex(MSG_CHAPTER_RESULTS_TITLE),
				    CR_Tile(5, ROW_TITLE));
	}
	for (k = 0; k < LINE_COUNT; k++) {
		InitText(&CR->lines[k], LINE_W);
		CR_DrawLine(k, 0, FALSE);
	}
	CR_PutOnce(MVP_X, RANK_ROW, 5, TEXT_COLOR_SYSTEM_GOLD, GetStringFromIndex(MSG_CHAPTER_RESULTS_RANK));
	CR_DrawStampGfx(CR->rank);
	CR_StartStep(0);
	CR->timer = 16; /* after the fade in */

	SetDispEnable(1, 1, 1, 1, 1);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT | BG3_SYNC_BIT);
}

enum { PL_CR_EXIT = 1 };

static void CR_Loop(struct ChapterResultsProc *proc)
{
	u16 keys = gKeyStatusPtr->newKeys;

	CR_PutStamp();

	if (keys & (A_BUTTON | B_BUTTON)) {
		if (CR->step == STEP_DONE) {
			PlaySoundEffect(SE_LEAVE);
			Proc_Goto(proc, PL_CR_EXIT);
			return;
		}
		CR_ShowAll();
		return;
	}

	if (CR->step == STEP_DONE)
		return;
	if (CR->timer > 0) {
		CR->timer--;
		return;
	}

	switch (CR->step) {
	case STEP_MVP:
		CR_DrawMvp();
		BG_EnableSyncByMask(BG0_SYNC_BIT);
		PlaySoundEffect(SE_MVP);
		CR->step = STEP_STAMP;
		CR->stamp_frame = 0;
		CR->timer = MVP_FRAMES;
		break;

	case STEP_STAMP:
		if (++CR->stamp_frame == STAMP_LANDS)
			PlaySoundEffect(SE_STAMP);
		if (CR->stamp_frame >= STAMP_FRAMES - 1) {
			CR->step = STEP_DONE;
			CR_DrawHint();
		}
		break;

	default:
		CR_Tick();
		break;
	}
}

/* Keeps the stamp up while the screen fades out */
static bool CR_FadeOutStamp(struct ChapterResultsProc *proc)
{
	CR_PutStamp();
	return FadeOutExists();
}

static void CR_OnEnd(struct ChapterResultsProc *proc)
{
	EndMuralBackground();
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);
	ClearBg0Bg1();
	SetDefaultColorEffects();
}

/* What vanilla's GameControl_CallPostChapterSaveMenu would start: the awards (if due) or the save menu */
static void CR_Continue(struct ChapterResultsProc *proc)
{
	if (proc->save)
		ChapterAwards_StartOrSaveMenu(proc->proc_parent);
}

const struct ProcCmd ProcScr_ChapterResults[] = {
	PROC_NAME("ChapterResults"),
	PROC_YIELD,
	PROC_SET_END_CB(CR_OnEnd),

	PROC_CALL(CR_Init),
	PROC_CALL_ARG(NewFadeIn, 0x10),
	PROC_WHILE(FadeInExists),
	PROC_REPEAT(CR_Loop),

	PROC_LABEL(PL_CR_EXIT),
	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(CR_FadeOutStamp),
	PROC_CALL(CR_Continue),
	PROC_END,
};

bool ChapterResults_Enabled(void)
{
	return gpKernelDesignerConfig->chapter_results_screen && !gPlaySt.config2.chapter_results_off;
}

bool ChapterResults_Start(ProcPtr parent, bool save)
{
	struct ChapterResultsProc *proc;

	/* a chapter the player played: the snapshot is taken at its first player phase (not in a cutscene chapter) */
	if (!ChapterResults_Enabled() || !ChapterAwards_ChapterCleared() || !CR_HasSnapshot())
		return FALSE;
	proc = Proc_StartBlocking(ProcScr_ChapterResults, parent);
	proc->save = save;
	return TRUE;
}

#endif /* CONFIG_DISTRICT_GUILD */
