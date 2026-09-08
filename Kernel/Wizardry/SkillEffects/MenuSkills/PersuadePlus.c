#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"
#include "unit-expa.h"
#include "playst-expa.h"
#include "jester_headers/miscellaneous.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_PersuadePlus) && (COMMON_SKILL_VALID(SID_PersuadePlus))

u8 PersuadePlus_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_PersuadePlus_Used))
		return MENU_DISABLED;

    if (!HasSelectTarget(gActiveUnit, MakeTargetListForAdjacentNonBossEnemies))
		return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

static u8 PersuadePlus_OnSelectTarget(ProcPtr proc, struct SelectTarget * target)
{
    gActionData.targetIndex = target->uid;

    gActionData.xOther = target->x;
    gActionData.yOther = target->y;

    HideMoveRangeGraphics();

    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);

    gActionData.unk08 = SID_PersuadePlus;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

u8 PersuadePlus_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_Persuade_NO_USE);
        return MENU_ACT_SND6B;
    }

    ClearBg0Bg1();

    MakeTargetListForAdjacentNonBossEnemies(gActiveUnit);
    BmMapFill(gBmMapMovement, -1);

    StartSubtitleHelp(
        NewTargetSelection_Specialized(&gSelectInfo_PutTrap, PersuadePlus_OnSelectTarget),
        GetStringFromIndex(MSG_SKILL_Common_Target));

    PlaySoundEffect(0x6A);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void callback_anim(ProcPtr proc)
{

}

static void callback_exec(ProcPtr proc)
{
    struct Unit *unit = gActiveUnit;
    struct Unit *unit_tar = GetUnit(gActionData.targetIndex);

    if (!UNIT_ALIVE(unit) || UNIT_STONED(unit))
        return;

    if (!UNIT_ALIVE(unit_tar) || UNIT_STONED(unit_tar))
        return;

    if (SkillTester(unit, SID_PersuadePlus))
    {
        if (UNIT_FACTION(unit_tar) == FACTION_RED)
            UnitChangeFaction(unit_tar, FACTION_BLUE);
        else
            UnitChangeFaction(unit_tar, FACTION_RED);
    }
}

bool Action_PersuadePlus(ProcPtr parent)
{
    PlayStExpa_SetBit(PLAYSTEXPA_BIT_PersuadePlus_Used);

	NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
	return true;
}
#endif