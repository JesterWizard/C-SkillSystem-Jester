#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "constants/songs.h"
#include "jester_headers/custom-functions.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#ifndef CONFIG_ITEM_INDEX_MAG_BOOSTER
    #define CONFIG_ITEM_INDEX_MAG_BOOSTER 0xBC
#endif

extern const struct ProcCmd ProcScr_AddExp[];

static void ExecHeal_Target(int healAmount)
{
    int healingAmount = healAmount;

#if (defined(SID_ItemLore) && (COMMON_SKILL_VALID(SID_ItemLore)))
    if (SkillTester(gActiveUnit, SID_ItemLore))
        healingAmount *= 2;
#endif

    //BattleInitItemEffect(GetUnit(gActionData.targetIndex), gActionData.itemSlotIndex);
    AddUnitHp(GetUnit(gActionData.targetIndex), healingAmount);
    gBattleHitIterator->hpChange = gBattleTarget.unit.curHP - GetUnitCurrentHp(GetUnit(gActionData.targetIndex));
    gBattleTarget.unit.curHP = GetUnitCurrentHp(GetUnit(gActionData.targetIndex));

    gActiveUnit->items[gActionData.itemSlotIndex] = GetItemAfterUse(gActiveUnit->items[gActionData.itemSlotIndex]);

    if (gBattleTarget.unit.curHP == 0)
    {
        UnitKill(GetUnit(gActionData.targetIndex));
        AddExp_Event(30);
    }
    else
    {
        AddExp_Event(10);
    }
}

static void ExecPureWaterItem_Target(void)
{
    //BattleInitItemEffect(GetUnit(gActionData.targetIndex), gActionData.itemSlotIndex);
    GetUnit(gActionData.targetIndex)->barrierDuration = 7;
    return;
}

static void ExecStatBoostItem_Target(void) {
    struct Unit* unit = GetUnit(gActionData.targetIndex);

    int item = gActiveUnit->items[gActionData.itemSlotIndex];

    const struct ItemStatBonuses* statBonuses = GetItemStatBonuses(item);

    unit->maxHP += statBonuses->hpBonus;
    unit->curHP += statBonuses->hpBonus;
    unit->pow += statBonuses->powBonus;
    unit->skl += statBonuses->sklBonus;
    unit->spd += statBonuses->spdBonus;
    unit->def += statBonuses->defBonus;
    unit->res += statBonuses->resBonus;
    unit->lck += statBonuses->lckBonus;
    unit->movBonus += statBonuses->movBonus;
    unit->conBonus += statBonuses->conBonus;

    UnitCheckStatCaps(unit);

    return;
}

static void TryAddUnitToTargetList(struct Unit* unit) {

    if (unit->state & US_RESCUED) {
        return;
    }

#if defined(SID_AcidicPlus) && (COMMON_SKILL_VALID(SID_AcidicPlus))
    if (SkillTesterPlus(gActiveUnit, SID_AcidicPlus) && gActionData.unk08 == SID_AcidicPlus)
    { 
        if (UNIT_FACTION(gSubjectUnit) != UNIT_FACTION(unit))
            AddTarget(unit->xPos, unit->yPos, unit->index, 0);
    }
#endif

#if defined(SID_Acidic) && (COMMON_SKILL_VALID(SID_Acidic))
    if (SkillTester(gActiveUnit, SID_Acidic) && gActionData.unk08 == SID_Acidic)
    { 
        if (UNIT_FACTION(gSubjectUnit) != UNIT_FACTION(unit))
            AddTarget(unit->xPos, unit->yPos, unit->index, 0);
    }
#endif

    if (UNIT_FACTION(gSubjectUnit) != UNIT_FACTION(unit))
    {
        return;
    }

    u8 activeItem = GetItemIndex(gSubjectUnit->items[gActionData.itemSlotIndex]);

    switch (activeItem)
    {
        case ITEM_VULNERARY:
        case ITEM_VULNERARY_2:
        case ITEM_ELIXIR:
            if (GetUnitCurrentHp(unit) != GetUnitMaxHp(unit))
                AddTarget(unit->xPos, unit->yPos, unit->index, 0);

            break;

        case ITEM_BOOSTER_HP:
        case ITEM_BOOSTER_POW:
        case ITEM_BOOSTER_SKL:
        case ITEM_BOOSTER_SPD:
        case ITEM_BOOSTER_LCK:
        case ITEM_BOOSTER_DEF:
        case ITEM_BOOSTER_RES:
        case ITEM_BOOSTER_MOV:
        case ITEM_BOOSTER_CON:
        case CONFIG_ITEM_INDEX_MAG_BOOSTER:
            if (CanUnitUseStatGainItem(unit, activeItem))
                AddTarget(unit->xPos, unit->yPos, unit->index, 0);

            break;

        default:
            break;
    }

    return;
}

static void ForEachUnit_Custom(int x, int y, void(*func)(struct Unit* unit), u8 minRange, u8 maxRange) {
    InitTargets(x, y);

    MapAddInRange(x, y, maxRange, 1);
    MapAddInRange(x, y, minRange, -1);

    ForEachUnitInRange(func);

    return;
}

static void MakeTargetListForUnit(struct Unit* unit, u8 minRange, u8 maxRange)  {
    int x = unit->xPos;
    int y = unit->yPos;

    gSubjectUnit = unit;

    BmMapFill(gBmMapRange, 0);

    ForEachUnit_Custom(x, y, TryAddUnitToTargetList, minRange, maxRange);

    // For the active unit
    int activeItem = GetItemIndex(gBattleActor.unit.items[gActionData.itemSlotIndex]);

    switch (activeItem)
    {
        case ITEM_VULNERARY:
        case ITEM_VULNERARY_2:
            if (GetUnitCurrentHp(gActiveUnit) != GetUnitMaxHp(gActiveUnit))
                AddTarget(gActiveUnit->xPos, gActiveUnit->yPos, gActiveUnit->index, 0);

            break;
        case ITEM_BOOSTER_HP:
        case ITEM_BOOSTER_POW:
        case ITEM_BOOSTER_SKL:
        case ITEM_BOOSTER_SPD:
        case ITEM_BOOSTER_LCK:
        case ITEM_BOOSTER_DEF:
        case ITEM_BOOSTER_RES:
        case ITEM_BOOSTER_MOV:
        case ITEM_BOOSTER_CON:
        case CONFIG_ITEM_INDEX_MAG_BOOSTER:
            if (CanUnitUseStatGainItem(gActiveUnit, activeItem))
                AddTarget(gActiveUnit->xPos, gActiveUnit->yPos, gActiveUnit->index, 0);

            break;

        default:
            break;
    }
    return;
}

static u8 Salve_OnSelectTarget(ProcPtr proc, struct SelectTarget * target)
{
    gActionData.targetIndex = target->uid;

    gActionData.xOther = target->x;
    gActionData.yOther = target->y;

    HideMoveRangeGraphics();

    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);

    gActionData.unk08 = SID_Salve;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    int activeItem = GetItemIndex(gBattleActor.unit.items[gActionData.itemSlotIndex]);

    switch (activeItem)
    {
    case ITEM_VULNERARY:
    case ITEM_VULNERARY_2:
        ExecHeal_Target(10);
        break;
    case ITEM_ELIXIR:
        ExecHeal_Target(80);
        break;
    case ITEM_PUREWATER:
        ExecPureWaterItem_Target();
        break;

    case ITEM_BOOSTER_HP:
    case ITEM_BOOSTER_POW:
    case ITEM_BOOSTER_SKL:
    case ITEM_BOOSTER_SPD:
    case ITEM_BOOSTER_LCK:
    case ITEM_BOOSTER_DEF:
    case ITEM_BOOSTER_RES:
    case ITEM_BOOSTER_MOV:
    case ITEM_BOOSTER_CON:
    case CONFIG_ITEM_INDEX_MAG_BOOSTER:
        ExecStatBoostItem_Target();
        DoItemUse(GetUnit(gActionData.targetIndex), gActiveUnit->items[gActionData.itemSlotIndex]);
        break;

    default:
        break;
    };

    UnitRemoveInvalidItems(gActiveUnit);

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

static u8 Acidic_OnSelectTarget(ProcPtr proc, struct SelectTarget * target)
{
    gActionData.targetIndex = target->uid;

    gActionData.xOther = target->x;
    gActionData.yOther = target->y;

    HideMoveRangeGraphics();

    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);

    int activeItem = GetItemIndex(gBattleActor.unit.items[gActionData.itemSlotIndex]);

    switch (activeItem)
    {
    case ITEM_VULNERARY:
    case ITEM_VULNERARY_2:
        ExecHeal_Target(-10);
        break;
    case ITEM_ELIXIR:
        ExecHeal_Target(-80);

    default:
        break;
    };

    gActiveUnit->items[gActionData.itemSlotIndex] = GetItemAfterUse(gActiveUnit->items[gActionData.itemSlotIndex]);

    UnitRemoveInvalidItems(gActiveUnit);

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

LYN_REPLACE_CHECK(ItemSubMenu_UseItem);
u8 ItemSubMenu_UseItem(struct MenuProc* menu, struct MenuItemProc* menuItem) {

    if (menuItem->availability == MENU_DISABLED) {
        MenuFrozenHelpBox(menu, GetItemCantUseMsgid(gActiveUnit, gActiveUnit->items[gActionData.itemSlotIndex]));
        return MENU_ACT_SND6B;
    }

    ClearBg0Bg1();

#if defined(SID_AcidicPlus) && (COMMON_SKILL_VALID(SID_AcidicPlus))
    if (SkillTesterPlus(gActiveUnit, SID_AcidicPlus) && gActionData.unk08 == SID_AcidicPlus)
    { 
        u8 minRange = 0;
        u8 maxRange = 3;
        u8 activeItem = GetItemIndex(gBattleActor.unit.items[gActionData.itemSlotIndex]);

        switch (activeItem)
        {
            case ITEM_VULNERARY:
            case ITEM_VULNERARY_2:
            case ITEM_ELIXIR:
                MakeTargetListForUnit(gActiveUnit, minRange, maxRange);
                BmMapFill(gBmMapMovement, -1);
                StartSubtitleHelp(NewTargetSelection_Specialized(&gSelectInfo_PutTrap, Acidic_OnSelectTarget), GetStringFromIndex(MSG_SKILL_Common_Target));
                break;

            default:
                break;
        };
    }
#endif

#if defined(SID_Acidic) && (COMMON_SKILL_VALID(SID_Acidic))
    if (SkillTester(gActiveUnit, SID_Acidic) && gActionData.unk08 == SID_Acidic)
    { 
        u8 minRange = 0;
        u8 maxRange = 1;
        u8 activeItem = GetItemIndex(gBattleActor.unit.items[gActionData.itemSlotIndex]);

        switch (activeItem)
        {
            case ITEM_VULNERARY:
            case ITEM_VULNERARY_2:
            case ITEM_ELIXIR:
                MakeTargetListForUnit(gActiveUnit, minRange, maxRange);
                BmMapFill(gBmMapMovement, -1);
                StartSubtitleHelp(NewTargetSelection_Specialized(&gSelectInfo_PutTrap, Acidic_OnSelectTarget), GetStringFromIndex(MSG_SKILL_Common_Target));
                break;

            default:
                break;
        };
    }
#endif

#if defined(SID_Salve) && (COMMON_SKILL_VALID(SID_Salve))
    if (SkillTester(gActiveUnit, SID_Salve))
    {   
        u8 minRange = 0;
        u8 maxRange = 1;
        u8 activeItem = GetItemIndex(gBattleActor.unit.items[gActionData.itemSlotIndex]);

        switch (activeItem)
        {
            case ITEM_VULNERARY:
            case ITEM_VULNERARY_2:
            case ITEM_BOOSTER_HP:
            case ITEM_BOOSTER_POW:
            case ITEM_BOOSTER_SKL:
            case ITEM_BOOSTER_SPD:
            case ITEM_BOOSTER_LCK:
            case ITEM_BOOSTER_DEF:
            case ITEM_BOOSTER_RES:
            case ITEM_BOOSTER_MOV:
            case ITEM_BOOSTER_CON:
            case CONFIG_ITEM_INDEX_MAG_BOOSTER:
                MakeTargetListForUnit(gActiveUnit, minRange, maxRange);
                BmMapFill(gBmMapMovement, -1);
                StartSubtitleHelp(NewTargetSelection_Specialized(&gSelectInfo_PutTrap, Salve_OnSelectTarget), GetStringFromIndex(MSG_SKILL_Common_Target));
                break;

            default:
                DoItemUse(gActiveUnit, gActiveUnit->items[gActionData.itemSlotIndex]);
                break;

        };
    }
    else
    {
        if (gActionData.unk08 != SID_Acidic && gActionData.unk08 != SID_AcidicPlus)
            DoItemUse(gActiveUnit, gActiveUnit->items[gActionData.itemSlotIndex]);
    }
#else
    DoItemUse(gActiveUnit, gActiveUnit->items[gActionData.itemSlotIndex]);
#endif

    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    SetTextFont(NULL);
    ResetTextFont();
    EndAllMenus();
    return MENU_ACT_SKIPCURSOR | MENU_ACT_ENDFACE;
}

LYN_REPLACE_CHECK(CanUnitUseHealItem);
s8 CanUnitUseHealItem(struct Unit* unit)
{

#if defined(SID_AcidicPlus) && (COMMON_SKILL_VALID(SID_AcidicPlus))
    if (SkillTesterPlus(gActiveUnit, SID_AcidicPlus) && gActionData.unk08 == SID_AcidicPlus)
    {
        for (int i = 0; i < ARRAY_COUNT_RANGE3x3; i++)
        {
            int _x = gActiveUnit->xPos + gVecs_3x3[i].x;
            int _y = gActiveUnit->yPos + gVecs_3x3[i].y;

            struct Unit * unit_enemy = GetUnitAtPosition(_x, _y);

            if (!UNIT_IS_VALID(unit_enemy))
                continue;

            if (AreUnitsAllied(gActiveUnit->index, unit_enemy->index))
                continue;

            if (unit_enemy->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
                continue;

            return TRUE;
        }
            
        return FALSE;
    }
#endif

#if defined(SID_Acidic) && (COMMON_SKILL_VALID(SID_Acidic))
    if (SkillTester(gActiveUnit, SID_Acidic) && gActionData.unk08 == SID_Acidic)
    {
        for (int i = 0; i < ARRAY_COUNT_RANGE1x1; i++)
        {
            int _x = gActiveUnit->xPos + gVecs_1x1[i].x;
            int _y = gActiveUnit->yPos + gVecs_1x1[i].y;

            struct Unit * unit_enemy = GetUnitAtPosition(_x, _y);

            if (!UNIT_IS_VALID(unit_enemy))
                continue;

            if (AreUnitsAllied(gActiveUnit->index, unit_enemy->index))
                continue;

            if (unit_enemy->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
                continue;

            return TRUE;
        }
            
        return FALSE;
    }
#endif

#if defined(SID_Salve) && (COMMON_SKILL_VALID(SID_Salve))
    if (SkillTester(gActiveUnit, SID_Salve))
    {
        for (int i = 0; i < ARRAY_COUNT_RANGE1x1; i++)
        {
            int _x = gActiveUnit->xPos + gVecs_1x1[i].x;
            int _y = gActiveUnit->yPos + gVecs_1x1[i].y;

            struct Unit * unit_ally = GetUnitAtPosition(_x, _y);
            if (!UNIT_IS_VALID(unit_ally))
                continue;

            if (unit_ally->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
                continue;

            if (!AreUnitsAllied(gActiveUnit->index, unit_ally->index))
                continue;

            if (GetUnitCurrentHp(unit_ally) != GetUnitMaxHp(unit_ally))
                return TRUE;
        }

        if (GetUnitCurrentHp(unit) != GetUnitMaxHp(unit))
            return TRUE;
            
        return FALSE;
    }
    else
    {
        if (GetUnitCurrentHp(unit) == GetUnitMaxHp(unit))
            return FALSE;
    }
#else
    if (GetUnitCurrentHp(unit) == GetUnitMaxHp(unit))
        return FALSE;
#endif

    return TRUE;
}