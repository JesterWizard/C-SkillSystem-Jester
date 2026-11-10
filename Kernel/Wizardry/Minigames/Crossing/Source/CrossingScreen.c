#include "common-chax.h"
#include "kernel-lib.h"
#include "mu.h"
#include "crossing.h"
#include "crossing-art.h"
#include "constants/texts.h"
#include "constants/classes.h"
#include "m4a.h"
#ifdef CONFIG_DISTRICT_GUILD
#include "guild.h"
#endif
#include "constants/items.h"
#include "wm-story-chapter.h"

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
 *   RECORDS R in READY: each district's best score, medal and fastest round 1. B, A or R closes it.
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
 * Twists, one per district: Serafew's middle river lane is a line of turtles that dive in turn (under water they hold
 * nothing up); at Port Kiris a gull swoops along a row after a "!" warning; Caer Pelyn's rafts are ice floes, and a
 * sideways hop on one slides a cell further; Jehanna's sandstorms hide a road lane's riders for a while.
 *
 * Pickups and helpers: a coin (+50) or a potion (a life, up to 5) rides a raft for a while; a lost child rides one
 * too, and hopping onto it picks it up: a dock brings it home (+200, RESCUED!), a lost life loses it. Now and then a
 * dock shows a star (double points, BONUS!) or a skull, a trap that costs a life.
 *
 * Replay goals: each district's first clear gives an Elixir and its first round 5 a stat booster (to the unit, or the
 * convoy); the fastest round 1 is saved; each story chapter sets a challenge per district (fill the docks left to
 * right, clear round 1 in 60 seconds, rescue a child, or clear round 1 with no miss), worth 1000 gold once. The best
 * score earns a medal: bronze 500, silver 1200, gold 2500.
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
#define PAL_FX2 6           /* the twists' and pickups' sprites */
#define PAL_ICE 7           /* Caer Pelyn's ice floes */
#define PAL_NPC 0xE         /* the map sprites' green (NPC) palette */
#define CX (&gCrossingExtra)
#define AFF_POP 1
#define AFF_UNIT 2
#define SCENE_CHR_VRAM 0x06008000

#define CHR_UNIT 0x100      /* the unit's walking sheet (12 frames of 16 tiles); also the riders' load buffer */
#define CHR_POPUP 0x220     /* the pop-up word (64 tiles) */
#define CHR_TIMER 0x260     /* the timer meter (6 tiles) */
#define CHR_RIDER 0x280     /* the riders' side frames (3 classes x 64 tiles); 0x380 up is the map's moving units */
#define CHR_CHILD 0x340     /* the lost child's side frames (64 tiles) */

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
#define TURTLE_LANE 4                       /* Serafew: the middle river lane is a line of turtles */
#define GULL_WARN 45                        /* Port Kiris: frames of warning before a gull swoops */
#define STORM_FRAMES 180                    /* Jehanna: a sandstorm's length; riders vanish after the first 30 */
#define CHILD_CLASS CLASS_CIVILIAN_M3
#define COIN_POINTS 50
#define RESCUE_POINTS 200
#define MAX_LIVES 5
#define REWARD_FRAMES 150                   /* a reward's message stays on the status line this long */
#define CHAL_GOLD 1000

enum {
	CS_READY,
	CS_PLAY,
	CS_DYING,
	CS_CLEAR,
	CS_PAUSE,
	CS_OVER,
	CS_RECORDS,
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

/* Serafew's turtles dive in turn: 0 swimming, 1 diving (a warning), 2 under water (they hold nothing up) */
static int Crossing_TurtlePhase(int lane, int k)
{
	int t;

	if (CS->level != 0 || lane != TURTLE_LANE)
		return 0;
	t = (CX->tick + k * 85) & 0xFF;
	return t < 160 ? 0 : (t < 200 ? 1 : 2);
}

/* Port Kiris's gull: does it catch the unit (standing in its row, within 10 pixels)? */
static bool Crossing_GullHit(void)
{
	if (CS->level != 1 || CX->gullRow == 0xFF || CX->gullWarn > 0 || CS->hopFrames > 0)
		return FALSE;
	return CS->row == CX->gullRow && Crossing_Abs(CX->gullX + 8 - Crossing_UnitCX()) < 10;
}

/* The twists' clocks: run every frame from Crossing_UpdateLanes; new gulls and storms only start in play */
static void Crossing_UpdateTwists(void)
{
	int k;

	CX->tick++;

	if (CS->level == 1) {
		if (CX->gullRow == 0xFF) {
			if (CS->state == CS_PLAY && CX->gullNext > 0 && --CX->gullNext == 0) {
				CX->gullRow = CROSSING_ROW_RIVER_TOP + NextRN_N(CROSSING_ROW_ROAD_BOTTOM - CROSSING_ROW_RIVER_TOP + 1);
				CX->gullWarn = GULL_WARN;
				CX->gullX = -16;
				PlaySoundEffect(SE_CLOSE);
			}
		} else if (CX->gullWarn > 0) {
			CX->gullWarn--;
		} else {
			CX->gullX += 3;
			if (CX->gullX > 240) {
				CX->gullRow = 0xFF;
				CX->gullNext = 180 + NextRN_N(240);
			}
		}
	}

	if (CS->level == 3 && CS->state == CS_PLAY) {
		if (CX->stormLane == 0xFF) {
			if (CX->stormNext > 0 && --CX->stormNext == 0) {
				CX->stormLane = NextRN_N(3); /* lanes 0..2 are the road */
				CX->stormT = STORM_FRAMES;
			}
		} else if (--CX->stormT == 0) {
			CX->stormLane = 0xFF;
			CX->stormNext = 360 + NextRN_N(240);
		}
	}

	/* The status line shows each reward's message in turn */
	if (CX->rewardT > 0) {
		CX->rewardT--;
	} else if (CX->rewardQueue) {
		for (k = 1; k < 8 && !(CX->rewardQueue & (1 << k)); k++)
			;
		CX->rewardQueue &= ~(1 << k);
		CX->rewardMsg = k;
		CX->rewardT = REWARD_FRAMES;
	}
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

static const u16 sChalMsgs[CROSSING_CHAL_COUNT] = {
	MSG_CROSSING_CHAL_ORDER, MSG_CROSSING_CHAL_FAST, MSG_CROSSING_CHAL_RESCUE, MSG_CROSSING_CHAL_NODEATH,
};

static const u16 sRewardMsgs[] = {
	0, MSG_CROSSING_REWARD_FIRST, MSG_CROSSING_REWARD_ROUND5, MSG_CROSSING_CHAL_DONE,
};

static void Crossing_Line1(char *buf)
{
	int i = 0;

	buf[0] = 0;
	switch (CS->state) {
	case CS_READY:
		/* the chapter's challenge, until it is met */
		Crossing_AppendMsg(buf, &i, CX->chalDone ? MSG_CROSSING_READY : sChalMsgs[CX->chalKind]);
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
		if (CX->rewardT > 0) {
			Crossing_AppendMsg(buf, &i, sRewardMsgs[CX->rewardMsg]);
			break;
		}
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
	return (CS->state << 28) ^ (CS->round << 22) ^ (CS->score << 1) ^ CS->gold ^ (CS->shownScore << 9) ^ CS->countDone ^
	       (CX->rewardT > 0 ? (CX->rewardMsg + 1) << 18 : 0);
}

/* ---- The buttons (bottom row, no window, as in the prep screens) ---- */

#define HINT_ROW 18

enum { HINT_DPAD, HINT_A, HINT_B, HINT_START, HINT_R };

static const u16 sHintChr[] = {
	CROSSING_CHR_BTN_DPAD, CROSSING_CHR_BTN_A, CROSSING_CHR_BTN_B, CROSSING_CHR_BTN_START, CROSSING_CHR_BTN_R,
};

struct Hint {
	u8 icon;
	u16 msg;
};

static int Crossing_Hints(const struct Hint **out)
{
	static const struct Hint ready[] = {
		{ HINT_DPAD, MSG_CROSSING_K_MOVE }, { HINT_B, MSG_CROSSING_K_EXIT }, { HINT_R, MSG_CROSSING_K_RECORDS },
	};
	static const struct Hint pause[] = { { HINT_START, MSG_CROSSING_K_RESUME }, { HINT_B, MSG_CROSSING_K_EXIT } };
	static const struct Hint over[] = { { HINT_A, MSG_CROSSING_K_DONE } };
	static const struct Hint records[] = { { HINT_B, MSG_CROSSING_K_DONE } };

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
	case CS_RECORDS:
		*out = records;
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

/* ---- The records screen (R in READY): each district's best score, medal and fastest round 1 ---- */

#define REC_Y 3                     /* the window's top row */
#define REC_ROW_Y(n) (REC_Y + 5 + (n) * 2)
#define REC_COL_BEST 80             /* pixels into a row's text (the text starts at column 2) */
#define REC_COL_MEDAL 112
#define REC_COL_TIME 152

static const u16 sDistrictNames[CROSSING_LEVELS] = {
	MSG_FISHING_SHORT_SERAFEW, MSG_FISHING_SHORT_PORT_KIRIS, MSG_FISHING_SHORT_CAER_PELYN, MSG_FISHING_SHORT_JEHANNA,
};

static void Crossing_OpenRecords(void)
{
	struct Text *t;
	char buf[64];
	int n, i, s, m;

	CS->state = CS_RECORDS;
	PlaySoundEffect(SE_DOCK);
	DrawUiFrame2(1, REC_Y, 28, 14, 0);

	t = &CX->texts[0];
	ClearText(t);
	Text_InsertDrawString(t, 0, TEXT_COLOR_SYSTEM_GOLD, GetStringFromIndex(MSG_CROSSING_RECORDS_TITLE));
	PutText(t, TILEMAP_LOCATED(gBG0TilemapBuffer, 2, REC_Y + 1));

	t = &CX->texts[1];
	ClearText(t);
	Text_InsertDrawString(t, REC_COL_BEST, TEXT_COLOR_SYSTEM_GOLD, GetStringFromIndex(MSG_CROSSING_COL_BEST));
	Text_InsertDrawString(t, REC_COL_TIME, TEXT_COLOR_SYSTEM_GOLD, GetStringFromIndex(MSG_CROSSING_COL_FASTEST));
	PutText(t, TILEMAP_LOCATED(gBG0TilemapBuffer, 2, REC_Y + 3));

	for (n = 0; n < CROSSING_LEVELS; n++) {
		t = &CX->texts[2 + n];
		ClearText(t);
		Text_InsertDrawString(t, 0, n == CS->level ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_WHITE,
				      GetStringFromIndex(sDistrictNames[n]));
		i = 0;
		Crossing_AppendNum(buf, &i, CrossingRecords_Best(n));
		Text_InsertDrawString(t, REC_COL_BEST, TEXT_COLOR_SYSTEM_BLUE, buf);
		/* the fastest round 1 as m:ss, or -- */
		s = CrossingRecords_Fastest(n);
		i = 0;
		if (s == 0) {
			Crossing_Append(buf, &i, "--");
		} else {
			m = Div(s, 60);
			Crossing_AppendNum(buf, &i, m);
			Crossing_Append(buf, &i, s - m * 60 < 10 ? ":0" : ":");
			Crossing_AppendNum(buf, &i, s - m * 60);
		}
		Text_InsertDrawString(t, REC_COL_TIME, TEXT_COLOR_SYSTEM_BLUE, buf);
		PutText(t, TILEMAP_LOCATED(gBG0TilemapBuffer, 2, REC_ROW_Y(n)));
	}

	SetDispEnable(1, 1, 0, 1, 1);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static void Crossing_CloseRecords(void)
{
	int x, y;

	BG_Fill(BG_GetMapBuffer(1), 0);
	/* the window's text rows go; the status line (rows 0-1) and the buttons (rows 18-19) stay */
	for (y = REC_Y; y < REC_Y + 14; y++)
		for (x = 0; x < 30; x++)
			gBG0TilemapBuffer[y * 32 + x] = 0;
	CS->state = CS_READY;
	CS->line1Sig = 0xFFFFFFFF;
	CS->hintKey = 0xFF;
	SetDispEnable(1, 0, 0, 1, 1);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

/* The records window's medals */
static void Crossing_DrawRecords(void)
{
	int n, m;

	if (CS->state != CS_RECORDS)
		return;
	for (n = 0; n < CROSSING_LEVELS; n++) {
		m = CrossingRecords_Medal(n);
		if (m > 0)
			PutSpriteExt(4, OAM1_X(16 + REC_COL_MEDAL), OAM0_Y(REC_ROW_Y(n) * 8), gObject_16x16,
				     OAM2_PAL(PAL_FX2) + OAM2_LAYER(0) +
				     OAM2_CHR(CROSSING_CHR_MEDAL + (m - 1) * CROSSING_CHR_MEDAL_FRAME));
	}
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
	0x0000, 0x6800, 0,  /* BG1: the records window */
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
	CX->slide = 0;
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
	SetDefaultColorEffects(); /* no blending left over from the map: the records window is opaque */
	SetDispEnable(0, 0, 0, 0, 0);
	BG_Fill(BG_GetMapBuffer(0), 0);
	BG_Fill(BG_GetMapBuffer(1), 0);
	BG_Fill(BG_GetMapBuffer(2), 0);
	BG_Fill(map, 0);
	gLCDControlBuffer.bg0cnt.priority = 0;
	gLCDControlBuffer.bg3cnt.priority = 3;
	BG_SetPosition(0, 0, 0);
	BG_SetPosition(3, 0, 0);
	BG_SetPosition(1, 0, 0);
	gLCDControlBuffer.bg1cnt.priority = 1;

	ResetText();
	SetTextFont(0);
	InitSystemTextFont();
	LoadUiFrameGraphics();
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
	ApplyPalette(gCrossingObjPalFx2, 0x10 + PAL_FX2);
	ApplyPalette(gCrossingObjPalIce, 0x10 + PAL_ICE);

	/* The riders' side frames (0..3) first, through the unit's slot; then the unit's own sheet stays there */
	for (i = 0; i < RIDERS; i++)
		Crossing_LoadSheet(sRiderClasses[i], CHR_RIDER + i * 64, 64);
	Crossing_LoadSheet(CHILD_CLASS, CHR_CHILD, 64);
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
	CpuFill16(0, CX, sizeof(*CX));
	CX->gullRow = 0xFF;
	CX->gullNext = 240;
	CX->stormLane = 0xFF;
	CX->stormNext = 360;
	CX->bonusDock = 0xFF;
	CX->pickNext = 300;
	CX->childNext = 600;
	CX->bonusNext = 480;
	CX->chalKind = (gWmStoryChapter + CS->level) & 3;
	CX->chalDone = CrossingRecords_ChalDone(CS->level, gWmStoryChapter);
	CX->medalBefore = CrossingRecords_Medal(CS->level);
	for (i = 0; i < 6; i++)
		InitText(&CX->texts[i], 26);
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

/* ---- Pickups and helpers ---- */

static int Crossing_CellX(int lane, int k, int c)
{
	return Crossing_ObjX(lane, k) + c * CELL;
}

/* A random raft cell for a pickup or the child (lanes 3..5 are the river; never Serafew's turtles) */
static void Crossing_RandomCell(u8 *lane, u8 *obj, u8 *cell)
{
	const struct CrossingLane *l;
	int i = 3 + NextRN_N(3);

	if (CS->level == 0 && i == TURTLE_LANE)
		i = 5;
	l = Crossing_Lane(i);
	*lane = i;
	*obj = NextRN_N(l->count);
	*cell = NextRN_N(l->len);
}

/* Is the unit standing (not hopping) on cell c of raft k in that lane? */
static bool Crossing_OnCell(int lane, int k, int c)
{
	int x, cx = Crossing_UnitCX();

	if (CS->hopFrames > 0 || CS->row != Crossing_Lane(lane)->row)
		return FALSE;
	x = Crossing_CellX(lane, k, c);
	return cx >= x && cx < x + CELL;
}

/* Spawns, collects and times out the pickups, the lost child and the star or skull dock (in play only) */
static void Crossing_UpdateHelpers(void)
{
	int d, cx = Crossing_UnitCX();

	/* A coin or a potion rides a raft for about 8 seconds */
	if (CX->pickKind == CROSSING_PICK_NONE) {
		if (CX->pickNext > 0 && --CX->pickNext == 0) {
			Crossing_RandomCell(&CX->pickLane, &CX->pickObj, &CX->pickCell);
			CX->pickKind = CS->lives < MAX_LIVES && NextRN_N(4) == 0 ? CROSSING_PICK_POTION : CROSSING_PICK_COIN;
			CX->pickT = 0xFF;
		}
	} else if (Crossing_OnCell(CX->pickLane, CX->pickObj, CX->pickCell)) {
		if (CX->pickKind == CROSSING_PICK_COIN) {
			CS->score += COIN_POINTS;
			Crossing_PopNumber(10, COIN_POINTS, cx, CS->py - 10);
			PlaySoundEffect(SE_GOLD);
		} else {
			if (CS->lives < MAX_LIVES)
				CS->lives++;
			Crossing_Pop(CROSSING_POP_1UP, cx, CS->py - 16);
			PlaySoundEffect(SONG_SE_UPDATE);
		}
		Crossing_Burst(cx, CS->py + 8, 0);
		CX->pickKind = CROSSING_PICK_NONE;
		CX->pickNext = 240 + NextRN_N(240);
	} else if ((CX->tick & 1) && --CX->pickT == 0) {
		CX->pickKind = CROSSING_PICK_NONE;
		CX->pickNext = 240 + NextRN_N(240);
	}

	/* A lost child rides a raft until the unit hops onto it */
	if (CX->childState == CROSSING_CHILD_NONE) {
		if (CX->childNext > 0 && --CX->childNext == 0) {
			Crossing_RandomCell(&CX->childLane, &CX->childObj, &CX->childCell);
			CX->childState = CROSSING_CHILD_RAFT;
		}
	} else if (CX->childState == CROSSING_CHILD_RAFT && Crossing_OnCell(CX->childLane, CX->childObj, CX->childCell)) {
		CX->childState = CROSSING_CHILD_CARRIED;
		PlaySoundEffect(SE_DOCK);
	}

	/* Now and then an empty dock shows a star (double points) or a skull (a trap) */
	if (CX->bonusDock == 0xFF) {
		if (CX->bonusNext > 0 && --CX->bonusNext == 0) {
			d = NextRN_N(CROSSING_DOCKS);
			if (CS->docks & (1 << d)) {
				CX->bonusNext = 60;
			} else {
				CX->bonusDock = d;
				CX->bonusKind = NextRN_N(3) == 0;
				CX->bonusT = 360;
			}
		}
	} else if (--CX->bonusT == 0) {
		CX->bonusDock = 0xFF;
		CX->bonusNext = 420 + NextRN_N(300);
	}
}

/* ---- Replay goals: rewards and the challenge ---- */

static const u8 sBoosters[] = {
	ITEM_BOOSTER_HP, ITEM_BOOSTER_POW, ITEM_BOOSTER_SKL, ITEM_BOOSTER_SPD,
	ITEM_BOOSTER_LCK, ITEM_BOOSTER_DEF, ITEM_BOOSTER_RES,
};

/* An item to the unit, or else to the convoy */
static void Crossing_GiveItem(int item)
{
	struct Unit *unit = gActiveUnit ? gActiveUnit : GetUnit(1);

	if (!UnitAddItem(unit, MakeNewItem(item)))
		AddItemToConvoy(MakeNewItem(item));
}

/* The challenge is met: 1000 gold, once per story chapter */
static void Crossing_ChalMet(void)
{
	u32 gold;

	if (CX->chalDone)
		return;
	CX->chalDone = 1;
	CrossingRecords_SetChalDone(CS->level, gWmStoryChapter);
	gold = GetPartyGoldAmount() + CHAL_GOLD;
	SetPartyGoldAmount(gold > 999999 ? 999999 : gold);
	CS->pendPop = CROSSING_POP_DONE;
	CX->rewardQueue |= 1 << CROSSING_REWARD_CHAL;
	PlaySoundEffect(SE_GOLD);
}

/* Round 1 is clear: the fastest time, the district's first-clear Elixir, and the round 1 challenges */
static void Crossing_RoundOneClear(void)
{
	int secs = Div(CX->roundFrames + 59, 60);

	CrossingRecords_SetFastest(CS->level, secs);
	if (!CrossingRecords_Milestone(CS->level, 0)) {
		CrossingRecords_SetMilestone(CS->level, 0);
		Crossing_GiveItem(ITEM_ELIXIR);
		CX->rewardQueue |= 1 << CROSSING_REWARD_FIRST;
	}
	if ((CX->chalKind == CROSSING_CHAL_ORDER && !CX->chalFailed) ||
	    (CX->chalKind == CROSSING_CHAL_FAST && secs <= CROSSING_CHAL_FAST_SECS) ||
	    (CX->chalKind == CROSSING_CHAL_NODEATH && CS->roundDeaths == 0))
		Crossing_ChalMet();
}

/* Round 5, for the first time in this district: a stat booster */
static void Crossing_RoundFive(void)
{
	if (CrossingRecords_Milestone(CS->level, 1))
		return;
	CrossingRecords_SetMilestone(CS->level, 1);
	Crossing_GiveItem(sBoosters[NextRN_N(sizeof(sBoosters))]);
	CX->rewardQueue |= 1 << CROSSING_REWARD_ROUND5;
	PlaySoundEffect(SONG_SE_UPDATE);
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
	if (CX->childState == CROSSING_CHILD_CARRIED) {
		/* the lost child is lost again; another turns up later */
		CX->childState = CROSSING_CHILD_NONE;
		CX->childNext = 600;
	}
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
#ifdef CONFIG_DISTRICT_GUILD
	Guild_AddTokens(Div(CS->score, 100));	/* the guild board's tokens: 1 per 100 points */
#endif
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
			/* a new medal, or just a new best */
			CS->pendPop = CrossingRecords_Medal(CS->level) > CX->medalBefore ? CROSSING_POP_MEDAL :
				      CROSSING_POP_NEW_BEST;
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
#ifdef CONFIG_DISTRICT_GUILD
	Guild_AddTokens(Div(CS->score, 100));
#endif
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
	int i, pts, word, cx = Crossing_UnitCX();

	for (i = 0; i < CROSSING_DOCKS; i++)
		if (Crossing_Abs(cx - (sDockCols[i] * CELL + 8)) <= 8)
			break;
	if (i == CROSSING_DOCKS)
		return;
	pts = 50 + Div(CS->timer, 60);
	word = 0xFF;
	if (i == CX->bonusDock) {
		CX->bonusDock = 0xFF;
		CX->bonusNext = 420 + NextRN_N(300);
		if (CX->bonusKind) {
			/* The skull dock is a trap */
			Crossing_Lose(CROSSING_DEATH_RIDER);
			return;
		}
		pts *= 2;
		word = CROSSING_POP_BONUS;
	}
	if (CX->childState == CROSSING_CHILD_CARRIED) {
		CX->childState = CROSSING_CHILD_NONE;
		CX->childNext = 900;
		pts += RESCUE_POINTS;
		word = CROSSING_POP_RESCUED;
		if (CX->chalKind == CROSSING_CHAL_RESCUE)
			Crossing_ChalMet();
	}
	/* The ORDER challenge: round 1's docks filled left to right */
	if (CS->round == 1) {
		if (i != CX->nextDock)
			CX->chalFailed = 1;
		CX->nextDock++;
	}
	CS->docks |= 1 << i;
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
		if (word != 0xFF)
			CS->pendPop = word;
		if (CS->round == 1)
			Crossing_RoundOneClear();
		Crossing_PopNumber(10, pts + 250 + (CS->roundDeaths == 0 ? PERFECT_BONUS : 0), 120, 84);
		Crossing_Shake(10, 2);
		PlaySoundEffect(CS->roundDeaths == 0 ? SONG_SE_UPDATE : SE_GOLD);
	} else {
		Crossing_Pop(word != 0xFF ? word : CROSSING_POP_HOME, cx, 52);
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
	if (Crossing_GullHit()) {
		Crossing_Lose(CROSSING_DEATH_RIDER);
		return;
	}
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
		if (cx >= x + 2 && cx < x + l->len * CELL - 2 && Crossing_TurtlePhase(lane, k) != 2)
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
	Crossing_UpdateTwists();
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
			/* Caer Pelyn's ice: a sideways hop onto a floe slides one cell further */
			if (CS->level == 2 && CS->hopDX != 0 && !CX->slide && CS->row >= CROSSING_ROW_RIVER_TOP &&
			    CS->row <= CROSSING_ROW_RIVER_BOTTOM) {
				CX->slide = 1;
				CS->squash = 0;
				Crossing_StartHop(CS->hopDX, 0);
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
	} else if (newKeys & (DPAD_UP | DPAD_DOWN | DPAD_LEFT | DPAD_RIGHT)) {
		CX->slide = 0;
		if (newKeys & DPAD_UP)
			Crossing_StartHop(0, -1);
		else if (newKeys & DPAD_DOWN)
			Crossing_StartHop(0, 1);
		else if (newKeys & DPAD_LEFT)
			Crossing_StartHop(-1, 0);
		else
			Crossing_StartHop(1, 0);
	}

	if (CS->round == 1 && CX->roundFrames < 0xFFFF)
		CX->roundFrames++;
	if (CS->timer > 0)
		CS->timer--;
	if (CS->timer == 0) {
		Crossing_Lose(CROSSING_DEATH_TIME);
		return;
	}
	Crossing_UpdateHelpers();
	Crossing_CheckCell();
}

/* Returns TRUE to leave the screen. */
static bool Crossing_Update(void)
{
	u16 newKeys = gKeyStatusPtr->newKeys;

	if ((newKeys & B_BUTTON) && CS->state != CS_OVER && CS->state != CS_RECORDS) {
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
		if (newKeys & R_BUTTON) {
			Crossing_OpenRecords();
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
		if (CS->round == 5)
			Crossing_RoundFive();
		CS->roundDeaths = 0;
		CS->state = CS_PLAY;
		break;

	case CS_PAUSE:
		if (newKeys & START_BUTTON)
			CS->state = CS->prevState;
		return FALSE;

	case CS_RECORDS:
		if (newKeys & (A_BUTTON | B_BUTTON | R_BUTTON)) {
			Crossing_CloseRecords();
			PlaySoundEffect(SE_LEAVE);
		}
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
	int phase;
	int clock = GetGameClock();

	for (i = 0; i < CROSSING_LANES; i++) {
		l = Crossing_Lane(i);
		y = l->row * CELL;
		for (k = 0; k < l->count; k++) {
			x = Crossing_ObjX(i, k);
			if (x + l->len * CELL <= 0 || x >= 240 + 16)
				continue;
			if (l->kind == 1) {
				phase = Crossing_TurtlePhase(i, k);
				for (c = 0; c < l->len; c++) {
					if (x + c * CELL <= -16 || x + c * CELL >= 240)
						continue;
					if (CS->level == 0 && i == TURTLE_LANE) {
						/* Turtles face right; under water they are not drawn */
						if (phase == 2)
							continue;
						SPR(x + c * CELL, y, gObject_16x16,
								CROSSING_CHR_TURTLE + (phase == 1 ? 2 : ((clock >> 4) + c) & 1) * CROSSING_CHR_TURTLE_FRAME,
								PAL_FX2, 3, l->dir < 0 ? OAM1_HFLIP : 0);
						continue;
					}
					SPR(x + c * CELL, y, gObject_16x16,
								CROSSING_CHR_RAFT + (c == 0 ? 0 : (c == l->len - 1 ? 2 : 1)) * CROSSING_CHR_RAFT_FRAME,
								CS->level == 2 ? PAL_ICE : PAL_FX, 3, 0);
				}
			} else {
				/* Jehanna's sandstorm hides the lane's riders once it has built up */
				if (i == CX->stormLane && CX->stormT <= STORM_FRAMES - 30)
					continue;
				/* The side-walking frames face left; a rider going right is flipped */
				frame = (clock >> 3) & 3;
				SPR(x - 8, y - 16, gObject_32x32, CHR_RIDER + l->rider * 64 + frame * 16, CS->riderPal, 2,
				    l->dir > 0 ? OAM1_HFLIP : 0);
			}
		}
	}
}

/* The gull and its warning, and the sandstorm's drifting sand */
static void Crossing_DrawTwists(void)
{
	int c, x, y;

	if (CX->gullRow != 0xFF) {
		y = CX->gullRow * CELL;
		if (CX->gullWarn > 0) {
			if (CX->gullWarn & 4)
				SPR(4, y, gObject_8x16, CROSSING_CHR_WARN, PAL_FX2, 0, 0);
		} else {
			SPR(CX->gullX, y - 2, gObject_16x16,
				CROSSING_CHR_GULL + ((CX->tick >> 3) & 1) * CROSSING_CHR_GULL_FRAME, PAL_FX2, 0, 0);
		}
	}
	if (CX->stormLane != 0xFF) {
		y = Crossing_Lane(CX->stormLane)->row * CELL;
		for (c = 0; c < 16; c++) {
			/* thin while it builds up and dies down */
			if ((c & 1) && (CX->stormT > STORM_FRAMES - 30 || CX->stormT < 30))
				continue;
			x = c * CELL - ((CX->tick << 1) & 15);
			SPR(x, y, gObject_16x16, CROSSING_CHR_SAND + ((c + (CX->tick >> 3)) & 1) * CROSSING_CHR_SAND_FRAME,
				PAL_FX2, 1, 0);
		}
	}
}

/* The coin or potion, the lost child (on its raft, or on the unit's back) and the star or skull dock */
static void Crossing_DrawHelpers(void)
{
	const struct CrossingLane *l;
	int x, y, chr;

	if (CX->pickKind != CROSSING_PICK_NONE && !(CX->pickT < 40 && (CX->tick & 4))) {
		x = Crossing_CellX(CX->pickLane, CX->pickObj, CX->pickCell);
		y = Crossing_Lane(CX->pickLane)->row * CELL - 2 - ((CX->tick >> 4) & 1);
		chr = CX->pickKind == CROSSING_PICK_COIN ?
		      CROSSING_CHR_COIN + ((CX->tick >> 3) & 1) * CROSSING_CHR_COIN_FRAME : CROSSING_CHR_POTION;
		if (x > -16 && x < 240)
			SPR(x, y, gObject_16x16, chr, PAL_FX2, 2, 0);
	}
	if (CX->childState == CROSSING_CHILD_RAFT) {
		/* side frames face left: flip on a raft going right */
		l = Crossing_Lane(CX->childLane);
		x = Crossing_CellX(CX->childLane, CX->childObj, CX->childCell);
		if (x > -16 && x < 240)
			SPR(x - 8, l->row * CELL - 16, gObject_32x32, CHR_CHILD + ((CX->tick >> 4) & 1) * 16, PAL_NPC, 2,
			    l->dir > 0 ? OAM1_HFLIP : 0);
	} else if (CX->childState == CROSSING_CHILD_CARRIED && CS->state == CS_PLAY) {
		/* piggyback, a little behind and above the unit */
		x = (CS->px >> 8) - 8 + (CS->facing == FACE_RIGHT ? -4 : 4);
		SPR(x, CS->py - 24, gObject_32x32, CHR_CHILD, PAL_NPC, 2, CS->facing == FACE_RIGHT ? OAM1_HFLIP : 0);
	}
	if (CX->bonusDock != 0xFF && !(CX->bonusT < 60 && (CX->tick & 4))) {
		chr = CX->bonusKind ? CROSSING_CHR_SKULL :
		      CROSSING_CHR_STAR + ((CX->tick >> 3) & 1) * CROSSING_CHR_STAR_FRAME;
		SPR(sDockCols[CX->bonusDock] * CELL, CROSSING_ROW_BANK * CELL, gObject_16x16, chr, PAL_FX2, 2, 0);
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
	int i, icon, m;

	for (i = 0; i < CS->lives && i < 5; i++)
		PutSpriteExt(4, OAM1_X(150 + i * 10), OAM0_Y(4), gObject_8x8,
			     OAM2_PAL(PAL_FX) + OAM2_LAYER(0) + OAM2_CHR(CROSSING_CHR_HEART));
	/* The district's medal, once the final score has counted up */
	m = CrossingRecords_Medal(CS->level);
	if (CS->state == CS_OVER && CS->countDone && m > 0)
		PutSpriteExt(4, OAM1_X(150), OAM0_Y(0), gObject_16x16,
			     OAM2_PAL(PAL_FX2) + OAM2_LAYER(0) + OAM2_CHR(CROSSING_CHR_MEDAL + (m - 1) * CROSSING_CHR_MEDAL_FRAME));
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
	Crossing_DrawRecords();
	/* Under the records window the rafts and riders would show through the medals (OBJ priority is per OAM order) */
	if (CS->state != CS_RECORDS) {
		Crossing_DrawLanes();
		Crossing_DrawHelpers();
	}
	Crossing_DrawLandFx();
	Crossing_DrawUnit();
	Crossing_DrawTwists();
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
