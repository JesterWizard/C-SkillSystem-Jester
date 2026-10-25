#include "global.h"
#include "EAstdlib.h"
#include "bmbattle.h"
#include "face.h"
#include "fontgrp.h"
#include "functions.h"
#include "hardware.h"
#include "mapanim.h"
#include "proc.h"
#include "scene.h"
#include "uiutils.h"
#include "variables.h"

struct LevelUpQuoteEntry {
	u8 charId;
	u8 pad[3];
	u16 values[3];
};

extern const struct LevelUpQuoteEntry gLevelUpQuoteTable[];

/* Vanilla talk state pointer (static in the decomp, not exported by FE-CLib). */
#define sTalkState (*(struct TalkState **)0x0859133C)

/*
 * Talk slot the level-up portrait is handed to. MidRight (HPos 24) is the
 * closest slot to the level-up portrait at x = 184, so the bubble tail lands
 * on the face.
 */
#define LVUP_SPEECH_TALK_FACE 4

/* Frames the finished quote stays up before closing (A/B closes it early). */
#define LVUP_SPEECH_HOLD_FRAMES 90

/*
 * A/B is ignored for this many frames so the bubble-open animation (which
 * keeps writing BG1 tiles for ~12 frames) is done before we restore them.
 */
#define LVUP_SPEECH_MIN_HOLD_FRAMES 16

/* Level-up window offsets when fully scrolled in / fully scrolled out. */
#define LVUP_WINDOW_SHOWN_OFFSET  (-48)
#define LVUP_WINDOW_HIDDEN_OFFSET (-144)

enum {
	LVUP_LABEL_SCROLL_OUT = 1,
};

static int GetLevelUpStatGainCount(struct BattleUnit *bu)
{
	return (bu->changeHP > 0) + (bu->changePow > 0) + (bu->changeSkl > 0) +
		(bu->changeSpd > 0) + (bu->changeLck > 0) + (bu->changeDef > 0) +
		(bu->changeRes > 0);
}

static u16 GetLevelUpQuoteId(u8 unitId, int tier)
{
	const struct LevelUpQuoteEntry *it;

	for (it = gLevelUpQuoteTable; it->charId != 0; it++) {
		if (it->charId == unitId)
			return it->values[tier];
	}

	return 0;
}

static u16 GetLevelUpSpeechQuoteId(struct ManimLevelUpProc *proc)
{
	int message;
	int statGains;
	u16 quoteId;
	u8 unitId;

	if (gEventSlots[EVT_SLOT_8] > 0)
		return 0;

	unitId = gManimSt.actor[proc->actor_id].unit->pCharacterData->number;
	statGains = GetLevelUpStatGainCount(gManimSt.actor[proc->actor_id].bu);

	if (statGains <= 2)
		message = 0;
	else if (statGains <= 5)
		message = 1;
	else if (statGains <= 7)
		message = 2;
	else
		message = 0;

	quoteId = GetLevelUpQuoteId(unitId, message);
	if (quoteId == 0 || *GetStringFromIndex(quoteId) == '\0')
		return 0;

	return quoteId;
}

/* Skip the speech steps when this unit has nothing to say. */
static void LevelUpSpeech_Init(struct ManimLevelUpProc *proc)
{
	if (GetLevelUpSpeechQuoteId(proc) == 0)
		Proc_Goto(proc, LVUP_LABEL_SCROLL_OUT);
}

/* Scroll the level-up window away while the portrait stays where it is. */
static void LevelUpSpeech_ScrollOutWindow(struct ManimLevelUpProc *proc)
{
	proc->y_scroll_offset -= 8;

	BG_SetPosition(BG_0, 0, proc->y_scroll_offset);
	BG_SetPosition(BG_1, 0, proc->y_scroll_offset);

	if (proc->y_scroll_offset <= LVUP_WINDOW_HIDDEN_OFFSET)
		Proc_Break(proc);
}

/*
 * Open the quote in the standard talk bubble, speaking through the portrait
 * already on screen instead of loading a fresh one.
 */
static void LevelUpSpeech_StartTalk(struct ManimLevelUpProc *proc)
{
	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

	InitTalk(0x80, 2, TRUE);
	StartTalkMsgExt(1, 1, GetLevelUpSpeechQuoteId(proc), proc);
	SetTalkFlag(TALK_FLAG_NOSKIP);

	/* InitTalk clears the face refs, so hand the portrait over afterwards. */
	sTalkState->faces[LVUP_SPEECH_TALK_FACE] = gFaces[0];
	SetActiveTalkFace(LVUP_SPEECH_TALK_FACE);

	proc->clock = 0;
}

static void LevelUpSpeech_Hold(struct ManimLevelUpProc *proc)
{
	proc->clock++;

	if (proc->clock >= LVUP_SPEECH_HOLD_FRAMES ||
		(proc->clock >= LVUP_SPEECH_MIN_HOLD_FRAMES &&
		 (gKeyStatusPtr->newKeys & (A_BUTTON | B_BUTTON))))
		Proc_Break(proc);
}

static void LevelUpSpeech_EndTalk(struct ManimLevelUpProc *proc)
{
	ClearTalkBubble();
	ClearTalkFaceRefs();

	SetTextFont(NULL);
	InitSystemTextFont();
	LoadUiFrameGraphics();
	SetDefaultColorEffects();

	/* Let ManimLevelUp_ScrollOut slide the portrait down from where it sits. */
	proc->y_scroll_offset = LVUP_WINDOW_SHOWN_OFFSET;
}

const struct ProcCmd ProcScr_ManimLevelUp_UnitComment[] = {
	PROC_SET_END_CB(ManimLevelUp_Clear),
	PROC_SLEEP(1),
	PROC_CALL(InitManimLevelUpWindow),
	PROC_CALL(ManimLevelUp_DimBgm),
	PROC_YIELD,
	PROC_CALL(ManimLevelUp_StartLevelUpText),
	PROC_SLEEP(70),
	PROC_CALL(ManimLevelUp_EndLevelUpText),
	PROC_SLEEP(1),
	PROC_CALL(ManimLevelUp_RestoreBgm),
	PROC_YIELD,
	PROC_CALL(ManimLevelUp_InitMainScreen),
	PROC_YIELD,
	PROC_REPEAT(ManimLevelUp_ScrollIn),
	PROC_SLEEP(30),
	PROC_REPEAT(ManimLevelUp_PutStatGainLabels),
	PROC_SLEEP(60),
	PROC_CALL(EndManimLevelUpStatGainLabels),
	PROC_SLEEP(1),
	PROC_CALL(LevelUpSpeech_Init),
	PROC_REPEAT(LevelUpSpeech_ScrollOutWindow),
	PROC_CALL(LevelUpSpeech_StartTalk),
	PROC_REPEAT(LevelUpSpeech_Hold),
	PROC_CALL(LevelUpSpeech_EndTalk),
PROC_LABEL(LVUP_LABEL_SCROLL_OUT),
	PROC_REPEAT(ManimLevelUp_ScrollOut),
	PROC_CALL(ClearManimLevelUpWindow),
	PROC_CALL(ResetDialogueScreen),
	PROC_SLEEP(4),
	PROC_END,
};

void StartManimLevelUp_TalkOnLevelUp(int actor_id, ProcPtr parent)
{
	struct ManimLevelUpProc *proc;

	proc = Proc_StartBlocking(ProcScr_ManimLevelUp_UnitComment, parent);
	proc->actor_id = actor_id;
}
