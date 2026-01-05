#include "common-chax.h"
#include "jester_headers/custom-arrays.h"
#include "kernel-lib.h"

const struct BiorhythmPInfoConfig gBiorhythmPInfoConfigList[0x100] = {
    [CHARACTER_EIRIKA]      = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_SETH]        = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_GILLIAM]     = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_FRANZ]       = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_MOULDER]     = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_VANESSA]     = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_ROSS]        = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_NEIMI]       = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_COLM]        = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_GARCIA]      = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_INNES]       = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_LUTE]        = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_NATASHA]     = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_CORMAG]      = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_EPHRAIM]     = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_FORDE]       = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_KYLE]        = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_AMELIA]      = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_ARTUR]       = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_GERIK]       = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_TETHYS]      = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_MARISA]      = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_SALEH]       = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_EWAN]        = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_LARACHEL]    = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_DOZLA]       = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_RENNAC]      = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_DUESSEL]     = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_MYRRH]       = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_KNOLL]       = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_JOSHUA]      = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_SYRENE]      = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_TANA]        = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_LYON_CC]     = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_ORSON_CC]    = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_GLEN_CC]     = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_SELENA_CC]   = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_VALTER_CC]   = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_RIEV_CC]     = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_CAELLACH_CC] = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_FADO_CC]     = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_ISMAIRE_CC]  = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
    [CHARACTER_HAYDEN_CC]   = { .biorhythm = { -15, -5, 0, 5, 15 }, .startOffset = 0 },
};

static inline bool IsValidConfig(const struct BiorhythmPInfoConfig* config) {
    for (int i = 0; i < MAX_BIORHYTHM_STATES; i++) {
        if (config->biorhythm[i] != 0)
            return true;
    }
    return false;
}

int GetBiorhythmBonus(struct BattleUnit* bu, int turnCounter) {
    int charId = UNIT_CHAR_ID(&bu->unit);

    const struct BiorhythmPInfoConfig* config = &gBiorhythmPInfoConfigList[charId];

    if (!IsValidConfig(config))
        return 0;

    // Apply per-character starting offset
    int index = k_umod((turnCounter - 1) + config->startOffset, MAX_BIORHYTHM_STATES);
    return config->biorhythm[index];
}