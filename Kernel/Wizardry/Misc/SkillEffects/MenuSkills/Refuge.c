#include "common-chax.h"
#include "kernel-lib.h"
#include "map-anims.h"
#include "skill-system.h"
#include "event-rework.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "unit-expa.h"
#include "action-expa.h"

#ifdef CONFIG_REFUGE_FEATURE
static s8 CanUnitRefuge(struct Unit* actor, struct Unit* target) {
    int actorCon  = UNIT_CON(actor);
    int targetAid = GetUnitAid(target);

    return (actorCon <= targetAid) ? TRUE : FALSE;
}

static void TryAddUnitToRefugeTargetList(struct Unit* unit) {

    if (!IsSameAllegiance(gSubjectUnit->index, unit->index)) {
        return;
    }

    if (gSubjectUnit->pClassData->number == CLASS_PHANTOM || unit->pClassData->number == CLASS_PHANTOM) {
        return;
    }

    if (unit->statusIndex == UNIT_STATUS_BERSERK) {
        return;
    }

    if (unit->state & (US_RESCUING | US_RESCUED)) {
        return;
    }

    if (!CanUnitRefuge(gSubjectUnit, unit)) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);

    return;
}

static void MakeRefugeTargetList(struct Unit* unit) {
    int x = unit->xPos;
    int y = unit->yPos;

    gSubjectUnit = unit;

    BmMapFill(gBmMapRange, 0);

    ForEachAdjacentUnit(x, y, TryAddUnitToRefugeTargetList);

    return;
}

u8 Refuge_Usability(const struct MenuItemDef * def, int number)
{
    
#ifndef CONFIG_REFUGE_FEATURE
    return MENU_NOTSHOWN;
#endif

    if (gActiveUnit->state & US_HAS_MOVED) {
        return MENU_NOTSHOWN;
    }

    if (gActiveUnit->state & (US_IN_BALLISTA | US_RESCUING)) {
        return MENU_NOTSHOWN;
    }

    MakeRefugeTargetList(gActiveUnit);

    if (GetSelectTargetCount() == 0) {
        return MENU_NOTSHOWN;
    }

    return MENU_ENABLED;
}

static u8 Refuge_OnSelectTarget(ProcPtr proc, struct SelectTarget * target)
{
    gActionData.targetIndex = target->uid;

    gActionData.xOther = target->x;
    gActionData.yOther = target->y;

    HideMoveRangeGraphics();

    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);

    Action_Refuge(proc);

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A |
        TARGETSELECTION_ACTION_CLEARBGS;
}

static void MakeRefugeTargetListForAdjacentAlly(struct Unit * unit)
{
    InitTargets(0, 0);
    int x = unit->xPos;
    int y = unit->yPos;

    gSubjectUnit = unit;

    BmMapFill(gBmMapRange, 0);

    ForEachAdjacentUnit(x, y, TryAddUnitToRefugeTargetList);

    return;
}

u8 Refuge_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
        return MENU_ACT_SND6B;

    ClearBg0Bg1();

    MakeRefugeTargetListForAdjacentAlly(gActiveUnit);
    BmMapFill(gBmMapMovement, -1);

    StartSubtitleHelp(
        NewTargetSelection_Specialized(&gSelectInfo_PutTrap, Refuge_OnSelectTarget),
        GetStringFromIndex(MSG_SKILL_Common_Target));

    PlaySoundEffect(0x6A);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

bool Action_Refuge(ProcPtr parent)
{
    struct Unit * targetUnit = GetUnit(gActionData.targetIndex);

    /* Set the states of both units */
    targetUnit->state  |= US_RESCUING;
    gActiveUnit->state |= US_RESCUED | US_HIDDEN;

    targetUnit->rescue = gActiveUnit->index;
    gActiveUnit->rescue = targetUnit->index;

    /* Give the new active unit their old target unit map coordinates */
    gActionData.xMove = gActionData.xOther;
    gActionData.yMove = gActionData.yOther;

    gActionData.subjectIndex = targetUnit->index;
    gActionData.targetIndex = gActiveUnit->index;

    /* Make the target unit the active unit */
    gActiveUnit = targetUnit;

    if (!(targetUnit->state & US_HAS_MOVED))
        gActionDataExpa.refrain_action = true;

    return true;
}
#endif