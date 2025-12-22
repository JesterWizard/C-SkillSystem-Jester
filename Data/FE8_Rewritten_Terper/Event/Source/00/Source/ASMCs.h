#include "common-chax.h"
#include "rn.h"
#include "bwl.h"
#include "bmunit.h"
#include "skill-system.h"
#include "jester_headers/miscellaneous.h"
#include "debuff.h"
#include "jester_headers/Forging.h"
#include "jester_headers/custom-functions.h"

void SetGameOptions() 
{
    SetGameOption(GAME_OPTION_ANIMATION, 2);  // Set battle animations off
    SetGameOption(GAME_OPTION_TEXT_SPEED, 1); // Set game speed to max
    SetGameOption(GAME_OPTION_GAME_SPEED, 3); // Set text speed to max

    SetFlag(0xB4); // Guide Flag
};
