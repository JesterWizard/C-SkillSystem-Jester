#include "common-chax.h"
#include "kernel-lib.h"
#include "map-anims.h"
#include "skill-system.h"
#include "event-rework.h"
#include "playst-expa.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "unit-expa.h"
#include "action-expa.h"
#include "strmag.h"
#include "bmtarget.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_SpatialShiftPlus) && (COMMON_SKILL_VALID(SID_SpatialShiftPlus))

static void TryAddUnitToSpatialShiftPlusTargetList(struct Unit* unit) {
    /* No faction restriction: any unit in range is a valid target */

#if defined(SID_Anchor) && (COMMON_SKILL_VALID(SID_Anchor))
    if (SkillTester(unit, SID_Anchor))
        return;
#endif

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);
}

static void MakeTargetListForSpatialShiftPlus(struct Unit* unit) {
    int x = unit->xPos;
    int y = unit->yPos;
    int range;

    BmMapFill(gBmMapRange, 0);

    range = GetUnitMagic(unit) / 2;
    if (range < 1)
        range = 1;

    MapAddInRange(x, y, range, 1);
    ForEachUnitInRange(TryAddUnitToSpatialShiftPlusTargetList);
}

u8 SpatialShiftPlus_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_SpatialShiftPlus_Used))
        return MENU_NOTSHOWN;

    if (!HasSelectTarget(gActiveUnit, MakeTargetListForSpatialShiftPlus))
        return MENU_DISABLED;

    return MENU_ENABLED;
}

STATIC_DECLAR void PrepareMenuPositionSpatialShiftPlus(void)
{
    EndAllMus();
    RefreshUnitSprites();
}

static void SpatialShiftPlus_set_target_unit(void)
{
    struct Unit * unit_tar = GetUnit(gActionData.targetIndex);
    gEventSlots[EVT_SLOT_2] = unit_tar->index;
}

static void SpatialShiftPlus_set_actor_unit(void)
{
    gEventSlots[EVT_SLOT_2] = gActiveUnit->index;
}

static void SpatialShiftPlus_set_position(void)
{
    struct Unit * unita = gActiveUnit;
    struct Unit * unitb = GetUnit(gActionData.targetIndex);

    int x = unita->xPos;
    int y = unita->yPos;

    unita->xPos = unitb->xPos;
    unita->yPos = unitb->yPos;

    gActionData.xMove = unitb->xPos;
    gActionData.yMove = unitb->yPos;

    unitb->xPos = x;
    unitb->yPos = y;
}

static void SpatialShiftPlus_mark_used(void)
{
    PlayStExpa_SetBit(PLAYSTEXPA_BIT_SpatialShiftPlus_Used);
}

static u8 SpatialShiftPlus_OnSelectTarget(ProcPtr proc, struct SelectTarget * target)
{
    gActionData.targetIndex = target->uid;

    gActionData.xOther = target->x;
    gActionData.yOther = target->y;

    HideMoveRangeGraphics();

    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);

    gActionData.unk08 = SID_SpatialShiftPlus;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

u8 SpatialShiftPlus_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
        return MENU_ACT_SND6B;
    }

    ClearBg0Bg1();

    MakeTargetListForSpatialShiftPlus(gActiveUnit);
    BmMapFill(gBmMapMovement, -1);

    StartSubtitleHelp(
        NewTargetSelection_Specialized(&gSelectInfo_Rescue, SpatialShiftPlus_OnSelectTarget),
        GetStringFromIndex(MSG_SKILL_Common_Target));

    PlaySoundEffect(0x6A);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

STATIC_DECLAR const EventScr EventScr_MenuPositionSpatialShiftPlus[] = {

LABEL(0)
    SVAL(EVT_SLOT_B, SID_SpatialShiftPlus)
    CALL(EventScr_MuSkillAnim)
    ASMC(PrepareMenuPositionSpatialShiftPlus)
    ASMC(SpatialShiftPlus_mark_used)
    ASMC(SpatialShiftPlus_set_actor_unit)
    CALL(EventScr_UidWarpOUT)
    STAL(20)
    ASMC(SpatialShiftPlus_set_target_unit)
    CALL(EventScr_UidFlushingOUT)
    STAL(60)
    ASMC(SpatialShiftPlus_set_position)
    ASMC(SpatialShiftPlus_set_target_unit)
    CALL(EventScr_UidFlushingIN)
    STAL(20)
    ASMC(SpatialShiftPlus_set_actor_unit)
    CALL(EventScr_UidWarpIN)
    STAL(20)

LABEL(99)
    NOFADE
    ENDA
};

bool Action_SpatialShiftPlus(ProcPtr parent)
{
    KernelCallEvent(EventScr_MenuPositionSpatialShiftPlus, EV_EXEC_CUTSCENE, parent);
    return true;
}
#endif
