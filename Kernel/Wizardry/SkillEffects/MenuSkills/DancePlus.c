#include "common-chax.h"
#include "weapon-range.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "strmag.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

static void MakeTargetListForRangedRefresh(struct Unit* unit) {
    gSubjectUnit = unit;

    BmMapFill(gBmMapRange, 0);

    ForEachUnitInMagBy2Range(TryAddUnitToRefreshTargetList);

    return;
}

#if defined(SID_DancePlus) && (COMMON_SKILL_VALID(SID_DancePlus))
u8 DancePlus_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (!HasSelectTarget(gActiveUnit, MakeTargetListForRangedRefresh))
        return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

int DancePlus_Hover(struct MenuProc * menu, struct MenuItemProc * item)
{
    BmMapFill(gBmMapMovement, -1);
    int range = GetUnitMagic(gActiveUnit) / 2;
    GenerateUnitStandingReachRange(gActiveUnit, range);
    MapAddInBoundedRange(gActiveUnit->xPos, gActiveUnit->yPos, 1, range);
    DisplayMoveRangeGraphics(MOVLIMITV_MMAP_BLUE | MOVLIMITV_RMAP_GREEN);
    return 0;
}

int DancePlus_Unhover(struct MenuProc * menu, struct MenuItemProc * menuItem)
{
    HideMoveRangeGraphics();
    return 0;
}

static u8 DancePlus_OnSelectTarget(ProcPtr proc, struct SelectTarget * target)
{
    struct Unit * targetUnit = GetUnitAtPosition(target->x, target->y);

    targetUnit->state &= ~(US_UNSELECTABLE | US_HAS_MOVED | US_HAS_MOVED_AI);
    
    // return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
    gActionData.unk08 = SID_DancePlus;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

u8 DancePlus_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_No_Allies);
        return MENU_ACT_SND6B;
    }

    ClearBg0Bg1();

    MakeTargetListForRangedRefresh(gActiveUnit);
    BmMapFill(gBmMapMovement, -1);

    StartSubtitleHelp(
        NewTargetSelection_Specialized(&gSelectInfo_Dance, DancePlus_OnSelectTarget),
        GetStringFromIndex(0x87E));

    PlaySoundEffect(0x6A);

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void callback_anim(ProcPtr proc)
{
    PlaySoundEffect(0x269);
    Proc_StartBlocking(ProcScr_DanceringAnim, proc);

    BG_SetPosition(
        BG_0,
        -SCREEN_TILE_IX(gActiveUnit->xPos - 1),
        -SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

static void callback_exec(ProcPtr proc)
{

}

bool Action_DancePlus(ProcPtr parent)
{
    NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
    return true;
}
#endif
