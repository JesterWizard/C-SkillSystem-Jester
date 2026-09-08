#include "common-chax.h"
#include "kernel-lib.h"
#include "map-anims.h"
#include "skill-system.h"
#include "event-rework.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "unit-expa.h"
#include "action-expa.h"
#include "strmag.h"
#include "bmtarget.h"
#include "playst-expa.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/macros.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_Transcendence) && (COMMON_SKILL_VALID(SID_Transcendence))

u8 Transcendence_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_Transcendence_Used))
        return MENU_NOTSHOWN;

    if (!HasSelectTarget(gActiveUnit, MakeTargetListForAdjacentNonBossEnemies))
		return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

static u8 Transcendence_OnSelectedTarget(ProcPtr proc, struct SelectTarget * target)
{
    gActionData.targetIndex = target->uid;

    gActionData.xOther = target->x;
    gActionData.yOther = target->y;

    HideMoveRangeGraphics();

    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);

    PlayStExpa_SetBit(PLAYSTEXPA_BIT_Transcendence_Used);

    gActionData.unk08 = SID_Transcendence;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

u8 Transcendence_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_SKILL_Transform_FRtext);
        return MENU_ACT_SND6B;
    }

    ClearBg0Bg1();

    MakeTargetListForAdjacentNonBossEnemies(gActiveUnit);
    BmMapFill(gBmMapMovement, -1);

    StartSubtitleHelp(
        NewTargetSelection_Specialized(&gSelectInfo_PutTrap, Transcendence_OnSelectedTarget),
        GetStringFromIndex(MSG_SKILL_Common_Target));

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
    struct Unit * targetUnit = GetUnit(gActionData.targetIndex);
    ApplyUnitDefaultPromotion(targetUnit);
    targetUnit->state |= US_DROP_ITEM;
}

bool Action_Transcendence(ProcPtr parent)
{
    NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
    return true;
}
#endif