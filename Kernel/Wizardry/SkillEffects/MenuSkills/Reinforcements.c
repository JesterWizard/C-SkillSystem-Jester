#include "common-chax.h"
#include "map-anims.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "unit-expa.h"
#include "action-expa.h"
#include "playst-expa.h"
#include "event-rework.h"
#include "weapon-range.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_Reinforcements) && (COMMON_SKILL_VALID(SID_Reinforcements))

// Helper: Count adjacent free and passable spaces around (x,y)
static int CountFreeAdjacentSpaces(int x, int y) {
    int count = 0;
    int dx[4] = { -1, 1, 0, 0 };
    int dy[4] = {  0, 0, -1, 1 };

    for (int i = 0; i < 4; i++) {
        int tx = x + dx[i];
        int ty = y + dy[i];

        // Check map boundaries.
        if (tx < 0 || ty < 0 || tx >= gBmMapSize.x || ty >= gBmMapSize.y)
            continue;

        // Tile must be empty.
        if (gBmMapUnit[ty][tx] != 0)
            continue;

        // Tile must be passable.
        if (TerrainTable_MovCost_HorseT1Normal[gBmMapTerrain[ty][tx]] < 0)
            continue;

        count++;
    }
    return count;
}

u8 Reinforcements_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;
    
    // Require at least 3 free adjacent spaces to summon reinforcements.
    if (CountFreeAdjacentSpaces(gActiveUnit->xPos, gActiveUnit->yPos) < 3)
        return MENU_DISABLED;

    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_Reinforcements_Used))
        return MENU_DISABLED;

    return MENU_ENABLED;
}

u8 Reinforcements_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_Reinforcements_NO_USE);
        return MENU_ACT_SND6B;
    }

    gActionData.unk08 = SID_Reinforcements;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;
    
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

/* Removed Reinforcements_Hover and Reinforcements_Unhover */

// Prepare the screen before summoning.
STATIC_DECLAR void PrepareMenuReinforcements(void)
{
    //EndAllMus();
    RefreshUnitSprites();
}

/* 
   Helper: Summon a green cavalier unit at (x, y).
   Adjust the unit definition as needed.
*/
static void SummonGreenCavalier(int x, int y)
{
    gUnitDef1.charIndex       = CHARACTER_MESSENGER;
    gUnitDef1.classIndex      = CLASS_CAVALIER;
    gUnitDef1.autolevel       = TRUE;
    gUnitDef1.allegiance      = FACTION_ID_GREEN;
    gUnitDef1.level           = gActiveUnit->level;
    gUnitDef1.xPosition       = x;
    gUnitDef1.yPosition       = y;
    gUnitDef1.redaCount       = 0;
    gUnitDef1.redas           = NULL;
    gUnitDef1.genMonster      = FALSE;
    gUnitDef1.itemDrop        = FALSE;
    gUnitDef1.items[0]        = ITEM_SWORD_STEEL;
    gUnitDef1.items[1]        = ITEM_LANCE_STEEL;

    LoadUnits(&gUnitDef1);
}

/*
    Function: spawn_reinforcements
    - Automatically finds free adjacent spaces to gActiveUnit.
    - If at least three free adjacent spaces exist, summons reinforcements
      in the first three available positions.
*/
static void spawn_reinforcements(void)
{
    int ax = gActiveUnit->xPos;
    int ay = gActiveUnit->yPos;
    int positions[4][2];
    int count = 0;
    int dx[4] = { -1, 1, 0, 0 };
    int dy[4] = {  0, 0, -1, 1 };

    for (int i = 0; i < 4; i++) {
         int tx = ax + dx[i];
         int ty = ay + dy[i];

         if (tx < 0 || ty < 0 || tx >= gBmMapSize.x || ty >= gBmMapSize.y)
             continue;
         if (gBmMapUnit[ty][tx] != 0)
             continue;
         if (TerrainTable_MovCost_FlyNormal[gBmMapTerrain[ty][tx]] <= 0)
             continue;

         positions[count][0] = tx;
         positions[count][1] = ty;
         count++;
    }

    if (count < 3)
        return; // Should not happen because usability check approved the skill.

    // Summon reinforcements in the first 3 free adjacent positions.
    for (int i = 0; i < 3; i++) {
         SummonGreenCavalier(positions[i][0], positions[i][1]);
    }

    RefreshUnitSprites();
}

static void set_actor_unit(void)
{
    gEventSlots[EVT_SLOT_2] = gActiveUnit->index;
}

/* Event Script for summoning reinforcements */
STATIC_DECLAR const EventScr EventScr_MenuReinforcements[] = {

LABEL(0)
    CALL(EventScr_MuSkillAnim)
    ASMC(PrepareMenuReinforcements)
    ASMC(set_actor_unit)
    ASMC(spawn_reinforcements)
    LABEL(99)
    NOFADE
    ENDA
};

bool Action_Reinforcements(ProcPtr parent)
{
    PlayStExpa_SetBit(PLAYSTEXPA_BIT_Reinforcements_Used);
    KernelCallEvent(EventScr_MenuReinforcements, EV_EXEC_CUTSCENE, parent);
    return true;
}

#endif