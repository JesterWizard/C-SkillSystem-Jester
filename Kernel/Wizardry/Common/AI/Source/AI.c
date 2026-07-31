#include "common-chax.h"
#include "skill-system.h"
#include "playst-expa.h"
#include "popup.h"
#include "constants/skills.h"

static u32* const sUnitPriorityArray = (void*) gGenericBuffer;

// LYN_REPLACE_CHECK(gProcScr_CpOrder);
// struct ProcCmd const gProcScr_CpOrder[] =
// {
//     PROC_NAME("E_CPORDER"),

//     PROC_REPEAT(CpOrderMain),

//     PROC_END,
// };

// LYN_REPLACE_CHECK(gProcScr_BerserkCpOrder);
// struct ProcCmd const gProcScr_BerserkCpOrder[] =
// {
//     PROC_NAME("E_BSKORDER"),

//     PROC_CALL(CpOrderBerserkInit),
//     PROC_REPEAT(CpOrderFunc_End),

//     PROC_END,
// };

// static ProcFunc const sCpOrderFuncList[] =
// {
//     CpOrderFunc_BeginDecide,
//     CpOrderFunc_End,
// };

// LYN_REPLACE_CHECK(CpOrderMain);
// void CpOrderMain(ProcPtr proc)
// {
//     sCpOrderFuncList[gAiState.orderState++](proc);
// }

static int GetRearmWeapon(int weaponType, int weaponRank)
{
    switch (weaponType)
    {
        case ITYPE_SWORD:
            return (weaponRank == WPN_EXP_E) ? ITEM_SWORD_IRON :
                   (weaponRank == WPN_EXP_D) ? ITEM_SWORD_STEEL :
                   (weaponRank == WPN_EXP_C) ? ITEM_SWORD_KILLER :
                   (weaponRank == WPN_EXP_B) ? ITEM_SWORD_BRAVE :
                   (weaponRank == WPN_EXP_A) ? ITEM_SWORD_SILVER :
                   (weaponRank == WPN_EXP_S) ? ITEM_SWORD_AUDHULMA : 0;

        case ITYPE_LANCE:
            return (weaponRank == WPN_EXP_E) ? ITEM_LANCE_IRON :
                   (weaponRank == WPN_EXP_D) ? ITEM_LANCE_STEEL :
                   (weaponRank == WPN_EXP_C) ? ITEM_LANCE_KILLER :
                   (weaponRank == WPN_EXP_B) ? ITEM_LANCE_BRAVE :
                   (weaponRank == WPN_EXP_A) ? ITEM_LANCE_SILVER :
                   (weaponRank == WPN_EXP_S) ? ITEM_LANCE_VIDOFNIR : 0;

        case ITYPE_AXE:
            return (weaponRank == WPN_EXP_E) ? ITEM_AXE_IRON :
                   (weaponRank == WPN_EXP_D) ? ITEM_AXE_STEEL :
                   (weaponRank == WPN_EXP_C) ? ITEM_AXE_KILLER :
                   (weaponRank == WPN_EXP_B) ? ITEM_AXE_BRAVE :
                   (weaponRank == WPN_EXP_A) ? ITEM_AXE_SILVER :
                   (weaponRank == WPN_EXP_S) ? ITEM_AXE_GARM : 0;

        case ITYPE_BOW:
            return (weaponRank == WPN_EXP_E) ? ITEM_BOW_IRON :
                   (weaponRank == WPN_EXP_D) ? ITEM_BOW_STEEL :
                   (weaponRank == WPN_EXP_C) ? ITEM_BOW_KILLER :
                   (weaponRank == WPN_EXP_B) ? ITEM_BOW_BRAVE :
                   (weaponRank == WPN_EXP_A) ? ITEM_BOW_SILVER :
                   (weaponRank == WPN_EXP_S) ? ITEM_BOW_NIDHOGG : 0;

        case ITYPE_ANIMA:
            return (weaponRank == WPN_EXP_E) ? ITEM_ANIMA_FIRE :
                   (weaponRank == WPN_EXP_D) ? ITEM_ANIMA_THUNDER :
                   (weaponRank == WPN_EXP_C) ? ITEM_ANIMA_ELFIRE :
                   (weaponRank == WPN_EXP_B) ? ITEM_ANIMA_BOLTING :
                   (weaponRank == WPN_EXP_A) ? ITEM_ANIMA_FIMBULVETR :
                   (weaponRank == WPN_EXP_S) ? ITEM_ANIMA_EXCALIBUR : 0;

        case ITYPE_LIGHT:
            return (weaponRank == WPN_EXP_E) ? ITEM_LIGHT_LIGHTNING :
                   (weaponRank == WPN_EXP_D) ? ITEM_LIGHT_SHINE :
                   (weaponRank == WPN_EXP_C) ? ITEM_LIGHT_DIVINE :
                   (weaponRank == WPN_EXP_B) ? ITEM_LIGHT_PURGE :
                   (weaponRank == WPN_EXP_A) ? ITEM_LIGHT_AURA :
                   (weaponRank == WPN_EXP_S) ? ITEM_LIGHT_IVALDI : 0;

        case ITYPE_DARK:
            return (weaponRank == WPN_EXP_E) ? ITEM_DARK_FLUX :
                   (weaponRank == WPN_EXP_D) ? ITEM_DARK_FLUX :
                   (weaponRank == WPN_EXP_C) ? ITEM_DARK_LUNA :
                   (weaponRank == WPN_EXP_B) ? ITEM_DARK_NOSFERATU :
                   (weaponRank == WPN_EXP_A) ? ITEM_DARK_FENRIR :
                   (weaponRank == WPN_EXP_S) ? ITEM_DARK_GLEIPNIR : 0;

        case ITYPE_STAFF:
            return (weaponRank == WPN_EXP_E) ? ITEM_STAFF_HEAL :
                   (weaponRank == WPN_EXP_D) ? ITEM_STAFF_MEND :
                   (weaponRank == WPN_EXP_C) ? ITEM_STAFF_BARRIER :
                   (weaponRank == WPN_EXP_B) ? ITEM_STAFF_BERSERK :
                   (weaponRank == WPN_EXP_A) ? ITEM_STAFF_FORTIFY :
                   (weaponRank == WPN_EXP_S) ? ITEM_STAFF_LATONA : 0;

        default:
            return 0;
    }
}

static int BuildAiUnitList_Proc(ProcPtr proc) {
    int i, aiNum = 0;

    u32 faction = gPlaySt.faction;
    u32* prioIt = sUnitPriorityArray;

#ifdef CONFIG_FOURTH_ALLEGIANCE
    int factionUnitCountLut[4] = { 62, 20, 50, 20 }; // TODO: named constant for those
#else
    int factionUnitCountLut[3] = { 62, 20, 50 }; // TODO: named constant for those
#endif

    for (i = 0; i < factionUnitCountLut[faction >> 6]; ++i)
    {
        struct Unit* unit = GetUnit(faction + i + 1);

        if (!unit->pCharacterData)
            continue;

#if (defined(SID_Rearm) && (COMMON_SKILL_VALID(SID_Rearm)))
        if (SkillTester(unit, SID_Rearm))
        {
            if (GetUnitEquippedWeapon(unit) == 0)
            {
                int weaponType = GetUnitBestWRankType(unit);
                int weaponRank = UNIT_WRANK(unit, weaponType);
                int chosen_weapon = GetRearmWeapon(weaponType, weaponRank);

                if (chosen_weapon)
                    UnitAddItem(unit, MakeNewItem(chosen_weapon));
            }
        }
#endif

        if (unit->statusIndex == UNIT_STATUS_SLEEP)
            continue;

        if (unit->statusIndex == UNIT_STATUS_BERSERK)
            continue;

        if (unit->state & (US_HIDDEN | US_UNSELECTABLE | US_DEAD | US_RESCUED | US_HAS_MOVED_AI))
            continue;

        gAiState.units[aiNum] = faction + i + 1;
        *prioIt++ = GetUnitAiPriority(unit);

        aiNum++;
    }

    return aiNum;
}

LYN_REPLACE_CHECK(CpOrderBerserkInit);
void CpOrderBerserkInit(ProcPtr proc)
{
    int i, aiNum = 0;

    u32 faction = gPlaySt.faction;

#ifdef CONFIG_FOURTH_ALLEGIANCE
    int factionUnitCountLut[4] = { CONFIG_UNIT_AMT_ALLY, CONFIG_UNIT_AMT_NPC, CONFIG_UNIT_AMT_ENEMY, CONFIG_UNIT_AMT_FOURTH }; // TODO: named constant for those
#else
    int factionUnitCountLut[3] = { CONFIG_UNIT_AMT_ALLY, CONFIG_UNIT_AMT_NPC, CONFIG_UNIT_AMT_ENEMY }; // TODO: named constant for those
#endif

    for (i = 0; i < factionUnitCountLut[faction >> 6]; ++i)
    {
        struct Unit* unit = GetUnit(faction + i + 1);

        if (!unit->pCharacterData)
            continue;

        if 
        (
            unit->statusIndex != UNIT_STATUS_BERSERK &&
#if (defined(SID_Rampage) && (COMMON_SKILL_VALID(SID_Rampage)))
                !SkillTester(unit, SID_Rampage)
#endif
        )
            continue;

        if (unit->state & (US_HIDDEN | US_UNSELECTABLE | US_DEAD | US_RESCUED | US_HAS_MOVED_AI))
            continue;

        gAiState.units[aiNum++] = faction + i + 1;
    }

    if (aiNum != 0)
    {
        gAiState.units[aiNum] = 0;
        gAiState.unitIt = gAiState.units;

        AiDecideMainFunc = AiDecideMain;

        Proc_StartBlocking(gProcScr_CpDecide, proc);
    }
}

LYN_REPLACE_CHECK(CpOrderFunc_BeginDecide);
void CpOrderFunc_BeginDecide(ProcPtr proc)
{
    int unitAmt = BuildAiUnitList_Proc(proc);

    if (unitAmt != 0)
    {
        SortAiUnitList(unitAmt);

        gAiState.units[unitAmt] = 0;
        gAiState.unitIt = gAiState.units;

        AiDecideMainFunc = AiDecideMain;

        Proc_StartBlocking(gProcScr_CpDecide, proc);
    }
}

LYN_REPLACE_CHECK(GetUnitBattleAiPriority);
int GetUnitBattleAiPriority(struct Unit* unit)
{
    int i, item;

    u8 rangedAmt = 0;
    u8 meleeAmt = 0;

    for (i = 0; (i < UNIT_ITEM_COUNT) && !!(item = unit->items[i]); ++i)
    {
        if (!CanUnitUseWeapon(unit, item) && !CanUnitUseStaff(unit, item))
            continue;

        if (GetItemAttributes(item) & IA_STAFF)
        {
            return 72;
        }

        if (GetItemAttributes(item) & IA_WEAPON)
        {
            int range = GetItemMaxRange(item);

            if (range > 1)
                rangedAmt++;
            else
                meleeAmt++;
        }
    }

    if (rangedAmt != 0)
        return 40;

    if (meleeAmt != 0)
        return 20;

    return 87;
}

LYN_REPLACE_CHECK(GetUnitAiPriority);
int GetUnitAiPriority(struct Unit* unit)
{
    int priority = UNIT_MOV(unit);

    u16 lead = GetUnitLeaderCharId(unit);

    if (UNIT_CATTRIBUTES(unit) & (CA_DANCE | CA_PLAY))
    {
        /* With AI dance enabled, dancers act after every other unit
         * (even leader-grouped ones) so that there is someone worth
         * refreshing; SortAiUnitList is stable for ties */
        if (gpKernelDesignerConfig->ai_dance_use == true)
            return 0x7FFF;

        return priority - 149;
    }

#if defined(SID_Dance) && (COMMON_SKILL_VALID(SID_Dance))
    if (gpKernelDesignerConfig->ai_dance_use == true && SkillTester(unit, SID_Dance))
        return 0x7FFF;
#endif

    if (!(unit->aiFlags & AI_UNIT_FLAG_0))
    {
        priority += lead << 8;

        if (UNIT_CATTRIBUTES(unit) & CA_STEAL)
            return priority + 60;

        if ((unit->pCharacterData->number == lead) || (UNIT_CATTRIBUTES(unit) & CA_LORD))
            return priority + 87;

        priority = priority + GetUnitBattleAiPriority(unit);
    }

    return priority;
}

LYN_REPLACE_CHECK(BuildAiUnitList);
int BuildAiUnitList(void)
{
    int i, aiNum = 0;

    u32 faction = gPlaySt.faction;
    u32* prioIt = sUnitPriorityArray;

#ifdef CONFIG_FOURTH_ALLEGIANCE
    int factionUnitCountLut[4] = { CONFIG_UNIT_AMT_ALLY, CONFIG_UNIT_AMT_NPC, CONFIG_UNIT_AMT_ENEMY, CONFIG_UNIT_AMT_FOURTH }; // TODO: named constant for those
#else
    int factionUnitCountLut[3] = { CONFIG_UNIT_AMT_ALLY, CONFIG_UNIT_AMT_NPC, CONFIG_UNIT_AMT_ENEMY }; // TODO: named constant for those
#endif

    for (i = 0; i < factionUnitCountLut[faction >> 6]; ++i)
    {
        struct Unit* unit = GetUnit(faction + i + 1);

        if (!unit->pCharacterData)
            continue;

        if (unit->statusIndex == UNIT_STATUS_SLEEP)
            continue;

        if (unit->statusIndex == UNIT_STATUS_BERSERK)
            continue;

        if (unit->state & (US_HIDDEN | US_UNSELECTABLE | US_DEAD | US_RESCUED | US_HAS_MOVED_AI))
            continue;

        gAiState.units[aiNum] = faction + i + 1;
        *prioIt++ = GetUnitAiPriority(unit);

        aiNum++;
    }

    return aiNum;
}

LYN_REPLACE_CHECK(SortAiUnitList);
void SortAiUnitList(int count)
{
    int i, j;

    if (count <= 1) // Redundant check
        return;
    ++count; --count;

    // this is a bubble sort, I think

    for (i = 0; i <= count - 2; ++i)
    {
        for (j = count - 2; j >= i; --j)
        {
            if (sUnitPriorityArray[j] > sUnitPriorityArray[j+1])
            {
                // swap

                int tmp;

                tmp = sUnitPriorityArray[j];
                sUnitPriorityArray[j] = sUnitPriorityArray[j+1];
                sUnitPriorityArray[j+1] = tmp;

                tmp = gAiState.units[j];
                gAiState.units[j] = gAiState.units[j+1];
                gAiState.units[j+1] = tmp;
            }
        }
    }
}

// LYN_REPLACE_CHECK(CpOrderFunc_End);
// void CpOrderFunc_End(ProcPtr proc)
// {
//     Proc_Break(proc);
// }

//! FE8U = 0x0803A788
LYN_REPLACE_CHECK(AiFindTargetInReachByCharId);
s8 AiFindTargetInReachByCharId(int uid, struct Vec2* out) 
{
    int i;

    GenerateExtendedMovementMapOnRange(gActiveUnit->xPos, gActiveUnit->yPos, GetUnitMovementCost(gActiveUnit));
    MarkWorkingMapEdges();

    out->x = -1;

#ifdef CONFIG_FOURTH_ALLEGIANCE
    for (i = 1; i < 0xD0; i++)
#else
    for (i = 1; i < 0xC0; i++)
#endif
    {
        struct Unit* unit = GetUnit(i);

        if (!UNIT_IS_VALID(unit)) {
            continue;
        }

        if (gBmMapRange[unit->yPos][unit->xPos] > MAP_MOVEMENT_MAX) {
            continue;
        }

        if (unit->pCharacterData->number != uid) {
            continue;
        }

        if (unit->state & US_DEAD) {
            gAiState.cmd_result[0] = 1;
            return 0;
        }

        if (unit->state & US_RESCUED) {
            gAiState.cmd_result[0] = 3;
        }

        out->x = unit->xPos;
        out->y = unit->yPos;
    }

    if (out->x >= 0) {
        return 1;
    }

    if (!(GetUnitFromCharId(uid)->state & US_UNAVAILABLE)) {
        gAiState.cmd_result[0] = 4;
        return 0;
    }

    gAiState.cmd_result[0] = 1;

    return 0;
}

//! FE8U = 0x0803A878
LYN_REPLACE_CHECK(AiFindTargetInReachByClassId);
s8 AiFindTargetInReachByClassId(int classId, struct Vec2* out) {
    int i;

    u8 bestDistance = 0xff;

    GenerateExtendedMovementMapOnRange(gActiveUnit->xPos, gActiveUnit->yPos, GetUnitMovementCost(gActiveUnit));

    out->x = -1;
    
#ifdef CONFIG_FOURTH_ALLEGIANCE
    for (i = 1; i < 0xD0; i++)
#else
    for (i = 1; i < 0xC0; i++)
#endif
    {
        struct Unit* unit = GetUnit(i);

        if (!UNIT_IS_VALID(unit)) {
            continue;
        }

        if (unit->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16)) {
            continue;
        }

        if (gBmMapRange[unit->yPos][unit->xPos] > MAP_MOVEMENT_MAX) {
            continue;
        }

        if (unit->pClassData->number != classId) {
            continue;
        }

        if (bestDistance < gMapRangeSigned[unit->yPos][unit->xPos]) {
            continue;
        }

        bestDistance = gBmMapRange[unit->yPos][unit->xPos];
        out->x = unit->xPos;
        out->y = unit->yPos;
    }

    if (out->x >= 0) {
        return 1;
    }

    return 0;
}

//! FE8U = 0x08015450
extern void DisableDebuggerAiControl(void);
extern void AiPhaseClearPerformedFlags(void);

static bool ShouldAiControlPlayerPhase(void)
{
    if (AreAnyEnemyUnitDead() == 0
            || CheckFlag(EVFLAG_DEFEAT_ALL)
            || CheckFlag(EVFLAG_WIN))
    {
        DisableDebuggerAiControl();
        return false;
    }

    if (gPlaySt.config.debugControlRed || gPlaySt.config.debugControlGreen)
        return true;

#ifdef CONFIG_FOURTH_ALLEGIANCE
    if (gPlaySt.config.debugControlPurple)
        return true;
#endif

    return false;
}

LYN_REPLACE_CHECK(BmMain_StartPhase);
void BmMain_StartPhase(ProcPtr proc)
{
    AiPhaseClearPerformedFlags();

//     int phaseControl = gPlaySt.faction;
//     if (gPlaySt.faction == FACTION_RED) 
//         phaseControl = FACTION_BLUE; 

// #ifdef CONFIG_FOURTH_ALLEGIANCE
//     if (gPlaySt.faction == FACTION_GREEN) 
//         phaseControl = FACTION_PURPLE; 

//     if (gPlaySt.faction == FACTION_PURPLE) 
//         phaseControl = FACTION_BLUE; 

// #else
//     if (gPlaySt.faction == FACTION_GREEN) 
//         phaseControl = FACTION_BLUE; 
// #endif

    switch (gPlaySt.faction)
    {
    case FACTION_BLUE:
        if (gpKernelDesignerConfig->ai_player_phase == true
                || ShouldAiControlPlayerPhase())
            Proc_StartBlocking(gProcScr_CpPhase, proc);
        else
            Proc_StartBlocking(gProcScr_PlayerPhase, proc);

        break;

    case FACTION_RED:
        Proc_StartBlocking(gProcScr_CpPhase, proc);
        break;

    case FACTION_GREEN:
        if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_AssumingCommand_Used))
            Proc_StartBlocking(gProcScr_PlayerPhase, proc);
        else
            Proc_StartBlocking(gProcScr_CpPhase, proc);
        break;

#ifdef CONFIG_FOURTH_ALLEGIANCE
    case FACTION_PURPLE:
        Proc_StartBlocking(gProcScr_CpPhase, proc);
        break;
#endif
    }

    Proc_Break(proc);
}
