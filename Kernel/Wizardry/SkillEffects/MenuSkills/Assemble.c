#include "common-chax.h"
#include "kernel-lib.h"
#include "map-anims.h"
#include "weapon-range.h"
#include "status-getter.h"
#include "skill-system.h"
#include "bmidoten.h"
#include "bmtarget.h"
#include "constants/skills.h"
#include "constants/texts.h"

#if defined(SID_Assemble) && (COMMON_SKILL_VALID(SID_Assemble))

#define ASSEMBLE_RANGE 10

static int Assemble_Abs(int value)
{
    return value < 0 ? -value : value;
}

static bool Assemble_IsValidTarget(struct Unit *actor, struct Unit *target)
{
    if (!UNIT_IS_VALID(actor) || !UNIT_IS_VALID(target) || actor == target)
        return false;

    if (!UnitOnMapAvaliable(target) ||
        !AreUnitsAllied(actor->index, target->index))
        return false;

    if (target->state & (US_UNSELECTABLE | US_HAS_MOVED | US_HAS_MOVED_AI |
        US_RESCUING | US_RESCUED | US_IN_BALLISTA))
        return false;

    if (target->statusIndex == UNIT_STATUS_BERSERK ||
        target->statusIndex == UNIT_STATUS_PETRIFY)
        return false;

#if defined(SID_Anchor) && (COMMON_SKILL_VALID(SID_Anchor))
    if (SkillTester(target, SID_Anchor))
        return false;
#endif

    return Assemble_Abs(target->xPos - actor->xPos) +
        Assemble_Abs(target->yPos - actor->yPos) <= ASSEMBLE_RANGE;
}

static void TryAddUnitToAssembleTargetList(struct Unit *unit)
{
    if (Assemble_IsValidTarget(gSubjectUnit, unit))
        AddTarget(unit->xPos, unit->yPos, unit->index, 1);
}

static void MakeTargetListForAssemble(struct Unit *unit)
{
    gSubjectUnit = unit;

    BmMapFill(gBmMapRange, 0);
    MapAddInBoundedRange(unit->xPos, unit->yPos, 1, ASSEMBLE_RANGE);

    InitTargets(unit->xPos, unit->yPos);
    ForEachUnitInRange(TryAddUnitToAssembleTargetList);
}

u8 Assemble_Usability(const struct MenuItemDef *def, int number)
{
    (void)def;
    (void)number;

    if (gActiveUnit->state & (US_CANTOING | US_IN_BALLISTA | US_RESCUING))
        return MENU_NOTSHOWN;

    if (!HasSelectTarget(gActiveUnit, MakeTargetListForAssemble))
        return MENU_DISABLED;

    return MENU_ENABLED;
}

int Assemble_Hover(struct MenuProc *menu, struct MenuItemProc *item)
{
    (void)menu;
    (void)item;

    BmMapFill(gBmMapMovement, -1);
    BmMapFill(gBmMapRange, 0);
    MapAddInBoundedRange(gActiveUnit->xPos, gActiveUnit->yPos, 1, ASSEMBLE_RANGE);
    DisplayMoveRangeGraphics(MOVLIMITV_MMAP_BLUE | MOVLIMITV_RMAP_GREEN);
    return 0;
}

int Assemble_Unhover(struct MenuProc *menu, struct MenuItemProc *item)
{
    (void)menu;
    (void)item;

    HideMoveRangeGraphics();
    return 0;
}

u8 Assemble_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_No_Allies);
        return MENU_ACT_SND6B;
    }

    gActionData.unk08 = SID_Assemble;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

struct AssembleMovementProc
{
    PROC_HEADER;

    struct Unit *actor;
    struct Unit *unit;
    struct MuProc *mu;
    int destinationX;
    int destinationY;
    u8 processed[0x20];
    u8 movementScript[MOVE_CMD_MAX_COUNT];
    u8 waitTimer;
};

static const struct ProcCmd ProcScr_AssembleMovement[];

static bool Assemble_IsProcessed(
    const struct AssembleMovementProc *proc, int unitId)
{
    return (proc->processed[unitId >> 3] & (1 << (unitId & 7))) != 0;
}

static void Assemble_MarkProcessed(struct AssembleMovementProc *proc, int unitId)
{
    proc->processed[unitId >> 3] |= 1 << (unitId & 7);
}

static struct Unit *Assemble_FindNextUnit(
    struct AssembleMovementProc *proc)
{
    struct Unit *result = NULL;
    int resultDistance = ASSEMBLE_RANGE + 1;

    FOR_UNITS_ONMAP_ALL(unit, {
        int distance;

        if (Assemble_IsProcessed(proc, unit->index) ||
            !Assemble_IsValidTarget(proc->actor, unit))
            continue;

        distance = Assemble_Abs(unit->xPos - proc->actor->xPos) +
            Assemble_Abs(unit->yPos - proc->actor->yPos);

        if (!result || distance < resultDistance)
        {
            result = unit;
            resultDistance = distance;
        }
    });

    return result;
}

static bool Assemble_FindDestination(
    struct Unit *actor, struct Unit *unit, int *outX, int *outY)
{
    int currentDistance;
    int bestDistance;
    int bestMovement = -1;
    int ix;
    int iy;

    currentDistance = Assemble_Abs(unit->xPos - actor->xPos) +
        Assemble_Abs(unit->yPos - actor->yPos);
    if (currentDistance <= 1)
        return false;

    SetWorkingMoveCosts(GetUnitMovementCost(unit));
    SetWorkingBmMap(gBmMapMovement);
    GenerateMovementMap(
        unit->xPos, unit->yPos, MovGetter(unit), unit->index);

    /*
     * The active unit is hidden while its skill animation runs, so its tile
     * is not present in gBmMapUnit. Keep it blocked for forced movement.
     */
    gBmMapMovement[actor->yPos][actor->xPos] = -1;

    bestDistance = currentDistance;

    for (iy = 0; iy < gBmMapSize.y; ++iy)
    {
        for (ix = 0; ix < gBmMapSize.x; ++ix)
        {
            int movement = (s8)gBmMapMovement[iy][ix];
            int distance;

            if (movement < 0 || (ix == actor->xPos && iy == actor->yPos) ||
                gBmMapUnit[iy][ix] ||
                (gBmMapHidden[iy][ix] & HIDDEN_BIT_UNIT) ||
                !CanUnitCrossTerrain(unit, gBmMapTerrain[iy][ix]))
                continue;

            distance = Assemble_Abs(ix - actor->xPos) +
                Assemble_Abs(iy - actor->yPos);

            if (distance >= currentDistance)
                continue;

            if (distance < bestDistance ||
                (distance == bestDistance && movement > bestMovement))
            {
                bestDistance = distance;
                bestMovement = movement;
                *outX = ix;
                *outY = iy;
            }
        }
    }

    return bestMovement >= 0;
}

static bool Assemble_StartNextMovement(struct AssembleMovementProc *proc)
{
    struct Unit *unit;

    for (;;)
    {
        unit = Assemble_FindNextUnit(proc);
        if (!unit)
            return false;

        Assemble_MarkProcessed(proc, unit->index);

        if (Assemble_FindDestination(
                proc->actor, unit,
                &proc->destinationX, &proc->destinationY))
        {
            GenerateBestMovementScript(
                proc->destinationX, proc->destinationY,
                proc->movementScript);

            HideUnitSprite(unit);
            proc->mu = StartMu(unit);
            if (!proc->mu)
            {
                ShowUnitSprite(unit);
                continue;
            }

            proc->unit = unit;
            SetMuDefaultFacing(proc->mu);
            EnableMuCamera(proc->mu);
            SetMuMoveScript(proc->mu, proc->movementScript);
            proc->waitTimer = 1;
            return true;
        }
    }
}

static void Assemble_FinishMovement(struct AssembleMovementProc *proc)
{
    EndMu(proc->mu);
    proc->unit->xPos = proc->destinationX;
    proc->unit->yPos = proc->destinationY;
    ShowUnitSprite(proc->unit);
    proc->unit = NULL;
    proc->mu = NULL;

    RefreshEntityBmMaps();
    RenderBmMap();
    RefreshUnitSprites();
}

static void Assemble_MovementInit(struct AssembleMovementProc *proc)
{
    memset(proc->processed, 0, sizeof(proc->processed));
    proc->actor = gActiveUnit;
    proc->unit = NULL;
    proc->mu = NULL;
    proc->waitTimer = 0;
}

static void Assemble_MovementLoop(struct AssembleMovementProc *proc)
{
    if (proc->mu)
    {
        if (proc->waitTimer)
        {
            proc->waitTimer--;
            return;
        }

        if (IsMuActive(proc->mu))
            return;

        Assemble_FinishMovement(proc);
        return;
    }

    if (!Assemble_StartNextMovement(proc))
        Proc_Break(proc);
}

static void Assemble_MovementEnd(struct AssembleMovementProc *proc)
{
    if (proc->mu)
        EndMu(proc->mu);

    if (proc->unit)
        ShowUnitSprite(proc->unit);

    RefreshEntityBmMaps();
    RenderBmMap();
    RefreshUnitSprites();
}

static const struct ProcCmd ProcScr_AssembleMovement[] = {
    PROC_NAME("AssembleMovement"),
    PROC_SET_END_CB(Assemble_MovementEnd),
    PROC_CALL(Assemble_MovementInit),
    PROC_REPEAT(Assemble_MovementLoop),
    PROC_END,
};

static void Assemble_Execute(ProcPtr proc)
{
    Proc_StartBlocking(ProcScr_AssembleMovement, proc);
}

static void Assemble_Anim(ProcPtr proc)
{
    PlaySoundEffect(0x269);
    Proc_StartBlocking(ProcScr_DanceringAnim, proc);

    BG_SetPosition(BG_0, -SCREEN_TILE_IX(gActiveUnit->xPos - 1),
        -SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

bool Action_Assemble(ProcPtr parent)
{
    NewMuSkillAnimOnActiveUnitWithDeamon(
        parent, gActionData.unk08, Assemble_Anim, Assemble_Execute);
    return true;
}

#endif
