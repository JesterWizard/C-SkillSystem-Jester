#include "common-chax.h"
#include "kernel-lib.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

/* Unique tag in trap->extra to identify Hellscape void tiles */
#define HELLSCAPE_TRAP_TAG   0x48
/* Number of phases before the void tile expires */
#define HELLSCAPE_TRAP_TURNS 3

/**
 * PrePhase hook: decrement turn counters on all active Hellscape void tiles and
 * remove any that have expired.  Runs at the start of every phase.
 */
/**
 * PostAction hook: if the active unit has landed on a Hellscape void tile,
 * instantly kill them (HP->0, UnitKill) and remove the trap so that
 * HandlePostActionTraps finds nothing and exits cleanly.
 */
static void Hellscape_KillCallback(ProcPtr proc)
{
    struct Trap *trap = GetTrapAt(gActiveUnit->xPos, gActiveUnit->yPos);
    UnitKill(gActiveUnit);
    if (trap) RemoveTrap(trap);
}

void PostAction_HellscapeVoidTile(ProcPtr proc)
{
    struct Trap *trap = GetTrapAt(gActiveUnit->xPos, gActiveUnit->yPos);
    if (!trap || trap->type != TRAP_MINE || trap->extra != HELLSCAPE_TRAP_TAG)
        return;

    // sets HP to 0, plays hit flash, then fires the callback
    CallMapAnim_HurtExt(proc, gActiveUnit, gActiveUnit->curHP,
                        NULL, Hellscape_KillCallback);
}
bool PrePhase_HellscapeTrapDecay(ProcPtr proc)
{
    for (int i = 0; i < TRAP_MAX_COUNT; i++) {
        struct Trap *trap = GetTrap(i);
        if (trap->type == TRAP_MINE && trap->extra == HELLSCAPE_TRAP_TAG) {
            trap->data[TRAP_EXTDATA_TRAP_COUNTER]--;
            if (trap->data[TRAP_EXTDATA_TRAP_COUNTER] <= 0)
                RemoveTrap(trap);
        }
    }
    return false;
}

#if defined(SID_Hellscape) && (COMMON_SKILL_VALID(SID_Hellscape))

u8 Hellscape_Usability(const struct MenuItemDef *def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    // if (!HasSelectTarget(gActiveUnit, MakeTargetListForMine))
    //     return MENU_DISABLED;

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
    /* Place a mine tagged as a Hellscape void tile */
    struct Trap *trap = AddTrap(
        gActionData.xOther, gActionData.yOther,
        TRAP_MINE, HELLSCAPE_TRAP_TAG);

    if (trap)
        trap->data[TRAP_EXTDATA_TRAP_COUNTER] = HELLSCAPE_TRAP_TURNS;
}

bool Action_Hellscape(ProcPtr parent)
{
    NewMuSkillAnimOnActiveUnitWithDeamon(
        parent, gActionData.unk08, callback_anim, callback_exec);
    return true;
}

#endif /* defined(SID_Hellscape) && COMMON_SKILL_VALID(SID_Hellscape) */
