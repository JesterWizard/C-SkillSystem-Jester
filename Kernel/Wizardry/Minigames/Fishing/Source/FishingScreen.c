#include "common-chax.h"
#include "kernel-lib.h"
#include "kernel/unit-icon.h"
#include "mu.h"
#include "fishing.h"
#include "fishing-art.h"
#include "constants/texts.h"

#ifdef CONFIG_DISTRICT_FISHING

/*
 * The fishing screen, a side view of a scene 448 rows tall that scrolls with the hook: sky and the text window at the
 * top, the bank on the right where the unit stands with a rod, and deep water on the left where fish swim at depths that
 * suit their size.
 *
 *   WALKIN  the unit walks in from the right edge and takes its place on the bank.
 *   READY   Left/Right pick a bait (paid on the cast). A casts. B leaves. START opens the prize table.
 *   CAST    the hook drops to the depth of the last cast.
 *   WAIT    Up/Down move the hook (the screen follows). A reels the line in (the bait stays on). A fish that likes the
 *           bait may swim to the hook, nibble (don't press A or move the hook now) and bite.
 *   BITE    press A quickly: within 8 frames is a perfect hook (+50%), within 14 a good one (+25%). Too slow and it
 *           swims away with the bait.
 *   REEL    the tension game. A pulls (raises the tension), the fish pulls back, harder during its runs. Progress is
 *           made only while the needle is in the green zone. Full tension snaps the line, no tension for a second lets
 *           the fish go, and too long tires you out. A quick landing is worth up to +50% more.
 *   LAND    a splash, then the catch: the species of this district's water for the kind of fish, a size, the payout
 *           (the fish's prize times the bonuses, up to x2, times 0.75 to 1.25 for the size) and the journal.
 *   RESULT  CAUGHT!, then the catch card: the fish's portrait, its size and best, stamped NEW! (a first catch) or
 *           RECORD! (the biggest yet). A fishes again, B leaves, START opens the journal.
 *   PRIZES  the fish journal: one page for each district's water, Left/Right to turn. A species not caught yet shows
 *           as a silhouette and "???".
 *
 * B leaves the screen from any state but the walk-in and the prize table (where it closes the table).
 *
 * Layers: BG3 the scene (palette FISHING_BG_PAL, tiles at 0x06008000, a 256x512 map at 0x06007000 written straight to
 * VRAM, scrolled with BG_SetPosition), BG1 the window frames, BG0 their text, OBJ (1D) the sprites. The shadows of the
 * fish are semi-transparent OBJs blended over BG3. Avoid `%` and unsigned `/` (CLAUDE.md): this file uses masks, Div
 * and shifts instead.
 */

#define FS (&gFishingState)

#define FISHING_BG_PAL 8
#define PAL_FX 2
#define PAL_UI 3
#define PAL_GOLD 4
#define PAL_POP 5
#define AFF_ROD 0
#define ROD_BASE 11                     /* the rod's angle at rest, 256 to a turn (about 15 degrees up) */
#define AFF_POP 1
#define SCENE_CHR_VRAM 0x06008000
#define SCENE_MAP_VRAM 0x06007000          /* not 0xF000 up: the map's tile animations write there */
#define CHR_POPUP 0x120                 /* the pop-up word on screen (64 tiles, copied in when it pops) */
#define CHR_BARS 0x160                  /* three 8-tile meters, painted every frame */
#define CHR_UNIT 0x180                  /* the unit's walking sheet (up to 0x2200 bytes); 0x380 up holds the map's moving units */
#define CHR_CARD 0x290                  /* the catch card's portrait (32 tiles), or the journal's five icons (8 tiles each) */
#define CHR_STAMP 0x2B0                 /* the catch card's stamp word (64 tiles) */
#define PAL_CARD 6                      /* the portrait; the journal's icons use 6 to 10, one for each row */
#define PAL_STAMP 11
#define AFF_STAMP 2
#define CARD_DELAY 30                   /* frames from the catch to its card */
#define STAMP_AT 14                     /* frames from the card to its stamp */
#define STAMP_SCALE 256                 /* the stamp's size once down (256 is full size) */

#define HOOK_X FISHING_ROD_TIP_X
#define HOOK_MIN (FISHING_SURFACE_Y + 12)
#define HOOK_MAX (FISHING_BED_Y - 8)
#define HOOK_START (FISHING_SURFACE_Y + 40)
#define HOOK_REST (FISHING_ROD_TIP_Y + 8)
#define MOUTH_X (HOOK_X + 3)
#define BOBBER_Y (FISHING_SURFACE_Y - 5)
#define CAM_MAX (FISHING_WORLD_H - 160)
#define UNIT_Y (FISHING_BANK_TOP - 30)

#define TENSION_MAX (100 * 16)
#define TENSION_PRESS (11 * 16)
#define TENSION_START (50 * 16)
#define SLACK_FRAMES 60
#define RUN_FRAMES 40

#define METER_X 16
#define METER_Y 36

#define SE_CAST 0xC8
#define SE_BITE SONG_SE_SYS_WINDOW_SELECT1
#define SE_PULL SONG_SE_SYS_CURSOR_UD1
#define SE_PICK SONG_SE_SYS_CURSOR_LR1
#define SE_LOST SONG_6C
#define SE_GOLD SONG_SE_MONEY
#define SE_LEAVE SONG_SE_SYS_WINDOW_CANSEL1

enum {
	FS_WALKIN,
	FS_READY,
	FS_CAST,
	FS_WAIT,
	FS_RECALL,
	FS_BITE,
	FS_REEL,
	FS_LAND,
	FS_RESULT,
	FS_MESSAGE,
	FS_PRIZES,
};

enum {
	FISHM_HIDDEN,
	FISHM_SWIM,
	FISHM_APPROACH,
	FISHM_NIBBLE,
	FISHM_HOOKED,
	FISHM_FLEE,
};

/* The texts in FS->texts */
enum {
	TX_LINE1,
	TX_LINE2,
	TX_TABLE,       /* the title row, then one row per fish */
};

struct FishType {
	u8 w, h;
	u8 chr, frameTiles;
	u8 pal, blend;
	const u16 *obj;
	s16 speed;      /* swim speed, pixels << 8 per frame */
	s16 minY, maxY; /* swim depths (centre rows of the scene) */
	u8 weight;      /* spawn chance in percent (the weights add up to 100) */
	u8 window;      /* frames to press A after the bite */
	u8 zoneLo;      /* the green zone of the tension meter, in percent */
	u8 zoneHi;
	u8 pull;        /* tension lost per frame, 1/16 percent (doubled during a run) */
	u8 run;         /* a run starts on 1 frame in `run` (0: never) */
	u16 need;       /* frames in the green zone to land it */
	u16 gold;       /* the prize */
	u16 nameMsg;
};

/* The fish. Change the odds, the difficulty and the prizes here; the prize table shows these values. */
static const struct FishType sFishTypes[FISH_TYPES] = {
	[FISH_SMALL] = {
		16, 8, FISHING_CHR_FISH_SMALL, FISHING_CHR_FISH_SMALL_FRAME, PAL_FX, TRUE, gObject_16x8,
		0xC0, FISHING_SURFACE_Y + 12, 200, 45,
		30, 25, 85, 5, 0, 90, 40, MSG_FISHING_FISH_SMALL,
	},
	[FISH_MEDIUM] = {
		32, 16, FISHING_CHR_FISH_MEDIUM, FISHING_CHR_FISH_MEDIUM_FRAME, PAL_FX, TRUE, gObject_32x16,
		0x90, 150, 290, 28,
		24, 30, 75, 7, 150, 150, 120, MSG_FISHING_FISH_MEDIUM,
	},
	[FISH_LARGE] = {
		64, 32, FISHING_CHR_FISH_LARGE, FISHING_CHR_FISH_LARGE_FRAME, PAL_FX, TRUE, gObject_64x32,
		0x70, 240, 370, 15,
		20, 35, 70, 9, 120, 220, 350, MSG_FISHING_FISH_LARGE,
	},
	[FISH_HUGE] = {
		64, 32, FISHING_CHR_FISH_HUGE, FISHING_CHR_FISH_HUGE_FRAME, PAL_FX, TRUE, gObject_64x32,
		0x58, 320, FISHING_BED_Y - 16, 7,
		16, 40, 65, 11, 100, 300, 900, MSG_FISHING_FISH_HUGE,
	},
	[FISH_GOLDEN] = {
		32, 16, FISHING_CHR_FISH_GOLDEN, FISHING_CHR_FISH_GOLDEN_FRAME, PAL_GOLD, FALSE, gObject_32x16,
		0xE0, 200, 400, 5,
		14, 44, 60, 12, 80, 360, 9000, MSG_FISHING_FISH_GOLDEN,
	},
};

/* The species' names, water by water (FISHING_WATER_*), in the order of the kinds of fish (FISH_*) */
static const u16 sSpeciesNames[FISHING_SPECIES] = {
	MSG_FISHING_SP_MINNOW, MSG_FISHING_SP_PERCH, MSG_FISHING_SP_CARP, MSG_FISHING_SP_PIKE, MSG_FISHING_SP_GOLD_KOI,
	MSG_FISHING_SP_SARDINE, MSG_FISHING_SP_MACKEREL, MSG_FISHING_SP_SEA_BASS, MSG_FISHING_SP_TUNA,
	MSG_FISHING_SP_GOLD_BREAM,
	MSG_FISHING_SP_LOACH, MSG_FISHING_SP_TROUT, MSG_FISHING_SP_CHAR, MSG_FISHING_SP_SALMON,
	MSG_FISHING_SP_GOLDEN_TROUT,
	MSG_FISHING_SP_PUPFISH, MSG_FISHING_SP_TILAPIA, MSG_FISHING_SP_CATFISH, MSG_FISHING_SP_ARAPAIMA,
	MSG_FISHING_SP_GOLD_AROWANA,
};

static const u16 sWaterNames[FISHING_WATERS] = {
	MSG_FISHING_WATER_SERAFEW, MSG_FISHING_WATER_PORT_KIRIS, MSG_FISHING_WATER_CAER_PELYN, MSG_FISHING_WATER_JEHANNA,
};

typedef char FishingSpecies_CountCheck[(FISHING_SPECIES == FISHING_JOURNAL_SPECIES) ? 1 : -1];

/* The district's water, by its chapter (District.c's gDistrictNodes) */
static int Fishing_WaterOf(int chapter)
{
	switch (chapter) {
	case 0x3C:
		return FISHING_WATER_PORT_KIRIS;
	case 0x41:
		return FISHING_WATER_CAER_PELYN;
	case 0x42:
		return FISHING_WATER_JEHANNA;
	default:
		return FISHING_WATER_SERAFEW;
	}
}

struct BaitType {
	u16 price;
	u16 nameMsg;
	u8 likes[FISH_TYPES]; /* how readily each fish takes it, in percent (0: never) */
};

/* The baits. Only the golden lure draws the golden fish. */
static const struct BaitType sBaitTypes[BAIT_TYPES] = {
	[BAIT_WORM] = { 10, MSG_FISHING_BAIT_WORM, { 100, 50, 15, 5, 0 } },
	[BAIT_SHRIMP] = { 40, MSG_FISHING_BAIT_SHRIMP, { 40, 100, 70, 35, 0 } },
	[BAIT_LURE] = { 150, MSG_FISHING_BAIT_LURE, { 10, 35, 100, 100, 100 } },
};

static const s8 sBob[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };

struct ProcFishing {
	PROC_HEADER;
	struct DispCnt dispBak;
};

/* ---- Helpers ---- */

static int Fishing_Abs(int v)
{
	return v < 0 ? -v : v;
}

static int Fishing_Clamp(int v, int lo, int hi)
{
	return v < lo ? lo : (v > hi ? hi : v);
}

static int Fishing_Px(const struct FishingFish *f)
{
	return f->x >> 8;
}

static int Fishing_MouthX(const struct FishingFish *f)
{
	const struct FishType *t = &sFishTypes[f->type];

	return f->vx > 0 ? Fishing_Px(f) + t->w - 1 - (t->w >> 4) : Fishing_Px(f) + (t->w >> 4);
}

static void Fishing_SetMouthX(struct FishingFish *f, int mx)
{
	const struct FishType *t = &sFishTypes[f->type];

	f->x = (f->vx > 0 ? mx - t->w + 1 + (t->w >> 4) : mx - (t->w >> 4)) << 8;
}

static int Fishing_RollType(void)
{
	int r = NextRN_N(100);
	int i;

	for (i = 0; i < FISH_TYPES - 1; i++) {
		if (r < sFishTypes[i].weight)
			return i;
		r -= sFishTypes[i].weight;
	}
	return FISH_TYPES - 1;
}

/* A new fish. anywhere: placed at random in its depths (the opening), else it enters from the left edge. */
static void Fishing_Spawn(struct FishingFish *f, bool anywhere)
{
	const struct FishType *t;

	f->type = Fishing_RollType();
	t = &sFishTypes[f->type];
	f->baseY = t->minY + NextRN_N(t->maxY - t->minY + 1);
	f->y = f->baseY;
	f->vx = t->speed - 0x10 + NextRN_N(0x21);
	if (anywhere) {
		f->x = NextRN_N(FISHING_WATER_RIGHT - 8 - t->w) << 8;
		if (NextRN_N(2))
			f->vx = -f->vx;
	} else {
		f->x = -t->w << 8;
	}
	f->mode = FISHM_SWIM;
	f->timer = 0;
}

static void Fishing_Hide(struct FishingFish *f)
{
	f->mode = FISHM_HIDDEN;
	f->timer = 30 + NextRN_N(150);
}

static void Fishing_Flee(int i)
{
	struct FishingFish *f = &FS->fish[i];

	f->mode = FISHM_FLEE;
	f->vx = Fishing_MouthX(f) <= MOUTH_X ? -0x280 : 0x280;
	f->baseY = f->y;
	if (FS->biter == i)
		FS->biter = -1;
}

/* ---- Text ---- */

static void Fishing_Append(char *buf, int *i, const char *s)
{
	while (*s != 0 && *i < 62)
		buf[(*i)++] = *s++;
	buf[*i] = 0;
}

static void Fishing_AppendMsg(char *buf, int *i, int msg)
{
	Fishing_Append(buf, i, GetStringFromIndex(msg));
}

static void Fishing_AppendNum(char *buf, int *i, int n)
{
	char digits[10];
	int d = 0, q;

	do {
		q = Div(n, 10);
		digits[d++] = '0' + (n - q * 10);
		n = q;
	} while (n > 0 && d < 9);
	while (d > 0 && *i < 62)
		buf[(*i)++] = digits[--d];
	buf[*i] = 0;
}

static void Fishing_PutLine(int tx, const char *str, int color, int col, int row)
{
	struct Text *text = &FS->texts[tx];

	ClearText(text);
	Text_SetParams(text, 0, color);
	Text_DrawString(text, str);
	PutText(text, TILEMAP_LOCATED(gBG0TilemapBuffer, col, row));
}

/* ---- The window's first line: what is going on ---- */

static int Fishing_Depth(void)
{
	int d = FS->hookY - FISHING_SURFACE_Y;

	return d > 0 ? d >> 3 : 0;
}

static void Fishing_Line1(char *buf)
{
	int i = 0;

	buf[0] = 0;
	switch (FS->state) {
	case FS_WALKIN:
		Fishing_AppendMsg(buf, &i, MSG_FISHING_WELCOME);
		break;
	case FS_READY:
		Fishing_AppendMsg(buf, &i, MSG_FISHING_S_BAIT);
		Fishing_Append(buf, &i, " ");
		Fishing_AppendMsg(buf, &i, sBaitTypes[FS->bait].nameMsg);
		Fishing_Append(buf, &i, " (");
		if (FS->baited == FS->bait)
			Fishing_Append(buf, &i, "on");
		else {
			Fishing_AppendNum(buf, &i, sBaitTypes[FS->bait].price);
			Fishing_Append(buf, &i, "G");
		}
		Fishing_Append(buf, &i, ")   ");
		Fishing_AppendMsg(buf, &i, MSG_FISHING_S_GOLD);
		Fishing_Append(buf, &i, " ");
		Fishing_AppendNum(buf, &i, GetPartyGoldAmount());
		break;
	case FS_CAST:
	case FS_WAIT:
	case FS_RECALL:
		Fishing_AppendMsg(buf, &i, MSG_FISHING_S_DEPTH);
		Fishing_Append(buf, &i, " ");
		Fishing_AppendNum(buf, &i, Fishing_Depth());
		Fishing_Append(buf, &i, "m   ");
		Fishing_AppendMsg(buf, &i, MSG_FISHING_S_BAIT);
		Fishing_Append(buf, &i, " ");
		Fishing_AppendMsg(buf, &i, sBaitTypes[FS->baited == BAIT_NONE ? FS->bait : FS->baited].nameMsg);
		break;
	case FS_BITE:
		Fishing_AppendMsg(buf, &i, MSG_FISHING_BITE);
		break;
	case FS_REEL:
		if (FS->bonusTimer > 0)
			Fishing_AppendMsg(buf, &i, FS->hookQ >= 2 ? MSG_FISHING_PERFECT : MSG_FISHING_GOOD);
		else
			Fishing_AppendMsg(buf, &i, MSG_FISHING_TENSION);
		break;
	case FS_LAND:
		Fishing_AppendMsg(buf, &i, MSG_FISHING_LANDING);
		break;
	case FS_RESULT:
		Fishing_AppendMsg(buf, &i, sSpeciesNames[FS->species]);
		Fishing_Append(buf, &i, "!  +");
		Fishing_AppendNum(buf, &i, FS->payout);
		Fishing_Append(buf, &i, "G  (x");
		Fishing_AppendNum(buf, &i, (4 + FS->hookQ + FS->landQ) >> 2);
		Fishing_Append(buf, &i, ".");
		Fishing_AppendNum(buf, &i, ((4 + FS->hookQ + FS->landQ) & 3) * 25);
		Fishing_Append(buf, &i, ")");
		break;
	case FS_MESSAGE:
		Fishing_AppendMsg(buf, &i, FS->msg);
		break;
	case FS_PRIZES:
		Fishing_AppendMsg(buf, &i, MSG_FISHING_PRIZES);
		Fishing_Append(buf, &i, " ");
		Fishing_AppendMsg(buf, &i, sWaterNames[FS->page]);
		Fishing_Append(buf, &i, "   ");
		Fishing_AppendNum(buf, &i, FishingRecords_Found());
		Fishing_Append(buf, &i, "/");
		Fishing_AppendNum(buf, &i, FISHING_SPECIES);
		break;
	}
}

/* A cheap signature of what Fishing_Line1 would show, so it is only rebuilt when something changes. */
static u32 Fishing_Line1Sig(void)
{
	u32 sig = FS->state | (FS->bait << 4) | ((FS->bonusTimer > 0) << 6) | (FS->baited << 7) | ((u32)FS->msg << 16);

	switch (FS->state) {
	case FS_READY:
		return sig * 31 + GetPartyGoldAmount();
	case FS_CAST:
	case FS_WAIT:
	case FS_RECALL:
		return sig * 31 + Fishing_Depth();
	case FS_RESULT:
		return sig * 31 + FS->payout;
	case FS_PRIZES:
		return sig * 31 + FS->page;
	default:
		return sig;
	}
}

/* ---- The window's second line: the buttons ---- */

/* The buttons float at the bottom of the screen with no window behind them, as in the prep screens */
#define HINT_ROW 18

enum {
	HINT_A,
	HINT_B,
	HINT_START,
	HINT_UPDOWN,
	HINT_LEFTRIGHT,
};

static const u16 sHintChr[] = {
	FISHING_CHR_BTN_A, FISHING_CHR_BTN_B, FISHING_CHR_BTN_START, FISHING_CHR_BTN_UPDOWN, FISHING_CHR_BTN_LEFTRIGHT,
};

struct Hint {
	u8 icon;
	u16 msg;
};

static int Fishing_Hints(int state, const struct Hint **out)
{
	static const struct Hint ready[] = {
		{ HINT_LEFTRIGHT, MSG_FISHING_K_BAIT }, { HINT_A, MSG_FISHING_K_CAST },
		{ HINT_B, MSG_FISHING_K_EXIT }, { HINT_START, MSG_FISHING_K_PRIZES },
	};
	static const struct Hint wait[] = {
		{ HINT_UPDOWN, MSG_FISHING_K_DEPTH }, { HINT_A, MSG_FISHING_K_REEL_IN },
		{ HINT_B, MSG_FISHING_K_EXIT }, { HINT_START, MSG_FISHING_K_PRIZES },
	};
	static const struct Hint bite[] = { { HINT_A, MSG_FISHING_K_HOOK }, { HINT_B, MSG_FISHING_K_EXIT } };
	static const struct Hint reel[] = { { HINT_A, MSG_FISHING_K_REEL }, { HINT_B, MSG_FISHING_K_EXIT } };
	static const struct Hint exit[] = { { HINT_B, MSG_FISHING_K_EXIT } };
	static const struct Hint result[] = {
		{ HINT_A, MSG_FISHING_K_AGAIN }, { HINT_B, MSG_FISHING_K_EXIT }, { HINT_START, MSG_FISHING_K_PRIZES },
	};
	static const struct Hint message[] = { { HINT_A, MSG_FISHING_K_OK }, { HINT_B, MSG_FISHING_K_EXIT } };
	static const struct Hint prizes[] = { { HINT_LEFTRIGHT, MSG_FISHING_K_WATER }, { HINT_B, MSG_FISHING_K_CLOSE } };

	switch (state) {
	case FS_READY:
		*out = ready;
		return 4;
	case FS_WAIT:
		*out = wait;
		return 4;
	case FS_BITE:
		*out = bite;
		return 2;
	case FS_REEL:
		*out = reel;
		return 2;
	case FS_CAST:
	case FS_RECALL:
		*out = exit;
		return 1;
	case FS_RESULT:
		*out = result;
		return 3;
	case FS_MESSAGE:
		*out = message;
		return 2;
	case FS_PRIZES:
		*out = prizes;
		return 2;
	default:
		*out = NULL;
		return 0;
	}
}

/* Lays the second line out: each button icon (an OBJ, drawn every frame) and its label (BG0 text). */
static void Fishing_DrawLine2(void)
{
	struct Text *text = &FS->texts[TX_LINE2];
	const struct Hint *hints;
	const char *label;
	int n = Fishing_Hints(FS->state, &hints);
	int i, x = 0, w;

	ClearText(text);
	for (i = 0; i < n && i < 4; i++) {
		w = hints[i].icon == HINT_START ? 32 : 16;
		FS->hintIcon[i] = hints[i].icon;
		FS->hintX[i] = 8 + x;
		label = GetStringFromIndex(hints[i].msg);
		Text_InsertDrawString(text, x + w + 2, TEXT_COLOR_SYSTEM_WHITE, label);
		x += w + 2 + GetStringTextLen(GetStringFromIndex(hints[i].msg)) + 8;
	}
	FS->hintCount = i;
	PutText(text, TILEMAP_LOCATED(gBG0TilemapBuffer, 1, HINT_ROW));
	FS->line2Key = FS->state;
}

static void Fishing_UpdateWindow(void)
{
	char buf[64];
	u32 sig = Fishing_Line1Sig();
	bool sync = FALSE;

	if (sig != FS->line1Sig) {
		Fishing_Line1(buf);
		Fishing_PutLine(TX_LINE1, buf, TEXT_COLOR_SYSTEM_WHITE, 1, 1);
		FS->line1Sig = sig;
		sync = TRUE;
	}
	if (FS->line2Key != FS->state) {
		Fishing_DrawLine2();
		sync = TRUE;
	}
	if (sync)
		BG_EnableSyncByMask(BG0_SYNC_BIT);
}

/* ---- The fish journal ---- */

#define TABLE_TOP 4
#define TABLE_ROWS 14
#define COL_NAME 36
#define COL_PRIZE 112
#define COL_CAUGHT 152
#define COL_BEST 188
#define ICON_X 10

static void Fishing_ClearRows(int y0, int rows)
{
	int x, y;

	for (y = y0; y < y0 + rows; y++) {
		for (x = 0; x < 32; x++) {
			gBG0TilemapBuffer[TILEMAP_INDEX(x, y)] = 0;
			gBG1TilemapBuffer[TILEMAP_INDEX(x, y)] = 0;
		}
	}
}

static void Fishing_TableCell(struct Text *text, int x, int color, const char *s)
{
	Text_InsertDrawString(text, x, color, s);
}

static void Fishing_AppendCm(char *buf, int *i, int cm)
{
	Fishing_AppendNum(buf, i, cm);
	Fishing_Append(buf, i, "cm");
}

/* The page of one water: its five species, each with its icon (a silhouette until it is caught) */
static void Fishing_OpenPrizes(void)
{
	struct Text *text;
	char buf[64];
	int t, i, sp, color, caught;

	DrawUiFrame2(0, TABLE_TOP, 30, TABLE_ROWS, 0);

	text = &FS->texts[TX_TABLE];
	ClearText(text);
	Fishing_TableCell(text, COL_NAME, TEXT_COLOR_SYSTEM_BLUE, GetStringFromIndex(MSG_FISHING_COL_FISH));
	Fishing_TableCell(text, COL_PRIZE, TEXT_COLOR_SYSTEM_BLUE, GetStringFromIndex(MSG_FISHING_COL_PRIZE));
	Fishing_TableCell(text, COL_CAUGHT, TEXT_COLOR_SYSTEM_BLUE, GetStringFromIndex(MSG_FISHING_COL_CAUGHT));
	Fishing_TableCell(text, COL_BEST, TEXT_COLOR_SYSTEM_BLUE, GetStringFromIndex(MSG_FISHING_COL_BEST));
	PutText(text, TILEMAP_LOCATED(gBG0TilemapBuffer, 1, TABLE_TOP + 1));

	for (t = 0; t < FISH_TYPES; t++) {
		sp = FS->page * FISH_TYPES + t;
		caught = FishingRecords_Caught(sp);
		text = &FS->texts[TX_TABLE + 1 + t];
		color = t == FISH_GOLDEN ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_WHITE;
		ClearText(text);
		Fishing_TableCell(text, COL_NAME, caught ? color : TEXT_COLOR_SYSTEM_GRAY,
				  caught ? GetStringFromIndex(sSpeciesNames[sp]) : "???");
		i = 0;
		Fishing_AppendNum(buf, &i, sFishTypes[t].gold);
		Fishing_Append(buf, &i, "G");
		Fishing_TableCell(text, COL_PRIZE, color, buf);
		i = 0;
		Fishing_AppendNum(buf, &i, caught);
		Fishing_TableCell(text, COL_CAUGHT, color, buf);
		i = 0;
		if (caught)
			Fishing_AppendCm(buf, &i, FishingRecords_Best(sp));
		else
			Fishing_Append(buf, &i, "-");
		Fishing_TableCell(text, COL_BEST, color, buf);
		PutText(text, TILEMAP_LOCATED(gBG0TilemapBuffer, 1, TABLE_TOP + 3 + t * 2));

		CpuFastCopy(gFishingSpeciesIcons + sp * FISHING_ICON_TILES * 0x20,
			    (void *)(OBJ_VRAM0 + (CHR_CARD + t * FISHING_ICON_TILES) * 0x20), FISHING_ICON_TILES * 0x20);
		ApplyPalette(caught ? gFishingSpeciesPals + sp * 16 : gFishingSilhouettePal, 0x10 + PAL_CARD + t);
	}
	EnablePaletteSync();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static void Fishing_ClosePrizes(void)
{
	Fishing_ClearRows(TABLE_TOP, TABLE_ROWS);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static void Fishing_DrawJournalIcons(void)
{
	int t;

	for (t = 0; t < FISH_TYPES; t++)
		PutSpriteExt(4, OAM1_X(ICON_X), OAM0_Y((TABLE_TOP + 3 + t * 2) * 8), gObject_32x16,
			     OAM2_PAL(PAL_CARD + t) + OAM2_LAYER(0) + OAM2_CHR(CHR_CARD + t * FISHING_ICON_TILES));
}

/* ---- The catch card ---- */

#define CARD_X 3
#define CARD_Y 6
#define CARD_W 24
#define CARD_H 10
#define CARD_TEXT_COL 13
#define PIC_X 36
#define PIC_Y 72
#define STAMP_CX 56
#define STAMP_CY 108

static void Fishing_CardLine(int row, int color, const char *str)
{
	struct Text *text = &FS->texts[TX_TABLE + row];

	ClearText(text);
	Text_InsertDrawString(text, 0, color, str);
	PutText(text, TILEMAP_LOCATED(gBG0TilemapBuffer, CARD_TEXT_COL, CARD_Y + 1 + row * 2));
}

/* The card: the portrait, the name, the size, the best (or "First catch!") and the payout */
static void Fishing_OpenCard(void)
{
	char buf[64];
	int i, sp = FS->species;

	DrawUiFrame2(CARD_X, CARD_Y, CARD_W, CARD_H, 0);
	Fishing_CardLine(0, FS->lastType == FISH_GOLDEN ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_WHITE,
			 GetStringFromIndex(sSpeciesNames[sp]));
	i = 0;
	Fishing_AppendMsg(buf, &i, MSG_FISHING_CARD_SIZE);
	Fishing_Append(buf, &i, " ");
	Fishing_AppendCm(buf, &i, FS->cm);
	Fishing_CardLine(1, TEXT_COLOR_SYSTEM_WHITE, buf);
	i = 0;
	if (FS->prevBest == 0) {
		Fishing_AppendMsg(buf, &i, MSG_FISHING_CARD_FIRST);
	} else {
		Fishing_AppendMsg(buf, &i, FS->cm > FS->prevBest ? MSG_FISHING_CARD_OLD : MSG_FISHING_CARD_BEST);
		Fishing_Append(buf, &i, " ");
		Fishing_AppendCm(buf, &i, FS->prevBest);
	}
	Fishing_CardLine(2, TEXT_COLOR_SYSTEM_BLUE, buf);
	i = 0;
	Fishing_Append(buf, &i, "+");
	Fishing_AppendNum(buf, &i, FS->payout);
	Fishing_Append(buf, &i, "G");
	Fishing_CardLine(3, TEXT_COLOR_SYSTEM_GOLD, buf);

	CpuFastCopy(gFishingSpeciesPics + sp * FISHING_PIC_TILES * 0x20, (void *)(OBJ_VRAM0 + CHR_CARD * 0x20),
		    FISHING_PIC_TILES * 0x20);
	ApplyPalette(gFishingSpeciesPals + sp * 16, 0x10 + PAL_CARD);
	if (FS->stamp != 0xFF) {
		CpuFastCopy(gFishingPopTiles + FS->stamp * FISHING_POP_TILES * 0x20, (void *)(OBJ_VRAM0 + CHR_STAMP * 0x20),
			    FISHING_POP_TILES * 0x20);
		ApplyPalette(gFishingPopPals + gFishingPopWordPal[FS->stamp] * 16, 0x10 + PAL_STAMP);
	}
	EnablePaletteSync();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
	FS->cardT = 1;
	/* CAUGHT! makes way: it floats off and blinks out */
	if (FS->popWord != 0xFF && FS->popTimer < 44)
		FS->popTimer = 44;
}

static void Fishing_CloseCard(void)
{
	if (FS->cardT == 0)
		return;
	Fishing_ClearRows(CARD_Y, CARD_H);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
	FS->cardT = 0;
}

/*
 * The stamp slams down onto the card, tilted: it shrinks from almost twice its size, a little past its size, and
 * settles. Its two halves turn and scale together about its centre (one affine slot).
 */
static void Fishing_DrawStamp(void)
{
	int k = FS->cardT - STAMP_AT;
	int scale, inv, c, s, d;
	const int angle = 252;          /* about 6 degrees, rising to the right */

	if (FS->stamp == 0xFF || k < 0)
		return;
	if (k < 6)
		scale = 496 - k * 48;           /* down to 0.9 */
	else if (k < 10)
		scale = 224 + (k - 6) * 8;      /* and back up */
	else
		scale = STAMP_SCALE;
	c = COS(angle);
	s = SIN(angle);
	inv = Div(0x10000, scale);
	SetObjAffine(AFF_STAMP, (c * inv) >> 12, (s * inv) >> 12, (-s * inv) >> 12, (c * inv) >> 12);
	d = (32 * scale) >> 8;
	PutSpriteExt(4, OAM1_X((STAMP_CX - ((d * c) >> 12) - 64) & 0x1FF) | OAM1_AFFINE_ID(AFF_STAMP),
		     OAM0_Y((STAMP_CY - ((d * s) >> 12) - 32) & 0xFF) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE,
		     gObject_64x32, OAM2_PAL(PAL_STAMP) + OAM2_LAYER(0) + OAM2_CHR(CHR_STAMP));
	PutSpriteExt(4, OAM1_X((STAMP_CX + ((d * c) >> 12) - 64) & 0x1FF) | OAM1_AFFINE_ID(AFF_STAMP),
		     OAM0_Y((STAMP_CY + ((d * s) >> 12) - 32) & 0xFF) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE,
		     gObject_64x32, OAM2_PAL(PAL_STAMP) + OAM2_LAYER(0) + OAM2_CHR(CHR_STAMP + 32));
}

static void Fishing_DrawCard(void)
{
	if (FS->cardT == 0)
		return;
	PutSpriteExt(4, OAM1_X(PIC_X), OAM0_Y(PIC_Y), gObject_64x32,
		     OAM2_PAL(PAL_CARD) + OAM2_LAYER(1) + OAM2_CHR(CHR_CARD));
	Fishing_DrawStamp();
}

/* ---- Setup ---- */

static const u16 sFishingBgConfig[12] = {
	0x0000, 0x6000, 0,  /* BG0: text */
	0x0000, 0x6800, 0,  /* BG1: window frames */
	0x0000, 0xE000, 0,  /* BG2: unused (its map out of the way of BG3's) */
	0x8000, 0x7000, 2,  /* BG3: the scene, 256x512 */
};

/*
 * The map is 32 x 64 cells. Rows 0..31 go through the BG3 buffer (a BG3 sync copies only those 2 KB), rows 32..63
 * straight to the second screen block in VRAM.
 */
static void Fishing_LoadScene(void)
{
	u16 *top = BG_GetMapBuffer(3);
	u16 *bottom = (u16 *)(SCENE_MAP_VRAM + 0x800);
	u16 *row;
	int x, y;

	CpuFastCopy(gFishingSceneTiles, (void *)SCENE_CHR_VRAM, FISHING_SCENE_TILES * 0x20);
	BG_Fill(top, 0);
	CpuFastFill(0, bottom, 0x800);
	for (y = 0; y < FISHING_SCENE_ROWS; y++) {
		row = y < 32 ? top + y * 32 : bottom + (y - 32) * 32;
		for (x = 0; x < 30; x++)
			row[x] = gFishingSceneMap[y * 30 + x] | (FISHING_BG_PAL << 12);
	}
	ApplyPalette(gFishingScenePal, FISHING_BG_PAL);
}

/* The unit's walking sheet (32x32 frames: 0-3 side, 4-7 down, 8-11 up), straight into OBJ VRAM */
static void Fishing_LoadUnit(struct Unit *unit)
{
	const u8 *img = gMuInfoTable[unit->pClassData->number - 1].img;

	/* The LZ77 header holds the unpacked size in bytes 1..3 */
	if (img == NULL || (img[1] | (img[2] << 8) | (img[3] << 16)) > 0x2200) {
		/* No usable sheet: the standing sprite stands in (its first frame) */
		img = (const u8 *)gUnitIconWaitTable[GetUnitSMSId(unit) & 0x7F].sheet;
	}
	Decompress(img, (void *)(OBJ_VRAM0 + CHR_UNIT * 0x20));
	ApplyUnitSpritePalettes();
	FS->smsPal = GetUnitSpritePalette(unit);
}

static void Fishing_Init(struct ProcFishing *proc)
{
	struct Unit *unit = gActiveUnit ? gActiveUnit : GetUnit(1);
	int i;

	proc->dispBak = gLCDControlBuffer.dispcnt;

	SetupBackgrounds((u16 *)sFishingBgConfig);
	SetDispEnable(0, 0, 0, 0, 0);
	BG_Fill(BG_GetMapBuffer(0), 0);
	BG_Fill(BG_GetMapBuffer(1), 0);
	BG_Fill(BG_GetMapBuffer(2), 0);
	gLCDControlBuffer.bg0cnt.priority = 0;
	gLCDControlBuffer.bg1cnt.priority = 1;
	gLCDControlBuffer.bg2cnt.priority = 2;
	gLCDControlBuffer.bg3cnt.priority = 3;
	BG_SetPosition(0, 0, 0);
	BG_SetPosition(1, 0, 0);
	BG_SetPosition(3, 0, 0);

	ResetText();
	SetTextFont(0);
	InitSystemTextFont();
	LoadUiFrameGraphics();

	Fishing_LoadScene();
	CpuFastCopy(gFishingObjTiles, (void *)OBJ_VRAM0, FISHING_OBJ_TILES * 0x20);
	ApplyPalette(gFishingObjPalFx, 0x10 + PAL_FX);
	ApplyPalette(gFishingObjPalUi, 0x10 + PAL_UI);
	ApplyPalette(gFishingObjPalGold, 0x10 + PAL_GOLD);
	Fishing_LoadUnit(unit);
	gPaletteBuffer[PAL_BACKDROP_OFFSET] = 0;
	EnablePaletteSync();

	DrawUiFrame2(0, 0, 30, 4, 0);
	for (i = 0; i < 8; i++)
		InitText(&FS->texts[i], 28);

	/* The fish shadows are semi-transparent OBJs over the scene. */
	SetBlendAlpha(8, 8);
	SetBlendTargetA(0, 0, 0, 0, 0);
	SetBlendTargetB(0, 0, 0, 1, 0);

	SetDispEnable(1, 1, 0, 1, 1);
	gLCDControlBuffer.dispcnt.obj1dMap = 1;

	for (i = 0; i < FISHING_FISH_MAX; i++)
		Fishing_Spawn(&FS->fish[i], TRUE);
	FS->biter = -1;
	FS->targetY = HOOK_START;
	FS->hookY = HOOK_REST;
	FS->camY = 0;
	FS->splash = 0;
	FS->bait = BAIT_WORM;
	FS->baited = BAIT_NONE;
	FS->unitX = 240;
	FS->unitFrame = 0;
	FS->rodAngle = ROD_BASE << 8;
	FS->shake = 0;
	FS->shakeX = 0;
	FS->shakeY = 0;
	FS->popWord = 0xFF;
	for (i = 0; i < 16; i++)
		FS->parts[i].life = 0;
	FS->bonusTimer = 0;
	FS->water = Fishing_WaterOf(gPlaySt.chapterIndex);
	FS->page = FS->water;
	FS->cardT = 0;
	FS->stamp = 0xFF;
	FS->state = FS_WALKIN;
	FS->timer = 0;
	FS->line1Sig = 0xFFFFFFFF;
	FS->line2Key = 0xFF;
	FS->hintCount = 0;
	Fishing_UpdateWindow();

	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG3_SYNC_BIT);
}

/* ---- The fish ---- */

static void Fishing_UpdateFish(int i)
{
	struct FishingFish *f = &FS->fish[i];
	const struct FishType *t = &sFishTypes[f->type];
	int clock = GetGameClock();
	int mx, dx, dy, step, likes;

	switch (f->mode) {
	case FISHM_HIDDEN:
		if (f->timer > 0)
			f->timer--;
		else
			Fishing_Spawn(f, FALSE);
		return;

	case FISHM_SWIM:
		f->x += f->vx;
		if (f->vx > 0 && Fishing_Px(f) + t->w >= FISHING_WATER_RIGHT - 4)
			f->vx = -f->vx;
		if (f->vx < 0 && Fishing_Px(f) < -t->w) {
			Fishing_Hide(f);
			return;
		}
		f->y = f->baseY + sBob[((clock >> 3) + i * 3) & 7];

		/* Notice the hook: only while waiting, one fish at a time, facing it, near its depth, and if it likes the bait. */
		if (FS->state == FS_WAIT && FS->biter < 0 && FS->baited != BAIT_NONE) {
			likes = sBaitTypes[FS->baited].likes[f->type];
			mx = Fishing_MouthX(f);
			dx = f->vx > 0 ? MOUTH_X - mx : mx - MOUTH_X;
			dy = f->baseY - FS->hookY;
			if (likes > 0 && dx > 0 && dx < 64 && Fishing_Abs(dy) < 24 && NextRN_N(4000) < likes) {
				f->mode = FISHM_APPROACH;
				FS->biter = i;
			}
		}
		return;

	case FISHM_APPROACH:
		if (Fishing_Abs(f->baseY - FS->hookY) > 32 || FS->state != FS_WAIT) {
			f->mode = FISHM_SWIM;
			FS->biter = -1;
			return;
		}
		mx = Fishing_MouthX(f);
		step = (Fishing_Abs(f->vx) * 3) >> 2;
		if (step < 0x50)
			step = 0x50;
		if (mx < MOUTH_X)
			f->x += step;
		else if (mx > MOUTH_X)
			f->x -= step;
		if ((clock & 1) == 0) {
			if (f->baseY < FS->hookY)
				f->baseY++;
			else if (f->baseY > FS->hookY)
				f->baseY--;
		}
		f->y = f->baseY;
		if (Fishing_Abs(Fishing_MouthX(f) - MOUTH_X) <= 1 && Fishing_Abs(f->baseY - FS->hookY) <= 1) {
			Fishing_SetMouthX(f, MOUTH_X);
			f->baseY = f->y = FS->hookY;
			f->mode = FISHM_NIBBLE;
			f->timer = 20 + NextRN_N(40);
			FS->nibbleY = FS->hookY;
		}
		return;

	case FISHM_NIBBLE:
		if (FS->state != FS_WAIT) {
			f->mode = FISHM_SWIM;
			FS->biter = -1;
			return;
		}
		/* Moving the hook, or pressing A too soon, scares it off. */
		if (FS->hookY != FS->nibbleY || (gKeyStatusPtr->newKeys & A_BUTTON)) {
			Fishing_Flee(i);
			PlaySoundEffect(SE_LOST);
			FS->msg = MSG_FISHING_SCARED;
			FS->state = FS_MESSAGE;
			FS->timer = 120;
			return;
		}
		Fishing_SetMouthX(f, MOUTH_X + ((clock & 8) ? 1 : 0));
		if (f->timer > 0) {
			f->timer--;
			return;
		}
		Fishing_SetMouthX(f, MOUTH_X);
		f->mode = FISHM_HOOKED;
		FS->state = FS_BITE;
		FS->timer = t->window;
		FS->biteClock = 0;
		PlaySoundEffect(SE_BITE);
		return;

	case FISHM_HOOKED:
		step = FS->state == FS_REEL ? ((clock & 4) ? 2 : 0) : 0;
		if (FS->burst > 0)
			step = (clock & 2) ? 4 : -2;
		Fishing_SetMouthX(f, MOUTH_X + step);
		f->baseY = f->y = FS->hookY;
		return;

	case FISHM_FLEE:
		f->x += f->vx;
		f->y = f->baseY;
		if (Fishing_Px(f) < -t->w || Fishing_Px(f) > FISHING_WATER_RIGHT)
			Fishing_Hide(f);
		return;
	}
}

/* ---- The rod, the pop-ups, the particles and the shake ---- */

/* Where the rod's tip is in the scene (the line hangs from it) */
static void Fishing_RodTip(int *x, int *y)
{
	int a = (FS->rodAngle >> 8) & 0xFF;

	*x = FISHING_HAND_X - ((FISHING_ROD_LEN * COS(a)) >> 12);
	*y = FISHING_HAND_Y - ((FISHING_ROD_LEN * SIN(a)) >> 12);
}

/* The rod eases towards an angle that suits the moment, so it never jumps. */
static void Fishing_UpdateRod(void)
{
	int clock = GetGameClock();
	int target = ROD_BASE + (SIN((clock << 1) & 0xFF) >> 11);

	switch (FS->state) {
	case FS_WAIT:
		if (FS->biter >= 0 && FS->fish[FS->biter].mode == FISHM_NIBBLE)
			target = ROD_BASE - ((clock & 8) ? 3 : 1);
		break;
	case FS_BITE:
		target = ROD_BASE - ((clock & 4) ? 9 : 5);
		break;
	case FS_REEL:
		/* The rod rises with the tension, and bucks while the fish runs */
		target = ROD_BASE - 6 + Div(FS->tension * 22, TENSION_MAX);
		if (FS->burst > 0)
			target -= (clock & 4) ? 5 : 1;
		break;
	case FS_RECALL:
		target = ROD_BASE + 14;
		break;
	case FS_LAND:
		target = ROD_BASE + 22;
		break;
	}
	FS->rodAngle += ((target << 8) - FS->rodAngle) >> 3;
}

/* The rod is an affine sprite: its art is drawn at FISHING_ROD_ART_ANGLE with the hand at the centre, and turned. */
static void Fishing_DrawRod(void)
{
	int phi = ((FS->rodAngle >> 8) - FISHING_ROD_ART_ANGLE) & 0xFF;
	int c = COS(phi), s = SIN(phi);
	int y = FISHING_HAND_Y - 64 - FS->camY - FS->shakeY;

	if (y <= -128 || y >= 160)
		return;
	SetObjAffine(AFF_ROD, c >> 4, s >> 4, -s >> 4, c >> 4);
	PutSpriteExt(4, OAM1_X((FISHING_HAND_X - 64 - FS->shakeX) & 0x1FF) | OAM1_AFFINE_ID(AFF_ROD),
		     OAM0_Y(y & 0xFF) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE, gObject_64x64,
		     OAM2_PAL(PAL_FX) + OAM2_LAYER(2) + OAM2_CHR(FISHING_CHR_ROD));
}

static void Fishing_Shake(int frames)
{
	if (frames > FS->shake)
		FS->shake = frames;
}

static void Fishing_UpdateShake(void)
{
	int clock = GetGameClock();
	int amp;

	if (FS->shake == 0) {
		FS->shakeX = FS->shakeY = 0;
		return;
	}
	FS->shake--;
	amp = FS->shake > 6 ? 3 : (FS->shake > 2 ? 2 : 1);
	FS->shakeX = (clock & 2) ? amp : -amp;
	FS->shakeY = (clock & 4) ? (amp >> 1) : -(amp >> 1);
}

/* A burst of n sparks (kind 0) or coins (kind 1) from a screen point */
static void Fishing_Burst(int x, int y, int n, int kind)
{
	struct FishingParticle *p;
	int i, k = 0, a, speed;

	for (i = 0; i < 16 && k < n; i++) {
		p = &FS->parts[i];
		if (p->life != 0)
			continue;
		a = ((k << 8) / n + NextRN_N(16)) & 0xFF;
		speed = kind == 0 ? 24 + NextRN_N(24) : 16 + NextRN_N(20);
		p->x = x << 4;
		p->y = y << 4;
		p->vx = (COS(a) * speed) >> 12;
		p->vy = ((SIN(a) * speed) >> 12) - (kind == 1 ? 28 : 6);
		p->life = 24 + NextRN_N(14);
		p->kind = kind;
		k++;
	}
}

/* A word pops up: it zooms in past full size, settles, floats a moment, then rises and blinks out. */
static void Fishing_Pop(int word, int x, int y)
{
	CpuFastCopy(gFishingPopTiles + word * FISHING_POP_TILES * 0x20, (void *)(OBJ_VRAM0 + CHR_POPUP * 0x20),
		    FISHING_POP_TILES * 0x20);
	ApplyPalette(gFishingPopPals + gFishingPopWordPal[word] * 16, 0x10 + PAL_POP);
	EnablePaletteSync();
	FS->popWord = word;
	FS->popTimer = 0;
	FS->popX = x;
	FS->popY = Fishing_Clamp(y, 52, 120);
}

static void Fishing_UpdateEffects(void)
{
	struct FishingParticle *p;
	int i;

	for (i = 0; i < 16; i++) {
		p = &FS->parts[i];
		if (p->life == 0)
			continue;
		p->life--;
		p->x += p->vx;
		p->y += p->vy;
		if (p->kind == 1 || (p->life & 1))
			p->vy = p->vy < 100 ? p->vy + (p->kind == 1 ? 2 : 1) : p->vy;
		if (p->y >> 4 > 168)
			p->life = 0;
	}
	if (FS->popWord != 0xFF && ++FS->popTimer >= 70)
		FS->popWord = 0xFF;
}

static void Fishing_DrawParticles(void)
{
	const struct FishingParticle *p;
	int i, x, y, frame;

	for (i = 0; i < 16; i++) {
		p = &FS->parts[i];
		if (p->life == 0)
			continue;
		x = (p->x >> 4) - 4;
		y = (p->y >> 4) - 4;
		if (x < -8 || x >= 240 || y < -8 || y >= 160)
			continue;
		frame = (p->life >> 2) & 1;
		PutSpriteExt(4, OAM1_X(x & 0x1FF), OAM0_Y(y & 0xFF), gObject_8x8,
			     OAM2_PAL(p->kind == 1 ? PAL_GOLD : PAL_FX) + OAM2_LAYER(1) +
			     OAM2_CHR((p->kind == 1 ? FISHING_CHR_COIN : FISHING_CHR_SPARK) + frame));
	}
}

static void Fishing_DrawPopup(void)
{
	int t = FS->popTimer;
	int scale, inv, half, cy;

	if (FS->popWord == 0xFF || (t > 56 && (t & 2)))
		return;
	if (t < 6)
		scale = 64 + t * 48;            /* 0.25 to 1.375 */
	else if (t < 14)
		scale = 352 - (t - 6) * 12;     /* settles to 1.0 */
	else
		scale = 256;
	cy = FS->popY - (t > 44 ? (t - 44) >> 1 : 0) + ((t >= 14 && t <= 44) ? sBob[(t >> 2) & 7] : 0);
	inv = Div(0x10000, scale);
	half = (32 * scale) >> 8;
	SetObjAffine(AFF_POP, inv, 0, 0, inv);
	PutSpriteExt(4, OAM1_X((FS->popX - half - 64) & 0x1FF) | OAM1_AFFINE_ID(AFF_POP),
		     OAM0_Y((cy - 32) & 0xFF) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE, gObject_64x32,
		     OAM2_PAL(PAL_POP) + OAM2_LAYER(0) + OAM2_CHR(CHR_POPUP));
	PutSpriteExt(4, OAM1_X((FS->popX + half - 64) & 0x1FF) | OAM1_AFFINE_ID(AFF_POP),
		     OAM0_Y((cy - 32) & 0xFF) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE, gObject_64x32,
		     OAM2_PAL(PAL_POP) + OAM2_LAYER(0) + OAM2_CHR(CHR_POPUP + 32));
}

/* The fish is hooked: the word for the hook, sparks from the hook and a jolt. */
static void Fishing_OnHooked(void)
{
	static const u8 words[3] = { FISHING_POP_HOOKED, FISHING_POP_GREAT, FISHING_POP_PERFECT };
	int y = FS->hookY - FS->camY;

	Fishing_Pop(words[FS->hookQ > 2 ? 2 : FS->hookQ], 96, y - 28);
	Fishing_Burst(HOOK_X + 2, y, FS->hookQ == 2 ? 14 : 10, 0);
	Fishing_Shake(FS->hookQ == 2 ? 14 : 8);
}

/* The fish is landed: coins fly from the bobber, and the golden fish shakes the screen. */
static void Fishing_OnLanded(void)
{
	bool golden = FS->lastType == FISH_GOLDEN;
	int y = BOBBER_Y - FS->camY;

	Fishing_Pop(golden ? FISHING_POP_JACKPOT : FISHING_POP_CAUGHT, 96, y - 16);
	Fishing_Burst(HOOK_X, y, golden ? 16 : 10, 1);
	Fishing_Burst(HOOK_X, y, golden ? 6 : 0, 0);
	if (golden)
		Fishing_Shake(24);
}

/* ---- The states ---- */

static void Fishing_ToMessage(int msg)
{
	FS->msg = msg;
	FS->state = FS_MESSAGE;
	FS->timer = 120;
}

/* The fish is gone with the bait. */
static void Fishing_LoseFish(int msg)
{
	if (FS->biter >= 0)
		Fishing_Flee(FS->biter);
	FS->baited = BAIT_NONE;
	PlaySoundEffect(SE_LOST);
	Fishing_ToMessage(msg);
}

static void Fishing_ToReady(void)
{
	FS->state = FS_READY;
	FS->hookY = HOOK_REST;
}

static void Fishing_OpenTable(void)
{
	int i;

	/* The table covers the scene: no pop-up or particles over it */
	FS->popWord = 0xFF;
	for (i = 0; i < 16; i++)
		FS->parts[i].life = 0;
	FS->prevState = FS->state;
	FS->state = FS_PRIZES;
	FS->page = FS->water;
	Fishing_OpenPrizes();
}

static void Fishing_Payout(void)
{
	const struct FishType *t = &sFishTypes[FS->fish[FS->biter].type];
	int need = t->need;
	int lo, span, sizeQ;
	u32 gold;

	if (FS->reelFrames <= ((need * 5) >> 2))
		FS->landQ = 2;
	else if (FS->reelFrames <= ((need * 3) >> 1))
		FS->landQ = 1;
	else
		FS->landQ = 0;

	FS->lastType = FS->fish[FS->biter].type;

	/* The species and its size: two rolls averaged, so the ends of its range are rare */
	FS->species = FS->water * FISH_TYPES + FS->lastType;
	lo = gFishingSpeciesCm[FS->species * 2];
	span = gFishingSpeciesCm[FS->species * 2 + 1] - lo;
	FS->cm = lo + ((NextRN_N(span + 1) + NextRN_N(span + 1)) >> 1);
	sizeQ = span > 0 ? Div((FS->cm - lo) * 50, span) : 25;

	/* The prize, times the bonuses (quarters), times 0.75 to 1.25 for the size */
	FS->payout = Div(t->gold * (4 + FS->hookQ + FS->landQ) * (75 + sizeQ), 400);
	gold = GetPartyGoldAmount() + FS->payout;
	SetPartyGoldAmount(gold > 999999 ? 999999 : gold);

	FS->prevBest = FishingRecords_Caught(FS->species) ? FishingRecords_Best(FS->species) : 0;
	switch (FishingRecords_Add(FS->species, FS->cm)) {
	case FISHING_JOURNAL_NEW:
		FS->stamp = FISHING_POP_NEW;
		break;
	case FISHING_JOURNAL_RECORD:
		FS->stamp = FISHING_POP_RECORD;
		break;
	default:
		FS->stamp = 0xFF;
		break;
	}
}

/* Returns TRUE to leave the screen. */
static bool Fishing_UpdateState(void)
{
	u16 newKeys = gKeyStatusPtr->newKeys;
	u16 held = gKeyStatusPtr->heldKeys;
	const struct FishType *t = FS->biter >= 0 ? &sFishTypes[FS->fish[FS->biter].type] : NULL;
	int span;

	/* B leaves from anywhere but the walk-in and the prize table. */
	if ((newKeys & B_BUTTON) && FS->state != FS_WALKIN && FS->state != FS_PRIZES) {
		PlaySoundEffect(SE_LEAVE);
		return TRUE;
	}

	switch (FS->state) {
	case FS_WALKIN:
		FS->unitX--;
		FS->unitFrame = (GetGameClock() >> 3) & 3;
		if (FS->unitX <= FISHING_UNIT_X) {
			FS->unitX = FISHING_UNIT_X;
			FS->unitFrame = 0;
			Fishing_ToReady();
		}
		break;

	case FS_READY:
		if (newKeys & START_BUTTON) {
			Fishing_OpenTable();
			break;
		}
		if (newKeys & DPAD_LEFT) {
			FS->bait = FS->bait == 0 ? BAIT_TYPES - 1 : FS->bait - 1;
			PlaySoundEffect(SE_PICK);
		}
		if (newKeys & DPAD_RIGHT) {
			FS->bait = FS->bait == BAIT_TYPES - 1 ? 0 : FS->bait + 1;
			PlaySoundEffect(SE_PICK);
		}
		if (newKeys & A_BUTTON) {
			if (FS->baited != FS->bait) {
				if (GetPartyGoldAmount() < sBaitTypes[FS->bait].price) {
					PlaySoundEffect(SE_LOST);
					Fishing_ToMessage(MSG_FISHING_POOR);
					break;
				}
				SetPartyGoldAmount(GetPartyGoldAmount() - sBaitTypes[FS->bait].price);
				FS->baited = FS->bait;
			}
			FS->state = FS_CAST;
			FS->hookY = HOOK_REST;
			FS->rodAngle = 46 << 8;
			PlaySoundEffect(SE_CAST);
		}
		break;

	case FS_CAST:
		FS->hookY += 4;
		if (FS->hookY >= FISHING_SURFACE_Y && FS->hookY < FISHING_SURFACE_Y + 4)
			FS->splash = 16;
		if (FS->hookY >= FS->targetY) {
			FS->hookY = FS->targetY;
			FS->state = FS_WAIT;
		}
		break;

	case FS_WAIT:
		if (newKeys & START_BUTTON) {
			Fishing_OpenTable();
			break;
		}
		if (newKeys & A_BUTTON) {
			if (FS->biter < 0 || FS->fish[FS->biter].mode != FISHM_NIBBLE) {
				FS->state = FS_RECALL;
				PlaySoundEffect(SE_PULL);
			}
			break;
		}
		if ((held & DPAD_UP) && FS->targetY > HOOK_MIN)
			FS->targetY = Fishing_Clamp(FS->targetY - 2, HOOK_MIN, HOOK_MAX);
		if ((held & DPAD_DOWN) && FS->targetY < HOOK_MAX)
			FS->targetY = Fishing_Clamp(FS->targetY + 2, HOOK_MIN, HOOK_MAX);
		FS->hookY = FS->targetY;
		break;

	case FS_RECALL:
		FS->hookY -= 8;
		if (FS->hookY <= HOOK_REST)
			Fishing_ToReady();
		break;

	case FS_BITE:
		FS->biteClock++;
		if (newKeys & A_BUTTON) {
			FS->hookQ = FS->biteClock <= 8 ? 2 : (FS->biteClock <= 14 ? 1 : 0);
			FS->bonusTimer = FS->hookQ > 0 ? 90 : 0;
			FS->state = FS_REEL;
			FS->tension = TENSION_START;
			FS->progress = 0;
			FS->reelFrames = 0;
			FS->slack = 0;
			FS->burst = 0;
			FS->reelStartY = FS->hookY;
			PlaySoundEffect(SE_BITE);
			Fishing_OnHooked();
			break;
		}
		if (FS->timer > 0) {
			FS->timer--;
			break;
		}
		Fishing_LoseFish(MSG_FISHING_MISSED);
		break;

	case FS_REEL:
		if (FS->bonusTimer > 0)
			FS->bonusTimer--;
		FS->reelFrames++;
		if (newKeys & A_BUTTON) {
			FS->tension += TENSION_PRESS;
			FS->rodAngle += 4 << 8;
			PlaySoundEffect(SE_PULL);
		}

		/* The fish pulls back, twice as hard during a run. */
		if (FS->burst > 0)
			FS->burst--;
		else if (t->run != 0 && NextRN_N(t->run) == 0)
			FS->burst = RUN_FRAMES;
		FS->tension -= FS->burst > 0 ? t->pull * 2 : t->pull;
		/* Big fish jerk the line during a run: up to 6% either way every 8 frames */
		if (FS->burst > 0 && t->w >= 32 && (GetGameClock() & 7) == 0)
			FS->tension += (NextRN_N(13) - 6) * 16;

		if (FS->tension >= TENSION_MAX) {
			Fishing_LoseFish(MSG_FISHING_SNAPPED);
			break;
		}
		if (FS->tension <= 0) {
			FS->tension = 0;
			if (++FS->slack > SLACK_FRAMES) {
				Fishing_LoseFish(MSG_FISHING_SLACK);
				break;
			}
		} else {
			FS->slack = 0;
		}
		if (FS->tension >= t->zoneLo * 16 && FS->tension <= t->zoneHi * 16)
			FS->progress++;
		if (FS->progress >= t->need) {
			FS->state = FS_LAND;
			FS->timer = 30;
			FS->splash = 30;
			FS->hookY = FISHING_SURFACE_Y + 2;
			FS->burst = 0;
			PlaySoundEffect(SE_CAST);
			break;
		}
		if (FS->reelFrames > t->need * 2 + 120) {
			Fishing_LoseFish(MSG_FISHING_ESCAPED);
			break;
		}
		span = FS->reelStartY - (FISHING_SURFACE_Y + 6);
		FS->hookY = FS->reelStartY - Div(span * FS->progress, t->need);
		break;

	case FS_LAND:
		if (FS->timer > 0) {
			FS->timer--;
			break;
		}
		Fishing_Payout();
		Fishing_Hide(&FS->fish[FS->biter]);
		FS->biter = -1;
		FS->baited = BAIT_NONE;
		FS->state = FS_RESULT;
		FS->timer = CARD_DELAY;
		FS->hookY = HOOK_REST;
		PlaySoundEffect(SE_GOLD);
		Fishing_OnLanded();
		break;

	case FS_RESULT:
		/* The card opens after CAUGHT!, or at once with A */
		if (FS->cardT == 0) {
			if (FS->timer > 0)
				FS->timer--;
			if (FS->timer == 0 || (FS->timer < CARD_DELAY - 10 && (newKeys & A_BUTTON)))
				Fishing_OpenCard();
			break;
		}
		if (FS->cardT < 255)
			FS->cardT++;
		if (FS->cardT == STAMP_AT + 6 && FS->stamp != 0xFF) {
			PlaySoundEffect(SONG_SE_UPDATE);
			Fishing_Shake(8);
			Fishing_Burst(STAMP_CX, STAMP_CY, 10, 0);
		}
		if (FS->cardT < 8)
			break;
		if (newKeys & START_BUTTON) {
			Fishing_CloseCard();
			Fishing_ToReady();
			Fishing_OpenTable();
		} else if (newKeys & A_BUTTON) {
			Fishing_CloseCard();
			Fishing_ToReady();
		}
		break;

	case FS_MESSAGE:
		FS->hookY = FS->hookY - 8 > HOOK_REST ? FS->hookY - 8 : HOOK_REST;
		if (FS->timer > 0)
			FS->timer--;
		if (FS->timer == 0 || (FS->timer < 100 && (newKeys & A_BUTTON)))
			Fishing_ToReady();
		break;

	case FS_PRIZES:
		if (newKeys & (B_BUTTON | START_BUTTON)) {
			Fishing_ClosePrizes();
			FS->state = FS->prevState;
			PlaySoundEffect(SE_LEAVE);
		} else if (newKeys & (DPAD_LEFT | DPAD_RIGHT)) {
			if (newKeys & DPAD_LEFT)
				FS->page = FS->page == 0 ? FISHING_WATERS - 1 : FS->page - 1;
			else
				FS->page = FS->page == FISHING_WATERS - 1 ? 0 : FS->page + 1;
			PlaySoundEffect(SE_PICK);
			Fishing_OpenPrizes();
		}
		break;
	}
	return FALSE;
}

/* The screen follows the hook while the line is out, and the bank otherwise. */
static void Fishing_UpdateCamera(void)
{
	int target, d;

	switch (FS->state) {
	case FS_CAST:
	case FS_WAIT:
	case FS_BITE:
	case FS_REEL:
	case FS_RECALL:
		target = Fishing_Clamp(FS->hookY - 100, 0, CAM_MAX);
		break;
	case FS_PRIZES:
		return;
	default:
		target = 0;
		break;
	}
	d = target - FS->camY;
	FS->camY += Fishing_Clamp(d, -6, 6);
	Fishing_UpdateShake();
	BG_SetPosition(3, FS->shakeX, FS->camY + FS->shakeY);
}

/* ---- Drawing (back to front: a later PutSprite is drawn in front) ---- */

/* A sprite of the scene: it shakes with BG3 */
#define SPR(x, y, obj, chr, pal, flags0, flags1) \
	PutSpriteExt(4, OAM1_X(((x) - FS->shakeX) & 0x1FF) | (flags1), OAM0_Y(((y) - FS->shakeY) & 0xFF) | (flags0), \
		     (obj), OAM2_PAL(pal) + OAM2_LAYER(2) + OAM2_CHR(chr))

#define UI_SPR(x, y, obj, chr) \
	PutSpriteExt(4, OAM1_X((x) & 0x1FF), OAM0_Y((y) & 0xFF), (obj), \
		     OAM2_PAL(PAL_UI) + OAM2_LAYER(0) + OAM2_CHR(chr))

/* Scene row to screen row; FALSE when an object h rows tall at it is off screen */
static bool Fishing_OnScreen(int y, int h)
{
	int sy = y - FS->camY;

	return sy > -h && sy < 160;
}

static void Fishing_DrawFish(void)
{
	int i, clock = GetGameClock();
	const struct FishingFish *f;
	const struct FishType *t;
	int frame, top;

	for (i = 0; i < FISHING_FISH_MAX; i++) {
		f = &FS->fish[i];
		if (f->mode == FISHM_HIDDEN)
			continue;
		t = &sFishTypes[f->type];
		top = f->y - (t->h >> 1);
		if (!Fishing_OnScreen(top, t->h))
			continue;
		frame = (clock >> (f->mode == FISHM_FLEE || f->mode == FISHM_HOOKED ? 2 : 4)) & 1;
		SPR(Fishing_Px(f), top - FS->camY, t->obj, t->chr + frame * t->frameTiles, t->pal,
		    t->blend ? OAM0_BLEND : 0, f->vx < 0 ? OAM1_HFLIP : 0);
	}
}

static void Fishing_DrawLine(void)
{
	int bobY, x, y, dip = 0, bottom, top, tipX, tipY, span;
	int clock = GetGameClock();

	if (FS->state == FS_WALKIN || FS->state == FS_PRIZES)
		return;

	/* From the rod tip to the bobber the line slants (the tip moves with the rod), then hangs straight down. */
	Fishing_RodTip(&tipX, &tipY);
	bottom = FS->hookY - 3;
	span = BOBBER_Y + 4 - tipY;
	top = FS->camY + FISHING_WINDOW_H > tipY ? FS->camY + FISHING_WINDOW_H : tipY;
	for (y = top; y + 4 < bottom; y += 4) {
		x = HOOK_X;
		if (y < BOBBER_Y + 4 && span > 0)
			x = tipX + Div((HOOK_X - tipX) * (y - tipY), span);
		if (Fishing_OnScreen(y, 8))
			SPR(x, y - FS->camY, gObject_8x8, FISHING_CHR_LINE, PAL_FX, 0, 0);
	}
	if (bottom - 8 >= BOBBER_Y && Fishing_OnScreen(bottom - 8, 8))
		SPR(HOOK_X, bottom - 8 - FS->camY, gObject_8x8, FISHING_CHR_LINE, PAL_FX, 0, 0);

	if (FS->hookY > HOOK_REST && Fishing_OnScreen(FS->hookY - 3, 8)) {
		SPR(HOOK_X, FS->hookY - 3 - FS->camY, gObject_8x8, FISHING_CHR_HOOK, PAL_FX, 0, 0);
		if (FS->baited != BAIT_NONE && FS->state != FS_REEL && FS->state != FS_LAND)
			SPR(HOOK_X + 1, FS->hookY + 1 - FS->camY, gObject_8x8, FISHING_CHR_BAIT + FS->baited, PAL_FX, 0, 0);
	}

	/* The bobber floats on the surface (or rides the line above it during the cast). */
	bobY = FS->hookY - 12 < BOBBER_Y ? FS->hookY - 12 : BOBBER_Y;
	if (FS->state == FS_BITE || FS->state == FS_REEL)
		dip = (clock & 4) ? 3 : 1;
	else if (FS->biter >= 0 && FS->fish[FS->biter].mode == FISHM_NIBBLE)
		dip = (clock & 8) ? 1 : 0;
	else if (FS->state == FS_WAIT)
		dip = (clock & 32) ? 1 : 0;
	if (bobY + dip > tipY && Fishing_OnScreen(bobY + dip, 8))
		SPR(HOOK_X - 3, bobY + dip - FS->camY, gObject_8x8, FISHING_CHR_BOBBER + (dip >= 2 ? 1 : 0), PAL_FX, 0, 0);

	if (FS->splash > 0) {
		if (Fishing_OnScreen(FISHING_SURFACE_Y - 7, 8))
			SPR(HOOK_X - 7, FISHING_SURFACE_Y - 7 - FS->camY, gObject_16x8,
			    FISHING_CHR_SPLASH + ((FS->splash >> 2) & 1) * FISHING_CHR_SPLASH_FRAME, PAL_FX, 0, 0);
		FS->splash--;
	}
}

static void Fishing_DrawUnit(void)
{
	int y = UNIT_Y - FS->camY;

	if (!Fishing_OnScreen(UNIT_Y, 32))
		return;
	if (FS->state != FS_WALKIN)
		Fishing_DrawRod();
	PutSpriteExt(4, OAM1_X((FS->unitX - FS->shakeX) & 0x1FF), OAM0_Y((y - FS->shakeY) & 0xFF), gObject_32x32,
		     OAM2_PAL(FS->smsPal) + OAM2_LAYER(2) + OAM2_CHR(CHR_UNIT + FS->unitFrame * 16));
	if (FS->state == FS_BITE && (GetGameClock() & 8))
		SPR(FS->unitX + 12, y - 12, gObject_8x16, FISHING_CHR_BANG, PAL_FX, 0, 0);
}

/* Paints a 64x8 meter into its 8 OBJ tiles: a frame, the back, a fill to `fill` pixels, a zone and a needle. */
static void Fishing_PaintMeter(int bar, int fill, int fillColor, int zoneLo, int zoneHi, int needle)
{
	u32 tiles[8][8];
	int tx, py, px, x, c;
	u32 row;

	for (tx = 0; tx < 8; tx++) {
		for (py = 0; py < 8; py++) {
			row = 0;
			for (px = 0; px < 8; px++) {
				x = tx * 8 + px;
				if (py == 0 || py == 7 || x == 0 || x == 63)
					c = 13;
				else if (needle >= 0 && (x == needle || x == needle + 1))
					c = 5;
				else if (x < fill)
					c = fillColor;
				else if (x >= zoneLo && x <= zoneHi)
					c = 9;
				else
					c = 12;
				row |= c << (px * 4);
			}
			tiles[tx][py] = row;
		}
	}
	CpuFastCopy(tiles, (void *)(OBJ_VRAM0 + (CHR_BARS + bar * 8) * 0x20), sizeof(tiles));
}

static void Fishing_DrawMeters(void)
{
	const struct FishType *t;
	int i, needle;

	if (FS->state != FS_REEL || FS->biter < 0)
		return;
	t = &sFishTypes[FS->fish[FS->biter].type];

	needle = Div(FS->tension * 62, TENSION_MAX);
	Fishing_PaintMeter(0, 0, 0, Div(t->zoneLo * 64, 100), Div(t->zoneHi * 64, 100), Fishing_Clamp(needle, 1, 61));
	Fishing_PaintMeter(1, Div(FS->progress * 64, t->need), 10, -1, -1, -1);
	i = t->need * 2 + 120 - FS->reelFrames;
	Fishing_PaintMeter(2, Div((i > 0 ? i : 0) * 64, t->need * 2 + 120), FS->burst > 0 ? 14 : 11, -1, -1, -1);

	for (i = 0; i < 3; i++) {
		UI_SPR(METER_X - 12, METER_Y + i * 10, gObject_8x8, FISHING_CHR_METER_ICON + i);
		UI_SPR(METER_X, METER_Y + i * 10, gObject_32x8, CHR_BARS + i * 8);
		UI_SPR(METER_X + 32, METER_Y + i * 10, gObject_32x8, CHR_BARS + i * 8 + 4);
	}
}

static void Fishing_DrawHints(void)
{
	int i, icon;

	for (i = 0; i < FS->hintCount; i++) {
		icon = FS->hintIcon[i];
		UI_SPR(FS->hintX[i], HINT_ROW * 8, icon == HINT_START ? gObject_32x16 : gObject_16x16, sHintChr[icon]);
	}
}

/*
 * The window's sprites go last, so they come first in OAM: the GBA lets a sprite earlier in OAM hide a later one even
 * when the later one has the higher priority, and a fish behind the window would cut boxes out of the buttons.
 */
static void Fishing_Draw(void)
{
	if (FS->state != FS_PRIZES) {
		Fishing_DrawFish();
		Fishing_DrawLine();
		Fishing_DrawUnit();
		Fishing_DrawParticles();
	}
	Fishing_DrawMeters();
	Fishing_DrawPopup();
	Fishing_DrawHints();
	if (FS->state == FS_PRIZES)
		Fishing_DrawJournalIcons();
	else
		Fishing_DrawCard();
}

/* ---- The proc ---- */

enum { PL_FISHING_EXIT = 1 };

static void Fishing_Loop(struct ProcFishing *proc)
{
	int i;

	if (Fishing_UpdateState()) {
		Proc_Goto(proc, PL_FISHING_EXIT);
		return;
	}
	if (FS->state != FS_PRIZES) {
		for (i = 0; i < FISHING_FISH_MAX; i++)
			Fishing_UpdateFish(i);
		Fishing_UpdateRod();
		Fishing_UpdateEffects();
	}
	Fishing_UpdateCamera();
	Fishing_UpdateWindow();
	Fishing_Draw();
}

static void Fishing_OnEnd(struct ProcFishing *proc)
{
	ClearSprites();
	ClearBg0Bg1();
	BG_SetPosition(3, 0, 0);
	gLCDControlBuffer.dispcnt = proc->dispBak;
	SetDefaultColorEffects();
}

const struct ProcCmd ProcScr_FishingScreen[] = {
	PROC_NAME("FishingScreen"),
	PROC_YIELD,
	PROC_SET_END_CB(Fishing_OnEnd),

	PROC_CALL(Fishing_Init),
	PROC_CALL_ARG(NewFadeIn, 0x10),
	PROC_REPEAT(Fishing_Loop),

	PROC_LABEL(PL_FISHING_EXIT),
	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_END,
};

#endif /* CONFIG_DISTRICT_FISHING */
