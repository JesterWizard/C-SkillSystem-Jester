#include "common-chax.h"
#include "kernel-lib.h"
#include "map-anims.h"
#include "skill-system.h"
#include "bmtarget.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "unit-expa.h"
#include "action-expa.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_Drag) && (COMMON_SKILL_VALID(SID_Drag))

static bool Drag_IsValidTarget(struct Unit *actor, struct Unit *target)
{
    if (!UNIT_IS_VALID(actor) || !UNIT_IS_VALID(target))
        return false;

    if (AreUnitsAllied(actor->index, target->index))
        return false;

    if (UNIT_CATTRIBUTES(target) & CA_BOSS)
        return false;

    if (UNIT_IS_PHANTOM(actor) || UNIT_IS_PHANTOM(target))
        return false;

    if (target->statusIndex == UNIT_STATUS_BERSERK)
        return false;

    if (target->state & (US_HIDDEN | US_DEAD | US_RESCUING | US_RESCUED |
        US_IN_BALLISTA | US_BIT16))
        return false;

#if defined(SID_AidRefusal) && (COMMON_SKILL_VALID(SID_AidRefusal))
    if (SkillTester(target, SID_AidRefusal))
        return false;
#endif

    return CanUnitRescue(actor, target);
}

static void TryAddUnitToDragTargetList(struct Unit *unit)
{
    if (!Drag_IsValidTarget(gSubjectUnit, unit))
        return;

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);
}

void MakeTargetListForDrag(struct Unit *unit)
{
    InitTargets(unit->xPos, unit->yPos);
    gSubjectUnit = unit;

    BmMapFill(gBmMapRange, 0);
    ForEachAdjacentUnit(unit->xPos, unit->yPos, TryAddUnitToDragTargetList);
}

u8 Drag_Usability(const struct MenuItemDef *def, int number)
{
    (void)def;
    (void)number;

    if (gActiveUnit->state & (US_HAS_MOVED | US_IN_BALLISTA | US_RESCUING))
        return MENU_NOTSHOWN;

    MakeTargetListForDrag(gActiveUnit);

    return GetSelectTargetCount() ? MENU_ENABLED : MENU_NOTSHOWN;
}

static u8 Drag_OnSelectTarget(ProcPtr proc, struct SelectTarget *target)
{
    (void)proc;

    gActionData.targetIndex = target->uid;
    gActionData.xOther = target->x;
    gActionData.yOther = target->y;
    gActionData.unk08 = SID_Drag;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    HideMoveRangeGraphics();
    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END |
        TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

u8 Drag_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
        return MENU_ACT_SND6B;
    }

    ClearBg0Bg1();
    MakeTargetListForDrag(gActiveUnit);
    BmMapFill(gBmMapMovement, -1);

    StartSubtitleHelp(
        NewTargetSelection_Specialized(&gSelectInfo_PutTrap, Drag_OnSelectTarget),
        GetStringFromIndex(MSG_SKILL_Common_Target));

    PlaySoundEffect(0x6A);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void Drag_Anim(ProcPtr proc)
{
    (void)proc;
}

static void Drag_Exec(ProcPtr proc)
{
    struct Unit *actor = gActiveUnit;
    struct Unit *target = GetUnit(gActionData.targetIndex);

    (void)proc;

    if (!Drag_IsValidTarget(actor, target))
        return;

    UnitRescue(actor, target);
    HideUnitSprite(target);

    gActionData.xMove = actor->xPos;
    gActionData.yMove = actor->yPos;

    RefreshEntityBmMaps();
    RefreshUnitSprites();
}

bool Action_Drag(ProcPtr parent)
{
    (void)parent;

    NewMuSkillAnimOnActiveUnit(gActionData.unk08, Drag_Anim, Drag_Exec);
    return true;
}

#endif
