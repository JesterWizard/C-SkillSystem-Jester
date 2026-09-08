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


const EventScr EventScr_Tutorial_BONUS_EXP[] = {
    ASMC(EndAllMus) // ENOSUPP in EAstdlib
    EvtTextStartType5 // ENOSUPP in EAstdlib
    SVAL(EVT_SLOT_B, 0xFFFFFFFF) // Center text box
    TEXTSHOW(MSG_TUTORIAL_BONUS_EXP)
    TEXTEND
    REMA
    ENDA
};
