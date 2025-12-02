#include "common-chax.h"
#include "bwl.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "battle-system.h"
#include "eventinfo.h"
#include "jester_headers/custom-arrays.h"
#include "../../../../../Contents/Texts/build/msgs.h"

#define LOCAL_TRACE 0

STATIC_DECLAR u8 *GetUnitBwlSupports(u8 pid)
{
	struct NewBwl *bwl = GetNewBwl(pid);

	if (bwl)
		return bwl->supports;

	return NULL;
}

LYN_REPLACE_CHECK(ActionSupport);
s8 ActionSupport(ProcPtr proc)
{
	u8 *supp1, *supp2;

	struct Unit *target = GetUnit(gActionData.targetIndex);

	int targetSupportNum = GetUnitSupporterNum(gActiveUnit, target->pCharacterData->number);
	int subjectSupportNum = GetUnitSupporterNum(target, gActiveUnit->pCharacterData->number);

	CanUnitSupportNow(target, subjectSupportNum);

	UnitGainSupportLevel(gActiveUnit, targetSupportNum);
	UnitGainSupportLevel(target, subjectSupportNum);

	StartSupportTalk(
		gActiveUnit->pCharacterData->number,
		target->pCharacterData->number,
		GetUnitSupportLevel(gActiveUnit, targetSupportNum)
	);

	supp1 = GetUnitBwlSupports(UNIT_CHAR_ID(gActiveUnit));
	supp2 = GetUnitBwlSupports(UNIT_CHAR_ID(target));

	if (supp1 && supp2) {
		int subjectExp = supp1[targetSupportNum];
		int targetExp = supp2[subjectSupportNum];

		if (subjectExp != targetExp) {
			if (subjectExp > targetExp)
				supp2[subjectSupportNum] = subjectExp;

			if (subjectExp < targetExp)
				supp1[targetSupportNum] = targetExp;
		}
	}

	return 0;
}

#ifdef CONFIG_SUPPORT_REWARDS
	static const EventListScr EventScr_NewMapSupportConversation[] = {
		EVBIT_MODIFY(0x3)
		BEQ(0x0, EVT_SLOT_2, EVT_SLOT_0)
		MUSC(0xffff)
		GOTO(0x1)
	LABEL(0x0)
		MUSI
	LABEL(0x1)
		SADD(EVT_SLOT_2, EVT_SLOT_5, EVT_SLOT_0)
		TEXTSHOW(0xffff)
		TEXTEND
		REMA
		BEQ(0x2, EVT_SLOT_4, EVT_SLOT_7)
		GOTO(0x3) // This isn't an A support and/or these aren't the required support partners, so we skip giving the item

	LABEL(0x2)
		GIVEITEMTO(CHARACTER_EIRIKA)
		SVAL(EVT_SLOT_4, 0)

	LABEL(0x3)
		NOTIFY(0xc, SONG_SE_UPDATE)
		EVBIT_T(7)
		ENDA
	};
#endif

LYN_REPLACE_CHECK(CallMapSupportEvent);
void CallMapSupportEvent(u16 musicIndex, u16 textIndex) {
    // Calls text with music (just quiets music when id is 0)
    // On-map supports?
#ifdef CONFIG_SUPPORT_REWARDS
	gEventSlots[EVT_SLOT_7] = SUPPORT_EXP_A;
    CallEvent((u16 *)EventScr_NewMapSupportConversation, EV_EXEC_CUTSCENE);
#else
    CallEvent((u16 *)EventScr_MapSupportConversation, EV_EXEC_CUTSCENE);
#endif

    gEventSlots[0x2] = musicIndex;
    gEventSlots[0x5] = textIndex;
}

LYN_REPLACE_CHECK(CallSupportViewerEvent);
void CallSupportViewerEvent(u16 textIndex) {
    // Calls text with random background (support viewer?)
    CallEvent((u16 *)EventScr_SupportViewerConversation, EV_EXEC_QUIET);

    gEventSlots[0x2] = textIndex;
}

//! FE8U = 0x080847F8
LYN_REPLACE_CHECK(GetSupportTalkList);
struct SupportTalkEnt* GetSupportTalkList(void) {

#ifdef CONFIG_CUSTOM_SUPPORT_CONVOS
    return (struct SupportTalkEnt* )gNewSupportTalkList;
#else
	return gSupportTalkList;
#endif
}

//! FE8U = 0x08084748
LYN_REPLACE_CHECK(GetSupportTalkEntry);
struct SupportTalkEnt * GetSupportTalkEntry(u16 pidA, u16 pidB)
{
    const struct SupportTalkEnt * it;

#ifdef CONFIG_CUSTOM_SUPPORT_CONVOS
    for (it = gNewSupportTalkList; it->unitA != 0xFFFF; it++)
#else
    for (it = gSupportTalkList; it->unitA != 0xFFFF; it++)
#endif	
	{
        if ((pidA == it->unitA) && (pidB == it->unitB))
            return (struct SupportTalkEnt *)it;  // Cast const away only on return
            
        if ((pidB == it->unitA) && (pidA == it->unitB))
            return (struct SupportTalkEnt *)it;  // Cast const away only on return
    }
    
    return NULL;
}

LYN_REPLACE_CHECK(GetUnitSupportLevel);
int GetUnitSupportLevel(struct Unit *unit, int num)
{
	int exp;
	u8 *supp;

	if (!UNIT_IS_VALID(unit))
		return SUPPORT_LEVEL_NONE;

	supp = GetUnitBwlSupports(UNIT_CHAR_ID(unit));
	if (supp) {
		exp = supp[num];
		LTRACEF("[pid=%d, num=%d] exp=%d at BWL", UNIT_CHAR_ID(unit), num, exp);
	} else {
		/**
		 * for none-BWL characters,
		 * directly judge on its rom data
		 */
		exp = unit->pCharacterData->pSupportData->supportExpBase[num];
		LTRACEF("[pid=%d, num=%d] exp=%d at ROM", UNIT_CHAR_ID(unit), num, exp);
	}

	if (exp > 240)
		return SUPPORT_LEVEL_A;

	if (exp > 160)
		return SUPPORT_LEVEL_B;

	if (exp > 80)
		return SUPPORT_LEVEL_C;

	return SUPPORT_LEVEL_NONE;
}

LYN_REPLACE_CHECK(UnitGainSupportExp);
void UnitGainSupportExp(struct Unit * unit, int num)
{
    u8 * supp = GetUnitBwlSupports(UNIT_CHAR_ID(unit));

    if (UNIT_SUPPORT_DATA(unit) && supp)
    {
        int gain = 0;

#ifdef CONFIG_VESLY_SUPPORT_POST_BATTLE
    if (gBattleActorGlobalFlag.enemy_defeated)
        gain = SUPPORT_RATE_KILL;
    else if (gActionData.unitActionType == UNIT_ACTION_COMBAT)
        gain = SUPPORT_RATE_COMBAT;
    else if (gActionData.unitActionType == UNIT_ACTION_DANCE)
        gain = SUPPORT_RATE_DANCE;
    else if (gActionData.unitActionType == UNIT_ACTION_STAFF)
        gain = SUPPORT_RATE_STAFF;
#else
		gain = UNIT_SUPPORT_DATA(unit)->supportExpGrowth[num];
#endif

        int currentExp = supp[num];
        int maxExp = sSupportMaxExpLookup[GetUnitSupportLevel(unit, num)];

        FORCE_DECLARE struct Unit * other = GetUnitSupporterUnit(unit, num);

#if defined(SID_SocialButterfly) && (COMMON_SKILL_VALID(SID_SocialButterfly))
        if (SkillTester(unit, SID_SocialButterfly) || SkillTester(other, SID_SocialButterfly))
            gain *= 2;
#endif

        if (currentExp + gain > maxExp)
            gain = maxExp - currentExp;

        supp[num] = currentExp + gain;
        gPlaySt.chapterTotalSupportGain += gain;
    }
}

LYN_REPLACE_CHECK(UnitGainSupportLevel);
void UnitGainSupportLevel(struct Unit *unit, int num)
{
	u8 *supp = GetUnitBwlSupports(UNIT_CHAR_ID(unit));

	if (supp)
		supp[num]++;

	gPlaySt.chapterTotalSupportGain++;

	SetSupportLevelGained(unit->pCharacterData->number, GetUnitSupporterCharacter(unit, num));
}

LYN_REPLACE_CHECK(CanUnitSupportNow);
s8 CanUnitSupportNow(struct Unit *unit, int num)
{
	int exp, maxExp;
	u8 *supp = GetUnitBwlSupports(UNIT_CHAR_ID(unit));

	if (gPlaySt.chapterStateBits & PLAY_FLAG_EXTRA_MAP)
		return FALSE;

	if (gPlaySt.chapterStateBits & PLAY_FLAG_TUTORIAL)
		return FALSE;

	if (HasUnitGainedSupportLevel(unit, num))
		return FALSE;

	if (GetUnitTotalSupportLevel(unit) >= MAX_SIMULTANEOUS_SUPPORT_COUNT)
		return FALSE;

	if (GetUnitTotalSupportLevel(GetUnitSupporterUnit(unit, num)) >= MAX_SIMULTANEOUS_SUPPORT_COUNT)
		return FALSE;

	if (!supp)
		return FALSE;

	exp	= supp[num];
	maxExp = sSupportMaxExpLookup[GetUnitSupportLevel(unit, num)];

	if (exp == SUPPORT_EXP_A)
		return FALSE;

	return (exp == maxExp) ? TRUE : FALSE;
}

LYN_REPLACE_CHECK(InitUnitsupports);
void InitUnitsupports(struct Unit *unit)
{
	u8 *supp1, *supp2;
	int i, count = GetUnitSupporterCount(unit);

	for (i = 0; i < count; ++i) {
		struct Unit *other = GetUnitSupporterUnit(unit, i);

		if (!other)
			continue;

		supp1 = GetUnitBwlSupports(UNIT_CHAR_ID(unit));
		supp2 = GetUnitBwlSupports(UNIT_CHAR_ID(other));

		if (!supp1 || !supp2)
			continue;

		supp2[GetUnitSupporterNum(other, unit->pCharacterData->number)] = 0;
		supp1[i] = 0;
	}
}

LYN_REPLACE_CHECK(UnitLoadSupports);
void UnitLoadSupports(struct Unit *unit)
{
	int count = GetUnitSupporterCount(unit);
	u8 *supp = GetUnitBwlSupports(UNIT_CHAR_ID(unit));

	if (supp) {
		int i;

		CpuFill16(0, supp, UNIT_SUPPORT_MAX_COUNT);

		for (i = 0; i < count; ++i)
			supp[i] = GetUnitSupporterInitialExp(unit, i);
	}
}

//! FE8U = 0x0808371C
LYN_REPLACE_CHECK(StartSupportTalk);
void StartSupportTalk(u8 pidA, u8 pidB, int rank) {
    struct SupportTalkEnt* ent = GetSupportTalkEntry(pidA, pidB);
    
    /* If this is an A rank support, set up slot 4 for item rewards based on the support partners */
#ifdef CONFIG_SUPPORT_REWARDS
    if (rank == SUPPORT_LEVEL_A)
    {
		int item = 0;

		switch (pidA) 
		{
			case CHARACTER_EIRIKA:
			case CHARACTER_SETH:
			case CHARACTER_GILLIAM:
			case CHARACTER_FRANZ:
			case CHARACTER_MOULDER:
			case CHARACTER_VANESSA:
			case CHARACTER_ROSS:
			case CHARACTER_NEIMI:
			case CHARACTER_COLM:
			case CHARACTER_GARCIA:
			case CHARACTER_INNES:
			case CHARACTER_LUTE:
			case CHARACTER_NATASHA:
			case CHARACTER_CORMAG:
			case CHARACTER_EPHRAIM:
			case CHARACTER_FORDE:
			case CHARACTER_KYLE:
			case CHARACTER_AMELIA:
			case CHARACTER_ARTUR:
			case CHARACTER_GERIK:
			case CHARACTER_TETHYS:
			case CHARACTER_MARISA:
			case CHARACTER_SALEH:
			case CHARACTER_EWAN:
			case CHARACTER_LARACHEL:
			case CHARACTER_DOZLA:
			case CHARACTER_ENEMY_1B:
			case CHARACTER_RENNAC:
			case CHARACTER_DUESSEL:
			case CHARACTER_MYRRH:
			case CHARACTER_KNOLL:
			case CHARACTER_JOSHUA:
			case CHARACTER_SYRENE:
			case CHARACTER_TANA:
			case CHARACTER_LYON:
			case CHARACTER_ORSON:
			case CHARACTER_ORSON_CH5X:
			case CHARACTER_GLEN:
			case CHARACTER_SELENA:
			case CHARACTER_VALTER:
			case CHARACTER_RIEV:
			case CHARACTER_CAELLACH:
			case CHARACTER_FADO:
			case CHARACTER_ISMAIRE:
			case CHARACTER_HAYDEN:
				item = ITEM_LANCE_KILLER;
		}


        gEventSlots[EVT_SLOT_3] = item;
		gEventSlots[EVT_SLOT_4] = SUPPORT_EXP_A;
    }
#endif

    if (ent) {
        CallMapSupportEvent(
            GetSupportTalkSong(pidA, pidB, rank),
            ent->msgSupports[rank - 1]
        );

        UpdateBestGlobalSupportValue(pidA, pidB, rank);
    }

    return;
}