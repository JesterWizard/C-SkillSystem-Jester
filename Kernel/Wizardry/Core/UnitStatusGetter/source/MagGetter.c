#include "common-chax.h"
#include "skill-system.h"
#include "status-getter.h"
#include "strmag.h"
#include "constants/skills.h"
#include "bwl.h"
#include "unit-expa.h"
#include "debuff.h"
#include "jester_headers/custom-functions.h"

int _GetUnitMagic(struct Unit *unit)
{
	const StatusGetterFunc_t *it;
	int status = UNIT_MAG(unit);

#if defined(SID_Unaware) && (COMMON_SKILL_VALID(SID_Unaware))
    if (unit == GetUnit(gBattleActor.unit.index) && GetUnit(gBattleTarget.unit.index) && SkillTester(GetUnit(gBattleTarget.unit.index), SID_Unaware))
        return status;
    else if (unit == GetUnit(gBattleTarget.unit.index) && GetUnit(gBattleActor.unit.index) && SkillTester(GetUnit(gBattleActor.unit.index), SID_Unaware))
        return status;
#endif

	for (it = gpMagGetters; *it; it++)
		status = (*it)(status, unit);

    if (gpExternalMagGetters)
        status = gpExternalMagGetters(status, unit);

    status += MagTonic(unit);
    status += OmniTonic(unit);

	return status;
}

int MagGetterWeaponBonus(int status, struct Unit *unit)
{
	const struct ItemStatBonuses *bonus;
	u16 weapon = GetUnitEquippedWeapon(unit);

	if (weapon) {
		bonus = GetItemStatBonuses(weapon);

		if (bonus)
			status = status + ITEM_MAG_BONUS(bonus);
	}

	return status;
}

int MagGetterSkills(int status, struct Unit *unit)
{
	int cur_hp = GetUnitCurrentHp(unit);
	int max_hp = GetUnitMaxHp(unit);

    int originalStatus = status;

    FORCE_DECLARE struct NewBwl * bwl = GetNewBwl(UNIT_CHAR_ID(unit));

#if defined(SID_LifeAndDeath) && (COMMON_SKILL_VALID(SID_LifeAndDeath))
	if (SkillTester(unit, SID_LifeAndDeath))
		status += SKILL_EFF0(SID_LifeAndDeath);
#endif

#if defined(SID_MagBonus) && (COMMON_SKILL_VALID(SID_MagBonus))
	if (SkillTester(unit, SID_MagBonus))
		status += SKILL_EFF0(SID_MagBonus);
#endif

#if defined(SID_DefiantMag) && (COMMON_SKILL_VALID(SID_DefiantMag))
	if (SkillTester(unit, SID_DefiantMag))
		if ((cur_hp * 4) < max_hp)
			status += SKILL_EFF0(SID_DefiantMag);
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

#if defined(SID_LuckySeven) && (COMMON_SKILL_VALID(SID_LuckySeven))
	if (SkillTester(unit, SID_LuckySeven) && (gPlaySt.chapterTurnNumber & 0x7) == LUCKY7_MAG)
		status += SKILL_EFF0(SID_LuckySeven);
#endif

#if defined(SID_MagicBoost) && (COMMON_SKILL_VALID(SID_MagicBoost))
	if (SkillTester(unit, SID_MagicBoost)) {
		int __buf = SKILL_EFF0(SID_MagicBoost);

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

#if (defined(SID_TakerMagic) && (COMMON_SKILL_VALID(SID_TakerMagic)))
    if (SkillTester(unit, SID_TakerMagic) && gpKernelDesignerConfig->reset_bwl_stats_each_chapter == true)
    {
        int takerBoost = bwl->winAmt * SKILL_EFF0(SID_TakerMagic);
        
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

	if (cur_hp == max_hp) {
#if defined(SID_PushMagic) && (COMMON_SKILL_VALID(SID_PushMagic))
		if (SkillTester(unit, SID_PushMagic))
			status += SKILL_EFF0(SID_PushMagic);
#endif

#if defined(SID_PushSpectrum) && (COMMON_SKILL_VALID(SID_PushSpectrum))
		if (SkillTester(unit, SID_PushSpectrum))
			status += SKILL_EFF0(SID_PushSpectrum);
#endif
	}

#if (defined(SID_PairUp) && (COMMON_SKILL_VALID(SID_PairUp)))
    if (SkillTester(unit, SID_PairUp))
        if (unit->state & US_RESCUING)
            status += Div(_GetUnitMagic(GetUnit(unit->rescue)) * SKILL_EFF0(SID_PairUp), 100);
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

	return status;
}

int MagPsychUpCheck(int status, struct Unit *unit)
{
   FORCE_DECLARE int stolen_status = 0;

#if (defined(SID_PsychUp) && (COMMON_SKILL_VALID(SID_PsychUp)))
    if (unit == GetUnit(gBattleActor.unit.index) && SkillTester(unit, SID_PsychUp))
    {
        stolen_status = MagGetterWeaponBonus(0, GetUnit(gBattleTarget.unit.index)) + 
                        MagGetterSkills(0, GetUnit(gBattleTarget.unit.index)) +
                        GetStatDebuffMsgBuf(GetUnit(gBattleTarget.unit.index))->mag;

        return status + stolen_status;
    }
    else if (unit == GetUnit(gBattleTarget.unit.index) && SkillTester(unit, SID_PsychUp))
    {
        stolen_status = MagGetterWeaponBonus(0, GetUnit(gBattleTarget.unit.index)) + 
                        MagGetterSkills(0, GetUnit(gBattleTarget.unit.index)) +
                        GetStatDebuffMsgBuf(GetUnit(gBattleTarget.unit.index))->mag;

        return status + stolen_status;
    }
#endif

    if (GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_ENFEEBLE)
    {
        status -= GetUnitStatusDuration(unit) * 2;
    };

    return status;
}

int MagGetterStaffBoost(int status, struct Unit *unit)
{
    if (unit->boostType == 1)
	    return status + unit->barrierDuration;

    if (unit->boostType == 7)
	    return status + unit->barrierDuration;

    return status;
}