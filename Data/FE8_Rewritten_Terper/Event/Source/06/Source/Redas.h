static const struct REDA REDAs_NULL[] = {
    { } 
};

/**
 * Players
 */
static const struct REDA REDAs_EIRIKA_1[] = {
    { .x = 4, .y = 17, .b = -1, .delayFrames = 0, },
};

static const struct REDA REDAs_SETH_1[] = {
    { .x = 6, .y = 17, .b = -1, .delayFrames = 0, },
};

static const struct REDA REDAs_EIRIKA_ARRIVES[] = {
    { .x = 3, .y = 0, .b = -1, .delayFrames = 0, },
    { .x = 3, .y = 2, .b = -1, .delayFrames = 0, },
    { .x = 4, .y = 2, .b = -1, .delayFrames = 0, },
    { .x = 4, .y = 8, .b = -1, .delayFrames = 0, },
};
static const struct REDA REDAs_SETH_ARRIVES[] = {
    { .x = 3, .y = 0, .b = 0xFFFF, .a = CHARACTER_EIRIKA, .delayFrames = 0, },
    { .x = 3, .y = 2, .b = -1, .delayFrames = 0, },
    { .x = 4, .y = 2, .b = -1, .delayFrames = 0, },
    { .x = 4, .y = 7, .b = -1, .delayFrames = 0, },
    { .x = 5, .y = 7, .b = -1, .delayFrames = 0, },
};

/**
 * Enemies
 */
static const struct REDA REDAs_CAVALIER_1[] = {
    { .x = 3, .y = 11, .b = -1, .delayFrames = 0, },
};

static const struct REDA REDAs_CAVALIER_2[] = {
    { .x = 2, .y = 9, .b = -1, .delayFrames = 0, },
};

static const struct REDA REDAs_CAVALIER_3[] = {
    { .x = 2, .y = 13, .b = -1, .delayFrames = 0, },
};

/**
 * Green units
 */