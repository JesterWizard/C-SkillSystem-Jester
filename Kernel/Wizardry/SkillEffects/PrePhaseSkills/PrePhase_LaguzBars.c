#include "common-chax.h"
#include "unit-expa.h"
#include "bwl.h"
#include "debuff.h"
#include "jester_headers/class-pairs.h"
#include "kernel-lib.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "skill-system.h"

void PrePhase_LaguzBars(ProcPtr proc)
{
    if (gpKernelDesignerConfig->laguz_bars == true)
    {
        int uid;
        struct Unit *unit;
        FORCE_DECLARE struct NewBwl * bwl;
    
        for (uid = gPlaySt.faction + 1; uid <= (gPlaySt.faction + GetFactionUnitAmount(gPlaySt.faction)); uid++)
        {
            unit = GetUnit(uid);
            if (UNIT_IS_VALID(unit))
            {
                bwl = GetNewBwl(UNIT_CHAR_ID(unit));
                if (gPlaySt.chapterTurnNumber == 1)
                    bwl->laguzBar = 10;
                else
                {
                    if (CheckUnitStatDebuff(unit, UNIT_STAT_BUFF_LAGUZ) || CheckUnitStatDebuff(unit, UNIT_STAT_BUFF_LAGUZ_HALFSHIFT))
                    {
                        if (bwl->laguzBar - 4 <= 0)
                        {
                            bwl->laguzBar = 0;
                            for (int i = 0; i < (int)ARRAY_COUNT(laguzPairs); i++)
                            {
                                if (unit->pClassData->number == laguzPairs[i][1])
                                {
                                    unit->pClassData = GetClassData(laguzPairs[i][0]);
                                    ClearUnitStatDebuff(unit, UNIT_STAT_BUFF_LAGUZ);
                                    ClearUnitStatDebuff(unit, UNIT_STAT_BUFF_LAGUZ_HALFSHIFT);
                                    break;
                                }
                            }
                        }
                        else
                            #if defined(SID_FormShift) && (COMMON_SKILL_VALID(SID_FormShift))
                                if (!SkillTester(unit, SID_FormShift))
                                    bwl->laguzBar -= 4;
                            #else
                                bwl->laguzBar -= 4;
                            #endif
                    }
                    else
                    {
                        if (bwl->laguzBar + 4 > 30)
                            bwl->laguzBar = 30;
                        else    
                            bwl->laguzBar += 4;
                    }
                }
            }
        }
    }
}
