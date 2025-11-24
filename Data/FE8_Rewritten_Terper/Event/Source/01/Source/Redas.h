/**
 * Players
 */
static const struct REDA REDAs_SETH[] = { 
    { .x = 13, .y = 8, .b = -1, .delayFrames = 0 },
};

static const struct REDA REDAs_EIRIKA[] = {
    { .x = 12, .y = 9, .b = -1, .delayFrames = 0 },
};

static const struct REDA REDAs_FRANZ[] = { 
    { .x = 7, .y = 1, .b = -1, .delayFrames = 0 },
};

static const struct REDA REDAs_GILLIAM[] = {
    { .x = 6, .y = 1, .b = -1, .delayFrames = 0 },
};

/**
 * Enemies
 */
static const struct REDA REDAs_BREGUET_ENEMY[] = {
    { .x = 2, .y = 5, .b = -1, .delayFrames = 0}
};
static const struct REDA REDAs_SOLDIER1_ENEMY[] = { 
    { .x = 1, .y = 6, .b = -1, } 
};
static const struct REDA REDAs_SOLDIER2_ENEMY[] = { 
    { .x = 3, .y = 8, .b = -1, },
    { .x = 3, .y = 6, .b = -1, } 
};
static const struct REDA REDAs_SOLDIER3_ENEMY[] = { 
    { .x = 2, .y = 7, .b = -1, } 
};
static const struct REDA REDAs_FIGHTER1_ENEMY[] = {
    { .x = 1, .y = 8, .b = -1, .a = CHARACTER_BREGUET }
};
static const struct REDA REDAs_FIGHTER2_ENEMY[] = {
    { .x = 3, .y = 8, .b = -1, .a = CHARACTER_BREGUET }
};
static const struct REDA REDAs_FIGHTER3_ENEMY[] = {
    { .x = 2, .y = 9, .b = -1, .a = CHARACTER_BREGUET } 
};


static const struct REDA REDAs_FIGHTER4_ENEMY[] = {
    { .x = 13, .y = 8, .b = -1, .a = CHARACTER_BREGUET }
};
static const struct REDA REDAs_SOLDIER4_ENEMY[] = {
    { .x = 13, .y = 9, .b = -1, .a = CHARACTER_BREGUET }
};
static const struct REDA REDAs_FIGHTER5_ENEMY[] = {
    { .x = 12, .y = 9, .b = -1, .a = CHARACTER_BREGUET } 
};

/**
 * Green units
 */
static const struct REDA REDAs_CAVALIER_GREEN[] = {
    { .x = 2, .y = 3, .b = -1, .delayFrames = 0 },
    { .x = 0, .y = 3, .b = -1, .delayFrames = 0 },
    { .x = 0, .y = 0, .b = -1, .delayFrames = 0 }
};