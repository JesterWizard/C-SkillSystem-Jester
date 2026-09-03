#include "global.h"
#include "EAstdlib.h"
#include "bmbattle.h"
#include "event.h"
#include "eventscript.h"
#include "face.h"
#include "hardware.h"
#include "mapanim.h"
#include "proc.h"
#include "scene.h"
#include "variables.h"

struct LevelUpQuoteEntry {
	u8 charId;
	u8 pad[3];
	u16 values[3];
};

extern const struct LevelUpQuoteEntry gLevelUpQuoteTable[];

struct ProcLevelUpEventCaller {
	PROC_HEADER;

	u8 exec_type;
	const EventScr *eventscr;
};

static void LevelUpEventCaller_Exec(struct ProcLevelUpEventCaller *proc)
{
	CallEvent((const void *)proc->eventscr, proc->exec_type);
}

static const struct ProcCmd ProcScr_LevelUpEventCaller[] = {
	PROC_YIELD,
	PROC_CALL(LevelUpEventCaller_Exec),
	PROC_YIELD,
	PROC_WHILE(EventEngineExists),
	PROC_END,
};

static const EventScr EventScr_LevelUpSpeech[] = {
	EVBIT_MODIFY(0x4)
	TEXTSTART
	TEXTSHOW(0xFFFF)
	TEXTEND
	REMA
	NOFADE
	ENDA
};

static ProcPtr CallLevelUpSpeechEvent(const EventScr *eventscr, u8 exec_type, ProcPtr parent)
{
	struct ProcLevelUpEventCaller *proc;

	if (!parent) {
		CallEvent((const void *)eventscr, exec_type);
		return NULL;
	}

	proc = Proc_StartBlocking(ProcScr_LevelUpEventCaller, parent);
	proc->exec_type = exec_type;
	proc->eventscr = eventscr;
	return proc;
}

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

void DisplayCharacterSpeech(struct ManimLevelUpProc *proc)
{
	int message;
	int statGains;
	u16 quoteId;
	u8 unitId;

	if (gEventSlots[EVT_SLOT_8] > 0)
		return;

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
	if (quoteId == 0)
		return;

	gEventSlots[EVT_SLOT_2] = quoteId;
	ResetFaces();
	EndManimLevelUpStatGainLabels();
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_EnableSyncByMask(BG1_SYNC_BIT);
	CallLevelUpSpeechEvent(EventScr_LevelUpSpeech, EV_EXEC_CUTSCENE, proc);
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
	PROC_CALL(DisplayCharacterSpeech),
	PROC_CALL(EndManimLevelUpStatGainLabels),
	PROC_SLEEP(1),
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
