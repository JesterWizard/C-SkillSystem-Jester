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

#if defined(SID_SpatialShift) && (COMMON_SKILL_VALID(SID_SpatialShift))

static void TryAddUnitToSpatialShiftTargetList(struct Unit* unit) {

    if (!AreUnitsAllied(gSubjectUnit->index, unit->index))
        return;

#if defined(SID_Anchor) && (COMMON_SKILL_VALID(SID_Anchor))
    if (SkillTester(unit, SID_Anchor))
        return;
#endif

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);
}

void MakeTargetListForSpatialShift(struct Unit* unit) {
    int x = unit->xPos;
    int y = unit->yPos;
    int range;

    BmMapFill(gBmMapRange, 0);

    range = GetUnitMagic(unit) / 2;
    if (range < 1)
        range = 1;

    MapAddInRange(x, y, range, 1);
    ForEachUnitInRange(TryAddUnitToSpatialShiftTargetList);
}

u8 SpatialShift_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_SpatialShift_Used))
        return MENU_NOTSHOWN;

    if (!HasSelectTarget(gActiveUnit, MakeTargetListForSpatialShift))
        return MENU_DISABLED;

    return MENU_ENABLED;
}

STATIC_DECLAR void PrepareMenuPositionSpatialShift(void)
{
    EndAllMus();
    RefreshUnitSprites();
}

static void SpatialShift_set_target_unit(void)
{
    struct Unit * unit_tar = GetUnit(gActionData.targetIndex);
    gEventSlots[EVT_SLOT_2] = unit_tar->index;
}

static void SpatialShift_set_actor_unit(void)
{
    gEventSlots[EVT_SLOT_2] = gActiveUnit->index;
}

static void SpatialShift_set_position(void)
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

static void SpatialShift_mark_used(void)
{
    PlayStExpa_SetBit(PLAYSTEXPA_BIT_SpatialShift_Used);
}

static u8 SpatialShift_OnSelectTarget(ProcPtr proc, struct SelectTarget * target)
{
    gActionData.targetIndex = target->uid;

    gActionData.xOther = target->x;
    gActionData.yOther = target->y;

    HideMoveRangeGraphics();

    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);

    gActionData.unk08 = SID_SpatialShift;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

u8 SpatialShift_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_No_Allies);
        return MENU_ACT_SND6B;
    }

    ClearBg0Bg1();

    MakeTargetListForSpatialShift(gActiveUnit);
    BmMapFill(gBmMapMovement, -1);

    StartSubtitleHelp(
        NewTargetSelection_Specialized(&gSelectInfo_Rescue, SpatialShift_OnSelectTarget),
        GetStringFromIndex(MSG_SKILL_Common_Target));

    PlaySoundEffect(0x6A);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

STATIC_DECLAR const EventScr EventScr_MenuPositionSpatialShift[] = {

LABEL(0)
    SVAL(EVT_SLOT_B, SID_SpatialShift)
    CALL(EventScr_MuSkillAnim)
    ASMC(PrepareMenuPositionSpatialShift)
    ASMC(SpatialShift_mark_used)
    ASMC(SpatialShift_set_actor_unit)
    CALL(EventScr_UidWarpOUT)
    STAL(20)
    ASMC(SpatialShift_set_target_unit)
    CALL(EventScr_UidFlushingOUT)
    STAL(60)
    ASMC(SpatialShift_set_position)
    ASMC(SpatialShift_set_target_unit)
    CALL(EventScr_UidFlushingIN)
    STAL(20)
    ASMC(SpatialShift_set_actor_unit)
    CALL(EventScr_UidWarpIN)
    STAL(20)

LABEL(99)
    NOFADE
    ENDA
};

bool Action_SpatialShift(ProcPtr parent)
{
    KernelCallEvent(EventScr_MenuPositionSpatialShift, EV_EXEC_CUTSCENE, parent);
    return true;
}
#endif
