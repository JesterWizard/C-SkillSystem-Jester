#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/macros.h"
#include "jester_headers/maps.h"
#include "jester_headers/flags.h"
#include "jester_headers/miscellaneous.h"
#include "jester_headers/event-call.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-arrays.h"
#include "jester_headers/custom-structs.h"
#include "EAstdlib.h"
#include "kernel-tutorial.h"

const EventScr EventScr_Tutorial_SKILL_SCROLL[] = {
    EvtTextStartType5 // ENOSUPP in EAstdlib
    SVAL(EVT_SLOT_B, 0xFFFFFFFF) // Center text box
    TEXTSHOW(MSG_TUTORIAL_SKILL_SCROLL)
    TEXTEND
    REMA
    ENDA
};

/*
 * Once, as the world map opens after Chapter 5x (Chapter 6's SET_NODE): Serafew's district has just opened. Shown
 * whatever the Show Tutorials option says, since it announces a place to go rather than teaching a mechanic.
 */
const EventScr EventScr_Tutorial_DISTRICTS[] = {
    CHECK_EVENTID(GLOBAL_FLAG_DISTRICT_SERAFEW)
    BEQ(0x1, EVT_SLOT_C, EVT_SLOT_0) // not open (yet)
    CHECK_EVENTID(GLOBAL_FLAG_DISTRICT_TUTORIAL)
    BNE(0x1, EVT_SLOT_C, EVT_SLOT_0) // already shown
    ENUT(GLOBAL_FLAG_DISTRICT_TUTORIAL)
    /*
     * The box is drawn with sprites, and one of its rows uses the OBJ tiles the lord's world-map sprite is animated
     * in (0x2C0): hidden, the lord's frames stop landing in the box.
     */
    WM_MAKELORDDISAPPEAR(0) // WM_MU_0
    EvtTextStartType5 // ENOSUPP in EAstdlib
    SVAL(EVT_SLOT_B, 0xFFFFFFFF) // Center text box
    TEXTSHOW(MSG_TUTORIAL_DISTRICTS)
    TEXTEND
    REMA
    WM_MAKELORDVISIBLE(0) // WM_MU_0

LABEL(0x1)
    ENDA
};


const EventScr EventScr_Tutorial_BONUS_EXP[] = {
    ASMC(EndAllMus) // ENOSUPP in EAstdlib
    EvtTextStartType5 // ENOSUPP in EAstdlib
    SVAL(EVT_SLOT_B, 0xFFFFFFFF) // Center text box
    TEXTSHOW(MSG_TUTORIAL_BONUS_EXP)
    TEXTEND
    REMA
    ENDA
};
