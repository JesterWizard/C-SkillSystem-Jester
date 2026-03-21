#include "common-chax.h"
#include "map-anims.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "action-expa.h"
#include "playst-expa.h"
#include "strmag.h"
#include "weapon-range.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "constants/terrains.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_Portal) && (COMMON_SKILL_VALID(SID_Portal))

static void Portal_ResetSelectionState(void)
{
    gActionDataExpa.flag2 = 0xFF;
    gActionDataExpa.flag3 = 0xFF;
    gActionDataExpa.flag4 = 0;
}

static void Portal_ClearPlacementMap(void)
{
    HideMoveRangeGraphics();
    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);
}

static bool Portal_IsForbiddenTerrain(int terrain)
{
    switch (terrain) {
    case TERRAIN_VILLAGE_03:
    case TERRAIN_VILLAGE_04:
    case TERRIAN_HOUSE:
    case TERRAIN_ARMORY:
    case TERRAIN_VENDOR:
    case TERRAIN_ARENA_08:
    case TERRAIN_ARENA_30:
    case TERRAIN_CHURCH:
    case TERRAIN_INN:
        return true;

    default:
        return false;
    }
}

static bool Portal_CanPlaceTileAt(int x, int y)
{
    if (x < 0 || y < 0 || x >= gBmMapSize.x || y >= gBmMapSize.y)
        return false;

    if (gBmMapUnit[y][x] != 0)
        return false;

    if (gBmMapHidden[y][x] & HIDDEN_BIT_UNIT)
        return false;

    if (GetTrapAt(x, y) != NULL)
        return false;

    if (Portal_IsForbiddenTerrain(gBmMapTerrain[y][x]))
        return false;

    return true;
}

static int Portal_GetPlacementRange(struct Unit *unit)
{
    int range = GetUnitMagic(unit) / 2;

    if (range < 0)
        range = 0;

    return range;
}

static int Portal_CountValidTilesInRange(struct Unit *unit)
{
    int count = 0;
    int range = Portal_GetPlacementRange(unit);
    int originX = unit->xPos;
    int originY = unit->yPos;

    for (int x = originX - range; x <= originX + range; ++x) {
        for (int y = originY - range; y <= originY + range; ++y) {
            if (abs(x - originX) + abs(y - originY) > range)
                continue;

            if (x == originX && y == originY)
                continue;

            if (Portal_CanPlaceTileAt(x, y))
                count++;
        }
    }

    return count;
}

static void Portal_MakeTargetList(struct Unit *unit, int excludedX, int excludedY)
{
    int range = Portal_GetPlacementRange(unit);
    int originX = unit->xPos;
    int originY = unit->yPos;

    InitTargets(originX, originY);

    for (int x = originX - range; x <= originX + range; ++x) {
        for (int y = originY - range; y <= originY + range; ++y) {
            if (abs(x - originX) + abs(y - originY) > range)
                continue;

            if (x == originX && y == originY)
                continue;

            if (x == excludedX && y == excludedY)
                continue;

            if (!Portal_CanPlaceTileAt(x, y))
                continue;

            AddTarget(x, y, 0, 0);
        }
    }
}

static void Portal_DrawPlacementMap(struct Unit *unit, int excludedX, int excludedY)
{
    int range = Portal_GetPlacementRange(unit);
    int originX = unit->xPos;
    int originY = unit->yPos;

    BmMapFill(gBmMapMovement, -1);
    BmMapFill(gBmMapRange, 0);

    for (int x = originX - range; x <= originX + range; ++x) {
        for (int y = originY - range; y <= originY + range; ++y) {
            if (abs(x - originX) + abs(y - originY) > range)
                continue;

            if (x == originX && y == originY)
                continue;

            if (x == excludedX && y == excludedY)
                continue;

            if (!Portal_CanPlaceTileAt(x, y))
                continue;

            gBmMapMovement[y][x] = 0;
        }
    }

    DisplayMoveRangeGraphics(MOVLIMITV_MMAP_BLUE);
}

static void MakeTargetListForPortalFirst(struct Unit *unit)
{
    Portal_MakeTargetList(unit, -1, -1);
}

static void MakeTargetListForPortalSecond(struct Unit *unit)
{
    Portal_MakeTargetList(unit, gActionDataExpa.flag2, gActionDataExpa.flag3);
}

static void DrawPlacementMapForPortalFirst(struct Unit *unit)
{
    Portal_DrawPlacementMap(unit, -1, -1);
}

static void DrawPlacementMapForPortalSecond(struct Unit *unit)
{
    Portal_DrawPlacementMap(unit, gActionDataExpa.flag2, gActionDataExpa.flag3);
}

u8 Portal_Usability(const struct MenuItemDef *def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_Portal_Used))
        return MENU_NOTSHOWN;

    if (Portal_CountValidTilesInRange(gActiveUnit) < 2)
        return MENU_DISABLED;

    if (!HasSelectTarget(gActiveUnit, MakeTargetListForPortalFirst))
        return MENU_DISABLED;

    return MENU_ENABLED;
}

static u8 Portal_OnSelectTarget(ProcPtr parent, struct SelectTarget *target)
{
    struct SelectTargetProc *proc = parent;

    if (gActionDataExpa.flag4 == 0) {
        gActionDataExpa.flag2 = target->x;
        gActionDataExpa.flag3 = target->y;
        gActionDataExpa.flag4 = 1;

        MakeTargetListForPortalSecond(gActiveUnit);
        DrawPlacementMapForPortalSecond(gActiveUnit);
        proc->currentTarget = GetLinkedTargets();

        if (proc->currentTarget == NULL) {
            Portal_ResetSelectionState();
            return TARGETSELECTION_ACTION_SE_6B;
        }

        if (proc->selectRoutines->onSwitchIn)
            proc->selectRoutines->onSwitchIn(proc, proc->currentTarget);

        return TARGETSELECTION_ACTION_SE_6A;
    }

    gActionData.xOther = gActionDataExpa.flag2;
    gActionData.yOther = gActionDataExpa.flag3;
    gActionData.targetIndex = target->x;
    gActionData.itemSlotIndex = target->y;
    gActionData.xMove = gActiveUnit->xPos;
    gActionData.yMove = gActiveUnit->yPos;
    gActionData.unk08 = SID_Portal;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    Portal_ClearPlacementMap();

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END
         | TARGETSELECTION_ACTION_SE_6A   | TARGETSELECTION_ACTION_CLEARBGS;
}

u8 Portal_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
    if (item->availability == MENU_DISABLED) {
        MenuFrozenHelpBox(menu, MSG_SKILL_Portal_FRtext);
        return MENU_ACT_SND6B;
    }

    Portal_ResetSelectionState();

    ClearBg0Bg1();
    MakeTargetListForPortalFirst(gActiveUnit);
    DrawPlacementMapForPortalFirst(gActiveUnit);

    StartSubtitleHelp(
        NewTargetSelection_Specialized(&gSelectInfo_PutTrap, Portal_OnSelectTarget),
        GetStringFromIndex(MSG_SKILL_Portal_Target));

    PlaySoundEffect(0x6A);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void Portal_PlayTileAnim(ProcPtr proc, int x, int y, int animIndex)
{
    BG_SetPosition(BG_0, -SCREEN_TILE_IX(gActiveUnit->xPos - 1), -SCREEN_TILE_IX(gActiveUnit->yPos - 2));

    (void) animIndex;
    StartLightRuneAnim(proc, x, y);
}

static void Portal_CreateTiles(void)
{
    if (Portal_CanPlaceTileAt(gActionData.xOther, gActionData.yOther)
        && Portal_CanPlaceTileAt(gActionData.targetIndex, gActionData.itemSlotIndex)) {
        AddTeleportTilePair(
            gActionData.xOther,
            gActionData.yOther,
            gActionData.targetIndex,
            gActionData.itemSlotIndex);

        PlayStExpa_SetBit(PLAYSTEXPA_BIT_Portal_Used);
    }
}

static void Portal_FinalizeAction(void)
{
    Portal_ResetSelectionState();
}

bool Action_Portal(ProcPtr parent)
{
    Portal_CreateTiles();

    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_Portal_Used)) {
        Portal_PlayTileAnim(parent, gActionData.xOther, gActionData.yOther, 0);
        Portal_PlayTileAnim(parent, gActionData.targetIndex, gActionData.itemSlotIndex, 1);
    }

    Portal_FinalizeAction();
    return true;
}
#endif