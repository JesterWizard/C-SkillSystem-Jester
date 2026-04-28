#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "bmtarget.h"
#include "uiselecttarget.h"
#include "jester_headers/custom-functions.h"
#include "weapon-range.h"
#include "status-getter.h"
#include "debuff.h"

extern const struct AiCombatScoreCoefficients *sCombatScoreCoefficients;

STATIC_DECLAR int AiGetDamageDealtCombatScoreComponentVanilla(void)
{
	int score;

	if (gBattleTarget.unit.curHP == 0)
		return 50;

	score = (gBattleActor.battleAttack - gBattleTarget.battleDefense) * gBattleActor.battleEffectiveHitRate;

	if (score < 0)
		score = 0;

	score = Div(score, 100);
	score = sCombatScoreCoefficients->coeffDamageDealt * score;

	if (score > 40)
		score = 40;

	return score;
}

STATIC_DECLAR bool AiMenuSkillSetCurrentTile(u16 sid)
{
    AiSetDecision(
        gActiveUnit->xPos,
        gActiveUnit->yPos,
        CONFIG_AI_ACTION_EXPA_MenuSkill,
        0,
        sid,
        gActiveUnit->xPos,
        gActiveUnit->yPos);

    return true;
}

STATIC_DECLAR bool AiMenuSkillSetTargetFromList(u16 sid, void (*makeTargetList)(struct Unit *unit))
{
    struct SelectTarget *target;

    makeTargetList(gActiveUnit);

    if (GetSelectTargetCount() == 0)
        return false;

    target = GetTarget(0);
    if (!target)
        return false;

    AiSetDecision(
        gActiveUnit->xPos,
        gActiveUnit->yPos,
        CONFIG_AI_ACTION_EXPA_MenuSkill,
        target->uid,
        sid,
        target->x,
        target->y);

    return true;
}

#define AI_MENU_SELF_CASE(sid) case sid: return AiMenuSkillSetCurrentTile(sid)
#define AI_MENU_TARGET_CASE(sid, func) case sid: return AiMenuSkillSetTargetFromList(sid, func)

bool AiTryDoMenuSkills(void)
{
    struct SkillList *list;
    int i;

    if (gAiDecision.actionPerformed)
        return true;

    if (gpKernelDesignerConfig->menu_skill_ai_use != true)
        return false;

    list = GetUnitSkillList(gActiveUnit);
    if (!list)
        return false;

    for (i = 0; i < list->amt; ++i) {
        u16 sid = list->sid[i];
        const struct MenuItemDef *def;

        if (sid > MAX_SKILL_NUM)
            continue;

        def = gpSkillMenuInfos[sid];
        if (!COMMON_SKILL_VALID(sid) || !def || !def->isAvailable)
            continue;

        if (def->isAvailable(def, 0) != MENU_ENABLED)
            continue;

        switch (sid) {
        #if defined(SID_HealingFocus) && (COMMON_SKILL_VALID(SID_HealingFocus))
        AI_MENU_SELF_CASE(SID_HealingFocus);
        #endif

        #if defined(SID_RallyDefense) && (COMMON_SKILL_VALID(SID_RallyDefense))
        AI_MENU_SELF_CASE(SID_RallyDefense);
        #endif

        #if defined(SID_RallyLuck) && (COMMON_SKILL_VALID(SID_RallyLuck))
        AI_MENU_SELF_CASE(SID_RallyLuck);
        #endif

        #if defined(SID_RallyMagic) && (COMMON_SKILL_VALID(SID_RallyMagic))
        AI_MENU_SELF_CASE(SID_RallyMagic);
        #endif

        #if defined(SID_RallyMovement) && (COMMON_SKILL_VALID(SID_RallyMovement))
        AI_MENU_SELF_CASE(SID_RallyMovement);
        #endif

        #if defined(SID_RallyResistance) && (COMMON_SKILL_VALID(SID_RallyResistance))
        AI_MENU_SELF_CASE(SID_RallyResistance);
        #endif

        #if defined(SID_RallySkill) && (COMMON_SKILL_VALID(SID_RallySkill))
        AI_MENU_SELF_CASE(SID_RallySkill);
        #endif

        #if defined(SID_RallyStrength) && (COMMON_SKILL_VALID(SID_RallyStrength))
        AI_MENU_SELF_CASE(SID_RallyStrength);
        #endif

        #if defined(SID_RallySpectrum) && (COMMON_SKILL_VALID(SID_RallySpectrum))
        AI_MENU_SELF_CASE(SID_RallySpectrum);
        #endif

        #if defined(SID_GoddessDance) && (COMMON_SKILL_VALID(SID_GoddessDance))
        AI_MENU_SELF_CASE(SID_GoddessDance);
        #endif

        #if defined(SID_Stride) && (COMMON_SKILL_VALID(SID_Stride))
        AI_MENU_SELF_CASE(SID_Stride);
        #endif

        #if defined(SID_Songstress) && (COMMON_SKILL_VALID(SID_Songstress))
        AI_MENU_SELF_CASE(SID_Songstress);
        #endif

        #if defined(SID_DancePlus) && (COMMON_SKILL_VALID(SID_DancePlus))
        AI_MENU_SELF_CASE(SID_DancePlus);
        #endif

        #if defined(SID_Dance) && (COMMON_SKILL_VALID(SID_Dance))
        AI_MENU_TARGET_CASE(SID_Dance, MakeTargetListForRefresh);
        #endif

        #if defined(SID_Transform) && (COMMON_SKILL_VALID(SID_Transform))
        AI_MENU_SELF_CASE(SID_Transform);
        #endif

        #if defined(SID_Dismount) && (COMMON_SKILL_VALID(SID_Dismount))
        AI_MENU_SELF_CASE(SID_Dismount);
        #endif

        #if defined(SID_Bide) && (COMMON_SKILL_VALID(SID_Bide))
        AI_MENU_SELF_CASE(SID_Bide);
        #endif

        #if defined(SID_CoinFlip) && (COMMON_SKILL_VALID(SID_CoinFlip))
        AI_MENU_SELF_CASE(SID_CoinFlip);
        #endif

        #if defined(SID_DoubleTime) && (COMMON_SKILL_VALID(SID_DoubleTime))
        AI_MENU_SELF_CASE(SID_DoubleTime);
        #endif

        #if defined(SID_BravelyDefault) && (COMMON_SKILL_VALID(SID_BravelyDefault))
        AI_MENU_SELF_CASE(SID_BravelyDefault);
        #endif

        #if defined(SID_GraceOfFire) && (COMMON_SKILL_VALID(SID_GraceOfFire))
        AI_MENU_SELF_CASE(SID_GraceOfFire);
        #endif

        #if defined(SID_GraceOfWater) && (COMMON_SKILL_VALID(SID_GraceOfWater))
        AI_MENU_SELF_CASE(SID_GraceOfWater);
        #endif

        #if defined(SID_Hide) && (COMMON_SKILL_VALID(SID_Hide))
        AI_MENU_SELF_CASE(SID_Hide);
        #endif

        #if defined(SID_Fogger) && (COMMON_SKILL_VALID(SID_Fogger))
        AI_MENU_SELF_CASE(SID_Fogger);
        #endif

        #if defined(SID_Reinforce) && (COMMON_SKILL_VALID(SID_Reinforce))
        AI_MENU_SELF_CASE(SID_Reinforce);
        #endif

        #if defined(SID_Reroll) && (COMMON_SKILL_VALID(SID_Reroll))
        AI_MENU_SELF_CASE(SID_Reroll);
        #endif

        #if defined(SID_BloodyAlchemy) && (COMMON_SKILL_VALID(SID_BloodyAlchemy))
        AI_MENU_SELF_CASE(SID_BloodyAlchemy);
        #endif

        #if defined(SID_Gungnir) && (COMMON_SKILL_VALID(SID_Gungnir))
        AI_MENU_SELF_CASE(SID_Gungnir);
        #endif

        #if defined(SID_Arise) && (COMMON_SKILL_VALID(SID_Arise))
        AI_MENU_SELF_CASE(SID_Arise);
        #endif

        #if defined(SID_EmergencyExit) && (COMMON_SKILL_VALID(SID_EmergencyExit))
        AI_MENU_SELF_CASE(SID_EmergencyExit);
        #endif

        #if defined(SID_EmergencyExitPlus) && (COMMON_SKILL_VALID(SID_EmergencyExitPlus))
        AI_MENU_SELF_CASE(SID_EmergencyExitPlus);
        #endif

        #if defined(SID_LightRune) && (COMMON_SKILL_VALID(SID_LightRune))
        AI_MENU_TARGET_CASE(SID_LightRune, MakeTargetListForLightRune);
        #endif

        #if defined(SID_Mine) && (COMMON_SKILL_VALID(SID_Mine))
        AI_MENU_TARGET_CASE(SID_Mine, MakeTargetListForMine);
        #endif

        #if defined(SID_Summon) && (COMMON_SKILL_VALID(SID_Summon))
        AI_MENU_TARGET_CASE(SID_Summon, MakeTargetListForSummon);
        #endif

        #if defined(SID_ArdentSacrifice) && (COMMON_SKILL_VALID(SID_ArdentSacrifice))
        AI_MENU_TARGET_CASE(SID_ArdentSacrifice, MakeTargetListForAdjacentHeal);
        #endif

        #if defined(SID_Sacrifice) && (COMMON_SKILL_VALID(SID_Sacrifice))
        AI_MENU_TARGET_CASE(SID_Sacrifice, MakeTargetListForAdjacentHeal);
        #endif

        #if defined(SID_ReciprocalAid) && (COMMON_SKILL_VALID(SID_ReciprocalAid))
        AI_MENU_TARGET_CASE(SID_ReciprocalAid, MakeTargetListForAdjacentHeal);
        #endif

        #if defined(SID_FocusEnergy) && (COMMON_SKILL_VALID(SID_FocusEnergy))
        AI_MENU_TARGET_CASE(SID_FocusEnergy, MakeTargetListForAdjacentSameFaction);
        #endif

        #if defined(SID_AssignDecoy) && (COMMON_SKILL_VALID(SID_AssignDecoy))
        AI_MENU_TARGET_CASE(SID_AssignDecoy, MakeTargetListForAdjacentSameFaction);
        #endif

        #if defined(SID_AssignDecoyPlus) && (COMMON_SKILL_VALID(SID_AssignDecoyPlus))
        AI_MENU_TARGET_CASE(SID_AssignDecoyPlus, MakeTargetListForAdjacentSameFaction);
        #endif

        #if defined(SID_GorillaTactics) && (COMMON_SKILL_VALID(SID_GorillaTactics))
        AI_MENU_TARGET_CASE(SID_GorillaTactics, MakeTargetListForAdjacentSameFaction);
        #endif

        #if defined(SID_SpellBlade) && (COMMON_SKILL_VALID(SID_SpellBlade))
        AI_MENU_TARGET_CASE(SID_SpellBlade, MakeTargetListForAdjacentUnits);
        #endif

        #if defined(SID_Kamikaze) && (COMMON_SKILL_VALID(SID_Kamikaze))
        AI_MENU_TARGET_CASE(SID_Kamikaze, MakeTargetListForAdjacentUnits);
        #endif

        #if defined(SID_Doppleganger) && (COMMON_SKILL_VALID(SID_Doppleganger))
        AI_MENU_TARGET_CASE(SID_Doppleganger, MakeTargetListForBarrier);
        #endif

        #if defined(SID_DeathBlight) && (COMMON_SKILL_VALID(SID_DeathBlight))
        AI_MENU_TARGET_CASE(SID_DeathBlight, MakeTargetListForAdjacentEnemies);
        #endif

        #if defined(SID_Transcendence) && (COMMON_SKILL_VALID(SID_Transcendence))
        AI_MENU_TARGET_CASE(SID_Transcendence, MakeTargetListForAdjacentNonBossEnemies);
        #endif

        #if defined(SID_Persuade) && (COMMON_SKILL_VALID(SID_Persuade))
        AI_MENU_TARGET_CASE(SID_Persuade, MakeTargetListForAdjacentNonBossEnemies);
        #endif

        #if defined(SID_PersuadePlus) && (COMMON_SKILL_VALID(SID_PersuadePlus))
        AI_MENU_TARGET_CASE(SID_PersuadePlus, MakeTargetListForAdjacentNonBossEnemies);
        #endif

        #if defined(SID_WyvernCrash) && (COMMON_SKILL_VALID(SID_WyvernCrash))
        AI_MENU_TARGET_CASE(SID_WyvernCrash, MakeTargetListForAdjacentEnemies);
        #endif

        default:
            continue;
        }
    }

    return false;
}

void AiAction_MenuSkill(struct CpPerformProc *proc)
{
    gActionData.subjectIndex = gActiveUnit->index;
    gActionData.unk08 = gAiDecision.itemSlot;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;
    gActionData.targetIndex = gAiDecision.targetId;
    gActionData.xMove = gAiDecision.xMove;
    gActionData.yMove = gAiDecision.yMove;
    gActionData.xOther = gAiDecision.xTarget;
    gActionData.yOther = gAiDecision.yTarget;

    ApplyUnitAction(proc);
}

STATIC_DECLAR bool AiAction_MenuSkillIdleExt(struct CpPerformProc *proc)
{
    (void)proc;
    gActiveUnit->state &= ~US_HAS_MOVED_AI;
    gAiState.unitIt--;
    return true;
}

bool AiAction_MenuSkillIdle(struct CpPerformProc *proc)
{
    proc->func = AiAction_MenuSkillIdleExt;
    return false;
}

LYN_REPLACE_CHECK(AiGetDamageDealtCombatScoreComponent);
int AiGetDamageDealtCombatScoreComponent(void)
{
	int score = AiGetDamageDealtCombatScoreComponentVanilla();

#if defined(SID_Provoke) && (COMMON_SKILL_VALID(SID_Provoke))
	if (BattleFastSkillTester(&gBattleTarget, SID_Provoke))
		score += SKILL_EFF0(SID_Provoke);
#endif

#if defined(SID_AssignDecoy) && (COMMON_SKILL_VALID(SID_AssignDecoy))
    if (GetUnitStatusIndex(GetUnit(gBattleTarget.unit.index)) == NEW_UNIT_STATUS_DECOY)
        score += SKILL_EFF0(SID_AssignDecoy);
#elif defined(SID_AssignDecoy) && (COMMON_SKILL_VALID(SID_AssignDecoy))
    if (GetUnitStatusIndex(GetUnit(gBattleTarget.unit.index)) == NEW_UNIT_STATUS_DECOY)
        score += SKILL_EFF0(SID_AssignDecoy);
#endif

#if defined(SID_LightningRod) && (COMMON_SKILL_VALID(SID_LightningRod))
    if (BattleFastSkillTester(&gBattleTarget, SID_LightningRod))
    {
        int weapon = GetItemIndex(GetUnitEquippedWeapon(&gBattleActor.unit)); 

        switch (weapon)
        {
            case ITEM_LIGHT_PURGE:
            case ITEM_ANIMA_BOLTING:
            case ITEM_BALLISTA_REGULAR:
            case ITEM_BALLISTA_KILLER:
            case ITEM_BALLISTA_LONG:
                score += SKILL_EFF0(SID_LightningRod);
                break;
            default:
                break;
        }
    }
#endif

	return score;
}
