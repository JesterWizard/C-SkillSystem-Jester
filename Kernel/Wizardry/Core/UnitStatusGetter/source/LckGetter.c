#include "common-chax.h"
#include "skill-system.h"
#include "status-getter.h"
#include "constants/skills.h"
#include "bwl.h"
#include "unit-expa.h"
#include "debuff.h"
#include "jester_headers/custom-functions.h"

int _GetUnitLuck(struct Unit *unit)
{
	const StatusGetterFunc_t *it;
	int status = unit->lck;

#if defined(SID_Unaware) && (COMMON_SKILL_VALID(SID_Unaware))
    if (unit == GetUnit(gBattleActor.unit.index) && GetUnit(gBattleTarget.unit.index) && SkillTester(GetUnit(gBattleTarget.unit.index), SID_Unaware))
        return status;
    else if (unit == GetUnit(gBattleTarget.unit.index) && GetUnit(gBattleActor.unit.index) && SkillTester(GetUnit(gBattleActor.unit.index), SID_Unaware))
        return status;
#endif

	for (it = gpLckGetters; *it; it++)
		status = (*it)(status, unit);

	if (gpExternalLckGetters)
		status = gpExternalLckGetters(status, unit);

    status += LckTonic(unit);
    status += OmniTonic(unit);

	return status;
}

/* Hooks */
int LckGetterWeaponBonus(int status, struct Unit *unit)
{
	u16 weapon = GetUnitEquippedWeapon(unit);
    FORCE_DECLARE struct NewBwl * bwl = GetNewBwl(UNIT_CHAR_ID(unit));

	status += GetItemLckBonus(weapon);
	return status;
}

int LckGetterSkills(int status, struct Unit *unit)
{
	int cur_hp = GetUnitCurrentHp(unit);
	int max_hp = GetUnitMaxHp(unit);

    int originalStatus = status;

    FORCE_DECLARE struct NewBwl * bwl = GetNewBwl(UNIT_CHAR_ID(unit));

#if defined(SID_LckBonus) && (COMMON_SKILL_VALID(SID_LckBonus))
	if (SkillTester(unit, SID_LckBonus))
		status += SKILL_EFF0(SID_LckBonus);
#endif

#if defined(SID_DefiantLck) && (COMMON_SKILL_VALID(SID_DefiantLck))
	if (SkillTester(unit, SID_DefiantLck))
		if ((cur_hp * 4) < max_hp)
			status += SKILL_EFF0(SID_DefiantLck);
#endif

#if defined(SID_Fury) && (COMMON_SKILL_VALID(SID_Fury))
	if (SkillTester(unit, SID_Fury))
		status += SKILL_EFF0(SID_Fury);
#endif

#if defined(SID_FuryPlus) && (COMMON_SKILL_VALID(SID_FuryPlus))
	if (SkillTesterPlus(unit, SID_FuryPlus))
		status += SKILL_EFF0(SID_FuryPlus);
#endif

#if defined(SID_LuckySeven) && (COMMON_SKILL_VALID(SID_LuckySeven))
	if (SkillTester(unit, SID_LuckySeven) && (gPlaySt.chapterTurnNumber & 0x7) == LUCKY7_LCK)
		status += SKILL_EFF0(SID_LuckySeven);
#endif

#if defined(SID_LuckBoost) && (COMMON_SKILL_VALID(SID_LuckBoost))
	if (SkillTester(unit, SID_LuckBoost)) {
		int __buf = SKILL_EFF0(SID_LuckBoost);

		if (gPlaySt.chapterTurnNumber >= __buf)
			status += __buf;
		else
			status += gPlaySt.chapterTurnNumber;
	}
#endif

#if defined(SID_SpectrumBoost) && (COMMON_SKILL_VALID(SID_SpectrumBoost))
	if (SkillTester(unit, SID_SpectrumBoost)) {
		int __buf = SKILL_EFF0(SID_SpectrumBoost);

		if (gPlaySt.chapterTurnNumber >= __buf)
			status += __buf;
		else
			status += gPlaySt.chapterTurnNumber;
	}
#endif

#if (defined(SID_TakerLuck) && (COMMON_SKILL_VALID(SID_TakerLuck)))
    if (SkillTester(unit, SID_TakerLuck) && gpKernelDesignerConfig->reset_bwl_stats_each_chapter == true)
    {
        int takerBoost = bwl->winAmt * SKILL_EFF0(SID_TakerLuck);
        
        if (takerBoost > 10)
            status += 10;
        else   
            status += takerBoost;
    }
#endif

#if (defined(SID_TakerSpectrum) && (COMMON_SKILL_VALID(SID_TakerSpectrum)))
    if (SkillTester(unit, SID_TakerSpectrum) && gpKernelDesignerConfig->reset_bwl_stats_each_chapter == true)
    {
        int takerBoost = bwl->winAmt * SKILL_EFF0(SID_TakerSpectrum);
        
        if (takerBoost > 10)
            status += 10;
        else   
            status += takerBoost;
    }
#endif

#if defined(SID_Freelancer) && (COMMON_SKILL_VALID(SID_Freelancer))
    if (SkillTester(unit, SID_Freelancer) && unit->ranks[ITYPE_BOW] >= 1)
        status += SKILL_EFF0(SID_Freelancer);
#endif

	if (cur_hp == max_hp) {
#if defined(SID_PushLuck) && (COMMON_SKILL_VALID(SID_PushLuck))
		if (SkillTester(unit, SID_PushLuck))
			status += SKILL_EFF0(SID_PushLuck);
#endif

#if defined(SID_PushSpectrum) && (COMMON_SKILL_VALID(SID_PushSpectrum))
		if (SkillTester(unit, SID_PushSpectrum))
			status += SKILL_EFF0(SID_PushSpectrum);
#endif
	}

#if (defined(SID_PairUp) && (COMMON_SKILL_VALID(SID_PairUp)))
    if (SkillTester(unit, SID_PairUp))
        if (unit->state & US_RESCUING)
            status += Div(_GetUnitLuck(GetUnit(unit->rescue)) * SKILL_EFF0(SID_PairUp), 100);
#endif

#if defined(SID_GoldenGlory) && (COMMON_SKILL_VALID(SID_GoldenGlory))
    if (SkillTester(unit, SID_GoldenGlory))
    {
        const int statBoost = gPlaySt.partyGoldAmount / 20000;
        status += statBoost > 5 ? 5 : statBoost;
    }
#endif

#if defined(SID_Sellsword) && (COMMON_SKILL_VALID(SID_Sellsword))
    if (SkillTester(unit, SID_Sellsword) && CheckBitUES(unit, UES_BIT_SELLSWORD_SKILL_USED))
        status += SKILL_EFF0(SID_Sellsword);
#endif

#if defined(SID_SupremeOverlord) && (COMMON_SKILL_VALID(SID_SupremeOverlord))
    if (SkillTester(unit, SID_SupremeOverlord))
    {
        int deadAllies = 0;

        for (int i = UNIT_FACTION(unit) + 1; i < (UNIT_FACTION(unit) + GetFactionUnitAmount(UNIT_FACTION(unit))); i++)
        {
            if (!(GetUnit(i)->pCharacterData))
                break;
            if (!UNIT_ALIVE(GetUnit(i)))
                deadAllies += 1;
        }

        if (UNIT_FACTION(unit) != FACTION_RED)
            status += deadAllies * 3;
        else
            status += deadAllies / 3;
    }
#endif

#if defined(SID_BonusDoubler) && (COMMON_SKILL_VALID(SID_BonusDoubler))
    if (SkillTester(unit, SID_BonusDoubler))
        status += (status - originalStatus);
#endif

    if (GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_ENFEEBLE)
    {
        status -= GetUnitStatusDuration(unit) * 2;
    };

	return status;
}

int LckPsychUpCheck(int status, struct Unit *unit)
{
    FORCE_DECLARE int stolen_status = 0;

#if (defined(SID_PsychUp) && (COMMON_SKILL_VALID(SID_PsychUp)))
    if (unit == GetUnit(gBattleActor.unit.index) && SkillTester(unit, SID_PsychUp))
    {
         stolen_status = LckGetterWeaponBonus(0, GetUnit(gBattleTarget.unit.index)) + 
                         LckGetterSkills(0, GetUnit(gBattleTarget.unit.index)) +
                         GetStatDebuffMsgBuf(GetUnit(gBattleTarget.unit.index))->lck;

        return status + stolen_status;
    }
    else if (unit == GetUnit(gBattleTarget.unit.index) && SkillTester(unit, SID_PsychUp))
    {
        stolen_status = LckGetterWeaponBonus(0, GetUnit(gBattleTarget.unit.index)) + 
                        LckGetterSkills(0, GetUnit(gBattleTarget.unit.index)) +
                        GetStatDebuffMsgBuf(GetUnit(gBattleTarget.unit.index))->lck;

        return status + stolen_status;
    }
#endif

    return status;
}

int LckGetterStaffBoost(int status, struct Unit *unit)
{
    extern u8 gUnitTonicState[];

    if (gUnitTonicState[unit->index] == 4)
	    return status + unit->barrierDuration;

	if (gUnitTonicState[unit->index] == 7)
	    return status + unit->barrierDuration;

    return status;
}