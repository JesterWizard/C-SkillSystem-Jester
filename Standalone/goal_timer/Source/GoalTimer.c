#include "global.h"
#include "bm.h"
#include "bmunit.h"
#include "bmudisp.h"
#include "chapterdata.h"
#include "constants/event-flags.h"
#include "constants/msg.h"
#include "event.h"
#include "eventinfo.h"
#include "fontgrp.h"
#include "hardware.h"
#include "mu.h"
#include "player_interface.h"
#include "proc.h"
#include "uichapterstatus.h"

u32 GetBattleMapKind(void);

/*
 * Remaining seconds live in vanilla PlaySt+0x48 (unk48).
 * PlaySt is already written to suspend and chapter saves, so remaining
 * time persists without changing the save format or allocating EWRAM.
 */
#define PLAYST_CHAPTER_TIMER_OFFSET 0x48
#define gChapterTimerSeconds (*(u16 *)((u8 *)&gPlaySt + PLAYST_CHAPTER_TIMER_OFFSET))

#define SMS_GFX_FRAME_SIZE (8 * 0x20 * 0x20)

struct ChapterTimerProc {
	PROC_HEADER;
	int frameClock;
};

struct ChapterTimerEntry {
	int chapter_id;
	int time_seconds;
};

extern const struct ChapterTimerEntry ChapterTimers[];
extern u8 gSMSGfxBuffer[3][SMS_GFX_FRAME_SIZE];
extern struct ProcCmd ProcScr_efxHPBarColorChange[];

const struct ProcCmd ProcScr_ChapterTimer[];

static u32 udiv(u32 a, u32 b);
static u32 umod(u32 a, u32 b);
static int ChapterTimerIsUrgent(void);
static void DrawTimeHMS(struct Text *text, int x, int seconds);
static void StartChapterTimer(int seconds);

u16 GetChapterTimerConfigSeconds(void)
{
	const struct ChapterTimerEntry *it;
	u8 chapter = gPlaySt.chapterIndex;

	for (it = ChapterTimers; it->chapter_id >= 0; it++) {
		if (it->chapter_id == chapter)
			return (u16)it->time_seconds;
	}

	return 0;
}

void ChapterInit_ResetChapterTimer(ProcPtr proc)
{
	(void)proc;

	Proc_EndEach(ProcScr_ChapterTimer);
	gChapterTimerSeconds = GetChapterTimerConfigSeconds();
	UndeployEveryone();
}

void GoalDisplay_Init_GoalTimer(struct PlayerInterfaceProc *proc)
{
	int goalTextId;
	int goalWindowType;
	int turnNumber;
	int lastTurnNumber;
	int textWidth;
	u16 configured;
	char *str;
	struct Text *text;

	configured = GetChapterTimerConfigSeconds();
	textWidth = (configured > 0) ? 9 : 8;

	proc->showHideClock = 0;
	proc->isRetracting = false;
	proc->cursorQuadrant = 0;
	proc->windowQuadrant = -1;

	InitText(&proc->texts[0], textWidth);
	InitText(&proc->texts[1], textWidth);

	StartGreenText(proc);

	ClearText(&proc->texts[0]);
	ClearText(&proc->texts[1]);

	if (GetBattleMapKind() != BATTLEMAP_KIND_SKIRMISH)
		goalTextId = GetROMChapterStruct(gPlaySt.chapterIndex)->goalWindowTextId;
	else
		goalTextId = MSG_19E; /* "Defeat enemy" */

	str = GetStringFromIndex(goalTextId);
	Text_InsertDrawString(
		&proc->texts[0],
		GetStringTextCenteredPos(64, str),
		TEXT_COLOR_SYSTEM_WHITE,
		str);

	if (GetBattleMapKind() != BATTLEMAP_KIND_SKIRMISH)
		goalWindowType = GetROMChapterStruct(gPlaySt.chapterIndex)->goalWindowDataType;
	else
		goalWindowType = GOAL_TYPE_DEFEAT_ALL;

	if (configured > 0)
		goalWindowType = GOAL_TYPE_TIMER;

	switch (goalWindowType) {
	case GOAL_TYPE_SEIZE:
	case GOAL_TYPE_DEFEAT_BOSS:
	case GOAL_TYPE_SPECIAL:
		proc->unitClock = 0;
		return;

	case GOAL_TYPE_DEFEAT_ALL:
		Text_InsertDrawString(
			&proc->texts[1], 16, TEXT_COLOR_SYSTEM_WHITE,
			GetStringFromIndex(MSG_1C1));

		if (gPlaySt.chapterVisionRange != 0) {
			Text_InsertDrawString(
				&proc->texts[1], 40, TEXT_COLOR_SYSTEM_GRAY,
				GetStringFromIndex(MSG_535));
		} else {
			Text_InsertDrawNumberOrBlank(
				&proc->texts[1], 48, TEXT_COLOR_SYSTEM_BLUE,
				CountUnitsByFaction(FACTION_RED));
		}
		break;

	case GOAL_TYPE_DEFENSE:
		turnNumber = gPlaySt.chapterTurnNumber;
		lastTurnNumber = (GetBattleMapKind() == BATTLEMAP_KIND_SKIRMISH)
			? -1
			: GetROMChapterStruct(gPlaySt.chapterIndex)->goalWindowEndTurnNumber - 1;

		if (turnNumber >= lastTurnNumber) {
			str = GetStringFromIndex(MSG_1C3);
			Text_InsertDrawString(
				&proc->texts[1],
				GetStringTextCenteredPos(64, str),
				TEXT_COLOR_SYSTEM_GREEN,
				str);
			break;
		}

		text = &proc->texts[1];
		Text_InsertDrawNumberOrBlank(text, 10, TEXT_COLOR_SYSTEM_BLUE, turnNumber);
		Text_InsertDrawString(text, 18, TEXT_COLOR_SYSTEM_WHITE, GetStringFromIndex(MSG_539));
		Text_InsertDrawNumberOrBlank(text, 34, TEXT_COLOR_SYSTEM_BLUE, lastTurnNumber);
		Text_InsertDrawString(text, 42, TEXT_COLOR_SYSTEM_WHITE, GetStringFromIndex(MSG_1C2));
		break;

	case GOAL_TYPE_TIMER:
		ClearText(&proc->texts[0]);
		Text_InsertDrawString(
			&proc->texts[0],
			GetStringTextCenteredPos(64, "Remaining:"),
			TEXT_COLOR_SYSTEM_WHITE,
			"Remaining:");

		if (!Proc_Find(ProcScr_ChapterTimer)) {
			u16 remaining = gChapterTimerSeconds;

			if (configured == 0) {
				gChapterTimerSeconds = 0;
			} else {
				if (remaining == 0 || remaining > configured)
					remaining = configured;

				gChapterTimerSeconds = remaining;
				StartChapterTimer(remaining);
			}
		}

		DrawTimeHMS(&proc->texts[1], 2, gChapterTimerSeconds);
		proc->unitClock = 1;
		break;

	default:
		return;
	}

	proc->unitClock = 1;
}

void GoalDisplay_Loop_Display_GoalTimer(struct PlayerInterfaceProc *proc)
{
	if (GetChapterTimerConfigSeconds() > 0) {
		ClearText(&proc->texts[1]);
		DrawTimeHMS(&proc->texts[1], 2, gChapterTimerSeconds);
		BG_EnableSyncByMask(BG0_SYNC_BIT);
	}

	proc->xCursorPrev = proc->xCursor;
	proc->yCursorPrev = proc->yCursor;

	proc->xCursor = gBmSt.playerCursor.x;
	proc->yCursor = gBmSt.playerCursor.y;

	if (proc->xCursor == proc->xCursorPrev && proc->yCursor == proc->yCursorPrev)
		return;

	if (Proc_Find(ProcScr_CamMove) == NULL) {
		int cursorQuadrant = GetCursorQuadrant();
		int quadrant = proc->cursorQuadrant;

		if (cursorQuadrant == quadrant)
			return;

		if ((sPlayerInterfaceConfigLut[cursorQuadrant].xGoal ==
		     sPlayerInterfaceConfigLut[quadrant].xGoal) &&
		    (sPlayerInterfaceConfigLut[cursorQuadrant].yGoal ==
		     sPlayerInterfaceConfigLut[quadrant].yGoal)) {
			return;
		}
	}

	proc->isRetracting = true;
	Proc_Break(proc);
}

void ChapterTimer_OnTick(struct ChapterTimerProc *proc)
{
	if (gChapterTimerSeconds == 0)
		return;

	/* Freeze timer when battle animations are on */
	if (Proc_Find(ProcScr_efxHPBarColorChange))
		return;

	proc->frameClock++;

	if (proc->frameClock >= 60) {
		proc->frameClock = 0;
		gChapterTimerSeconds--;

		if (CheckFlag(EVFLAG_WIN)) {
			Proc_End(proc);
			gChapterTimerSeconds = 0;
			return;
		}

		if (gChapterTimerSeconds == 0) {
			Proc_End(proc);

			BG_Fill(gBG0TilemapBuffer, 0);
			BG_Fill(gBG1TilemapBuffer, 0);
			BG_Fill(gBG2TilemapBuffer, 0);
			BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT);
			Proc_EndEach(gProcScr_UnitDisplay_MinimugBox);
			Proc_EndEach(gProcScr_UnitDisplay_Burst);
			Proc_EndEach(gProcScr_TerrainDisplay);
			Proc_EndEach(gProcScr_GoalDisplay);
			Proc_EndEach(gProcScr_PrepMap_MenuButtonDisplay);
			CallGameOverEvent();
		}
	}
}

const struct ProcCmd ProcScr_ChapterTimer[] = {
	PROC_YIELD,
	PROC_REPEAT(ChapterTimer_OnTick),
	PROC_END,
};

static const MuStateFunc sMuStateFuncs[] = {
	[MU_STATE_NONE] = Mu_OnStateNone,
	[MU_STATE_INACTIVE] = Mu_OnStateDoNothing,
	[MU_STATE_MOVEMENT] = Mu_OnStateMovement,
	[MU_STATE_SLEEPING] = Mu_OnStateSleeping,
	[MU_STATE_UNK4] = Mu_OnStateUnk4,
	[MU_STATE_BUMPING] = Mu_OnStateBump,
	[MU_STATE_DISPLAY_UI] = Mu_OnStateDoNothing,
	[MU_STATE_DEATHFADE] = Mu_OnStateDoNothing,
};

void Mu_OnLoop_GoalTimer(struct MuProc *proc)
{
	if (ChapterTimerIsUrgent())
		proc->sprite_anim->frameInterval = 0x60;

	if (proc->state) {
		if (proc->move_clock_q4 == 0)
			if (proc->state == MU_STATE_SLEEPING || proc->state == MU_STATE_MOVEMENT)
				RunMuMoveScript(proc);

		sMuStateFuncs[proc->state](proc);
	}

	if (proc->facing == MU_FACING_STANDING)
		PutMuSMS(proc);
	else
		PutMu(proc);
}

void SyncUnitSpriteSheet_GoalTimer(void)
{
	int clock = GetGameClock();
	int frame;

	if (ChapterTimerIsUrgent())
		clock *= 4;

	frame = (int)umod((u32)clock, 72);

	if (frame == 0)
		CpuFastCopy(gSMSGfxBuffer[0], (void *)0x06011000, SMS_GFX_FRAME_SIZE);

	if (frame == 32)
		CpuFastCopy(gSMSGfxBuffer[1], (void *)0x06011000, SMS_GFX_FRAME_SIZE);

	if (frame == 36)
		CpuFastCopy(gSMSGfxBuffer[2], (void *)0x06011000, SMS_GFX_FRAME_SIZE);

	if (frame == 68)
		CpuFastCopy(gSMSGfxBuffer[1], (void *)0x06011000, SMS_GFX_FRAME_SIZE);
}

static u32 udiv(u32 a, u32 b)
{
	u32 ret = 0;
	u32 tmp_a = a;
	u64 tmp_b = b;

	if (b == 0)
		return 0;

	while (tmp_b <= a)
		tmp_b = tmp_b << 1;

	while (tmp_b > b) {
		tmp_b = tmp_b >> 1;
		ret = ret << 1;

		if (tmp_a >= tmp_b) {
			tmp_a = tmp_a - tmp_b;
			ret = ret | 1;
		}
	}

	return ret;
}

static u32 umod(u32 a, u32 b)
{
	u32 tmp_a = a;
	u64 tmp_b = b;

	if (b == 0)
		return 0;

	while (tmp_b <= a)
		tmp_b = tmp_b << 1;

	while (tmp_b > b) {
		tmp_b = tmp_b >> 1;

		if (tmp_a >= tmp_b)
			tmp_a = tmp_a - tmp_b;
	}

	return tmp_a;
}

static int ChapterTimerIsUrgent(void)
{
	u16 configured = GetChapterTimerConfigSeconds();

	if (configured == 0)
		return 0;

	return gChapterTimerSeconds <= (configured >> 1);
}

static void DrawTimeHMS(struct Text *text, int x, int seconds)
{
	int h, m, s;
	int color;

	if (seconds < 0)
		seconds = 0;

	color = ChapterTimerIsUrgent() ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_BLUE;

	h = (int)udiv((u32)seconds, 3600);
	m = (int)umod(udiv((u32)seconds, 60), 60);
	s = (int)umod((u32)seconds, 60);

	Text_InsertDrawNumberOrBlank(text, x + 3, color, udiv((u32)h, 10));
	Text_InsertDrawNumberOrBlank(text, x + 12, color, umod((u32)h, 10));
	Text_InsertDrawString(text, x + 21, TEXT_COLOR_SYSTEM_WHITE, ":");

	Text_InsertDrawNumberOrBlank(text, x + 26, color, udiv((u32)m, 10));
	Text_InsertDrawNumberOrBlank(text, x + 35, color, umod((u32)m, 10));
	Text_InsertDrawString(text, x + 44, TEXT_COLOR_SYSTEM_WHITE, ":");

	Text_InsertDrawNumberOrBlank(text, x + 49, color, udiv((u32)s, 10));
	Text_InsertDrawNumberOrBlank(text, x + 58, color, umod((u32)s, 10));
}

static void StartChapterTimer(int seconds)
{
	struct ChapterTimerProc *proc;

	if (seconds <= 0)
		return;

	if (Proc_Find(ProcScr_ChapterTimer))
		return;

	gChapterTimerSeconds = (u16)seconds;

	proc = Proc_Start(ProcScr_ChapterTimer, PROC_TREE_3);
	proc->frameClock = 0;
}
