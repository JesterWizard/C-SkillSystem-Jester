#pragma once

#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/macros.h"

#include "jester_headers/maps.h"
#include "jester_headers/flags.h"
#include "jester_headers/miscellaneous.h"
#include "jester_headers/LimitedShopStock.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/voice-songs.h"
#include "EAstdlib.h"

/*
 * Leave a CG (TEXT_CG_CONSECUTIVE) through black: TEXT_CG's own ending less its fade back in. The next background
 * (SET_BACKGROUND, TEXT_BG...) then draws correctly, with its portraits, and no map shows in between.
 */
/*
 * The next speaker over a CG already shown (TEXT_CG_CONSECUTIVE starts one): only the text box changes. Setting the
 * background again (TEXT_CG_CONSECUTIVE, TEXT_CG) would fade the CG out and in. TEXT_CG_LAST is the same for the last
 * speaker, ending as TEXT_CG does (fade back to the map).
 */
#define TEXT_CG_NEXT(msg) \
    SVAL(EVT_SLOT_2, (msg)) \
    CGTEXTBOXSTART \
    TEXTSHOW(-1) \
    TEXTEND

#define TEXT_CG_LAST(msg) \
    TEXT_CG_NEXT(msg) \
    CALL(EventScr_TextShowWithFadeIn)

#define END_CG_TO_BLACK \
    FADE_TO_BLACK(16) \
    TEXTSTART \
    CLEAN

#define NO_REDA 0
#define NO_ITEMS 0