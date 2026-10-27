#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "unit-expa.h"
#include "action-expa.h"
#include "bwl.h"
#include "jester_headers/miscellaneous.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_VictorySmite) && (COMMON_SKILL_VALID(SID_VictorySmite))

/* SKILL_EFF0 damage for every SKILL_EFF1 battles the unit has won. */
static int GetVictorySmiteDamage(struct Unit *unit)
{
    struct NewBwl *bwl = GetNewBwl(UNIT_CHAR_ID(unit));

    if (!bwl || SKILL_EFF1(SID_VictorySmite) == 0)
        return 0;

    return (bwl->winAmt / SKILL_EFF1(SID_VictorySmite)) * SKILL_EFF0(SID_VictorySmite);
}

u8 VictorySmite_Usability(const struct MenuItemDef *def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (!HasSelectTarget(gActiveUnit, MakeTargetListForAdjacentEnemies))
        return MENU_NOTSHOWN;

    if (GetVictorySmiteDamage(gActiveUnit) <= 0)
        return MENU_DISABLED;

    return MENU_ENABLED;
}

static u8 VictorySmite_OnSelectTarget(ProcPtr proc, struct SelectTarget *target)
{
    gActionData.targetIndex = target->uid;

    gActionData.xOther = target->x;
    gActionData.yOther = target->y;

    HideMoveRangeGraphics();

    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);

    gActionData.unk08 = SID_VictorySmite;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

u8 VictorySmite_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
        return MENU_ACT_SND6B;
    }

    ClearBg0Bg1();

    MakeTargetListForAdjacentEnemies(gActiveUnit);
    BmMapFill(gBmMapMovement, -1);

    StartSubtitleHelp(
        NewTargetSelection_Specialized(&gSelectInfo_PutTrap, VictorySmite_OnSelectTarget),
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

static void ShowActiveUnit(ProcPtr proc)
{
    ShowUnitSprite(gActiveUnit);
}

static void VictorySmite_KillCheck(ProcPtr proc)
{
    struct Unit *target = GetUnit(gActionData.targetIndex);

    if (target->curHP <= 0)
        UnitKill(target);
}

static void callback_exec(ProcPtr proc)
{
    struct Unit *target = GetUnit(gActionData.targetIndex);
    int damage = GetVictorySmiteDamage(gActiveUnit);

    if (damage > target->curHP)
        damage = target->curHP;

    EndAllMus();
    ShowUnitSprite(gActiveUnit);

    CallMapAnim_HurtExt(proc, target, damage, ShowActiveUnit, VictorySmite_KillCheck);
}

bool Action_VictorySmite(ProcPtr parent)
{
    NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
    return true;
}

#endif
