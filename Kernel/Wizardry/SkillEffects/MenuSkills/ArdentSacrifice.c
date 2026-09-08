#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_ArdentSacrifice) && (COMMON_SKILL_VALID(SID_ArdentSacrifice))
u8 ArdentSacrifice_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (!HasSelectTarget(gActiveUnit, MakeTargetListForAdjacentHeal))
        return MENU_DISABLED;

    if (GetUnitCurrentHp(gActiveUnit) <= SKILL_EFF0(SID_ArdentSacrifice))
        return MENU_DISABLED;

    return MENU_ENABLED;
}

static u8 ArdentSacrifice_OnSelectTarget(ProcPtr proc, struct SelectTarget * target)
{
    gActionData.targetIndex = target->uid;

    gActionData.xOther = target->x;
    gActionData.yOther = target->y;

    HideMoveRangeGraphics();

    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);

    gActionData.unk08 = SID_ArdentSacrifice;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A |
        TARGETSELECTION_ACTION_CLEARBGS;
}

u8 ArdentSacrifice_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_SKILL_ArdentSacrifice_FRtext);
        return MENU_ACT_SND6B;
    }

    ClearBg0Bg1();

    MakeTargetListForAdjacentHeal(gActiveUnit);
    BmMapFill(gBmMapMovement, -1);

    StartSubtitleHelp(
        NewTargetSelection_Specialized(&gSelectInfo_PutTrap, ArdentSacrifice_OnSelectTarget),
        GetStringFromIndex(MSG_SKILL_Common_Target));

    PlaySoundEffect(0x6A);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void callback_anim(ProcPtr proc)
{
    PlaySoundEffect(0x269);
    Proc_StartBlocking(ProcScr_DanceringAnim, proc);

    BG_SetPosition(BG_0, -SCREEN_TILE_IX(gActiveUnit->xPos - 1), -SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

static void callback_exec(ProcPtr proc)
{
    CallMapAnim_Heal(proc, GetUnit(gActionData.targetIndex), SKILL_EFF0(SID_ArdentSacrifice));

    /* Can't seem to run this and the heal anim together without crashing, so we'll just do a silent HP reduction instead */
   // CallMapAnim_HurtExt(proc, gActiveUnit, SKILL_EFF0(SID_ArdentSacrifice), NULL, NULL);

    gActiveUnit->curHP -= SKILL_EFF0(SID_ArdentSacrifice);
}


bool Action_ArdentSacrifice(ProcPtr parent)
{

    NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
    return true;
}
#endif