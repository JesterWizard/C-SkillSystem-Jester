#include "common-chax.h"
#include "rn.h"
#include "bwl.h"
#include "bmunit.h"
#include "skill-system.h"
#include "jester_headers/miscellaneous.h"
#include "debuff.h"
#include "jester_headers/Forging.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-structs.h"

void SetGameOptions() 
{
    SetGameOption(GAME_OPTION_ANIMATION, 2);  // Set battle animations off
    SetGameOption(GAME_OPTION_TEXT_SPEED, 1); // Set game speed to max
    SetGameOption(GAME_OPTION_GAME_SPEED, 3); // Set text speed to max

    SetFlag(0xB4); // Guide Flag

    // AddToggleTorch(3, 3, 3, 1);

    // AddTeleportTilePair(1, 1, 3, 3);
    // AddTeleportTilePair(5, 5, 9, 5);
    // AddGrassTile(3, 3, 3);
    // AddBoulderTile(1, 1);

    // AddSpinTile(1, 3, SPIN_TILE_DIR_RIGHT);
    // AddSpinTile(2, 3, SPIN_TILE_DIR_RIGHT);
    // AddSpinTile(3, 3, SPIN_TILE_DIR_RIGHT);
    // AddSpinTile(4, 3, SPIN_TILE_DIR_DOWN);
    // AddSpinTile(4, 4, SPIN_TILE_DIR_DOWN);
    // AddSpinTile(4, 5, SPIN_TILE_DIR_RIGHT);
    // AddSpinTile(5, 5, SPIN_TILE_DIR_RIGHT);
    // AddSpinTile(6, 5, SPIN_TILE_DIR_RIGHT);
    // AddSpinTile(7, 5, SPIN_TILE_DIR_RIGHT);
    // AddSpinTile(8, 5, SPIN_TILE_DIR_RIGHT);
    // AddSpinTile(9, 5, SPIN_TILE_DIR_RIGHT);

    // gChapterTimerSeconds = 30;

    // gPlaySt.partyGoldAmount = 600000;
    // struct Unit * unit = GetUnitFromCharId(CHARACTER_EIRIKA);
    // unit->items[2] = MakeNewItem(ITEM_SWORD_IRON);
};

// void AddMoreTime()
// {
//     gChapterTimerSeconds += 30;
//     // NewPopup_VerySimple("Timer Increased", SONG_SE_UPDATE, Proc_Find(gProcScr_PlayerPhase));
// };