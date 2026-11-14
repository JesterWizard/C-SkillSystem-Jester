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
#define END_CG_TO_BLACK \
    FADE_TO_BLACK(16) \
    TEXTSTART \
    CLEAN

#define NO_REDA 0
#define NO_ITEMS 0