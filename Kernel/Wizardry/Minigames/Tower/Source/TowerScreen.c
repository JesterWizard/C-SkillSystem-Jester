#include "common-chax.h"
#include "kernel-lib.h"
#include "tower.h"
#include "tower-art.h"
#include "constants/texts.h"
#include "m4a.h"

#ifdef CONFIG_DISTRICT_TOWER

/*
 * Tower Stack: build a tower one block at a time.
 *
 *   READY   one block stands on the plinth. A starts. B leaves, START pauses.
 *   SLIDE   a block slides in from the left or the right, above the tower. A drops it. If it slides off the far side,
 *           it is missed.
 *   DROP    the block falls onto the tower. It lands if at least half of it (24 of its 48 pixels) is over the block
 *           below: +1. Within the slide speed of the block below (1 to 4 pixels: the three frames around the exact one)
 *           it is a perfect landing: it snaps into line, +2, and the chain of perfect landings grows (its pop-up turns
 *           from green to orange to gold). Less than half: it falls off.
 *   FALL    a missed block falls away. It costs a life, unless a fever is on.
 *   (fever) a chain of 10 perfect landings starts a 10 second fever: missed blocks cost no lives. The chain starts
 *           again from 0.
 *   OVER    no lives left: the score is paid out (score / 10 gold) and the best score is saved. A or B leaves.
 *
 * The blocks slide faster as the tower grows (1 to 4 pixels a frame). A block's path always passes exactly over the
 * block below, so a perfect landing is always possible. The screen climbs with the tower; past the top of the sky the
 * background stays put and only the tower moves. B leaves at any time and pays out what was scored.
 *
 * Layers: BG3 the scene (palette TOWER_BG_PAL, tiles at 0x06008000), BG0 the status line and the button labels, with no
 * window behind them, OBJ (1D) the sprites. Avoid `%` and unsigned `/` (CLAUDE.md).
 */

#define TS (&gTowerState)

#define TOWER_BG_PAL 8
#define PAL_BLOCK 2
#define PAL_FX 3
#define PAL_UI 4
#define PAL_POP 5
#define AFF_POP 1
#define AFF_SQUASH 2
#define SCENE_CHR_VRAM 0x06008000

#define CHR_POPUP 0x100     /* the pop-up word (64 tiles) */
#define CHR_TIMER 0x140     /* the fever meter (6 tiles) */

#define BW TOWER_BLOCK_W
#define BH TOWER_BLOCK_H
#define BASE_X 96                       /* the base block's left edge: centred on the plinth */
#define HOVER_GAP 40                    /* a sliding block's top is this far above the tower's top */
#define GROUND_SCREEN 136               /* where the ground sits on screen before the tower grows */
#define HOVER_SCREEN_MIN 36             /* the screen climbs to keep the sliding block below this row */
#define FEVER_FRAMES (10 * 60)
#define CHAIN_MAX 10
#define TIMER_X 186
#define TIMER_W 48

#define SE_START SONG_SE_SYS_WINDOW_SELECT1
#define SE_LAND SONG_SE_SYS_CURSOR_UD1
#define SE_MISS SONG_6C
#define SE_LEAVE SONG_SE_SYS_WINDOW_CANSEL1
#define SE_GOLD SONG_SE_MONEY
#define SE_NOTE SONG_SE_SYS_WINDOW_SELECT1   /* the chain's note, raised a scale step per perfect landing */
#define HIT_STOP 4                      /* frames frozen on a perfect landing (twice that on a fever) */
#define SQUASH_FRAMES 10
#define FLASH_FRAMES 6
#define PIP_X 8
#define PIP_Y 17


enum {
	TS_READY,
	TS_SLIDE,
	TS_DROP,
	TS_FALL,
	TS_PAUSE,
	TS_OVER,
};

struct ProcTower {
	PROC_HEADER;
	struct DispCnt dispBak;
};

/* ---- Helpers ---- */

static int Tower_Abs(int v)
{
	return v < 0 ? -v : v;
}

static int Tower_Clamp(int v, int lo, int hi)
{
	return v < lo ? lo : (v > hi ? hi : v);
}

/* The left edge of block n, and the scene row of its top */
static int Tower_BlockX(int n)
{
	return TS->xs[n & (TOWER_RING - 1)];
}

static int Tower_BlockTop(int n)
{
	return TOWER_GROUND_Y - BH * (n + 1);
}

static int Tower_TopX(void)
{
	return Tower_BlockX(TS->height - 1);
}

static int Tower_TopRow(void)
{
	return Tower_BlockTop(TS->height - 1);
}

/* The slide speed in whole pixels a frame: 1, then one more each 6 blocks, up to 4 */
static int Tower_Speed(void)
{
	int s = 1, h = TS->height - 1;

	while (h >= 6 && s < 4) {
		h -= 6;
		s++;
	}
	return s;
}

/* ---- Text ---- */

static void Tower_Append(char *buf, int *i, const char *s)
{
	while (*s != 0 && *i < 62)
		buf[(*i)++] = *s++;
	buf[*i] = 0;
}

static void Tower_AppendMsg(char *buf, int *i, int msg)
{
	Tower_Append(buf, i, GetStringFromIndex(msg));
}

static void Tower_AppendNum(char *buf, int *i, int n)
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

static void Tower_Line1(char *buf)
{
	int i = 0;

	buf[0] = 0;
	switch (TS->state) {
	case TS_READY:
		Tower_AppendMsg(buf, &i, MSG_TOWER_READY);
		break;
	case TS_PAUSE:
		Tower_AppendMsg(buf, &i, MSG_TOWER_PAUSED);
		Tower_Append(buf, &i, "   ");
		Tower_AppendMsg(buf, &i, MSG_TOWER_S_BEST);
		Tower_Append(buf, &i, " ");
		Tower_AppendNum(buf, &i, TS->best);
		break;
	case TS_OVER:
		Tower_AppendMsg(buf, &i, MSG_TOWER_S_SCORE);
		Tower_Append(buf, &i, " ");
		Tower_AppendNum(buf, &i, TS->score);
		Tower_Append(buf, &i, "   +");
		Tower_AppendNum(buf, &i, TS->gold);
		Tower_Append(buf, &i, "G");
		break;
	default:
		Tower_AppendMsg(buf, &i, MSG_TOWER_S_SCORE);
		Tower_Append(buf, &i, " ");
		Tower_AppendNum(buf, &i, TS->score);
		Tower_Append(buf, &i, "  ");
		Tower_AppendMsg(buf, &i, MSG_TOWER_S_HEIGHT);
		Tower_Append(buf, &i, " ");
		Tower_AppendNum(buf, &i, TS->height);
		Tower_Append(buf, &i, "  ");
		Tower_AppendMsg(buf, &i, MSG_TOWER_S_BEST);
		Tower_Append(buf, &i, " ");
		Tower_AppendNum(buf, &i, TS->best > TS->score ? TS->best : TS->score);
		break;
	}
}

static u32 Tower_Line1Sig(void)
{
	return (TS->state << 28) ^ (TS->height << 16) ^ (TS->score << 1) ^ TS->gold;
}

/* ---- The buttons (bottom row, no window, as in the prep screens) ---- */

#define HINT_ROW 18

enum { HINT_A, HINT_B, HINT_START };

static const u16 sHintChr[] = { TOWER_CHR_BTN_A, TOWER_CHR_BTN_B, TOWER_CHR_BTN_START };

struct Hint {
	u8 icon;
	u16 msg;
};

static int Tower_Hints(const struct Hint **out)
{
	static const struct Hint ready[] = {
		{ HINT_A, MSG_TOWER_K_START }, { HINT_B, MSG_TOWER_K_EXIT }, { HINT_START, MSG_TOWER_K_PAUSE },
	};
	static const struct Hint pause[] = { { HINT_START, MSG_TOWER_K_RESUME }, { HINT_B, MSG_TOWER_K_EXIT } };
	static const struct Hint over[] = { { HINT_A, MSG_TOWER_K_DONE } };

	switch (TS->state) {
	case TS_READY:
		*out = ready;
		return 3;
	case TS_PAUSE:
		*out = pause;
		return 2;
	case TS_OVER:
		*out = over;
		return 1;
	default:
		*out = NULL;
		return 0;
	}
}

static void Tower_DrawHintText(void)
{
	struct Text *text = &TS->texts[1];
	const struct Hint *hints;
	const char *label;
	int n = Tower_Hints(&hints);
	int i, x = 0, w;

	ClearText(text);
	for (i = 0; i < n && i < 4; i++) {
		w = hints[i].icon == HINT_START ? 32 : 16;
		TS->hintIcon[i] = hints[i].icon;
		TS->hintX[i] = 8 + x;
		label = GetStringFromIndex(hints[i].msg);
		Text_InsertDrawString(text, x + w + 2, TEXT_COLOR_SYSTEM_WHITE, label);
		x += w + 2 + GetStringTextLen(GetStringFromIndex(hints[i].msg)) + 8;
	}
	TS->hintCount = i;
	PutText(text, TILEMAP_LOCATED(gBG0TilemapBuffer, 1, HINT_ROW));
	TS->hintKey = TS->state;
}

static void Tower_UpdateText(void)
{
	char buf[64];
	u32 sig = Tower_Line1Sig();
	bool sync = FALSE;

	if (sig != TS->line1Sig) {
		Tower_Line1(buf);
		ClearText(&TS->texts[0]);
		Text_SetParams(&TS->texts[0], 0, TEXT_COLOR_SYSTEM_WHITE);
		Text_DrawString(&TS->texts[0], buf);
		PutText(&TS->texts[0], TILEMAP_LOCATED(gBG0TilemapBuffer, 1, 0));
		TS->line1Sig = sig;
		sync = TRUE;
	}
	if (TS->hintKey != TS->state) {
		Tower_DrawHintText();
		sync = TRUE;
	}
	if (sync)
		BG_EnableSyncByMask(BG0_SYNC_BIT);
}

/* ---- Pop-ups, sparks and the shake ---- */

static void Tower_LoadPopPal(int pal)
{
	ApplyPalette(gTowerPopPals + pal * 16, 0x10 + PAL_POP);
	EnablePaletteSync();
}

/* A word zooms in at (x, y) in the given pop-up palette */
static void Tower_PopWord(int word, int pal, int x, int y)
{
	CpuFastCopy(gTowerPopTiles + word * TOWER_POP_TILES * 0x20, (void *)(OBJ_VRAM0 + CHR_POPUP * 0x20),
		    TOWER_POP_TILES * 0x20);
	Tower_LoadPopPal(pal);
	TS->popWord = word;
	TS->popTimer = 0;
	TS->popX = Tower_Clamp(x, 64, 176);
	TS->popY = Tower_Clamp(y, 40, 120);
}

/*
 * A row of big glyphs ("+1", "x7", "20") centred on x, rising if `rise`; it shares the pop-up palette.
 * prefix: 10 "+", 11 "x", or -1 for none.
 */
static void Tower_PopNumber(int prefix, int n, int x, int y, bool rise)
{
	int len = 0, h = Div(n, 100), t = Div(n - h * 100, 10);

	if (prefix >= 0)
		TS->num[len++] = prefix;
	if (h > 0)
		TS->num[len++] = h;
	if (h > 0 || t > 0)
		TS->num[len++] = t;
	TS->num[len++] = n - h * 100 - t * 10;
	TS->numLen = len;
	TS->numTimer = 50;
	TS->numRise = rise;
	TS->numX = Tower_Clamp(x - len * 6, 0, 240 - len * 12);
	TS->numY = Tower_Clamp(y, 20, 140);
}

static void Tower_Burst(int x, int y, int n)
{
	struct TowerParticle *p;
	int i, k = 0, a, speed;

	for (i = 0; i < TOWER_PARTS && k < n; i++) {
		p = &TS->parts[i];
		if (p->life != 0)
			continue;
		a = ((k << 8) / n + NextRN_N(16)) & 0xFF;
		speed = 18 + NextRN_N(26);
		p->x = x << 4;
		p->y = y << 4;
		p->vx = (COS(a) * speed) >> 12;
		p->vy = ((SIN(a) * speed) >> 12) - 10;
		p->life = 20 + NextRN_N(16);
		k++;
	}
}

static void Tower_UpdateEffects(void)
{
	struct TowerParticle *p;
	int i, clock = GetGameClock();

	for (i = 0; i < TOWER_PARTS; i++) {
		p = &TS->parts[i];
		if (p->life == 0)
			continue;
		p->life--;
		p->x += p->vx;
		p->y += p->vy;
		if (p->life & 1)
			p->vy++;
	}
	if (TS->popWord != 0xFF && ++TS->popTimer >= 64)
		TS->popWord = 0xFF;
	/* A milestone waits for the pop-up on screen to end */
	if (TS->popWord == 0xFF && TS->pendWord != 0xFF) {
		Tower_PopWord(TS->pendWord, TS->pendPal, TS->pendNum ? 132 : 120, 60);
		if (TS->pendNum)
			Tower_PopNumber(-1, TS->pendNum, 52, 52, FALSE);
		TS->pendWord = 0xFF;
	}
	if (TS->squash > 0)
		TS->squash--;
	if (TS->flash > 0) {
		TS->flash--;
		if (TS->flash > 0)
			SetBlendConfig(BLEND_EFFECT_BRIGHTEN, 0, 0, TS->flash * 2);
		else
			SetBlendConfig(BLEND_EFFECT_NONE, 0x10, 0, 0);
	}
	if (TS->numTimer > 0) {
		TS->numTimer--;
		if (TS->numRise && (TS->numTimer & 1))
			TS->numY--;
	}
	if (TS->shake > 0) {
		TS->shake--;
		TS->shakeX = (clock & 2) ? TS->shakeAmp : -TS->shakeAmp;
	} else {
		TS->shakeX = 0;
	}
}

/* ---- The camera ---- */

static int Tower_CamTarget(void)
{
	int hover = Tower_TopRow() - HOVER_GAP;
	int cam = TOWER_GROUND_Y - GROUND_SCREEN;

	if (hover - cam < HOVER_SCREEN_MIN)
		cam = hover - HOVER_SCREEN_MIN;
	return cam;
}

static void Tower_UpdateCamera(void)
{
	int d = Tower_CamTarget() - TS->camY;

	TS->camY += Tower_Clamp(d, -3, 3);
	/* The background stops at the top of the sky; the sprites keep climbing */
	BG_SetPosition(3, TS->shakeX, TS->camY > 0 ? TS->camY : 0);
}

/* ---- Setup ---- */

static const u16 sTowerBgConfig[12] = {
	0x0000, 0x6000, 0,  /* BG0: text */
	0x0000, 0x6800, 0,  /* BG1: unused */
	0x0000, 0x7000, 0,  /* BG2: unused */
	0x8000, 0x7800, 0,  /* BG3: the scene */
};

/* The next block: from a random side, on a path that passes exactly over the block below */
static void Tower_Spawn(void)
{
	int s = Tower_Speed();
	int top = Tower_TopX();
	int k;

	if (NextRN_N(2)) {
		k = Div(top + BW + s - 1, s);
		TS->x = (top - k * s) << 8;
		TS->vx = s << 8;
	} else {
		k = Div(240 - top + s - 1, s);
		TS->x = (top + k * s) << 8;
		TS->vx = -(s << 8);
	}
	TS->dropY = Tower_TopRow() - HOVER_GAP;
	TS->style = TS->fever > 0 ? 4 : (TS->height & 3);
	TS->state = TS_SLIDE;
}

static void Tower_Init(struct ProcTower *proc)
{
	u16 *map = BG_GetMapBuffer(3);
	int i, x, y;

	proc->dispBak = gLCDControlBuffer.dispcnt;

	SetupBackgrounds((u16 *)sTowerBgConfig);
	SetDispEnable(0, 0, 0, 0, 0);
	BG_Fill(BG_GetMapBuffer(0), 0);
	BG_Fill(BG_GetMapBuffer(1), 0);
	BG_Fill(BG_GetMapBuffer(2), 0);
	BG_Fill(map, 0);
	gLCDControlBuffer.bg0cnt.priority = 0;
	gLCDControlBuffer.bg3cnt.priority = 3;
	BG_SetPosition(0, 0, 0);

	ResetText();
	SetTextFont(0);
	InitSystemTextFont();
	for (i = 0; i < 2; i++)
		InitText(&TS->texts[i], 28);

	CpuFastCopy(gTowerSceneTiles, (void *)SCENE_CHR_VRAM, TOWER_SCENE_TILES * 0x20);
	for (y = 0; y < 32; y++)
		for (x = 0; x < 30; x++)
			map[y * 32 + x] = gTowerSceneMap[y * 30 + x] | (TOWER_BG_PAL << 12);
	ApplyPalette(gTowerScenePal, TOWER_BG_PAL);

	CpuFastCopy(gTowerObjTiles, (void *)OBJ_VRAM0, TOWER_OBJ_TILES * 0x20);
	ApplyPalette(gTowerObjPalBlock, 0x10 + PAL_BLOCK);
	ApplyPalette(gTowerObjPalFx, 0x10 + PAL_FX);
	ApplyPalette(gTowerObjPalUi, 0x10 + PAL_UI);
	gPaletteBuffer[PAL_BACKDROP_OFFSET] = 0;
	EnablePaletteSync();

	SetDispEnable(1, 0, 0, 1, 1);
	gLCDControlBuffer.dispcnt.obj1dMap = 1;

	TS->height = 1;
	TS->xs[0] = BASE_X;
	TS->styles[0] = 0;
	TS->score = 0;
	TS->best = TowerRecords_Best();
	TS->gold = 0;
	TS->lives = 3;
	TS->chain = 0;
	TS->fever = 0;
	TS->popWord = 0xFF;
	TS->numTimer = 0;
	TS->shake = 0;
	TS->shakeX = 0;
	TS->shakeAmp = 0;
	TS->hitStop = 0;
	TS->squash = 0;
	TS->flash = 0;
	TS->pendWord = 0xFF;
	TS->newBest = 0;
	for (i = 0; i < TOWER_PARTS; i++)
		TS->parts[i].life = 0;
	TS->camY = Tower_CamTarget();
	BG_SetPosition(3, 0, TS->camY > 0 ? TS->camY : 0);
	TS->state = TS_READY;
	TS->line1Sig = 0xFFFFFFFF;
	TS->hintKey = 0xFF;
	Tower_UpdateText();

	BG_EnableSyncByMask(BG0_SYNC_BIT | BG3_SYNC_BIT);
}

/* ---- The game ---- */

/* The chain's note climbs a major scale, a step per perfect landing (256 to a semitone) */
static const u8 sChainScale[CHAIN_MAX] = { 0, 2, 4, 5, 7, 9, 11, 12, 14, 16 };

static void Tower_ChainNote(int chain)
{
	if (gPlaySt.config.disableSoundEffects)
		return;
	m4aSongNumStart(SE_NOTE);
	m4aMPlayPitchControl(gMPlayTable[gSongTable[SE_NOTE].ms].info, 0xFFFF,
			     sChainScale[Tower_Clamp(chain, 1, CHAIN_MAX) - 1] << 8);
}

static void Tower_Shake(int frames, int amp)
{
	if (frames >= TS->shake) {
		TS->shake = frames;
		TS->shakeAmp = amp;
	}
}

/* A pop-up word for after the current one: every ten floors, and the first time the best score is beaten */
static void Tower_Milestones(void)
{
	int h = TS->height;

	if (!TS->newBest && TS->best > 0 && TS->score > TS->best) {
		TS->newBest = 1;
		TS->pendWord = TOWER_POP_NEW_BEST;
		TS->pendPal = TOWER_PAL_CHAIN(CHAIN_MAX);
		TS->pendNum = 0;
		PlaySoundEffect(SONG_SE_UPDATE);
	} else if (h - Div(h, 10) * 10 == 0) {
		TS->pendWord = TOWER_POP_FLOORS;
		TS->pendPal = TOWER_PAL_BLUE;
		TS->pendNum = h > 255 ? 255 : h;
		PlaySoundEffect(SONG_SE_POINT_UP);
	}
}

static void Tower_Land(bool perfect)
{
	int n = TS->height;
	int x = TS->x >> 8;
	int sy = Tower_BlockTop(n) - TS->camY;

	TS->xs[n & (TOWER_RING - 1)] = x;
	TS->styles[n & (TOWER_RING - 1)] = TS->style;
	TS->height++;

	if (perfect) {
		TS->score += 2;
		TS->chain++;
		Tower_ChainNote(TS->chain);
		/* A beat of hit-stop, a deep squash, a jolt and a flash */
		TS->hitStop = HIT_STOP;
		TS->squash = SQUASH_FRAMES;
		TS->squashBig = TRUE;
		TS->flash = FLASH_FRAMES;
		Tower_Shake(8, 2);
		/* "PERFECT!" with the chain beside it: "x3" */
		Tower_PopWord(TOWER_POP_PERFECT, TOWER_PAL_CHAIN(TS->chain), 96, sy - 40);
		Tower_PopNumber(11, TS->chain, 162, TS->popY - 8, FALSE);
		Tower_Burst(x + BW / 2, sy, 4 + TS->chain);
		if (TS->chain >= CHAIN_MAX) {
			/* Ten in a row: a fever (no lives lost for misses), and the chain starts again */
			TS->fever = FEVER_FRAMES;
			TS->chain = 0;
			TS->hitStop = HIT_STOP * 2;
			Tower_Shake(16, 3);
			PlaySoundEffect(SONG_SE_UPDATE);
			Tower_PopWord(TOWER_POP_FEVER, TOWER_PAL_CHAIN(CHAIN_MAX), 96, sy - 40);
			Tower_Burst(x + BW / 2, sy, TOWER_PARTS);
		}
	} else {
		TS->score += 1;
		TS->chain = 0;
		TS->squash = SQUASH_FRAMES;
		TS->squashBig = FALSE;
		Tower_Shake(4, 1);
		PlaySoundEffect(SE_LAND);
		Tower_LoadPopPal(TOWER_PAL_WHITE);
		TS->popWord = 0xFF;
		Tower_PopNumber(10, 1, x + BW / 2, sy - 18, TRUE);
	}
	TS->perfect = perfect;
	Tower_Milestones();
	Tower_Spawn();
}

static void Tower_GameOver(void)
{
	u32 gold;

	TS->state = TS_OVER;
	TS->timer = 30;
	TS->gold = Div(TS->score, 10);
	gold = GetPartyGoldAmount() + TS->gold;
	SetPartyGoldAmount(gold > 999999 ? 999999 : gold);
	TowerRecords_Add(TS->score);
	Tower_PopWord(TOWER_POP_GAME_OVER, TOWER_PAL_RED, 120, 72);
	PlaySoundEffect(TS->gold > 0 ? SE_GOLD : SE_MISS);
}

/* Leaving with B pays out what was scored, as a game over would */
static void Tower_PayOnLeave(void)
{
	u32 gold;

	if (TS->state == TS_OVER || TS->score == 0)
		return;
	TS->gold = Div(TS->score, 10);
	gold = GetPartyGoldAmount() + TS->gold;
	SetPartyGoldAmount(gold > 999999 ? 999999 : gold);
	TowerRecords_Add(TS->score);
}

/* A missed block: it falls away (or has slid off), and costs a life unless a fever is on */
static void Tower_Miss(bool falls)
{
	TS->state = TS_FALL;
	TS->timer = 40;
	TS->fallVY = 0;
	TS->chain = 0;
	if (!falls)
		TS->dropY = 400;
	if (TS->fever == 0) {
		TS->lives--;
		Tower_Shake(8, 2);
		Tower_PopWord(TOWER_POP_MISS, TOWER_PAL_RED, 120, Tower_TopRow() - TS->camY - 40);
	}
	PlaySoundEffect(SE_MISS);
}

static void Tower_Drop(void)
{
	int target = Tower_TopRow() - BH;
	int dx;

	TS->dropY += 6;
	if (TS->dropY < target)
		return;
	TS->dropY = target;
	dx = (TS->x >> 8) - Tower_TopX();
	/* Perfect within the slide speed: the three frames around the exact one, at any speed */
	if (Tower_Abs(dx) <= (Tower_Abs(TS->vx) >> 8)) {
		TS->x = Tower_TopX() << 8;
		Tower_Land(TRUE);
	} else if (Tower_Abs(dx) <= BW / 2) {
		Tower_Land(FALSE);
	} else {
		/* It tips off the side it overhangs */
		TS->vx = dx > 0 ? 0x180 : -0x180;
		Tower_Miss(TRUE);
	}
}

/* Returns TRUE to leave the screen. */
static bool Tower_Update(void)
{
	u16 newKeys = gKeyStatusPtr->newKeys;

	if ((newKeys & B_BUTTON) && TS->state != TS_OVER) {
		Tower_PayOnLeave();
		PlaySoundEffect(SE_LEAVE);
		return TRUE;
	}
	if ((newKeys & START_BUTTON) && TS->state != TS_OVER) {
		if (TS->state == TS_PAUSE) {
			TS->state = TS->prevState;
		} else {
			TS->prevState = TS->state;
			TS->state = TS_PAUSE;
		}
		return FALSE;
	}
	if (TS->state == TS_PAUSE)
		return FALSE;
	/* Hit-stop: the whole game holds still for a beat */
	if (TS->hitStop > 0) {
		TS->hitStop--;
		return FALSE;
	}

	if (TS->fever > 0 && TS->state != TS_READY && TS->state != TS_OVER)
		TS->fever--;

	switch (TS->state) {
	case TS_READY:
		if (newKeys & A_BUTTON) {
			PlaySoundEffect(SE_START);
			Tower_Spawn();
		}
		break;

	case TS_SLIDE:
		if (newKeys & A_BUTTON) {
			TS->state = TS_DROP;
			break;
		}
		TS->x += TS->vx;
		if ((TS->vx > 0 && (TS->x >> 8) > 240) || (TS->vx < 0 && (TS->x >> 8) < -BW))
			Tower_Miss(FALSE);
		break;

	case TS_DROP:
		Tower_Drop();
		break;

	case TS_FALL:
		TS->x += TS->vx;
		TS->fallVY += 6;
		TS->dropY += TS->fallVY >> 4;
		if (--TS->timer > 0)
			break;
		if (TS->lives == 0) {
			Tower_GameOver();
			break;
		}
		Tower_Spawn();
		break;

	case TS_OVER:
		if (TS->timer > 0) {
			TS->timer--;
			break;
		}
		if (newKeys & (A_BUTTON | B_BUTTON)) {
			PlaySoundEffect(SE_LEAVE);
			return TRUE;
		}
		break;
	}
	Tower_UpdateEffects();
	Tower_UpdateCamera();
	return FALSE;
}

/* ---- Drawing (back to front: a later PutSprite is drawn in front) ---- */

static void Tower_PutBlock(int x, int sy, int style)
{
	int chr = TOWER_CHR_BLOCK + style * TOWER_CHR_BLOCK_STYLE;

	if (sy <= -BH || sy >= 160 || x <= -BW || x >= 240)
		return;
	x -= TS->shakeX;
	PutSpriteExt(4, OAM1_X(x & 0x1FF), OAM0_Y(sy & 0xFF), gObject_32x16,
		     OAM2_PAL(PAL_BLOCK) + OAM2_LAYER(2) + OAM2_CHR(chr));
	PutSpriteExt(4, OAM1_X((x + 32) & 0x1FF), OAM0_Y(sy & 0xFF), gObject_16x16,
		     OAM2_PAL(PAL_BLOCK) + OAM2_LAYER(2) + OAM2_CHR(chr + 8));
}

/*
 * The newest block squashes and springs back about its base: wider and flatter, then a little taller, then still.
 * Its two sprites (32x16 and 16x16) share one affine matrix; each is placed where the scale moves its centre.
 */
static const s8 sSquashCurve[SQUASH_FRAMES] = { 16, 13, 5, -5, -8, -5, 0, 2, 1, 0 };

static void Tower_PutBlockSquashed(int x, int sy, int style)
{
	int chr = TOWER_CHR_BLOCK + style * TOWER_CHR_BLOCK_STYLE;
	int k = sSquashCurve[SQUASH_FRAMES - TS->squash];
	int a = TS->squashBig ? 64 : 32;
	int scx = 256 + ((a * k) >> 4);
	int scy = 256 - ((a * k) >> 4);
	int cx = x + BW / 2 - TS->shakeX;
	int cy = sy + BH - ((8 * scy) >> 8);
	int lx = cx + ((-8 * scx) >> 8);
	int rx = cx + ((16 * scx) >> 8);

	if (sy <= -BH || sy >= 160)
		return;
	SetObjAffine(AFF_SQUASH, Div(0x10000, scx), 0, 0, Div(0x10000, scy));
	PutSpriteExt(4, OAM1_X((lx - 32) & 0x1FF) | OAM1_AFFINE_ID(AFF_SQUASH),
		     OAM0_Y((cy - 16) & 0xFF) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE, gObject_32x16,
		     OAM2_PAL(PAL_BLOCK) + OAM2_LAYER(2) + OAM2_CHR(chr));
	PutSpriteExt(4, OAM1_X((rx - 16) & 0x1FF) | OAM1_AFFINE_ID(AFF_SQUASH),
		     OAM0_Y((cy - 16) & 0xFF) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE, gObject_16x16,
		     OAM2_PAL(PAL_BLOCK) + OAM2_LAYER(2) + OAM2_CHR(chr + 8));
}

static void Tower_DrawTower(void)
{
	int n, sy;

	for (n = TS->height - 1; n >= 0 && n >= TS->height - TOWER_RING; n--) {
		sy = Tower_BlockTop(n) - TS->camY;
		if (sy >= 160)
			break;
		if (n == TS->height - 1 && n > 0 && TS->squash > 0)
			Tower_PutBlockSquashed(Tower_BlockX(n), sy, TS->styles[n & (TOWER_RING - 1)]);
		else
			Tower_PutBlock(Tower_BlockX(n), sy, TS->styles[n & (TOWER_RING - 1)]);
	}
	if (TS->state == TS_SLIDE || TS->state == TS_DROP || TS->state == TS_FALL ||
	    (TS->state == TS_PAUSE && TS->prevState != TS_READY))
		Tower_PutBlock(TS->x >> 8, TS->dropY - TS->camY, TS->style);
}

static void Tower_DrawFx(void)
{
	const struct TowerParticle *p;
	int i;

	for (i = 0; i < TOWER_PARTS; i++) {
		p = &TS->parts[i];
		if (p->life == 0)
			continue;
		PutSpriteExt(4, OAM1_X(((p->x >> 4) - 4) & 0x1FF), OAM0_Y(((p->y >> 4) - 4) & 0xFF), gObject_8x8,
			     OAM2_PAL(PAL_FX) + OAM2_LAYER(1) + OAM2_CHR(TOWER_CHR_SPARK + ((p->life >> 2) & 1)));
	}
}

static void Tower_DrawPopups(void)
{
	int t = TS->popTimer;
	int i, scale, inv, half, cy;

	if (TS->numTimer > 0 && !(TS->numTimer < 10 && (TS->numTimer & 2))) {
		for (i = 0; i < TS->numLen; i++)
			PutSpriteExt(4, OAM1_X((TS->numX + i * 12) & 0x1FF), OAM0_Y(TS->numY & 0xFF), gObject_16x16,
				     OAM2_PAL(PAL_POP) + OAM2_LAYER(0) + OAM2_CHR(TOWER_CHR_DIGIT + TS->num[i] * 4));
	}

	if (TS->popWord == 0xFF || (t > 52 && (t & 2)))
		return;
	if (t < 6)
		scale = 64 + t * 48;
	else if (t < 14)
		scale = 352 - (t - 6) * 12;
	else
		scale = 256;
	cy = TS->popY - (t > 40 ? (t - 40) >> 1 : 0);
	inv = Div(0x10000, scale);
	half = (32 * scale) >> 8;
	SetObjAffine(AFF_POP, inv, 0, 0, inv);
	PutSpriteExt(4, OAM1_X((TS->popX - half - 64) & 0x1FF) | OAM1_AFFINE_ID(AFF_POP),
		     OAM0_Y((cy - 32) & 0xFF) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE, gObject_64x32,
		     OAM2_PAL(PAL_POP) + OAM2_LAYER(0) + OAM2_CHR(CHR_POPUP));
	PutSpriteExt(4, OAM1_X((TS->popX + half - 64) & 0x1FF) | OAM1_AFFINE_ID(AFF_POP),
		     OAM0_Y((cy - 32) & 0xFF) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE, gObject_64x32,
		     OAM2_PAL(PAL_POP) + OAM2_LAYER(0) + OAM2_CHR(CHR_POPUP + 32));
}

/* Paints the 48x8 fever meter into its 6 OBJ tiles */
static void Tower_PaintTimer(void)
{
	u32 tiles[6][8];
	int fill = Div(TS->fever * TIMER_W, FEVER_FRAMES);
	int tx, py, px, x, c;
	u32 row;

	for (tx = 0; tx < 6; tx++) {
		for (py = 0; py < 8; py++) {
			row = 0;
			for (px = 0; px < 8; px++) {
				x = tx * 8 + px;
				if (py == 0 || py == 7 || x == 0 || x == TIMER_W - 1)
					c = 13;
				else if (x < fill)
					c = 14;
				else
					c = 12;
				row |= c << (px * 4);
			}
			tiles[tx][py] = row;
		}
	}
	CpuFastCopy(tiles, (void *)(OBJ_VRAM0 + CHR_TIMER * 0x20), sizeof(tiles));
}

static void Tower_DrawStatus(void)
{
	int i, icon;

	for (i = 0; i < TS->lives && i < 5; i++)
		PutSpriteExt(4, OAM1_X(150 + i * 10), OAM0_Y(4), gObject_8x8,
			     OAM2_PAL(PAL_FX) + OAM2_LAYER(0) + OAM2_CHR(TOWER_CHR_HEART));

	/* The chain meter: ten pips that light green, then orange, then gold; all gold and blinking in a fever */
	if (TS->state != TS_READY && TS->state != TS_OVER) {
		for (i = 0; i < CHAIN_MAX; i++) {
			int kind = 0;

			if (TS->fever > 0)
				kind = (GetGameClock() & 8) ? 3 : 0;
			else if (i < TS->chain)
				kind = i < 3 ? 1 : (i < 7 ? 2 : 3);
			PutSpriteExt(4, OAM1_X(PIP_X + i * 9), OAM0_Y(PIP_Y), gObject_8x8,
				     OAM2_PAL(PAL_BLOCK) + OAM2_LAYER(0) + OAM2_CHR(TOWER_CHR_PIP + kind));
		}
	}
	if (TS->fever > 0 && (TS->fever > 120 || (TS->fever & 8))) {
		Tower_PaintTimer();
		PutSpriteExt(4, OAM1_X(TIMER_X), OAM0_Y(4), gObject_32x8,
			     OAM2_PAL(PAL_UI) + OAM2_LAYER(0) + OAM2_CHR(CHR_TIMER));
		PutSpriteExt(4, OAM1_X(TIMER_X + 32), OAM0_Y(4), gObject_16x8,
			     OAM2_PAL(PAL_UI) + OAM2_LAYER(0) + OAM2_CHR(CHR_TIMER + 4));
	}
	for (i = 0; i < TS->hintCount; i++) {
		icon = TS->hintIcon[i];
		PutSpriteExt(4, OAM1_X(TS->hintX[i]), OAM0_Y(HINT_ROW * 8),
			     icon == HINT_START ? gObject_32x16 : gObject_16x16,
			     OAM2_PAL(PAL_UI) + OAM2_LAYER(0) + OAM2_CHR(sHintChr[icon]));
	}
}

/*
 * The scene first, the UI sprites last (so first in OAM): the GBA lets a sprite earlier in OAM hide a later one even
 * when the later one has the higher priority.
 */
static void Tower_Draw(void)
{
	Tower_DrawTower();
	Tower_DrawFx();
	Tower_DrawPopups();
	Tower_DrawStatus();
}

/* ---- The proc ---- */

enum { PL_TOWER_EXIT = 1 };

static void Tower_Loop(struct ProcTower *proc)
{
	if (Tower_Update()) {
		Proc_Goto(proc, PL_TOWER_EXIT);
		return;
	}
	Tower_UpdateText();
	Tower_Draw();
}

static void Tower_OnEnd(struct ProcTower *proc)
{
	ClearSprites();
	ClearBg0Bg1();
	BG_SetPosition(3, 0, 0);
	gLCDControlBuffer.dispcnt = proc->dispBak;
	SetDefaultColorEffects();
}

const struct ProcCmd ProcScr_TowerScreen[] = {
	PROC_NAME("TowerScreen"),
	PROC_YIELD,
	PROC_SET_END_CB(Tower_OnEnd),

	PROC_CALL(Tower_Init),
	PROC_CALL_ARG(NewFadeIn, 0x10),
	PROC_REPEAT(Tower_Loop),

	PROC_LABEL(PL_TOWER_EXIT),
	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_END,
};

#endif /* CONFIG_DISTRICT_TOWER */
