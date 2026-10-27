#include "common-chax.h"
#include "kernel-lib.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

/* Turns a Hellscape void stays open (counted down once per turn in DecayTraps) */
#define HELLSCAPE_VOID_TURNS 3

static void Hellscape_KillCallback(ProcPtr proc)
{
    struct Trap *trap = GetTypedTrapAt(gActiveUnit->xPos, gActiveUnit->yPos, TRAP_HELLSCAPE_VOID);

    UnitKill(gActiveUnit);

    if (trap)
        RemoveTrap(trap);
}

/**
 * PostAction hook: a unit that ends its action on an open Hellscape void is
 * slain (the hurt anim drains all its HP), and the void closes.
 */
bool PostAction_HellscapeVoidTile(ProcPtr proc)
{
    if (!UNIT_IS_VALID(gActiveUnit) || gActiveUnit->curHP <= 0)
        return false;

    if (!GetTypedTrapAt(gActiveUnit->xPos, gActiveUnit->yPos, TRAP_HELLSCAPE_VOID))
        return false;

    CallMapAnim_HurtExt(proc, gActiveUnit, gActiveUnit->curHP, NULL, Hellscape_KillCallback);
    return true;
}

#if defined(SID_Hellscape) && (COMMON_SKILL_VALID(SID_Hellscape))

u8 Hellscape_Usability(const struct MenuItemDef *def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    /* No empty adjacent tile to open the void on */
    if (!HasSelectTarget(gActiveUnit, MakeTargetListForMine))
        return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

static u8 Hellscape_OnSelectTarget(ProcPtr proc, struct SelectTarget *target)
{
    gActionData.xOther = target->x;
    gActionData.yOther = target->y;

    HideMoveRangeGraphics();

    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);

    gActionData.unk08 = SID_Hellscape;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END
         | TARGETSELECTION_ACTION_SE_6A   | TARGETSELECTION_ACTION_CLEARBGS;
}

u8 Hellscape_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
    if (item->availability == MENU_DISABLED) {
        MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
        return MENU_ACT_SND6B;
    }

    ClearBg0Bg1();

    MakeTargetListForMine(gActiveUnit);
    BmMapFill(gBmMapMovement, -1);

    StartSubtitleHelp(
        NewTargetSelection_Specialized(&gSelectInfo_PutTrap, Hellscape_OnSelectTarget),
        GetStringFromIndex(MSG_SKILL_Common_Target));

    PlaySoundEffect(0x6A);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void callback_anim(ProcPtr proc)
{
    /* Reuse the light-rune circle animation as the void tile appears */
    StartLightRuneAnim(proc, gActionData.xOther, gActionData.yOther);
}

static void callback_exec(ProcPtr proc)
{
    /* Open a visible void; DecayTraps closes it after HELLSCAPE_VOID_TURNS turns */
    struct Trap *trap = AddTrap(
        gActionData.xOther, gActionData.yOther,
        TRAP_HELLSCAPE_VOID, 0);

    if (trap)
        trap->data[TRAP_EXTDATA_HELLSCAPE_VOID_TURNSLEFT] = HELLSCAPE_VOID_TURNS;

    RefreshUnitSprites();
}

bool Action_Hellscape(ProcPtr parent)
{
    NewMuSkillAnimOnActiveUnitWithDeamon(
        parent, gActionData.unk08, callback_anim, callback_exec);
    return true;
}

#endif /* defined(SID_Hellscape) && COMMON_SKILL_VALID(SID_Hellscape) */
