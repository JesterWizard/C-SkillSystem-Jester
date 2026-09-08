#include "common-chax.h"
#include "map-anims.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_Override) && (COMMON_SKILL_VALID(SID_Override))

#define OVERRIDE_MAX_TARGETS 16

struct OverrideState {
    struct Unit *targets[OVERRIDE_MAX_TARGETS];
    int count;
    u8 destX, destY;
    u8 expAccum;  /* accumulated kill EXP, capped at 254 */
};

extern struct OverrideState sOverrideState;

/* Walk in direction (dx, dy) from actor, collecting every consecutive enemy. */
static void CollectLineEnemies(struct Unit *actor, int dx, int dy)
{
    int x = actor->xPos + dx;
    int y = actor->yPos + dy;

    while (x >= 0 && x < gBmMapSize.x && y >= 0 && y < gBmMapSize.y)
    {
        struct Unit *target = GetUnitAtPosition(x, y);

        if (!UNIT_IS_VALID(target))
            break;

        if (target->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
            break;

        if (AreUnitsAllied(actor->index, target->index))
            break;

        if (sOverrideState.count < OVERRIDE_MAX_TARGETS)
            sOverrideState.targets[sOverrideState.count++] = target;

        x += dx;
        y += dy;
    }
}

/*
 * Build a selectable target list: one entry per direction that has 2+
 * consecutive enemies.  The added target is the FIRST enemy in that direction
 * so the player's cursor aims at it to choose the direction.
 */
static void MakeTargetListForOverride(struct Unit *unit)
{
    static const int dirs[4][2] = { {0,-1}, {0,1}, {-1,0}, {1,0} };

    InitTargets(unit->xPos, unit->yPos);

    for (int d = 0; d < 4; d++)
    {
        int dx = dirs[d][0], dy = dirs[d][1];
        int x  = unit->xPos + dx;
        int y  = unit->yPos + dy;

        struct Unit *first   = NULL;
        int          chainLen = 0;

        while (x >= 0 && x < gBmMapSize.x && y >= 0 && y < gBmMapSize.y)
        {
            struct Unit *tgt = GetUnitAtPosition(x, y);

            if (!UNIT_IS_VALID(tgt))
                break;
            if (tgt->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
                break;
            if (AreUnitsAllied(unit->index, tgt->index))
                break;

            if (chainLen == 0)
                first = tgt;

            chainLen++;
            x += dx;
            y += dy;
        }

        /* Require an open tile at the far end of the chain. */
        bool hasOpenEnd = (x >= 0 && x < gBmMapSize.x && y >= 0 && y < gBmMapSize.y)
                       && !UNIT_IS_VALID(GetUnitAtPosition(x, y));

        if (chainLen >= 2 && hasOpenEnd)
            AddTarget(first->xPos, first->yPos, first->index, 0);
    }
}

/* ------ Usability: requires 2+ chained enemies in at least one direction ------ */

u8 Override_Usability(const struct MenuItemDef *def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (!HasSelectTarget(gActiveUnit, MakeTargetListForOverride))
        return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

/* ------ Target-cursor callback: stores chosen direction's first enemy ------ */

static u8 Override_OnSelectTarget(ProcPtr proc, struct SelectTarget *target)
{
    gActionData.xOther = target->x;
    gActionData.yOther = target->y;

    HideMoveRangeGraphics();

    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);

    gActionData.unk08 = SID_Override;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END
         | TARGETSELECTION_ACTION_SE_6A   | TARGETSELECTION_ACTION_CLEARBGS;
}

/* ------ OnSelected: clear menu, show direction cursor ------ */

u8 Override_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
        return MENU_ACT_SND6B;
    }

    ClearBg0Bg1();

    MakeTargetListForOverride(gActiveUnit);
    BmMapFill(gBmMapMovement, -1);

    StartSubtitleHelp(
        NewTargetSelection_Specialized(&gSelectInfo_PutTrap, Override_OnSelectTarget),
        GetStringFromIndex(MSG_SKILL_Common_Target));

    PlaySoundEffect(0x6A);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

/* ------ Sequential damage proc: one enemy at a time ------ */

struct ProcOverrideSeq {
    PROC_HEADER;
    int index;
    int damage;
};

/* Passed as callback1 to CallMapAnim_HurtExt — keeps the caster visible. */
static void ShowActiveUnit(ProcPtr proc)
{
    ShowUnitSprite(gActiveUnit);
}

static void SeqProc_Tick(struct ProcOverrideSeq *proc)
{
    /* After each hurt proc, check if the target just died. */
    if (proc->index > 0)
    {
        struct Unit *prev = sOverrideState.targets[proc->index - 1];
        if (UNIT_IS_VALID(prev) && prev->curHP <= 0)
        {
            int exp = GetUnitRoundExp(gActiveUnit, prev)
                    + GetUnitKillExpBonus(gActiveUnit, prev);
            sOverrideState.expAccum =
                (u8)MIN((int)sOverrideState.expAccum + exp, 254);
            UnitKill(prev);
        }
    }

    if (proc->index >= sOverrideState.count)
    {
        gActiveUnit->xPos = sOverrideState.destX;
        gActiveUnit->yPos = sOverrideState.destY;
        gActionData.xMove = sOverrideState.destX;
        gActionData.yMove = sOverrideState.destY;
        ShowUnitSprite(gActiveUnit);
        if (sOverrideState.expAccum > 0)
            AddExp_Event(sOverrideState.expAccum);
        Proc_Break(proc);
        return;
    }

    struct Unit *target = sOverrideState.targets[proc->index++];
    CallMapAnim_HurtExt((ProcPtr)proc, target, proc->damage, ShowActiveUnit, NULL);
}

static const struct ProcCmd ProcScr_OverrideSeq[] = {
    PROC_REPEAT(SeqProc_Tick),
    PROC_END
};

/* ------ Animation callbacks ------ */

static void callback_anim(ProcPtr proc)
{
    PlaySoundEffect(0x269);
    Proc_StartBlocking(ProcScr_DanceringAnim, proc);

    BG_SetPosition(BG_0, -SCREEN_TILE_IX(gActiveUnit->xPos - 1), -SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

static void callback_exec(ProcPtr proc)
{
    u8 tx = gActionData.xOther;
    u8 ty = gActionData.yOther;
    int ax = gActiveUnit->xPos;
    int ay = gActiveUnit->yPos;

    int dx = (tx > ax) ? 1 : (tx < ax) ? -1 : 0;
    int dy = (ty > ay) ? 1 : (ty < ay) ? -1 : 0;

    sOverrideState.count = 0;
    CollectLineEnemies(gActiveUnit, dx, dy);

    if (!sOverrideState.count)
        return;

    /* Destination: the open tile one step past the last enemy. */
    struct Unit *last = sOverrideState.targets[sOverrideState.count - 1];
    sOverrideState.destX  = (u8)(last->xPos + dx);
    sOverrideState.destY  = (u8)(last->yPos + dy);
    sOverrideState.expAccum = 0;

    /*
     * Kill the MU and restore the caster's map sprite NOW so the caster
     * is continuously visible throughout the entire damage sequence.
     * (anim_init hid the sprite when it created the MU; without this the
     * caster flickers invisible for ~2 frames at the start of each hurt.)
     */
    EndAllMus();
    ShowUnitSprite(gActiveUnit);

    struct ProcOverrideSeq *sp =
        (struct ProcOverrideSeq *)Proc_StartBlocking(ProcScr_OverrideSeq, proc);
    sp->index  = 0;
    sp->damage = SKILL_EFF0(SID_Override);
}

/* ------ Action ------ */

bool Action_Override(ProcPtr parent)
{
    NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
    return true;
}

#endif /* defined(SID_Override) && COMMON_SKILL_VALID(SID_Override) */
