#include "global.h"
#include "proc.h"
#include "bm.h"
#include "bmunit.h"
#include "bmbattle.h"
#include "mapanim.h"
#include "event.h"
#include "EAstdlib.h"

/**
 * EXP given after an on-map Support conversation.
 * Defined in Installer.event (gSupportTalkExpReward) so it can be changed
 * without rebuilding. 0 disables the reward.
 */
extern const u8 gSupportTalkExpReward;

/* Copy of vanilla BattleApplyMiscActionExpGains with a custom EXP amount */
static void SupportTalkExp_ApplyToBattleActor(int exp)
{
	if (UNIT_FACTION(&gBattleActor.unit) != FACTION_BLUE)
		return;

	if (!CanBattleUnitGainLevels(&gBattleActor))
		return;

	if (gPlaySt.chapterStateBits & PLAY_FLAG_EXTRA_MAP)
		return;

	gBattleActor.expGain = exp;
	gBattleActor.unit.exp += exp;

	CheckBattleUnitLevelUp(&gBattleActor);
}

static void SupportTalkExp_WriteBack(void)
{
	struct Unit *unit = GetUnit(gBattleActor.unit.index);
	u32 state = unit->state;

	UpdateUnitFromBattle(unit, &gBattleActor);

	/**
	 * The battle unit's state is a snapshot from before the action ended,
	 * keep the live state (e.g. "already acted") instead.
	 */
	unit->state = state;
}

static const struct ProcCmd ProcScr_SupportTalkExp[] = {
	PROC_NAME("SupportTalkExp"),
	PROC_CALL(LockGame),
	PROC_WHILE(BattleEventEngineExists),
	PROC_CALL(EndMapAnimInfoWindow),
	PROC_SLEEP(1),
	PROC_CALL(MapAnim_DisplayExpBar),
	PROC_YIELD,
	PROC_CALL(UnlockGame),
	PROC_CALL(MapAnim_Cleanup),
	PROC_CALL(SupportTalkExp_WriteBack),
	PROC_END
};

/* ASMC from the support conversation event */
void SupportTalkExpGain(ProcPtr proc)
{
	int exp = gSupportTalkExpReward;

	(void)proc;

	if (exp == 0 || gActiveUnit == NULL)
		return;

	gManimSt.actorCount = 1;
	gManimSt.hp_changing = 0;
	gManimSt.subjectActorId = 0;
	gManimSt.targetActorId = 0;

	InitBattleUnit(&gBattleActor, gActiveUnit);
	SupportTalkExp_ApplyToBattleActor(exp);

	/* SetupMapBattleAnim reads the first hit of the last battle: drop a stale triangle attack flag */
	gBattleHitArray[0].attributes &= ~BATTLE_HIT_ATTR_TATTACK;

	SetupMapBattleAnim(&gBattleActor, &gBattleTarget, gBattleHitArray);
	Proc_Start(ProcScr_SupportTalkExp, PROC_TREE_3);
}

/* Vanilla EventScr_MapSupportConversation (0x08591FF0) + ASMC SupportTalkExpGain */
static const EventListScr EventScr_MapSupportConversation_ExpReward[] = {
	EVBIT_MODIFY(0x3)
	BEQ(0x0, EVT_SLOT_2, EVT_SLOT_0)
	MUSC(0xffff)
	GOTO(0x1)
LABEL(0x0)
	MUSI
LABEL(0x1)
	SADD(EVT_SLOT_2, EVT_SLOT_3, EVT_SLOT_0)
	TEXTSHOW(0xffff)
	TEXTEND
	REMA
	NOTIFY(0xc, 0x5a)
	ASMC(SupportTalkExpGain)
	EVBIT_T(7)
	ENDA
};

/**
 * Replaces vanilla CallMapSupportEvent (0x0800D304), which is only called by
 * StartSupportTalk (the on-map Support command).
 */
void CallMapSupportEvent_ExpReward(u16 musicIndex, u16 textIndex)
{
	CallEvent((const u16 *)EventScr_MapSupportConversation_ExpReward, EV_EXEC_CUTSCENE);

	gEventSlots[EVT_SLOT_2] = musicIndex;
	gEventSlots[EVT_SLOT_3] = textIndex;
}
