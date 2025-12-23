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
#include "jester_headers/class-pairs.h"

/* JESTER - Weapon ranks get set in LoadUnits with GetFreeUnit, using it here might get overwritten so I'm sort of stuck */
static void SummonMinion(int x, int y, int classId, int allegiance, int itemId)
{
    gUnitDef1.charIndex       = CHARACTER_SOLDIER_80;
    gUnitDef1.classIndex      = classId;
    gUnitDef1.autolevel       = TRUE;
    gUnitDef1.allegiance      = allegiance;
    gUnitDef1.level           = gActiveUnit->level;
    gUnitDef1.xPosition       = x;
    gUnitDef1.yPosition       = y;
    gUnitDef1.redaCount       = 0;
    gUnitDef1.redas           = NULL;
    gUnitDef1.genMonster      = FALSE;
    gUnitDef1.itemDrop        = FALSE;
    gUnitDef1.items[0]        = itemId;

    LoadUnits(&gUnitDef1);
}

static void callback_anim(ProcPtr proc)
{
    PlaySoundEffect(0x269);
    New6C_SummonGfx(proc, gEventSlots[EVT_SLOT_7], gEventSlots[EVT_SLOT_8]);
}

static void callback_exec(ProcPtr proc)
{
    SummonMinion(gEventSlots[EVT_SLOT_7], gEventSlots[EVT_SLOT_8], gEventSlots[EVT_SLOT_9], gEventSlots[EVT_SLOT_A], gEventSlots[EVT_SLOT_B]);
}

bool PostAction_Minions(ProcPtr parent)
{
    if (gActionData.unitActionType != UNIT_ACTION_COMBAT)
        return false;

    FORCE_DECLARE struct Unit * unit_act = GetUnit(gBattleActor.unit.index);
    FORCE_DECLARE struct Unit * unit_tar = GetUnit(gBattleTarget.unit.index);

#if defined(SID_Minions) && (COMMON_SKILL_VALID(SID_Minions))
    if (SkillTester(unit_act, SID_Minions))
    {
        if (!UNIT_ALIVE(unit_act))
        {
            for (int i = 0; i < (int)ARRAY_COUNT(minions); i++)
            {
                if (minions[i][0] == unit_act->pClassData->number)
                {
                    gActionData.unk08 = SID_Minions; 
                    gEventSlots[EVT_SLOT_7] = unit_act->xPos;
                    gEventSlots[EVT_SLOT_8] = unit_act->yPos;
                    gEventSlots[EVT_SLOT_9] = minions[i][1];
                    gEventSlots[EVT_SLOT_A] = unit_act->index / 0x40;
                    gEventSlots[EVT_SLOT_B] = minions[i][2];
                    NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
                    return true;
                }
            }
        }
    };
    if (SkillTester(unit_tar, SID_Minions))
    {
        if (!UNIT_ALIVE(unit_tar))
        {
            for (int i = 0; i < (int)ARRAY_COUNT(minions); i++)
            {
                if (minions[i][0] == unit_tar->pClassData->number)
                {
                    gActionData.unk08 = SID_Minions; 
                    gEventSlots[EVT_SLOT_7] = unit_tar->xPos;
                    gEventSlots[EVT_SLOT_8] = unit_tar->yPos;
                    gEventSlots[EVT_SLOT_9] = minions[i][1];
                    gEventSlots[EVT_SLOT_A] = unit_tar->index / 0x40;
                    gEventSlots[EVT_SLOT_B] = minions[i][2];
                    NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
                    return true;
                }
            }
        }
    };
#endif

    return false;

}
