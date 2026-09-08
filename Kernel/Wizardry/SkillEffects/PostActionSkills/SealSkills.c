#include "common-chax.h"
#include "debuff.h"
#include "status-getter.h"
#include "action-expa.h"
#include "unit-expa.h"
#include "skill-system.h"
#include "battle-system.h"
#include "combat-art.h"
#include "constants/skills.h"
#include "constants/combat-arts.h"

// These seal skills will only currently work on the unit's given phase, but that's fine since they only last a turn anyways

void PostAction_SealSkills(ProcPtr parent)
{
    struct Unit * subject = GetUnit(gActionData.subjectIndex);
    struct Unit * target  = GetUnit(gActionData.targetIndex);
    bool isSubject = (gActiveUnit == subject);
    bool isTarget  = (gActiveUnit == target);

    if (!isSubject && !isTarget)
        return;

#if defined(SID_FullMetalBody) && (COMMON_SKILL_VALID(SID_FullMetalBody))
    if (SkillTester(target, SID_FullMetalBody))
        return;
#endif

#if defined(SID_SealDefense) && (COMMON_SKILL_VALID(SID_SealDefense))
    if (SkillListTester(gActiveUnit, SID_SealDefense)) 
    {
        if (isSubject)
            SetUnitStatDebuff(target, UNIT_STAT_DEBUFF_SEAL_DEF);
        else
            SetUnitStatDebuff(subject, UNIT_STAT_DEBUFF_SEAL_DEF);
    }
#endif

#if defined(SID_SealLuck) && (COMMON_SKILL_VALID(SID_SealLuck))
    if (SkillListTester(gActiveUnit, SID_SealLuck)) 
    {
        if (isSubject)
            SetUnitStatDebuff(target, UNIT_STAT_DEBUFF_SEAL_LCK);
        else
            SetUnitStatDebuff(subject, UNIT_STAT_DEBUFF_SEAL_LCK);
    }
#endif

#if defined(SID_SealMagic) && (COMMON_SKILL_VALID(SID_SealMagic))
    if (SkillListTester(gActiveUnit, SID_SealMagic)) 
    {
        if (isSubject)
            SetUnitStatDebuff(target, UNIT_STAT_DEBUFF_SEAL_MAG);
        else
            SetUnitStatDebuff(subject, UNIT_STAT_DEBUFF_SEAL_MAG);
    }
#endif

#if defined(SID_SealResistance) && (COMMON_SKILL_VALID(SID_SealResistance))
    if (SkillTester(gActiveUnit, SID_SealResistance)) 
    {
        if (isSubject)
            SetUnitStatDebuff(target, UNIT_STAT_DEBUFF_SEAL_RES);
        else
            SetUnitStatDebuff(subject, UNIT_STAT_DEBUFF_SEAL_RES);
    }
#endif

#if defined(SID_SealSkill) && (COMMON_SKILL_VALID(SID_SealSkill))
    if (SkillListTester(gActiveUnit, SID_SealSkill)) 
    {
        if (isSubject)
            SetUnitStatDebuff(target, UNIT_STAT_DEBUFF_SEAL_SKL);
        else
            SetUnitStatDebuff(subject, UNIT_STAT_DEBUFF_SEAL_SKL);
    }
#endif

#if defined(SID_SealSpeed) && (COMMON_SKILL_VALID(SID_SealSpeed))
    if (SkillListTester(gActiveUnit, SID_SealSpeed)) 
    {
        if (isSubject)
            SetUnitStatDebuff(target, UNIT_STAT_DEBUFF_SEAL_SPD);
        else
            SetUnitStatDebuff(subject, UNIT_STAT_DEBUFF_SEAL_SPD);
    }
#endif

#if defined(SID_SealStrength) && (COMMON_SKILL_VALID(SID_SealStrength))
    if (SkillListTester(gActiveUnit, SID_SealStrength)) 
    {
        if (isSubject)
            SetUnitStatDebuff(target, UNIT_STAT_DEBUFF_SEAL_POW);
        else
            SetUnitStatDebuff(subject, UNIT_STAT_DEBUFF_SEAL_POW);
    }
#endif

}
