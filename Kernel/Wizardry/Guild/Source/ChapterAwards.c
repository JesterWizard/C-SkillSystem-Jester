#include "common-chax.h"
#include "kernel-lib.h"
#include "guild.h"
#include "chapter-awards.h"
#include "district.h"
#include "constants/texts.h"

#ifdef CONFIG_DISTRICT_GUILD

/*
 * Chapter awards (include/kernel/chapter-awards.h): the counts kept through a story chapter, and the screen shown when
 * it is cleared, before the save menu (GameControl_CallPostChapterSaveMenu, replaced here).
 *
 * The screen: a title, then a window with the three awards, each a chibi portrait and two lines (the award, the unit
 * and its tokens, then what won it), shown one at a time; then the tokens in all and "A Continue". A (or START)
 * shows them all at once, then leaves. Layers: BG1 the window frame, BG0 the text and the portraits. Avoid `%` and
 * unsigned `/` (CLAUDE.md).
 */

#define CA (&gChapterAwardsScreen)

#define CA_TEXTS (CHAPTER_AWARD_COUNT * 2 + 2)
enum { TEXT_TITLE = CHAPTER_AWARD_COUNT * 2, TEXT_HINT };

#define ROW_TITLE 1
#define ROW_FRAME 3
#define ROW_AWARD0 4
#define ROW_AWARD_STEP 4
#define ROW_HINT 18

#define CHIBI_CHR 0x280      /* 16 tiles per portrait, under the BG0 tilemap at 0x6000 */
#define CHIBI_PAL 4
#define REVEAL_FRAMES 40

#define SE_REVEAL SONG_SE_MONEY
#define SE_LEAVE SONG_SE_SYS_WINDOW_SELECT1

struct ChapterAwardsScreen {
	/* 00 */ s8 winner[CHAPTER_AWARD_COUNT];
	/* 03 */ u8 shown;          /* awards on screen */
	/* 04 */ u8 timer;
	/* 05 */ u8 pad;
	/* 06 */ u16 paid;          /* tokens paid in all */
	/* 08 */ struct Text texts[CA_TEXTS];
};

extern struct ChapterAwardsScreen gChapterAwardsScreen;

typedef char ChapterAwardsScreen_SizeCheck[(sizeof(struct ChapterAwardsScreen) <= 0x60) ? 1 : -1];
typedef char ChapterAwardsData_SizeCheck[(sizeof(struct ChapterAwardsData) == 0x64) ? 1 : -1];

/* ---- The counts ---- */

static void ChapterAwards_Clear(void)
{
	CpuFill16(0, &gChapterAwards, sizeof(gChapterAwards));
	gChapterAwards.magic = CHAPTER_AWARDS_MAGIC;
	gChapterAwards.chapter = gPlaySt.chapterIndex;
}

void ChapterInit_ResetChapterAwards(ProcPtr proc)
{
	(void)proc;
	ChapterAwards_Clear();
}

static struct ChapterAwardsUnit *ChapterAwards_Get(int pid)
{
	struct ChapterAwardsUnit *it;
	int i;

	if (gChapterAwards.magic != CHAPTER_AWARDS_MAGIC)
		ChapterAwards_Clear();
	for (i = 0; i < gChapterAwards.count; i++)
		if (gChapterAwards.units[i].pid == pid)
			return &gChapterAwards.units[i];
	if (gChapterAwards.count >= CHAPTER_AWARDS_UNITS)
		return NULL;
	it = &gChapterAwards.units[gChapterAwards.count++];
	it->pid = pid;
	return it;
}

static void ChapterAwards_Add16(u16 *v, int n)
{
	if (n <= 0)
		return;
	n += *v;
	*v = n > 0xFFFF ? 0xFFFF : n;
}

/* gExternalBattleToUnitHook: a player unit's battle against an enemy, once the battle is over */
int ChapterAwards_OnBattleToUnit(struct BattleUnit *bu, struct Unit *unit)
{
	struct BattleUnit *foe = bu == &gBattleActor ? &gBattleTarget : &gBattleActor;
	struct ChapterAwardsUnit *it;

	if (UNIT_FACTION(unit) != FACTION_BLUE || unit->pCharacterData == NULL)
		return 0;
	if (foe->unit.pCharacterData == NULL || UNIT_FACTION(&foe->unit) != FACTION_RED)
		return 0;
	it = ChapterAwards_Get(unit->pCharacterData->number);
	if (it == NULL)
		return 0;
	if (it->battles < 0xFF)
		it->battles++;
	ChapterAwards_Add16(&it->taken, bu->hpInitial - bu->unit.curHP);
	ChapterAwards_Add16(&it->dealt, foe->hpInitial - foe->unit.curHP);
	if (foe->unit.curHP == 0 && foe->hpInitial != 0 && it->kills < 0xFF)
		it->kills++;
	return 0;
}

void ChapterAwards_Save(u8 *dst, const u32 size)
{
	WriteAndVerifySramFast(&gChapterAwards, dst, sizeof(gChapterAwards));
}

void ChapterAwards_Load(u8 *src, const u32 size)
{
	ReadSramFast(src, &gChapterAwards, sizeof(gChapterAwards));
	if (gChapterAwards.magic != CHAPTER_AWARDS_MAGIC || gChapterAwards.count > CHAPTER_AWARDS_UNITS)
		ChapterAwards_Clear();
}

/* ---- The awards ---- */

static bool ChapterAwards_Alive(int pid)
{
	struct Unit *unit = GetUnitFromCharId(pid);

	return unit != NULL && !(unit->state & US_DEAD);
}

static int ChapterAwards_Score(const struct ChapterAwardsUnit *it, int kind)
{
	switch (kind) {
	case CHAPTER_AWARD_MVP:
		return it->dealt + it->kills * CHAPTER_AWARDS_KILL_SCORE;
	case CHAPTER_AWARD_KILLS:
		return it->kills;
	default:
		/* fewer taken is better, more battles breaks a tie */
		return it->battles == 0 ? 0 : 0x1000000 - (it->taken << 8) + it->battles;
	}
}

int ChapterAwards_Winner(int kind)
{
	int i, score, best = 0, winner = -1;

	if (gChapterAwards.magic != CHAPTER_AWARDS_MAGIC)
		return -1;
	for (i = 0; i < gChapterAwards.count; i++) {
		score = ChapterAwards_Score(&gChapterAwards.units[i], kind);
		if (score > best && ChapterAwards_Alive(gChapterAwards.units[i].pid)) {
			best = score;
			winner = i;
		}
	}
	return winner;
}

static bool ChapterAwards_IsDistrict(int chapter)
{
	const struct DistrictNode *it;

	for (it = gDistrictNodes; it->node != DISTRICT_NODE_END; it++)
		if (it->chapter == chapter)
			return TRUE;
	return FALSE;
}

/* The chapter just ended was a story chapter, cleared, with battles to award */
static bool ChapterAwards_Due(void)
{
	if (gChapterAwards.magic != CHAPTER_AWARDS_MAGIC || gChapterAwards.count == 0)
		return FALSE;
	if (!CheckFlag(3) || ChapterAwards_IsDistrict(gChapterAwards.chapter))
		return FALSE;
	return TRUE;
}

/* ---- Text helpers ---- */

static void CA_Append(char *buf, int *i, const char *s)
{
	while (*s != 0 && *i < 62)
		buf[(*i)++] = *s++;
	buf[*i] = 0;
}

static void CA_AppendNum(char *buf, int *i, int n)
{
	char digits[10];
	int d = 0, q;

	if (n < 0)
		n = 0;
	do {
		q = Div(n, 10);
		digits[d++] = '0' + (n - q * 10);
		n = q;
	} while (n > 0 && d < 9);
	while (d > 0 && *i < 62)
		buf[(*i)++] = digits[--d];
	buf[*i] = 0;
}

static void CA_AppendNumMsg(char *buf, int *i, int n, int msg)
{
	CA_AppendNum(buf, i, n);
	CA_Append(buf, i, " ");
	CA_Append(buf, i, GetStringFromIndex(msg));
}

static u16 *CA_Tile(int x, int y)
{
	return TILEMAP_LOCATED(gBG0TilemapBuffer, x, y);
}

/* ---- Drawing ---- */

static void CA_DrawTitle(void)
{
	struct Text *t = &CA->texts[TEXT_TITLE];

	ClearText(t);
	Text_InsertDrawString(t, 0, TEXT_COLOR_SYSTEM_WHITE, GetStringFromIndex(MSG_AWARDS_TITLE));
	PutText(t, CA_Tile(9, ROW_TITLE));
}

static void CA_DrawAward(int kind)
{
	struct Text *name = &CA->texts[kind * 2], *detail = &CA->texts[kind * 2 + 1];
	int row = ROW_AWARD0 + kind * ROW_AWARD_STEP, i = 0, w = CA->winner[kind];
	const struct ChapterAwardsUnit *it;
	struct Unit *unit;
	char buf[64];

	ClearText(name);
	ClearText(detail);
	Text_InsertDrawString(name, 0, TEXT_COLOR_SYSTEM_GOLD, GetStringFromIndex(gChapterAwardTypes[kind].msg));
	if (w < 0) {
		Text_InsertDrawString(detail, 0, TEXT_COLOR_SYSTEM_GRAY, GetStringFromIndex(MSG_AWARDS_NOBODY));
	} else {
		it = &gChapterAwards.units[w];
		unit = GetUnitFromCharId(it->pid);
		Text_InsertDrawString(name, 64, TEXT_COLOR_SYSTEM_WHITE,
				      GetStringFromIndex(GetCharacterData(it->pid)->nameTextId));
		switch (kind) {
		case CHAPTER_AWARD_MVP:
			CA_AppendNumMsg(buf, &i, it->dealt, MSG_AWARDS_DAMAGE);
			CA_Append(buf, &i, ", ");
			CA_AppendNumMsg(buf, &i, it->kills, MSG_AWARDS_DEFEATED);
			break;
		case CHAPTER_AWARD_KILLS:
			CA_AppendNumMsg(buf, &i, it->kills, MSG_AWARDS_DEFEATED);
			break;
		default:
			CA_AppendNumMsg(buf, &i, it->taken, MSG_AWARDS_TAKEN);
			CA_Append(buf, &i, ", ");
			CA_AppendNumMsg(buf, &i, it->battles, it->battles == 1 ? MSG_AWARDS_BATTLE : MSG_AWARDS_BATTLES);
			break;
		}
		Text_InsertDrawString(detail, 0, TEXT_COLOR_SYSTEM_WHITE, buf);
		i = 0;
		CA_Append(buf, &i, "+");
		CA_AppendNumMsg(buf, &i, gChapterAwardTypes[kind].tokens, MSG_AWARDS_TOKENS);
		Text_InsertDrawString(name, 120, TEXT_COLOR_SYSTEM_GREEN, buf);
		PutFaceChibi(GetUnitMiniPortraitId(unit), CA_Tile(2, row), CHIBI_CHR + kind * 16, CHIBI_PAL + kind, 0);
	}
	PutText(name, CA_Tile(7, row));
	PutText(detail, CA_Tile(7, row + 2));
}

static void CA_DrawHint(void)
{
	struct Text *t = &CA->texts[TEXT_HINT];
	char buf[64];
	int i = 0;

	ClearText(t);
	CA_Append(buf, &i, GetStringFromIndex(MSG_AWARDS_EARNED));
	CA_Append(buf, &i, " ");
	CA_AppendNum(buf, &i, CA->paid);
	Text_InsertDrawString(t, 0, TEXT_COLOR_SYSTEM_GOLD, buf);
	Text_InsertDrawString(t, 160, TEXT_COLOR_SYSTEM_WHITE, GetStringFromIndex(MSG_AWARDS_HINT));
	PutText(t, CA_Tile(1, ROW_HINT));
}

/* Shows the next award, then the tokens in all; returns FALSE when everything is shown */
static bool CA_RevealNext(void)
{
	if (CA->shown > CHAPTER_AWARD_COUNT)
		return FALSE;
	if (CA->shown < CHAPTER_AWARD_COUNT) {
		CA_DrawAward(CA->shown);
		if (CA->winner[CA->shown] >= 0)
			PlaySoundEffect(SE_REVEAL);
	} else {
		CA_DrawHint();
	}
	CA->shown++;
	CA->timer = REVEAL_FRAMES;
	BG_EnableSyncByMask(BG0_SYNC_BIT);
	EnablePaletteSync();
	return TRUE;
}

/* ---- The proc ---- */

static void CA_Init(ProcPtr proc)
{
	int k, w;

	SetupBackgrounds(NULL);
	SetDefaultColorEffects();
	SetDispEnable(0, 0, 0, 0, 0);
	BG_Fill(BG_GetMapBuffer(0), 0);
	BG_Fill(BG_GetMapBuffer(1), 0);
	BG_Fill(BG_GetMapBuffer(2), 0);
	BG_Fill(BG_GetMapBuffer(3), 0);
	gLCDControlBuffer.bg0cnt.priority = 0;
	gLCDControlBuffer.bg1cnt.priority = 1;
	BG_SetPosition(0, 0, 0);
	BG_SetPosition(1, 0, 0);

	ResetText();
	SetTextFont(0);
	InitSystemTextFont();
	LoadUiFrameGraphics();

	CpuFill16(0, CA, sizeof(*CA));
	for (k = 0; k < CHAPTER_AWARD_COUNT * 2; k++)
		InitText(&CA->texts[k], 22);
	InitText(&CA->texts[TEXT_TITLE], 12);
	InitText(&CA->texts[TEXT_HINT], 28);

	/* The awards are paid now, so leaving the screen early cannot lose them */
	for (k = 0; k < CHAPTER_AWARD_COUNT; k++) {
		w = ChapterAwards_Winner(k);
		CA->winner[k] = w;
		if (w >= 0) {
			Guild_AddTokens(gChapterAwardTypes[k].tokens);
			CA->paid += gChapterAwardTypes[k].tokens;
		}
	}
	gChapterAwards.magic = 0;

	DrawUiFrame2(0, ROW_FRAME, 30, 14, 0);
	gPaletteBuffer[PAL_BACKDROP_OFFSET] = RGB(2, 3, 7);
	EnablePaletteSync();
	CA_DrawTitle();
	CA->timer = REVEAL_FRAMES;

	SetDispEnable(1, 1, 0, 0, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

enum { PL_CA_EXIT = 1 };

static void CA_Loop(ProcPtr proc)
{
	u16 keys = gKeyStatusPtr->newKeys;

	if (keys & (A_BUTTON | START_BUTTON)) {
		if (CA->shown > CHAPTER_AWARD_COUNT) {
			PlaySoundEffect(SE_LEAVE);
			Proc_Goto(proc, PL_CA_EXIT);
			return;
		}
		while (CA_RevealNext())
			;
		return;
	}
	if (CA->timer > 0 && --CA->timer == 0)
		CA_RevealNext();
}

static void CA_OnEnd(ProcPtr proc)
{
	ClearBg0Bg1();
	SetDefaultColorEffects();
}

/* The vanilla save menu after the chapter, as GameControl_CallPostChapterSaveMenu starts it */
static void CA_StartSaveMenu(ProcPtr proc)
{
	Make6C_SaveMenuPostChapter(((struct Proc *)proc)->proc_parent);
}

const struct ProcCmd ProcScr_ChapterAwards[] = {
	PROC_NAME("ChapterAwards"),
	PROC_YIELD,
	PROC_SET_END_CB(CA_OnEnd),

	PROC_CALL(CA_Init),
	PROC_CALL_ARG(NewFadeIn, 0x10),
	PROC_WHILE(FadeInExists),
	PROC_REPEAT(CA_Loop),

	PROC_LABEL(PL_CA_EXIT),
	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_CALL(CA_StartSaveMenu),
	PROC_END,
};

/* Vanilla starts the save menu after every chapter but 0x38; a cleared story chapter shows its awards first */
LYN_REPLACE_CHECK(GameControl_CallPostChapterSaveMenu);
void GameControl_CallPostChapterSaveMenu(struct GameCtrlProc *proc)
{
	if (gPlaySt.chapterIndex == 0x38)
		return;
	if (ChapterAwards_Due())
		Proc_StartBlocking(ProcScr_ChapterAwards, proc);
	else
		Make6C_SaveMenuPostChapter(proc);
}

#endif /* CONFIG_DISTRICT_GUILD */
