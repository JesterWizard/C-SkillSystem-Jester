#include "common-chax.h"
#include "kernel-lib.h"
#include "mu.h"
#include "crossing.h"
#include "crossing-art.h"
#include "constants/texts.h"
#include "constants/classes.h"
#include "m4a.h"

#ifdef CONFIG_DISTRICT_CROSSING

/*
 * River Crossing, a lane-crossing game on a 15 x 10 grid of 16x16 cells. Top to bottom: the status line, the far bank
 * with five docks, three river lanes of drifting rafts, a grass median, three road lanes of enemy riders, and the start.
 *
 *   READY   the unit waits at the start. The D-pad starts the game. B leaves, START pauses.
 *   PLAY    the D-pad hops one cell (8 frames). On the road, a rider that touches the unit tramples it. In the river the
 *           unit must stand on a raft (it drifts with it); the water or the screen's edge drowns it. Up from the top
 *           river lane reaches the bank only at an empty dock. Each life has 30 seconds.
 *   DYING   a pause after a lost life; then the unit starts again, or the game is over with no lives left.
 *   CLEAR   all five docks are filled: a bonus, the docks empty and the next round is faster.
 *   PAUSE   START: the game stops. START again goes on, B leaves.
 *   OVER    the payout (score / 10 gold) and the records. A or B leaves.
 *
 * Score: 10 for each new row reached in a life, 50 and the seconds left for a dock, 250 for a cleared round (750, and
 * PERFECT!, with no life lost in it). B leaves at any time and pays out what was scored.
 *
 * Feel: hops stretch and land with a squash, dust on the ground and a ripple on a raft; a rider passing within a few
 * pixels is a near miss (CLOSE!); a trampling freezes the game a moment and shakes it; a drowned unit sinks in a spray
 * of droplets; each dock in a row without losing a life plays the next note of a scale; score pop-ups rise from the
 * unit; at the end the score counts up, and a district record pops up NEW BEST!. Each district has its own course (sCourses): Serafew is the gentlest,
 * Jehanna the fastest.
 *
 * Layers: BG3 the scene (palette CROSSING_BG_PAL, tiles at 0x06008000), BG0 the status line and the button labels with
 * no window behind them, OBJ (1D) the sprites. The riders are the walking map sprites of real classes in the enemy
 * colours; the unit is its own walking sprite. Avoid `%` and unsigned `/` (CLAUDE.md).
 */

#define CS (&gCrossingState)

#define CROSSING_BG_PAL 8
#define PAL_FX 2
#define PAL_UI 3
#define PAL_POP 5
#define AFF_POP 1
#define AFF_UNIT 2
#define SCENE_CHR_VRAM 0x06008000

#define CHR_UNIT 0x100      /* the unit's walking sheet (12 frames of 16 tiles); also the riders' load buffer */
#define CHR_POPUP 0x220     /* the pop-up word (64 tiles) */
#define CHR_TIMER 0x260     /* the timer meter (6 tiles) */
#define CHR_RIDER 0x280     /* the riders' side frames (3 classes x 64 tiles); 0x380 up is the map's moving units */

#define CELL 16
#define START_COL 7
#define LIFE_FRAMES (30 * 60)
#define HOP_FRAMES 8
#define TIMER_X 186
#define TIMER_W 48

#define SE_HOP SONG_SE_SYS_CURSOR_LR1
#define SE_DOCK SONG_SE_SYS_WINDOW_SELECT1
#define SE_HIT 0xD4
#define SE_SPLASH 0xC8
#define SE_LOST SONG_6C
#define SE_GOLD SONG_SE_MONEY
#define SE_LEAVE SONG_SE_SYS_WINDOW_CANSEL1
#define SE_NOTE SONG_SE_SYS_WINDOW_SELECT1  /* a dock's note, a scale step higher for each dock in a row */
#define SE_CLOSE SONG_SE_SYS_CURSOR_UD1
#define SE_TICK SONG_SE_SYS_CURSOR_LR1
#define HIT_STOP 10                         /* frames frozen when trampled */
#define NEAR_MISS 5                         /* a rider passing within this many pixels of trampling */
#define SQUASH_FRAMES 6
#define SINK_FRAMES 24
#define PERFECT_BONUS 500                   /* on top of the round's 250, with no life lost in it */

enum {
	CS_READY,
	CS_PLAY,
	CS_DYING,
	CS_CLEAR,
	CS_PAUSE,
	CS_OVER,
};

enum {
	CROSSING_DEATH_RIDER,
	CROSSING_DEATH_WATER,
	CROSSING_DEATH_TIME,
};

enum {
	FACE_LEFT,
	FACE_RIGHT,
	FACE_DOWN,
	FACE_UP,
};

/* A lane: riders (kind 0) or rafts (kind 1), moving `dir` at `speed`, `count` of them `spacing` pixels apart. */
struct CrossingLane {
	u8 row;
	u8 kind;
	u8 rider;       /* riders: which of sRiderClasses */
	u8 len;         /* rafts: length in cells */
	s8 dir;         /* -1 left, 1 right */
	u8 count;
	u16 spacing;    /* pixels; count * spacing is the lane's period and must be at least 240 + 64 */
	u16 speed;      /* pixels << 8 per frame, in round 1 */
};

enum { RIDER_KNIGHT, RIDER_CAVALIER, RIDER_PALADIN, RIDERS };

static const u8 sRiderClasses[RIDERS] = { CLASS_ARMOR_KNIGHT, CLASS_CAVALIER, CLASS_PALADIN };
static const u8 sRiderHalf[RIDERS] = { 9, 13, 13 };   /* half the width a rider tramples */

/* The courses, one per district (Serafew, Port Kiris, Caer Pelyn, Jehanna Hall). Lanes listed bottom to top. */
static const struct CrossingLane sCourses[CROSSING_LEVELS][CROSSING_LANES] = {
	{
		{ 8, 0, RIDER_KNIGHT, 1, -1, 3, 112, 0x50 },
		{ 7, 0, RIDER_CAVALIER, 1, 1, 2, 160, 0x90 },
		{ 6, 0, RIDER_PALADIN, 1, -1, 2, 160, 0xC0 },
		{ 4, 1, 0, 4, 1, 3, 112, 0x50 },
		{ 3, 1, 0, 3, -1, 3, 112, 0x70 },
		{ 2, 1, 0, 4, 1, 2, 160, 0x60 },
	},
	{
		{ 8, 0, RIDER_KNIGHT, 1, -1, 3, 104, 0x60 },
		{ 7, 0, RIDER_CAVALIER, 1, 1, 3, 104, 0xA0 },
		{ 6, 0, RIDER_PALADIN, 1, -1, 2, 160, 0xE0 },
		{ 4, 1, 0, 3, 1, 3, 112, 0x60 },
		{ 3, 1, 0, 3, -1, 3, 112, 0x80 },
		{ 2, 1, 0, 3, 1, 3, 112, 0x70 },
	},
	{
		{ 8, 0, RIDER_KNIGHT, 1, -1, 4, 80, 0x60 },
		{ 7, 0, RIDER_CAVALIER, 1, 1, 3, 104, 0xB8 },
		{ 6, 0, RIDER_PALADIN, 1, -1, 3, 104, 0x100 },
		{ 4, 1, 0, 3, 1, 3, 112, 0x70 },
		{ 3, 1, 0, 2, -1, 4, 80, 0x90 },
		{ 2, 1, 0, 3, 1, 3, 112, 0x80 },
	},
	{
		{ 8, 0, RIDER_KNIGHT, 1, -1, 4, 80, 0x70 },
		{ 7, 0, RIDER_CAVALIER, 1, 1, 3, 104, 0xD0 },
		{ 6, 0, RIDER_PALADIN, 1, -1, 3, 104, 0x120 },
		{ 4, 1, 0, 3, 1, 3, 112, 0x80 },
		{ 3, 1, 0, 2, -1, 4, 80, 0xA0 },
		{ 2, 1, 0, 2, 1, 4, 80, 0x90 },
	},
};

static const u8 sDockCols[CROSSING_DOCKS] = CROSSING_DOCK_COLS;

struct ProcCrossing {
	PROC_HEADER;
	struct DispCnt dispBak;
};

/* ---- Helpers ---- */

static int Crossing_Abs(int v)
{
	return v < 0 ? -v : v;
}

static int Crossing_Clamp(int v, int lo, int hi)
{
	return v < lo ? lo : (v > hi ? hi : v);
}

static const struct CrossingLane *Crossing_Lane(int i)
{
	return &sCourses[CS->level][i];
}

static int Crossing_LaneOfRow(int row)
{
	int i;

	for (i = 0; i < CROSSING_LANES; i++)
		if (Crossing_Lane(i)->row == row)
			return i;
	return -1;
}

/* The lane's speed this round: 12% faster each round, up to round 6 */
static int Crossing_Speed(const struct CrossingLane *l)
{
	int r = CS->round > 6 ? 6 : CS->round;

	return l->speed + ((l->speed * (r - 1) * 12) >> 7);
}

static int Crossing_Period(const struct CrossingLane *l)
{
	return l->count * l->spacing;
}

/* The left edge of object k of lane i, in pixels (objects enter from the left of the screen, 64 pixels out) */
static int Crossing_ObjX(int i, int k)
{
	const struct CrossingLane *l = Crossing_Lane(i);
	int p = Crossing_Period(l);
	int x = (CS->laneX[i] >> 8) + k * l->spacing;

	while (x >= p)
		x -= p;
	return x - 64;
}

static int Crossing_UnitCX(void)
{
	return (CS->px >> 8) + 8;
}

/* ---- Text ---- */

static void Crossing_Append(char *buf, int *i, const char *s)
{
	while (*s != 0 && *i < 62)
		buf[(*i)++] = *s++;
	buf[*i] = 0;
}

static void Crossing_AppendMsg(char *buf, int *i, int msg)
{
	Crossing_Append(buf, i, GetStringFromIndex(msg));
}

static void Crossing_AppendNum(char *buf, int *i, int n)
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

static void Crossing_Line1(char *buf)
{
	int i = 0;

	buf[0] = 0;
	switch (CS->state) {
	case CS_READY:
		Crossing_AppendMsg(buf, &i, MSG_CROSSING_READY);
		break;
	case CS_PAUSE:
		Crossing_AppendMsg(buf, &i, MSG_CROSSING_PAUSED);
		Crossing_Append(buf, &i, "   ");
		Crossing_AppendMsg(buf, &i, MSG_CROSSING_S_BEST);
		Crossing_Append(buf, &i, " ");
		Crossing_AppendNum(buf, &i, CS->best);
		break;
	case CS_OVER:
		Crossing_AppendMsg(buf, &i, MSG_CROSSING_S_SCORE);
		Crossing_Append(buf, &i, " ");
		Crossing_AppendNum(buf, &i, CS->shownScore);
		if (CS->countDone) {
			Crossing_Append(buf, &i, "   +");
			Crossing_AppendNum(buf, &i, CS->gold);
			Crossing_Append(buf, &i, "G");
		}
		break;
	default:
		Crossing_AppendMsg(buf, &i, MSG_CROSSING_S_ROUND);
		Crossing_AppendNum(buf, &i, CS->round);
		Crossing_Append(buf, &i, "  ");
		Crossing_AppendMsg(buf, &i, MSG_CROSSING_S_SCORE);
		Crossing_Append(buf, &i, " ");
		Crossing_AppendNum(buf, &i, CS->score);
		Crossing_Append(buf, &i, "  ");
		Crossing_AppendMsg(buf, &i, MSG_CROSSING_S_BEST);
		Crossing_Append(buf, &i, " ");
		Crossing_AppendNum(buf, &i, CS->best > CS->score ? CS->best : CS->score);
		break;
	}
}

static u32 Crossing_Line1Sig(void)
{
	return (CS->state << 28) ^ (CS->round << 22) ^ (CS->score << 1) ^ CS->gold ^ (CS->shownScore << 9) ^ CS->countDone;
}

/* ---- The buttons (bottom row, no window, as in the prep screens) ---- */

#define HINT_ROW 18

enum { HINT_DPAD, HINT_A, HINT_B, HINT_START };

static const u16 sHintChr[] = { CROSSING_CHR_BTN_DPAD, CROSSING_CHR_BTN_A, CROSSING_CHR_BTN_B, CROSSING_CHR_BTN_START };

struct Hint {
	u8 icon;
	u16 msg;
};

static int Crossing_Hints(const struct Hint **out)
{
	static const struct Hint ready[] = {
		{ HINT_DPAD, MSG_CROSSING_K_MOVE }, { HINT_B, MSG_CROSSING_K_EXIT }, { HINT_START, MSG_CROSSING_K_PAUSE },
	};
	static const struct Hint pause[] = { { HINT_START, MSG_CROSSING_K_RESUME }, { HINT_B, MSG_CROSSING_K_EXIT } };
	static const struct Hint over[] = { { HINT_A, MSG_CROSSING_K_DONE } };

	switch (CS->state) {
	case CS_READY:
		*out = ready;
		return 3;
	case CS_PAUSE:
		*out = pause;
		return 2;
	case CS_OVER:
		*out = over;
		return 1;
	default:
		*out = NULL;
		return 0;
	}
}

static void Crossing_DrawHintText(void)
{
	struct Text *text = &CS->texts[1];
	const struct Hint *hints;
	const char *label;
	int n = Crossing_Hints(&hints);
	int i, x = 0, w, len;

	ClearText(text);
	for (i = 0; i < n && i < 4; i++) {
		w = hints[i].icon == HINT_START ? 32 : 16;
		/* Keep clear of the start cell in the middle of the row, where the unit stands */
		len = w + 2 + GetStringTextLen(GetStringFromIndex(hints[i].msg));
		if (8 + x < START_COL * CELL + CELL + 8 && 8 + x + len > START_COL * CELL - 8)
			x = START_COL * CELL + CELL + 8 - 8;
		CS->hintIcon[i] = hints[i].icon;
		CS->hintX[i] = 8 + x;
		label = GetStringFromIndex(hints[i].msg);
		Text_InsertDrawString(text, x + w + 2, TEXT_COLOR_SYSTEM_WHITE, label);
		x += w + 2 + GetStringTextLen(GetStringFromIndex(hints[i].msg)) + 8;
	}
	CS->hintCount = i;
	PutText(text, TILEMAP_LOCATED(gBG0TilemapBuffer, 1, HINT_ROW));
	CS->hintKey = CS->state;
}

static void Crossing_UpdateText(void)
{
	char buf[64];
	u32 sig = Crossing_Line1Sig();
	bool sync = FALSE;

	if (sig != CS->line1Sig) {
		Crossing_Line1(buf);
		ClearText(&CS->texts[0]);
		Text_SetParams(&CS->texts[0], 0, TEXT_COLOR_SYSTEM_WHITE);
		Text_DrawString(&CS->texts[0], buf);
		PutText(&CS->texts[0], TILEMAP_LOCATED(gBG0TilemapBuffer, 1, 0));
		CS->line1Sig = sig;
		sync = TRUE;
	}
	if (CS->hintKey != CS->state) {
		Crossing_DrawHintText();
		sync = TRUE;
	}
	if (sync)
		BG_EnableSyncByMask(BG0_SYNC_BIT);
}

/* ---- Effects: the pop-up word, sparks, the splash and the shake ---- */

static void Crossing_Pop(int word, int x, int y)
{
	CpuFastCopy(gCrossingPopTiles + word * CROSSING_POP_TILES * 0x20, (void *)(OBJ_VRAM0 + CHR_POPUP * 0x20),
		    CROSSING_POP_TILES * 0x20);
	ApplyPalette(gCrossingPopPals + gCrossingPopWordPal[word] * 16, 0x10 + PAL_POP);
	EnablePaletteSync();
	CS->popWord = word;
	CS->popTimer = 0;
	CS->popX = Crossing_Clamp(x, 64, 176);
	CS->popY = Crossing_Clamp(y, 40, 120);
}

/* A burst of sparks (kind 0) or water droplets (kind 1) from a screen point */
static void Crossing_Burst(int x, int y, int kind)
{
	struct CrossingParticle *p;
	int i, a, speed;

	for (i = 0; i < CROSSING_PARTS; i++) {
		p = &CS->parts[i];
		a = kind ? 64 + 128 * i / CROSSING_PARTS - 64 + NextRN_N(24) : ((i << 8) >> 3) + NextRN_N(16);
		speed = kind ? 24 + NextRN_N(16) : 20 + NextRN_N(20);
		p->x = x << 4;
		p->y = y << 4;
		p->vx = (COS(a & 0xFF) * speed) >> 12;
		p->vy = kind ? -(16 + NextRN_N(20)) : ((SIN(a & 0xFF) * speed) >> 12) - 8;
		p->life = 20 + NextRN_N(12);
		p->kind = kind;
	}
}

static void Crossing_Shake(int frames, int amp)
{
	if (frames >= CS->shake) {
		CS->shake = frames;
		CS->shakeAmp = amp;
	}
}

/* A number rising from a point: prefix 10 "+", 11 "x", -1 none */
static void Crossing_PopNumber(int prefix, int n, int x, int y)
{
	int len = 0, d[5], k = 0;

	if (prefix >= 0)
		CS->num[len++] = prefix;
	do {
		int q = Div(n, 10);

		d[k++] = n - q * 10;
		n = q;
	} while (n > 0 && k < 5);
	while (k > 0 && len < 6)
		CS->num[len++] = d[--k];
	CS->numLen = len;
	CS->numTimer = 32;
	CS->numX = Crossing_Clamp(x - len * 5 / 2, 0, 236 - len * 5);
	CS->numY = Crossing_Clamp(y, 18, 140);
}

/* The dock's note: up a major scale, a step for each dock filled in a row without losing a life */
static const u8 sScale[8] = { 0, 2, 4, 5, 7, 9, 11, 12 };

static void Crossing_Note(int chain)
{
	if (gPlaySt.config.disableSoundEffects)
		return;
	m4aSongNumStart(SE_NOTE);
	m4aMPlayPitchControl(gMPlayTable[gSongTable[SE_NOTE].ms].info, 0xFFFF,
			     sScale[Crossing_Clamp(chain, 1, 8) - 1] << 8);
}

static void Crossing_UpdateEffects(void)
{
	struct CrossingParticle *p;
	int i, clock = GetGameClock();

	for (i = 0; i < CROSSING_PARTS; i++) {
		p = &CS->parts[i];
		if (p->life == 0)
			continue;
		p->life--;
		p->x += p->vx;
		p->y += p->vy;
		if (p->kind || (p->life & 1))
			p->vy += p->kind ? 3 : 1;
	}
	if (CS->popWord != 0xFF && ++CS->popTimer >= 64)
		CS->popWord = 0xFF;
	if (CS->popWord == 0xFF && CS->pendPop != 0xFF) {
		Crossing_Pop(CS->pendPop, 120, 72);
		CS->pendPop = 0xFF;
	}
	if (CS->splash > 0)
		CS->splash--;
	if (CS->squash > 0)
		CS->squash--;
	if (CS->dustT > 0)
		CS->dustT--;
	if (CS->rippleT > 0)
		CS->rippleT--;
	if (CS->sinkT > 0)
		CS->sinkT--;
	if (CS->nearCool > 0)
		CS->nearCool--;
	if (CS->numTimer > 0) {
		CS->numTimer--;
		if (CS->numTimer & 1)
			CS->numY--;
	}
	if (CS->shake > 0) {
		CS->shake--;
		CS->shakeX = (clock & 2) ? CS->shakeAmp : -CS->shakeAmp;
	} else {
		CS->shakeX = 0;
	}
	BG_SetPosition(3, CS->shakeX, 0);
}

/* ---- Setup ---- */

static const u16 sCrossingBgConfig[12] = {
	0x0000, 0x6000, 0,  /* BG0: text */
	0x0000, 0x6800, 0,  /* BG1: unused */
	0x0000, 0x7000, 0,  /* BG2: unused */
	0x8000, 0x7800, 0,  /* BG3: the scene */
};

static void Crossing_LoadSheet(int cls, int chr, int tiles)
{
	const u8 *img = gMuInfoTable[cls - 1].img;

	/* The LZ77 header holds the unpacked size in bytes 1..3; CHR_UNIT has room for 0x2200 bytes */
	if (img == NULL || (img[1] | (img[2] << 8) | (img[3] << 16)) > 0x2200)
		img = gMuInfoTable[CLASS_CAVALIER - 1].img;
	Decompress(img, (void *)(OBJ_VRAM0 + CHR_UNIT * 0x20));
	if (chr != CHR_UNIT)
		CpuFastCopy((void *)(OBJ_VRAM0 + CHR_UNIT * 0x20), (void *)(OBJ_VRAM0 + chr * 0x20), tiles * 0x20);
}

static void Crossing_ResetLanes(void)
{
	const struct CrossingLane *l;
	int i;

	for (i = 0; i < CROSSING_LANES; i++) {
		l = Crossing_Lane(i);
		CS->laneX[i] = NextRN_N(Crossing_Period(l)) << 8;
	}
}

static void Crossing_Respawn(void)
{
	CS->row = CROSSING_ROW_START;
	CS->px = (START_COL * CELL) << 8;
	CS->py = CROSSING_ROW_START * CELL;
	CS->hopFrames = 0;
	CS->facing = FACE_UP;
	CS->maxRow = CROSSING_ROW_START;
	CS->timer = LIFE_FRAMES;
}

static int Crossing_LevelOfChapter(void)
{
	switch (gPlaySt.chapterIndex) {
	case 0x3C:
		return 1;
	case 0x41:
		return 2;
	case 0x42:
		return 3;
	default:
		return 0;
	}
}

static void Crossing_Init(struct ProcCrossing *proc)
{
	struct Unit *unit = gActiveUnit ? gActiveUnit : GetUnit(1);
	u16 *map = BG_GetMapBuffer(3);
	int i, x, y;

	proc->dispBak = gLCDControlBuffer.dispcnt;

	SetupBackgrounds((u16 *)sCrossingBgConfig);
	SetDispEnable(0, 0, 0, 0, 0);
	BG_Fill(BG_GetMapBuffer(0), 0);
	BG_Fill(BG_GetMapBuffer(1), 0);
	BG_Fill(BG_GetMapBuffer(2), 0);
	BG_Fill(map, 0);
	gLCDControlBuffer.bg0cnt.priority = 0;
	gLCDControlBuffer.bg3cnt.priority = 3;
	BG_SetPosition(0, 0, 0);
	BG_SetPosition(3, 0, 0);

	ResetText();
	SetTextFont(0);
	InitSystemTextFont();
	for (i = 0; i < 3; i++)
		InitText(&CS->texts[i], 28);

	CpuFastCopy(gCrossingSceneTiles, (void *)SCENE_CHR_VRAM, CROSSING_SCENE_TILES * 0x20);
	for (y = 0; y < 20; y++)
		for (x = 0; x < 30; x++)
			map[y * 32 + x] = gCrossingSceneMap[y * 30 + x] | (CROSSING_BG_PAL << 12);
	ApplyPalette(gCrossingScenePal, CROSSING_BG_PAL);

	CpuFastCopy(gCrossingObjTiles, (void *)OBJ_VRAM0, CROSSING_OBJ_TILES * 0x20);
	ApplyPalette(gCrossingObjPalFx, 0x10 + PAL_FX);
	ApplyPalette(gCrossingObjPalUi, 0x10 + PAL_UI);

	/* The riders' side frames (0..3) first, through the unit's slot; then the unit's own sheet stays there */
	for (i = 0; i < RIDERS; i++)
		Crossing_LoadSheet(sRiderClasses[i], CHR_RIDER + i * 64, 64);
	Crossing_LoadSheet(unit->pClassData->number, CHR_UNIT, 0);
	ApplyUnitSpritePalettes();
	CS->smsPal = GetUnitSpritePalette(unit);
	CS->riderPal = 0xD;

	gPaletteBuffer[PAL_BACKDROP_OFFSET] = 0;
	EnablePaletteSync();

	SetDispEnable(1, 0, 0, 1, 1);
	gLCDControlBuffer.dispcnt.obj1dMap = 1;

	CS->level = Crossing_LevelOfChapter();
	CS->best = CrossingRecords_Best(CS->level);
	CS->score = 0;
	CS->gold = 0;
	CS->lives = 3;
	CS->round = 1;
	CS->docks = 0;
	CS->popWord = 0xFF;
	CS->splash = 0;
	CS->shake = 0;
	CS->shakeX = 0;
	for (i = 0; i < CROSSING_PARTS; i++)
		CS->parts[i].life = 0;
	Crossing_ResetLanes();
	Crossing_Respawn();
	CS->hitStop = 0;
	CS->chain = 0;
	CS->roundDeaths = 0;
	CS->nearCool = 0;
	CS->squash = 0;
	CS->dustT = 0;
	CS->rippleT = 0;
	CS->sinkT = 0;
	CS->numTimer = 0;
	CS->shakeAmp = 2;
	CS->pendPop = 0xFF;
	CS->countDone = 0;
	CS->newBest = 0;
	CS->shownScore = 0;
	for (i = 0; i < CROSSING_PARTS; i++)
		CS->parts[i].kind = 0;
	CS->state = CS_READY;
	CS->line1Sig = 0xFFFFFFFF;
	CS->hintKey = 0xFF;
	Crossing_UpdateText();

	BG_EnableSyncByMask(BG0_SYNC_BIT | BG3_SYNC_BIT);
}

/* ---- The game ---- */

static void Crossing_Lose(int death)
{
	int cx = Crossing_UnitCX(), cy = CS->py + 8;

	CS->death = death;
	CS->state = CS_DYING;
	CS->stateTimer = 50;
	CS->hopFrames = 0;
	CS->squash = 0;
	CS->chain = 0;
	CS->roundDeaths++;
	if (death == CROSSING_DEATH_WATER) {
		/* It sinks, in a spray of droplets */
		CS->splash = 30;
		CS->splashX = Crossing_Clamp(cx - 8, 0, 224);
		CS->splashY = cy - 8;
		CS->sinkT = SINK_FRAMES;
		Crossing_Burst(cx, cy, 1);
		Crossing_Shake(8, 1);
		Crossing_Pop(CROSSING_POP_SPLASH, cx, cy < 72 ? cy + 30 : cy - 24);
		PlaySoundEffect(SE_SPLASH);
	} else {
		/* A trampling: the game stands still a moment, then shakes */
		Crossing_Burst(cx, cy, 0);
		if (death == CROSSING_DEATH_RIDER)
			CS->hitStop = HIT_STOP;
		Crossing_Shake(18, 3);
		Crossing_Pop(CROSSING_POP_OUCH, cx, cy < 72 ? cy + 30 : cy - 24);
		PlaySoundEffect(death == CROSSING_DEATH_RIDER ? SE_HIT : SE_LOST);
	}
}

static void Crossing_GameOver(void)
{
	u32 gold;

	CS->state = CS_OVER;
	CS->stateTimer = 30;
	CS->gold = Div(CS->score, 10);
	gold = GetPartyGoldAmount() + CS->gold;
	SetPartyGoldAmount(gold > 999999 ? 999999 : gold);
	CS->newBest = CS->score > CS->best && CS->score > 0;
	CrossingRecords_Add(CS->level, CS->score);
	Crossing_Pop(CROSSING_POP_GAME_OVER, 120, 72);
	CS->shownScore = 0;
	CS->countDone = 0;
}

/* The final score counts up, ticking; then the gold rings in, and a record pops up NEW BEST! */
static void Crossing_CountUp(void)
{
	int step;

	if (CS->countDone)
		return;
	step = Div(CS->score + 39, 40);
	if ((u32)(CS->shownScore + step) >= CS->score) {
		CS->shownScore = CS->score;
		CS->countDone = 1;
		if (CS->newBest) {
			CS->pendPop = CROSSING_POP_NEW_BEST;
			Crossing_Burst(120, 72, 0);
			PlaySoundEffect(SONG_SE_UPDATE);
		} else {
			PlaySoundEffect(CS->gold > 0 ? SE_GOLD : SE_LOST);
		}
		return;
	}
	CS->shownScore += step;
	if ((GetGameClock() & 3) == 0)
		PlaySoundEffect(SE_TICK);
}

/* Leaving with B pays out what was scored, as a game over would. */
static void Crossing_PayOnLeave(void)
{
	u32 gold;

	if (CS->state == CS_OVER || CS->score == 0)
		return;
	CS->gold = Div(CS->score, 10);
	gold = GetPartyGoldAmount() + CS->gold;
	SetPartyGoldAmount(gold > 999999 ? 999999 : gold);
	CrossingRecords_Add(CS->level, CS->score);
}

/* A hop's target, or FALSE when the way is shut (the edges, and the bank between the docks) */
static bool Crossing_StartHop(int dx, int dy)
{
	int x = CS->px >> 8;
	int i, cx, row = CS->row + dy;

	if (row > CROSSING_ROW_START || row < CROSSING_ROW_BANK)
		return FALSE;
	if (dx != 0 && (x + dx * CELL < 0 || x + dx * CELL > (CROSSING_COLS - 1) * CELL))
		return FALSE;

	if (row == CROSSING_ROW_BANK) {
		/* Only into an empty dock: the nearest dock within 10 pixels, and the hop lines up with it */
		cx = x + 8;
		for (i = 0; i < CROSSING_DOCKS; i++) {
			if (Crossing_Abs(cx - (sDockCols[i] * CELL + 8)) <= 10 && !(CS->docks & (1 << i)))
				break;
		}
		if (i == CROSSING_DOCKS)
			return FALSE;
		CS->px = (sDockCols[i] * CELL) << 8;
	}

	CS->hopDX = dx;
	CS->hopDY = dy;
	CS->hopFrames = HOP_FRAMES;
	CS->facing = dx < 0 ? FACE_LEFT : (dx > 0 ? FACE_RIGHT : (dy > 0 ? FACE_DOWN : FACE_UP));
	PlaySoundEffect(SE_HOP);
	return TRUE;
}

static void Crossing_Dock(void)
{
	int i, pts, cx = Crossing_UnitCX();

	for (i = 0; i < CROSSING_DOCKS; i++)
		if (Crossing_Abs(cx - (sDockCols[i] * CELL + 8)) <= 8)
			break;
	if (i == CROSSING_DOCKS)
		return;
	CS->docks |= 1 << i;
	pts = 50 + Div(CS->timer, 60);
	CS->score += pts;
	CS->chain++;
	Crossing_Note(CS->chain);
	Crossing_Burst(sDockCols[i] * CELL + 8, CROSSING_ROW_BANK * CELL + 8, 0);
	if (CS->docks == (1 << CROSSING_DOCKS) - 1) {
		/* The round is clear; with no life lost in it, PERFECT! and a bonus */
		CS->score += 250;
		if (CS->roundDeaths == 0)
			CS->score += PERFECT_BONUS;
		CS->state = CS_CLEAR;
		CS->stateTimer = 70;
		Crossing_Pop(CS->roundDeaths == 0 ? CROSSING_POP_PERFECT : CROSSING_POP_CLEAR, 120, 64);
		Crossing_PopNumber(10, pts + 250 + (CS->roundDeaths == 0 ? PERFECT_BONUS : 0), 120, 84);
		Crossing_Shake(10, 2);
		PlaySoundEffect(CS->roundDeaths == 0 ? SONG_SE_UPDATE : SE_GOLD);
	} else {
		Crossing_Pop(CROSSING_POP_HOME, cx, 52);
		Crossing_PopNumber(10, pts, cx, 34);
	}
	Crossing_Respawn();
}

static void Crossing_NearMiss(void)
{
	CS->nearCool = 45;
	Crossing_Pop(CROSSING_POP_CLOSE, Crossing_UnitCX(), CS->py - 16);
	Crossing_Shake(6, 1);
	PlaySoundEffect(SE_CLOSE);
}

/* Hopping out of a road lane just ahead of a rider (within 10 pixels of trampling, and closing in): a near miss */
static void Crossing_NearMissLeaving(void)
{
	int lane = Crossing_LaneOfRow(CS->row), k, x, cx = Crossing_UnitCX();
	const struct CrossingLane *l;

	if (lane < 0 || CS->nearCool > 0)
		return;
	l = Crossing_Lane(lane);
	if (l->kind != 0)
		return;
	for (k = 0; k < l->count; k++) {
		x = Crossing_ObjX(lane, k) + 8;
		if (Crossing_Abs(cx - x) < sRiderHalf[l->rider] + 10 && (l->dir > 0 ? x < cx : x > cx)) {
			Crossing_NearMiss();
			return;
		}
	}
}

/* Where the unit stands after (or during) a hop: is it trampled, drowned or carried? */
static void Crossing_CheckCell(void)
{
	int lane, k, x, d, cx = Crossing_UnitCX(), row;
	const struct CrossingLane *l;
	bool safe;

	/* Mid-hop, the unit counts as on the cell it is hopping to once it is halfway */
	if (CS->hopFrames == HOP_FRAMES / 2 && CS->hopDY != 0)
		Crossing_NearMissLeaving();
	row = CS->row + (CS->hopFrames > 0 && CS->hopFrames <= HOP_FRAMES / 2 ? CS->hopDY : 0);
	lane = Crossing_LaneOfRow(row);
	if (lane < 0)
		return;
	l = Crossing_Lane(lane);

	if (l->kind == 0) {
		for (k = 0; k < l->count; k++) {
			x = Crossing_ObjX(lane, k);
			d = Crossing_Abs(cx - (x + 8));
			if (d < sRiderHalf[l->rider]) {
				Crossing_Lose(CROSSING_DEATH_RIDER);
				return;
			}
			/* A rider that has just gone by within a few pixels: a near miss */
			if (d < sRiderHalf[l->rider] + NEAR_MISS && CS->nearCool == 0 &&
			    (l->dir > 0 ? x + 8 > cx : x + 8 < cx))
				Crossing_NearMiss();
		}
		return;
	}

	/* The river: drowned unless on a raft, and only checked between hops (a hop can cross open water) */
	if (CS->hopFrames > 0)
		return;
	safe = FALSE;
	for (k = 0; k < l->count && !safe; k++) {
		x = Crossing_ObjX(lane, k);
		if (cx >= x + 2 && cx < x + l->len * CELL - 2)
			safe = TRUE;
	}
	if (!safe || cx < 2 || cx > 238)
		Crossing_Lose(CROSSING_DEATH_WATER);
}

static void Crossing_UpdateLanes(void)
{
	const struct CrossingLane *l;
	int i, p, v;

	for (i = 0; i < CROSSING_LANES; i++) {
		l = Crossing_Lane(i);
		p = Crossing_Period(l) << 8;
		v = Crossing_Speed(l) * l->dir;
		CS->laneX[i] += v;
		while (CS->laneX[i] >= p)
			CS->laneX[i] -= p;
		while (CS->laneX[i] < 0)
			CS->laneX[i] += p;
		/* A raft carries the unit on it */
		if (l->kind == 1 && l->row == CS->row && CS->state == CS_PLAY)
			CS->px += v;
	}
}

static void Crossing_UpdatePlay(u16 newKeys)
{
	if (CS->hopFrames > 0) {
		CS->hopFrames--;
		CS->px += CS->hopDX * ((CELL << 8) / HOP_FRAMES);
		CS->py += CS->hopDY * (CELL / HOP_FRAMES);
		if (CS->hopFrames == 0) {
			CS->row += CS->hopDY;
			CS->py = CS->row * CELL;
			/* The landing: a squash, and dust on the ground or a ripple on a raft */
			CS->squash = SQUASH_FRAMES;
			if (CS->row >= CROSSING_ROW_RIVER_TOP && CS->row <= CROSSING_ROW_RIVER_BOTTOM) {
				CS->rippleT = 14;
			} else {
				CS->dustT = 12;
				CS->dustX = Crossing_UnitCX();
				CS->dustY = CS->py + 12;
			}
			if (CS->row < CS->maxRow) {
				CS->maxRow = CS->row;
				CS->score += 10;
				Crossing_PopNumber(10, 10, Crossing_UnitCX(), CS->py - 10);
			}
			if (CS->row == CROSSING_ROW_BANK) {
				Crossing_Dock();
				return;
			}
		}
	} else if (newKeys & DPAD_UP) {
		Crossing_StartHop(0, -1);
	} else if (newKeys & DPAD_DOWN) {
		Crossing_StartHop(0, 1);
	} else if (newKeys & DPAD_LEFT) {
		Crossing_StartHop(-1, 0);
	} else if (newKeys & DPAD_RIGHT) {
		Crossing_StartHop(1, 0);
	}

	if (CS->timer > 0)
		CS->timer--;
	if (CS->timer == 0) {
		Crossing_Lose(CROSSING_DEATH_TIME);
		return;
	}
	Crossing_CheckCell();
}

/* Returns TRUE to leave the screen. */
static bool Crossing_Update(void)
{
	u16 newKeys = gKeyStatusPtr->newKeys;

	if ((newKeys & B_BUTTON) && CS->state != CS_OVER) {
		Crossing_PayOnLeave();
		PlaySoundEffect(SE_LEAVE);
		return TRUE;
	}

	/* A trampling freezes everything but the shake for a moment */
	if (CS->hitStop > 0 && CS->state != CS_PAUSE) {
		CS->hitStop--;
		Crossing_UpdateEffects();
		return FALSE;
	}

	switch (CS->state) {
	case CS_READY:
		if (newKeys & START_BUTTON) {
			CS->prevState = CS->state;
			CS->state = CS_PAUSE;
			break;
		}
		if (newKeys & (DPAD_UP | DPAD_DOWN | DPAD_LEFT | DPAD_RIGHT)) {
			CS->state = CS_PLAY;
			Crossing_UpdatePlay(newKeys);
		}
		Crossing_UpdateLanes();
		break;

	case CS_PLAY:
		if (newKeys & START_BUTTON) {
			CS->prevState = CS->state;
			CS->state = CS_PAUSE;
			break;
		}
		Crossing_UpdateLanes();
		Crossing_UpdatePlay(newKeys);
		break;

	case CS_DYING:
		Crossing_UpdateLanes();
		if (--CS->stateTimer > 0)
			break;
		if (--CS->lives == 0) {
			Crossing_GameOver();
			break;
		}
		Crossing_Respawn();
		CS->state = CS_PLAY;
		break;

	case CS_CLEAR:
		Crossing_UpdateLanes();
		if (--CS->stateTimer > 0)
			break;
		CS->docks = 0;
		CS->round++;
		CS->roundDeaths = 0;
		CS->state = CS_PLAY;
		break;

	case CS_PAUSE:
		if (newKeys & START_BUTTON)
			CS->state = CS->prevState;
		return FALSE;

	case CS_OVER:
		Crossing_UpdateLanes();
		Crossing_CountUp();
		if (CS->stateTimer > 0) {
			CS->stateTimer--;
			break;
		}
		if (newKeys & (A_BUTTON | B_BUTTON)) {
			PlaySoundEffect(SE_LEAVE);
			return TRUE;
		}
		break;
	}
	Crossing_UpdateEffects();
	return FALSE;
}

/* ---- Drawing (back to front: a later PutSprite is drawn in front) ---- */

#define SPR(x, y, obj, chr, pal, layer, flags1) \
	PutSpriteExt(4, OAM1_X(((x) - CS->shakeX) & 0x1FF) | (flags1), OAM0_Y((y) & 0xFF), (obj), \
		     OAM2_PAL(pal) + OAM2_LAYER(layer) + OAM2_CHR(chr))

static void Crossing_DrawLanes(void)
{
	const struct CrossingLane *l;
	int i, k, x, y, c, frame;
	int clock = GetGameClock();

	for (i = 0; i < CROSSING_LANES; i++) {
		l = Crossing_Lane(i);
		y = l->row * CELL;
		for (k = 0; k < l->count; k++) {
			x = Crossing_ObjX(i, k);
			if (x + l->len * CELL <= 0 || x >= 240 + 16)
				continue;
			if (l->kind == 1) {
				for (c = 0; c < l->len; c++) {
					if (x + c * CELL <= -16 || x + c * CELL >= 240)
						continue;
					SPR(x + c * CELL, y, gObject_16x16,
					    CROSSING_CHR_RAFT + (c == 0 ? 0 : (c == l->len - 1 ? 2 : 1)) * CROSSING_CHR_RAFT_FRAME,
					    PAL_FX, 3, 0);
				}
			} else {
				/* The side-walking frames face left; a rider going right is flipped */
				frame = (clock >> 3) & 3;
				SPR(x - 8, y - 16, gObject_32x32, CHR_RIDER + l->rider * 64 + frame * 16, CS->riderPal, 2,
				    l->dir > 0 ? OAM1_HFLIP : 0);
			}
		}
	}
}

/* The landing squash, wider and flatter then back (scale offsets, 1/256) */
static const s8 sSquash[SQUASH_FRAMES] = { 0, 6, 14, 30, 44, 36 };

/*
 * The unit: stretched taller while it takes off, squashed as it lands, shrinking down into the water as it sinks. The
 * scaled sprite is affine (one slot), anchored at its feet; otherwise it is drawn plain.
 */
static void Crossing_DrawUnit(void)
{
	static const u8 base[4] = { 0, 0, 4, 8 };
	int frame, x = (CS->px >> 8) - 8, y = CS->py - 16;
	int sx = 256, sy = 256, c, pa;
	bool flip = CS->facing == FACE_RIGHT;

	if (CS->state == CS_DYING) {
		if (CS->death == CROSSING_DEATH_WATER) {
			if (CS->sinkT == 0)
				return;
			sy = Div(256 * CS->sinkT, SINK_FRAMES);
			sx = 256 + (SINK_FRAMES - CS->sinkT) * 2;
		} else if (CS->stateTimer & 4) {
			return;
		}
	}
	frame = base[CS->facing] + (CS->hopFrames > 0 ? ((HOP_FRAMES - CS->hopFrames) >> 1) & 3 : 0);
	if (CS->hopFrames > 0) {
		/* a small arc on each hop, stretched at the take-off */
		y -= (CS->hopFrames > HOP_FRAMES / 2 ? HOP_FRAMES - CS->hopFrames : CS->hopFrames) >> 1;
		if (CS->hopFrames > HOP_FRAMES / 2) {
			sx = 232;
			sy = 284;
		}
	} else if (CS->squash > 0) {
		c = sSquash[CS->squash - 1];
		sx = 256 + c;
		sy = 256 - c;
	}
	if (sx == 256 && sy == 256) {
		SPR(x, y, gObject_32x32, CHR_UNIT + frame * 16, CS->smsPal, 2, flip ? OAM1_HFLIP : 0);
		return;
	}
	if (sy < 8)
		sy = 8;
	pa = Div(0x10000, sx);
	SetObjAffine(AFF_UNIT, flip ? -pa : pa, 0, 0, Div(0x10000, sy));
	/* Double size: the 64x64 box is centred on the frame; the feet stay where they were */
	y += (16 * (256 - sy)) >> 8;
	PutSpriteExt(4, OAM1_X((x - 16 - CS->shakeX) & 0x1FF) | OAM1_AFFINE_ID(AFF_UNIT),
		     OAM0_Y((y - 16) & 0xFF) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE, gObject_32x32,
		     OAM2_PAL(CS->smsPal) + OAM2_LAYER(2) + OAM2_CHR(CHR_UNIT + frame * 16));
}

/* The landing dust (two puffs going out) and the ripple round a raft landing */
static void Crossing_DrawLandFx(void)
{
	int t, f, cx = Crossing_UnitCX();

	if (CS->dustT > 0) {
		t = 12 - CS->dustT;
		f = t < 4 ? 0 : (t < 8 ? 1 : 2);
		SPR(CS->dustX - 10 - (t >> 1), CS->dustY - (t >> 2), gObject_8x8, CROSSING_CHR_DUST + f, PAL_FX, 2, 0);
		SPR(CS->dustX + 2 + (t >> 1), CS->dustY - (t >> 2), gObject_8x8, CROSSING_CHR_DUST + f, PAL_FX, 2, 0);
	}
	if (CS->rippleT > 0 && CS->state == CS_PLAY)
		SPR(cx - 8, CS->py + 10, gObject_16x8, CROSSING_CHR_RIPPLE + (CS->rippleT < 7 ? 2 : 0), PAL_FX, 2, 0);
}

/* The score pop-up: small digits rising and blinking out */
static void Crossing_DrawNumber(void)
{
	int i;

	if (CS->numTimer == 0 || (CS->numTimer < 8 && (CS->numTimer & 2)))
		return;
	for (i = 0; i < CS->numLen; i++)
		PutSpriteExt(4, OAM1_X((CS->numX + i * 5) & 0x1FF), OAM0_Y(CS->numY & 0xFF), gObject_8x8,
			     OAM2_PAL(PAL_FX) + OAM2_LAYER(0) + OAM2_CHR(CROSSING_CHR_DIGIT + CS->num[i]));
}

static void Crossing_DrawDocksAndFx(void)
{
	const struct CrossingParticle *p;
	int i;

	for (i = 0; i < CROSSING_DOCKS; i++)
		if (CS->docks & (1 << i))
			SPR(sDockCols[i] * CELL, CROSSING_ROW_BANK * CELL, gObject_16x16, CROSSING_CHR_FLAG, PAL_FX, 2, 0);
	if (CS->splash > 0)
		SPR(CS->splashX, CS->splashY, gObject_16x16,
		    CROSSING_CHR_SPLASH + ((CS->splash >> 3) & 1) * CROSSING_CHR_SPLASH_FRAME, PAL_FX, 1, 0);
	for (i = 0; i < CROSSING_PARTS; i++) {
		p = &CS->parts[i];
		if (p->life == 0)
			continue;
		SPR((p->x >> 4) - 4, (p->y >> 4) - 4, gObject_8x8,
		    (p->kind ? CROSSING_CHR_DROP : CROSSING_CHR_SPARK) + ((p->life >> 2) & 1), PAL_FX, 1, 0);
	}
}

/* Paints the 48x8 timer meter into its 6 OBJ tiles */
static void Crossing_PaintTimer(void)
{
	u32 tiles[6][8];
	int fill = Div(CS->timer * TIMER_W, LIFE_FRAMES);
	int tx, py, px, x, c, color = CS->timer < 10 * 60 ? 11 : 9;
	u32 row;

	for (tx = 0; tx < 6; tx++) {
		for (py = 0; py < 8; py++) {
			row = 0;
			for (px = 0; px < 8; px++) {
				x = tx * 8 + px;
				if (py == 0 || py == 7 || x == 0 || x == TIMER_W - 1)
					c = 13;
				else if (x < fill)
					c = color;
				else
					c = 12;
				row |= c << (px * 4);
			}
			tiles[tx][py] = row;
		}
	}
	CpuFastCopy(tiles, (void *)(OBJ_VRAM0 + CHR_TIMER * 0x20), sizeof(tiles));
}

static void Crossing_DrawPopup(void)
{
	int t = CS->popTimer;
	int scale, inv, half, cy;

	if (CS->popWord == 0xFF || (t > 52 && (t & 2)))
		return;
	if (t < 6)
		scale = 64 + t * 48;
	else if (t < 14)
		scale = 352 - (t - 6) * 12;
	else
		scale = 256;
	cy = CS->popY - (t > 40 ? (t - 40) >> 1 : 0);
	inv = Div(0x10000, scale);
	half = (32 * scale) >> 8;
	SetObjAffine(AFF_POP, inv, 0, 0, inv);
	PutSpriteExt(4, OAM1_X((CS->popX - half - 64) & 0x1FF) | OAM1_AFFINE_ID(AFF_POP),
		     OAM0_Y((cy - 32) & 0xFF) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE, gObject_64x32,
		     OAM2_PAL(PAL_POP) + OAM2_LAYER(0) + OAM2_CHR(CHR_POPUP));
	PutSpriteExt(4, OAM1_X((CS->popX + half - 64) & 0x1FF) | OAM1_AFFINE_ID(AFF_POP),
		     OAM0_Y((cy - 32) & 0xFF) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE, gObject_64x32,
		     OAM2_PAL(PAL_POP) + OAM2_LAYER(0) + OAM2_CHR(CHR_POPUP + 32));
}

/* The status line's sprites: the lives and the timer */
static void Crossing_DrawStatus(void)
{
	int i, icon;

	for (i = 0; i < CS->lives && i < 5; i++)
		PutSpriteExt(4, OAM1_X(150 + i * 10), OAM0_Y(4), gObject_8x8,
			     OAM2_PAL(PAL_FX) + OAM2_LAYER(0) + OAM2_CHR(CROSSING_CHR_HEART));
	Crossing_PaintTimer();
	PutSpriteExt(4, OAM1_X(TIMER_X), OAM0_Y(4), gObject_32x8, OAM2_PAL(PAL_UI) + OAM2_LAYER(0) + OAM2_CHR(CHR_TIMER));
	PutSpriteExt(4, OAM1_X(TIMER_X + 32), OAM0_Y(4), gObject_16x8,
		     OAM2_PAL(PAL_UI) + OAM2_LAYER(0) + OAM2_CHR(CHR_TIMER + 4));
	for (i = 0; i < CS->hintCount; i++) {
		icon = CS->hintIcon[i];
		PutSpriteExt(4, OAM1_X(CS->hintX[i]), OAM0_Y(HINT_ROW * 8), icon == HINT_START ? gObject_32x16 : gObject_16x16,
			     OAM2_PAL(PAL_UI) + OAM2_LAYER(0) + OAM2_CHR(sHintChr[icon]));
	}
}

/*
 * The scene first, the UI sprites last (so first in OAM): the GBA lets a sprite earlier in OAM hide a later one even
 * when the later one has the higher priority.
 */
static void Crossing_Draw(void)
{
	Crossing_DrawLanes();
	Crossing_DrawLandFx();
	Crossing_DrawUnit();
	Crossing_DrawDocksAndFx();
	Crossing_DrawNumber();
	Crossing_DrawPopup();
	Crossing_DrawStatus();
}

/* ---- The proc ---- */

enum { PL_CROSSING_EXIT = 1 };

static void Crossing_Loop(struct ProcCrossing *proc)
{
	if (Crossing_Update()) {
		Proc_Goto(proc, PL_CROSSING_EXIT);
		return;
	}
	Crossing_UpdateText();
	Crossing_Draw();
}

static void Crossing_OnEnd(struct ProcCrossing *proc)
{
	ClearSprites();
	ClearBg0Bg1();
	BG_SetPosition(3, 0, 0);
	gLCDControlBuffer.dispcnt = proc->dispBak;
	SetDefaultColorEffects();
}

const struct ProcCmd ProcScr_CrossingScreen[] = {
	PROC_NAME("CrossingScreen"),
	PROC_YIELD,
	PROC_SET_END_CB(Crossing_OnEnd),

	PROC_CALL(Crossing_Init),
	PROC_CALL_ARG(NewFadeIn, 0x10),
	PROC_REPEAT(Crossing_Loop),

	PROC_LABEL(PL_CROSSING_EXIT),
	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_END,
};

#endif /* CONFIG_DISTRICT_CROSSING */
