#include "common-chax.h"
#include "skill-system.h"
#include "status-getter.h"
#include "constants/skills.h"
#include "bwl.h"
#include "unit-expa.h"
#include "debuff.h"
#include "jester_headers/custom-functions.h"

int _GetUnitPower(struct Unit *unit)
{
	const StatusGetterFunc_t *it;
	int status = unit->pow;

#if defined(SID_Unaware) && (COMMON_SKILL_VALID(SID_Unaware))
    if (unit == GetUnit(gBattleActor.unit.index) && GetUnit(gBattleTarget.unit.index) && SkillTester(GetUnit(gBattleTarget.unit.index), SID_Unaware))
        return status;
    else if (unit == GetUnit(gBattleTarget.unit.index) && GetUnit(gBattleActor.unit.index) && SkillTester(GetUnit(gBattleActor.unit.index), SID_Unaware))
        return status;
#endif

	for (it = gpPowGetters; *it; it++)
		status = (*it)(status, unit);

	if (gpExternalPowGetters)
		status = gpExternalPowGetters(status, unit);

    status += PowTonic(unit);
    status += OmniTonic(unit);

    if (gpKernelDesignerConfig->fe8_rewritten_specific_changes == true)
    {
        if (unit->pCharacterData->number == CHARACTER_SETH)
            status -= gPlaySt.chapterIndex < 5 ? 5 - gPlaySt.chapterIndex : 0;
    }

	return status;
}

/* Hooks */
int PowGetterWeaponBonus(int status, struct Unit *unit)
{
	u16 weapon = GetUnitEquippedWeapon(unit);

	status += GetItemPowBonus(weapon);
	return status;
}

int PowGetterSkills(int status, struct Unit *unit)
{
	int cur_hp = GetUnitCurrentHp(unit);
	int max_hp = GetUnitMaxHp(unit);

    int originalStatus = status;

    FORCE_DECLARE bool hugePowerPlus = false;
    FORCE_DECLARE struct NewBwl * bwl = GetNewBwl(UNIT_CHAR_ID(unit));

#if defined(SID_LifeAndDeath) && (COMMON_SKILL_VALID(SID_LifeAndDeath))
	if (SkillTester(unit, SID_LifeAndDeath))
		status += SKILL_EFF0(SID_LifeAndDeath);
#endif

#if defined(SID_StrBonus) && (COMMON_SKILL_VALID(SID_StrBonus))
	if (SkillTester(unit, SID_StrBonus))
		status += SKILL_EFF0(SID_StrBonus);
#endif

#if defined(SID_DefiantStr) && (COMMON_SKILL_VALID(SID_DefiantStr))
	if (SkillTester(unit, SID_DefiantStr))
		if ((cur_hp * 4) < max_hp)
			status += SKILL_EFF0(SID_DefiantStr);
#endif

#if defined(SID_Fury) && (COMMON_SKILL_VALID(SID_Fury))
	if (SkillTester(unit, SID_Fury))
		status += SKILL_EFF0(SID_Fury);
#endif

#if defined(SID_FuryPlus) && (COMMON_SKILL_VALID(SID_FuryPlus))
	if (SkillTesterPlus(unit, SID_FuryPlus))
		status += SKILL_EFF0(SID_FuryPlus);
#endif

#if defined(SID_FortressDef) && (COMMON_SKILL_VALID(SID_FortressDef))
	if (SkillTester(unit, SID_FortressDef))
		status += SKILL_EFF0(SID_FortressDef);
#endif

#if defined(SID_FortressRes) && (COMMON_SKILL_VALID(SID_FortressRes))
	if (SkillTester(unit, SID_FortressRes))
		status += SKILL_EFF0(SID_FortressRes);
#endif

#if defined(SID_RuinedBlade) && (COMMON_SKILL_VALID(SID_RuinedBlade))
	if (SkillTester(unit, SID_RuinedBlade))
		status += SKILL_EFF0(SID_RuinedBlade);
#endif

#if defined(SID_HeavyBladePlus) && (COMMON_SKILL_VALID(SID_HeavyBladePlus))
	if (SkillTesterPlus(unit, SID_HeavyBladePlus))
		status += SKILL_EFF0(SID_HeavyBladePlus);
#endif

#if defined(SID_LuckySeven) && (COMMON_SKILL_VALID(SID_LuckySeven))
	if (SkillTester(unit, SID_LuckySeven) && (gPlaySt.chapterTurnNumber & 0x7) == LUCKY7_POW)
		status += SKILL_EFF0(SID_LuckySeven);
#endif

#if defined(SID_StrengthBoost) && (COMMON_SKILL_VALID(SID_StrengthBoost))
	if (SkillTester(unit, SID_StrengthBoost)) {
		int __buf = SKILL_EFF0(SID_StrengthBoost);

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

#if (defined(SID_TakerStrength) && (COMMON_SKILL_VALID(SID_TakerStrength)))
    if (SkillTester(unit, SID_TakerStrength) && gpKernelDesignerConfig->reset_bwl_stats_each_chapter == true)
    {
        int takerBoost = bwl->winAmt * SKILL_EFF0(SID_TakerStrength);
        
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
    if (SkillTester(unit, SID_Freelancer) && unit->ranks[ITYPE_AXE] >= 1)
        status += SKILL_EFF0(SID_Freelancer);
#endif

	if (cur_hp == max_hp) {
#if defined(SID_PushStrength) && (COMMON_SKILL_VALID(SID_PushStrength))
		if (SkillTester(unit, SID_PushStrength))
			status += SKILL_EFF0(SID_PushStrength);
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

#if (defined(SID_HugePowerPlus) && (COMMON_SKILL_VALID(SID_HugePowerPlus)))
    if (SkillTesterPlus(unit, SID_HugePowerPlus))
    {
        status += unit->pow / 2;
        hugePowerPlus = true;
    }
#endif

#if (defined(SID_HugePower) && (COMMON_SKILL_VALID(SID_HugePower)))
    if (SkillTester(unit, SID_HugePower) && !hugePowerPlus)
    {
        if (cur_hp == max_hp)
            status += unit->pow / 2;
    }
#endif

#if (defined(SID_Rampage) && (COMMON_SKILL_VALID(SID_Rampage)))
    if (SkillTester(unit, SID_Rampage))
        status += unit->pow / 2;
#endif

#if (defined(SID_PairUp) && (COMMON_SKILL_VALID(SID_PairUp)))
    if (SkillTester(unit, SID_PairUp))
        if (unit->state & US_RESCUING)
            status += Div(_GetUnitPower(GetUnit(unit->rescue)) * SKILL_EFF0(SID_PairUp), 100);
#endif

#if defined(SID_Sellsword) && (COMMON_SKILL_VALID(SID_Sellsword))
    if (SkillTester(unit, SID_Sellsword) && CheckBitUES(unit, UES_BIT_SELLSWORD_SKILL_USED))
        status += SKILL_EFF0(SID_Sellsword);
#endif

#if defined(SID_GoldenGlory) && (COMMON_SKILL_VALID(SID_GoldenGlory))
    if (SkillTester(unit, SID_GoldenGlory))
    {
        const int statBoost = gPlaySt.partyGoldAmount / 20000;
        status += statBoost > 5 ? 5 : statBoost;
    }
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

#if defined(SID_SlowStart) && (COMMON_SKILL_VALID(SID_SlowStart))
    if (SkillTester(unit, SID_SlowStart) && gPlaySt.chapterTurnNumber < 6)
        status -= Div(status * SKILL_EFF0(SID_SlowStart), 100);
#endif

    if (GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_ENFEEBLE)
    {
        status -= GetUnitStatusDuration(unit) * 2;
    };

	return status;
}

int PowPsychUpCheck(int status, struct Unit * unit)
{
    FORCE_DECLARE int stolen_status = 0;

#if (defined(SID_PsychUp) && (COMMON_SKILL_VALID(SID_PsychUp)))
    if (unit == GetUnit(gBattleActor.unit.index) && SkillTester(unit, SID_PsychUp))
    {
        stolen_status = PowGetterWeaponBonus(0, GetUnit(gBattleTarget.unit.index)) + 
                        PowGetterSkills(0, GetUnit(gBattleTarget.unit.index)) +
                        GetStatDebuffMsgBuf(GetUnit(gBattleTarget.unit.index))->pow;

        return status + stolen_status;
    }
    else if (unit == GetUnit(gBattleTarget.unit.index) && SkillTester(unit, SID_PsychUp))
    {
        stolen_status = PowGetterWeaponBonus(0, GetUnit(gBattleTarget.unit.index)) + 
                        PowGetterSkills(0, GetUnit(gBattleTarget.unit.index)) +
                        GetStatDebuffMsgBuf(GetUnit(gBattleTarget.unit.index))->pow;

        return status + stolen_status;
    }
#endif

    return status;
}

int PowGetterStaffBoost(int status, struct Unit *unit)
{
    {
        extern u8 gUnitTonicState[];

    if (gUnitTonicState[unit->index] == 0)
	    return status + unit->barrierDuration;

	if (gUnitTonicState[unit->index] == 7)
	    return status + unit->barrierDuration;
    }

    return status;
}