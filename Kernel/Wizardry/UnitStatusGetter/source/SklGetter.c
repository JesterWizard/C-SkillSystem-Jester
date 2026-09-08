#include "common-chax.h"
#include "skill-system.h"
#include "status-getter.h"
#include "constants/skills.h"
#include "bwl.h"
#include "unit-expa.h"
#include "debuff.h"
#include "jester_headers/custom-functions.h"
#include "pair-up.h"

int _GetUnitSkill(struct Unit *unit)
{
	const StatusGetterFunc_t *it;
	int status = unit->skl;

	status = PairUp_RescueStatScale(status, unit, PAIR_UP_STAT_SKL);

	for (it = gpSklGetters; *it; it++)
		status = (*it)(status, unit);

#if defined(SID_Unaware) && (COMMON_SKILL_VALID(SID_Unaware))
    if (unit == GetUnit(gBattleActor.unit.index) && GetUnit(gBattleTarget.unit.index) && SkillTester(GetUnit(gBattleTarget.unit.index), SID_Unaware))
        return status;
    else if (unit == GetUnit(gBattleTarget.unit.index) && GetUnit(gBattleActor.unit.index) && SkillTester(GetUnit(gBattleActor.unit.index), SID_Unaware))
        return status;
#endif

    if (gpExternalSklGetters)
        status = gpExternalSklGetters(status, unit);

    status += SklTonic(unit);
    status += OmniTonic(unit);

    if (gpKernelDesignerConfig->fe8_rewritten_specific_changes == true)
    {
        if (unit->pCharacterData->number == CHARACTER_SETH)
            status -= gPlaySt.chapterIndex < 5 ? 5 - gPlaySt.chapterIndex : 0;
    }

	return status;
}

/* Hooks */
int SklGetterWeaponBonus(int status, struct Unit *unit)
{
	u16 weapon = GetUnitEquippedWeapon(unit);

	status += GetItemSklBonus(weapon);
	return status;
}

int SklGetterSkills(int status, struct Unit *unit)
{
	int cur_hp = GetUnitCurrentHp(unit);
	int max_hp = GetUnitMaxHp(unit);

    int originalStatus = status;

    FORCE_DECLARE struct NewBwl * bwl = GetNewBwl(UNIT_CHAR_ID(unit));

#if defined(SID_LifeAndDeath) && (COMMON_SKILL_VALID(SID_LifeAndDeath))
	if (SkillTester(unit, SID_LifeAndDeath))
		status += SKILL_EFF0(SID_LifeAndDeath);
#endif

#if defined(SID_SklBonus) && (COMMON_SKILL_VALID(SID_SklBonus))
	if (SkillTester(unit, SID_SklBonus))
		status += SKILL_EFF0(SID_SklBonus);
#endif

#if defined(SID_DefiantSkl) && (COMMON_SKILL_VALID(SID_DefiantSkl))
	if (SkillTester(unit, SID_DefiantSkl))
		if ((cur_hp * 4) < max_hp)
			status += SKILL_EFF0(SID_DefiantSkl);
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
	if (SkillTester(unit, SID_LuckySeven) && (gPlaySt.chapterTurnNumber & 0x7) == LUCKY7_SKL)
		status += SKILL_EFF0(SID_LuckySeven);
#endif

#if defined(SID_SkillBoost) && (COMMON_SKILL_VALID(SID_SkillBoost))
	if (SkillTester(unit, SID_SkillBoost)) {
		int __buf = SKILL_EFF0(SID_SkillBoost);

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

#if (defined(SID_TakerSkill) && (COMMON_SKILL_VALID(SID_TakerSkill)))
    if (SkillTester(unit, SID_TakerSkill) && gpKernelDesignerConfig->reset_bwl_stats_each_chapter == true)
    {
        int takerBoost = bwl->winAmt * SKILL_EFF0(SID_TakerSkill);
        
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
    if (SkillTester(unit, SID_Freelancer) && UNIT_WRANK(unit, ITYPE_LANCE) >= 1)
        status += SKILL_EFF0(SID_Freelancer);
#endif

	if (cur_hp == max_hp) {
#if defined(SID_PushSkill) && (COMMON_SKILL_VALID(SID_PushSkill))
		if (SkillTester(unit, SID_PushSkill))
			status += SKILL_EFF0(SID_PushSkill);
#endif

#if defined(SID_PushSpectrum) && (COMMON_SKILL_VALID(SID_PushSpectrum))
		if (SkillTester(unit, SID_PushSpectrum))
			status += SKILL_EFF0(SID_PushSpectrum);
#endif
	}

#if (defined(SID_Resolve) && (COMMON_SKILL_VALID(SID_Resolve)))
	if (SkillTester(unit, SID_Resolve)) {
		if ((cur_hp * 2) < max_hp)
			status += status / 2;
	}
#endif

#if (defined(SID_Rampage) && (COMMON_SKILL_VALID(SID_Rampage))) 
    if (SkillTester(unit, SID_Rampage))
            status += unit->skl / 2;
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

int SklPsychUpCheck(int status, struct Unit *unit)
{
    FORCE_DECLARE int stolen_status = 0;

#if (defined(SID_PsychUp) && (COMMON_SKILL_VALID(SID_PsychUp)))
    if (unit == GetUnit(gBattleActor.unit.index) && SkillTester(unit, SID_PsychUp))
    {
        stolen_status = SklGetterWeaponBonus(0, GetUnit(gBattleTarget.unit.index)) + 
                        SklGetterSkills(0, GetUnit(gBattleTarget.unit.index)) +
                        GetStatDebuffMsgBuf(GetUnit(gBattleTarget.unit.index))->skl;

        return status + stolen_status;
    }
    else if (unit == GetUnit(gBattleTarget.unit.index) && SkillTester(unit, SID_PsychUp))
    {
        stolen_status = SklGetterWeaponBonus(0, GetUnit(gBattleTarget.unit.index)) + 
                        SklGetterSkills(0, GetUnit(gBattleTarget.unit.index)) +
                        GetStatDebuffMsgBuf(GetUnit(gBattleTarget.unit.index))->skl;

        return status + stolen_status;
    }
#endif

    return status;
}

int SklGetterStaffBoost(int status, struct Unit *unit)
{
    extern u8 gUnitTonicState[];

    if (gUnitTonicState[unit->index] == 2)
	    return status + unit->barrierDuration;

	if (gUnitTonicState[unit->index] == 7)
	    return status + unit->barrierDuration;

    return status;
}