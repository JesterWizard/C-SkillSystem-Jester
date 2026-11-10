#include "common-chax.h"
#include "kernel-lib.h"
#include "guild.h"
#include "crossing.h"
#include "tower.h"
#include "fishing.h"
#include "hex.h"
#include "constants/texts.h"

#ifdef CONFIG_DISTRICT_GUILD

/*
 * The guild board's screen (ProcScr_GuildScreen): a title line with the tokens, three page tabs, a window of six text
 * lines and a hint line.
 *
 *   Quests   the first three quests of this board not yet claimed, two lines each: the request, then its progress (or
 *            "Done!") and its reward. A on a met quest claims it.
 *   Prizes   six prizes at a time, scrolling: icon, name and cost. A, then A again, buys one.
 *   Records  every minigame's bests.
 *
 * L and R (or LEFT and RIGHT) turn the page, UP and DOWN move the hand, B leaves. Layers: BG1 the window frames, BG0 the
 * text and the item icons, OBJ the hand. Avoid `%` and unsigned `/` (CLAUDE.md).
 */

#define GS (&gGuildScreen)

#define GUILD_PAGES 3
#define GUILD_LINES 6
#define GUILD_QUEST_ROWS (GUILD_LINES / 2)
#define GUILD_TEXTS (GUILD_LINES + 3)
#define GUILD_NOTICE_FRAMES 90

enum { PAGE_QUESTS, PAGE_PRIZES, PAGE_RECORDS };
enum { TEXT_TITLE = GUILD_LINES, TEXT_TABS, TEXT_HINT };

#define ROW_TITLE 0
#define ROW_TABS 2
#define ROW_FRAME 4
#define ROW_LINE0 5
#define ROW_HINT 18

#define SE_MOVE SONG_SE_SYS_CURSOR_UD1
#define SE_PAGE SONG_SE_SYS_CURSOR_LR1
#define SE_OK SONG_SE_SYS_WINDOW_SELECT1
#define SE_BAD SONG_6C
#define SE_LEAVE SONG_SE_SYS_WINDOW_CANSEL1
#define SE_REWARD SONG_SE_MONEY

struct GuildScreenState {
	/* 00 */ u8 page;
	/* 01 */ u8 cursor;        /* the row the hand is on */
	/* 02 */ u8 top;           /* the first prize shown */
	/* 03 */ u8 rows;          /* rows on this page (quests or prizes) */
	/* 04 */ u8 rowIndex[GUILD_LINES]; /* the quest or prize each row shows */
	/* 0A */ u8 confirm;       /* A was pressed once on this prize */
	/* 0B */ u8 noticeT;       /* frames the notice stays on the hint line */
	/* 0C */ u16 notice;       /* a message on the hint line, 0: the page's hint */
	/* 0E */ u8 redraw;
	/* 0F */ u8 pad;
	/* 10 */ struct Text texts[GUILD_TEXTS];
};

extern struct GuildScreenState gGuildScreen;

typedef char GuildScreen_SizeCheck[(sizeof(struct GuildScreenState) <= 0x80) ? 1 : -1];

struct ProcGuild {
	PROC_HEADER;
	struct DispCnt dispBak;
};

static const u16 sGuildBgConfig[12] = {
	0x0000, 0x6000, 0,  /* BG0: text and icons */
	0x0000, 0x6800, 0,  /* BG1: window frames */
	0x0000, 0x7000, 0,  /* BG2: unused */
	0x0000, 0x7800, 0,  /* BG3: unused */
};

/* ---- Text helpers ---- */

static void Guild_Append(char *buf, int *i, const char *s)
{
	while (*s != 0 && *i < 62)
		buf[(*i)++] = *s++;
	buf[*i] = 0;
}

static void Guild_AppendMsg(char *buf, int *i, int msg)
{
	Guild_Append(buf, i, GetStringFromIndex(msg));
}

static void Guild_AppendNum(char *buf, int *i, int n)
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

static u16 *Guild_Tile(int x, int y)
{
	return TILEMAP_LOCATED(gBG0TilemapBuffer, x, y);
}

static void Guild_PutLine(int line)
{
	PutText(&GS->texts[line], Guild_Tile(2, ROW_LINE0 + line * 2));
}

/* ---- The board's quests ---- */

/* The first quests of this board (its district's, and the ones on every board) not yet claimed */
static int Guild_ListQuests(void)
{
	int i, n = 0, count = Guild_QuestsCount(), here = gPlaySt.chapterIndex;
	int featured = Guild_FeaturedQuest(here);

	/* This chapter's featured quest tops the board */
	if (featured >= 0)
		GS->rowIndex[n++] = featured;
	for (i = 0; i < count && n < GUILD_QUEST_ROWS; i++) {
		if (gGuildQuests[i].district != 0 && gGuildQuests[i].district != here)
			continue;
		if (Guild_QuestClaimed(i) || i == featured)
			continue;
		GS->rowIndex[n++] = i;
	}
	return n;
}

static int Guild_PrizesCount(void)
{
	int n = 0;

	while (gGuildPrizes[n].item != 0)
		n++;
	return n;
}

/* ---- Drawing ---- */

static void Guild_DrawTitle(void)
{
	struct Text *t = &GS->texts[TEXT_TITLE];
	char buf[64];
	int i = 0;

	ClearText(t);
	Text_InsertDrawString(t, 0, TEXT_COLOR_SYSTEM_WHITE, GetStringFromIndex(MSG_GUILD_TITLE));
	Guild_AppendMsg(buf, &i, MSG_GUILD_TOKENS_TITLE);
	Guild_Append(buf, &i, " ");
	Guild_AppendNum(buf, &i, Guild_Tokens());
	Text_InsertDrawString(t, 150, TEXT_COLOR_SYSTEM_GOLD, buf);
	PutText(t, Guild_Tile(1, ROW_TITLE));
}

static void Guild_DrawTabs(void)
{
	static const u16 names[GUILD_PAGES] = { MSG_GUILD_TAB_QUESTS, MSG_GUILD_TAB_PRIZES, MSG_GUILD_TAB_RECORDS };
	struct Text *t = &GS->texts[TEXT_TABS];
	int p;

	ClearText(t);
	for (p = 0; p < GUILD_PAGES; p++)
		Text_InsertDrawString(t, 8 + p * 64, p == GS->page ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY,
				      GetStringFromIndex(names[p]));
	PutText(t, Guild_Tile(1, ROW_TABS));
}

static void Guild_DrawHint(void)
{
	static const u16 hints[GUILD_PAGES] = { MSG_GUILD_HINT_QUESTS, MSG_GUILD_HINT_PRIZES, MSG_GUILD_HINT_RECORDS };
	struct Text *t = &GS->texts[TEXT_HINT];

	ClearText(t);
	Text_InsertDrawString(t, 0, GS->notice ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_WHITE,
			      GetStringFromIndex(GS->notice ? GS->notice : hints[GS->page]));
	PutText(t, Guild_Tile(1, ROW_HINT));
}

/* A quest's second line: its progress (or Done!), then its reward */
static void Guild_DrawQuestDetail(struct Text *t, int index)
{
	const struct GuildQuest *q = &gGuildQuests[index];
	char buf[64];
	int i = 0, have = Guild_QuestProgress(q);
	bool met = have >= q->target;

	if (met) {
		Text_InsertDrawString(t, 8, TEXT_COLOR_SYSTEM_GREEN, GetStringFromIndex(MSG_GUILD_DONE));
	} else {
		Guild_AppendNum(buf, &i, have);
		Guild_Append(buf, &i, " / ");
		Guild_AppendNum(buf, &i, q->target);
		Text_InsertDrawString(t, 8, TEXT_COLOR_SYSTEM_BLUE, buf);
	}

	i = 0;
	Guild_Append(buf, &i, "+");
	Guild_AppendNum(buf, &i, q->rewardTokens + Guild_FeaturedBonus(index));
	Guild_Append(buf, &i, " ");
	Guild_AppendMsg(buf, &i, MSG_GUILD_TOKENS);
	if (q->rewardGold) {
		Guild_Append(buf, &i, " +");
		Guild_AppendNum(buf, &i, q->rewardGold);
		Guild_Append(buf, &i, "G");
	}
	if (q->rewardItem) {
		Guild_Append(buf, &i, " ");
		Guild_Append(buf, &i, GetItemName(q->rewardItem));
	}
	if (Guild_FeaturedBonus(index)) {
		Guild_Append(buf, &i, " ");
		Guild_AppendMsg(buf, &i, MSG_GUILD_FEATURED);
	}
	switch (Guild_QuestUnlockKind(index)) {
	case GUILD_UNLOCK_BIOGRAPHY:
		Guild_Append(buf, &i, " ");
		Guild_AppendMsg(buf, &i, MSG_GUILD_UNLOCK_STORY);
		break;
	case GUILD_UNLOCK_SUPPORT:
		Guild_Append(buf, &i, " ");
		Guild_AppendMsg(buf, &i, MSG_GUILD_UNLOCK_SUPPORT);
		break;
	}
	Text_InsertDrawString(t, 80, TEXT_COLOR_SYSTEM_WHITE, buf);
}

static void Guild_DrawQuests(void)
{
	int r, line;

	GS->rows = Guild_ListQuests();
	for (line = 0; line < GUILD_LINES; line++)
		ClearText(&GS->texts[line]);
	if (GS->rows == 0) {
		Text_InsertDrawString(&GS->texts[0], 0, TEXT_COLOR_SYSTEM_GRAY, GetStringFromIndex(MSG_GUILD_NO_QUESTS));
	}
	for (r = 0; r < GS->rows; r++) {
		int index = GS->rowIndex[r];
		bool met = Guild_QuestMet(index);

		Text_InsertDrawString(&GS->texts[r * 2], 0,
				      met ? TEXT_COLOR_SYSTEM_GREEN :
				      Guild_FeaturedBonus(index) ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_WHITE,
				      GetStringFromIndex(gGuildQuests[index].msg));
		Guild_DrawQuestDetail(&GS->texts[r * 2 + 1], index);
	}
	for (line = 0; line < GUILD_LINES; line++)
		Guild_PutLine(line);
}

static void Guild_DrawPrizes(void)
{
	int r, n = Guild_PrizesCount(), tokens = Guild_Tokens();
	char buf[64];

	GS->rows = 0;
	for (r = 0; r < GUILD_LINES; r++) {
		struct Text *t = &GS->texts[r];
		int index = GS->top + r, i = 0, color;
		const struct GuildPrize *p;
		u16 *icon = Guild_Tile(2, ROW_LINE0 + r * 2);

		ClearText(t);
		icon[0] = icon[1] = icon[32] = icon[33] = 0;
		if (index >= n) {
			PutText(t, Guild_Tile(4, ROW_LINE0 + r * 2));
			continue;
		}
		p = &gGuildPrizes[index];
		GS->rowIndex[r] = index;
		GS->rows++;
		color = (Guild_PrizeSoldOut(index) || p->cost > tokens) ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE;
		Text_InsertDrawString(t, 0, color, GetItemName(Guild_PrizeItem(p)));
		if (Guild_PrizeSoldOut(index)) {
			Guild_AppendMsg(buf, &i, MSG_GUILD_SOLD);
		} else {
			Guild_AppendNum(buf, &i, p->cost);
			Guild_Append(buf, &i, " ");
			Guild_AppendMsg(buf, &i, MSG_GUILD_TOKENS);
		}
		Text_InsertDrawString(t, 120, color == TEXT_COLOR_SYSTEM_GRAY ? color : TEXT_COLOR_SYSTEM_BLUE, buf);
		PutText(t, Guild_Tile(4, ROW_LINE0 + r * 2));
		DrawIcon(icon, GetItemIconId(Guild_PrizeItem(p)), 0x4000);
	}
}

static void Guild_RecordLine(int line, int x, int label, int value, int x2)
{
	char buf[16];
	int i = 0;

	Text_InsertDrawString(&GS->texts[line], x, TEXT_COLOR_SYSTEM_GOLD, GetStringFromIndex(label));
	Guild_AppendNum(buf, &i, value);
	Text_InsertDrawString(&GS->texts[line], x2, TEXT_COLOR_SYSTEM_BLUE, buf);
}

static int Guild_Progress(int kind)
{
	struct GuildQuest q = { .kind = kind };

	return Guild_QuestProgress(&q);
}

/*
 * The records page:
 *   quests done          tokens earned
 *   Crossing  S best  P best  C best  J best
 *   Tower Stack best     Snek best
 *   Fishing  rank        Journal n/20
 *   Legends n/4          Hex cards n
 *   Battles won n
 */
static void Guild_DrawRecords(void)
{
	char buf[64];
	int i, line;

	for (line = 0; line < GUILD_LINES; line++)
		ClearText(&GS->texts[line]);

	i = 0;
	Guild_AppendNum(buf, &i, Guild_QuestsClaimedCount());
	Guild_Append(buf, &i, "/");
	Guild_AppendNum(buf, &i, Guild_QuestsCount());
	Text_InsertDrawString(&GS->texts[0], 0, TEXT_COLOR_SYSTEM_GOLD, GetStringFromIndex(MSG_GUILD_REC_QUESTS));
	Text_InsertDrawString(&GS->texts[0], 64, TEXT_COLOR_SYSTEM_BLUE, buf);
	Guild_RecordLine(0, 112, MSG_GUILD_REC_EARNED, gGuildSave.earned, 184);

#ifdef CONFIG_DISTRICT_CROSSING
	{
		static const char courses[CROSSING_LEVELS] = { 'S', 'P', 'C', 'J' };
		int k;

		i = 0;
		for (k = 0; k < CROSSING_LEVELS; k++) {
			char c[2] = { courses[k], 0 };

			Guild_Append(buf, &i, c);
			Guild_Append(buf, &i, " ");
			Guild_AppendNum(buf, &i, CrossingRecords_Best(k));
			Guild_Append(buf, &i, "  ");
		}
		Text_InsertDrawString(&GS->texts[1], 0, TEXT_COLOR_SYSTEM_GOLD, GetStringFromIndex(MSG_GUILD_REC_CROSSING));
		Text_InsertDrawString(&GS->texts[1], 56, TEXT_COLOR_SYSTEM_BLUE, buf);
	}
#endif

#ifdef CONFIG_DISTRICT_TOWER
	Guild_RecordLine(2, 0, MSG_GUILD_REC_TOWER, gTowerRecords.best, 64);
#endif
	Guild_RecordLine(2, 112, MSG_GUILD_REC_SNEK, Guild_Progress(GUILD_QUEST_SNEK_SCORE), 152);

#ifdef CONFIG_DISTRICT_FISHING
	Text_InsertDrawString(&GS->texts[3], 0, TEXT_COLOR_SYSTEM_GOLD, GetStringFromIndex(MSG_GUILD_REC_FISHING));
	Text_InsertDrawString(&GS->texts[3], 48, TEXT_COLOR_SYSTEM_BLUE,
			      GetStringFromIndex(MSG_FISHING_RANK_0 + FishingProgress_Rank()));
	i = 0;
	Guild_AppendNum(buf, &i, Guild_Progress(GUILD_QUEST_FISH_SPECIES));
	Guild_Append(buf, &i, "/");
	Guild_AppendNum(buf, &i, FISHING_JOURNAL_SPECIES);
	Text_InsertDrawString(&GS->texts[3], 112, TEXT_COLOR_SYSTEM_GOLD, GetStringFromIndex(MSG_GUILD_REC_JOURNAL));
	Text_InsertDrawString(&GS->texts[3], 160, TEXT_COLOR_SYSTEM_BLUE, buf);

	i = 0;
	Guild_AppendNum(buf, &i, Guild_Progress(GUILD_QUEST_FISH_LEGENDS));
	Guild_Append(buf, &i, "/");
	Guild_AppendNum(buf, &i, FISHING_WATERS);
	Text_InsertDrawString(&GS->texts[4], 0, TEXT_COLOR_SYSTEM_GOLD, GetStringFromIndex(MSG_GUILD_REC_LEGENDS));
	Text_InsertDrawString(&GS->texts[4], 64, TEXT_COLOR_SYSTEM_BLUE, buf);
#endif
#ifdef CONFIG_HEX_CARDS
	Guild_RecordLine(4, 112, MSG_GUILD_REC_HEX, HexBinder_Total(), 168);
#endif
	Guild_RecordLine(5, 0, MSG_GUILD_REC_WINS, Guild_Progress(GUILD_QUEST_WINS), 64);

	GS->rows = 0;
	for (line = 0; line < GUILD_LINES; line++)
		Guild_PutLine(line);
}

static void Guild_Redraw(void)
{
	int r;

	/* The prize page's icons sit left of its text; the other pages start their text there */
	for (r = 0; r < GUILD_LINES; r++) {
		u16 *row = Guild_Tile(2, ROW_LINE0 + r * 2);
		int x;

		for (x = 0; x < 27; x++)
			row[x] = row[32 + x] = 0;
	}
	Guild_DrawTitle();
	Guild_DrawTabs();
	switch (GS->page) {
	case PAGE_QUESTS:
		Guild_DrawQuests();
		break;
	case PAGE_PRIZES:
		Guild_DrawPrizes();
		break;
	default:
		Guild_DrawRecords();
		break;
	}
	if (GS->cursor >= GS->rows)
		GS->cursor = GS->rows ? GS->rows - 1 : 0;
	Guild_DrawHint();
	BG_EnableSyncByMask(BG0_SYNC_BIT);
	GS->redraw = 0;
}

static void Guild_Notice(int msg)
{
	GS->notice = msg;
	GS->noticeT = msg ? GUILD_NOTICE_FRAMES : 0;
	GS->redraw = 1;
}

/* ---- The proc ---- */

static void Guild_Init(struct ProcGuild *proc)
{
	int i;

	proc->dispBak = gLCDControlBuffer.dispcnt;

	SetupBackgrounds((u16 *)sGuildBgConfig);
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
	LoadObjUIGfx();
	ResetIconGraphics();
	LoadIconPalettes(4);

	CpuFill16(0, GS, sizeof(*GS));
	for (i = 0; i < GUILD_LINES; i++)
		InitText(&GS->texts[i], 26);
	InitText(&GS->texts[TEXT_TITLE], 28);
	InitText(&GS->texts[TEXT_TABS], 28);
	InitText(&GS->texts[TEXT_HINT], 28);

	DrawUiFrame2(0, ROW_FRAME, 30, 14, 0);
	gPaletteBuffer[PAL_BACKDROP_OFFSET] = RGB(2, 3, 7);
	EnablePaletteSync();

	SetDispEnable(1, 1, 0, 0, 1);
	Guild_Redraw();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static void Guild_ClaimOrBuy(void)
{
	int index = GS->rowIndex[GS->cursor];

	if (GS->rows == 0) {
		PlaySoundEffect(SE_BAD);
		return;
	}
	if (GS->page == PAGE_QUESTS) {
		if (Guild_ClaimQuest(index)) {
			PlaySoundEffect(SE_REWARD);
			Guild_Notice(MSG_GUILD_CLAIMED);
		} else {
			PlaySoundEffect(SE_BAD);
			Guild_Notice(MSG_GUILD_NOT_YET);
		}
		return;
	}
	/* Prizes: the first A asks, the second buys */
	if (Guild_PrizeSoldOut(index) || gGuildPrizes[index].cost > Guild_Tokens()) {
		PlaySoundEffect(SE_BAD);
		Guild_Notice(Guild_PrizeSoldOut(index) ? MSG_GUILD_SOLD : MSG_GUILD_SHORT);
		return;
	}
	if (!GS->confirm) {
		GS->confirm = 1;
		PlaySoundEffect(SE_OK);
		Guild_Notice(MSG_GUILD_CONFIRM);
		return;
	}
	GS->confirm = 0;
	if (Guild_BuyPrize(index)) {
		PlaySoundEffect(SE_REWARD);
		Guild_Notice(MSG_GUILD_BOUGHT);
	}
}

/* Returns TRUE to leave */
static bool Guild_Update(void)
{
	u16 keys = gKeyStatusPtr->newKeys, repeat = gKeyStatusPtr->repeatedKeys;
	int n;

	if (GS->noticeT > 0 && --GS->noticeT == 0) {
		GS->notice = 0;
		GS->confirm = 0;
		GS->redraw = 1;
	}

	if (keys & B_BUTTON) {
		PlaySoundEffect(SE_LEAVE);
		return TRUE;
	}
	if (keys & (L_BUTTON | R_BUTTON | DPAD_LEFT | DPAD_RIGHT)) {
		GS->page = (keys & (R_BUTTON | DPAD_RIGHT)) ? (GS->page + 1 == GUILD_PAGES ? 0 : GS->page + 1)
							     : (GS->page == 0 ? GUILD_PAGES - 1 : GS->page - 1);
		GS->cursor = 0;
		GS->top = 0;
		GS->confirm = 0;
		GS->notice = 0;
		GS->noticeT = 0;
		PlaySoundEffect(SE_PAGE);
		GS->redraw = 1;
	} else if (repeat & DPAD_UP) {
		GS->confirm = 0;
		if (GS->cursor > 0) {
			GS->cursor--;
			PlaySoundEffect(SE_MOVE);
		} else if (GS->page == PAGE_PRIZES && GS->top > 0) {
			GS->top--;
			PlaySoundEffect(SE_MOVE);
			GS->redraw = 1;
		}
		if (GS->notice == MSG_GUILD_CONFIRM)
			Guild_Notice(0);
	} else if (repeat & DPAD_DOWN) {
		GS->confirm = 0;
		n = Guild_PrizesCount();
		if (GS->cursor + 1 < GS->rows) {
			GS->cursor++;
			PlaySoundEffect(SE_MOVE);
		} else if (GS->page == PAGE_PRIZES && GS->top + GUILD_LINES < n) {
			GS->top++;
			PlaySoundEffect(SE_MOVE);
			GS->redraw = 1;
		}
		if (GS->notice == MSG_GUILD_CONFIRM)
			Guild_Notice(0);
	} else if ((keys & A_BUTTON) && GS->page != PAGE_RECORDS) {
		Guild_ClaimOrBuy();
	}

	if (GS->redraw)
		Guild_Redraw();
	return FALSE;
}

static void Guild_Draw(void)
{
	int row;

	if (GS->page == PAGE_RECORDS || GS->rows == 0)
		return;
	/* The hand points at the row: a quest's first line, or the prize's icon */
	row = GS->page == PAGE_QUESTS ? GS->cursor * 2 : GS->cursor;
	DisplayUiHand(GS->page == PAGE_PRIZES ? 4 : 8, (ROW_LINE0 + row * 2) * 8);
}

enum { PL_GUILD_EXIT = 1 };

static void Guild_Loop(struct ProcGuild *proc)
{
	if (Guild_Update()) {
		Proc_Goto(proc, PL_GUILD_EXIT);
		return;
	}
	Guild_Draw();
}

static void Guild_OnEnd(struct ProcGuild *proc)
{
	ClearSprites();
	ClearBg0Bg1();
	gLCDControlBuffer.dispcnt = proc->dispBak;
	SetDefaultColorEffects();
}

const struct ProcCmd ProcScr_GuildScreen[] = {
	PROC_NAME("GuildScreen"),
	PROC_YIELD,
	PROC_SET_END_CB(Guild_OnEnd),

	PROC_CALL(Guild_Init),
	PROC_CALL_ARG(NewFadeIn, 0x10),
	PROC_REPEAT(Guild_Loop),

	PROC_LABEL(PL_GUILD_EXIT),
	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_END,
};

#endif /* CONFIG_DISTRICT_GUILD */
