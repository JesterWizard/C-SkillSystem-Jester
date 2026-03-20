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
    SetGameOption(GAME_OPTION_WINDOW_COLOR, 1);

    SetFlag(0xB4); // Guide Flag

    // AddToggleTorch(3, 3, 3, 1);

    AddTeleportTilePair(1, 1, 3, 3);
    AddTeleportTilePair(5, 5, 9, 5);

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