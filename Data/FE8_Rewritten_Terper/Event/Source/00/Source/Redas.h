/**
 * Players
 */
static const struct REDA REDAs_Seth[] = { 
    { .x = 9, .y = 5, .b = -1, .delayFrames = 0, },
    { .x = 6, .y = 5, .b = -1, .delayFrames = 0, },
};

static const struct REDA REDAs_Franz[] = {
    { .x = 9, .y = 4, .b = -1, .delayFrames = 16, },
    { .x = 5, .y = 4, .b = -1, .delayFrames = 0, },
    { .x = 5, .y = 5, .b = -1, .delayFrames = 0, },
};

static const struct REDA REDAs_Eirika[] = {
    { .x = 7, .y = 5, .a = 2, .b = -1, .flags = 0x10, .delayFrames = 16, },
};

static const struct REDA REDAs_Seth_Prologue[] = {
    { .x = 13, .y = 8, .b = -1, .flags = 0x18, .delayFrames = 0, },
    { .x = 10, .y = 8, .b = -1, .flags = 0x18, .delayFrames = 0, },
    { .x = 10, .y = 7, .b = -1, .flags = 0x18, .delayFrames = 0, },
    { .x = 9,  .y = 7, .b = -1, .flags = 0x18, .delayFrames = 0, },
    { .x = 9,  .y = 5, .b = -1, .flags = 0x18, .delayFrames = 0, },
    { .x = 8,  .y = 5, .b = -1, .flags = 0x18, .delayFrames = 0, }
};

static const struct REDA REDAs_Eirika_Prologue[] = {
    { .x = 9, .y = 5, .a = 2, .b = -1, .flags = 0x10, .delayFrames = 0, }
};

static const struct REDA REDAs_Soldier[] = {
    { .x = 9,  .y = 11, .b = -1, .delayFrames = 0, },
    { .x = 13, .y = 11, .b = -1, .delayFrames = 0, },
    { .x = 13, .y = 6,  .b = -1, .delayFrames = 0, }
};


/**
 * Enemies
 */
static const struct REDA REDAs_Druid1_Enemy[] = {
    { .x = 7, .y = 4, .b = -1, }
};
static const struct REDA REDAs_Druid2_Enemy[] = { 
    { .x = 6, .y = 6, .b = -1, } 
};
static const struct REDA REDAs_Druid3_Enemy[] = { 
    { .x = 19, .y = 3, .b = -1, } 
};
static const struct REDA REDAs_Druid4_Enemy[] = { 
    { .x = 20, .y = 6, .b = -1, } 
};
static const struct REDA REDAs_GreatKnight1_Enemy[] = {
    { .x = 10, .y = 4, .flags = 0x10, .b = -1, }
};
static const struct REDA REDAs_GreatKnight2_Enemy[] = {
    { .x = 16, .y = 4, .flags = 0x10, .b = -1,  }
};
static const struct REDA REDAs_Paladin1_Enemy[] = {
    { .x = 10, .y = 6, .flags = 0x10, .b = -1, }
};
static const struct REDA REDAs_Paladin2_Enemy[] = {
    { .x = 16, .y = 6, .flags = 0x10, .b = -1, }
};
static const struct REDA REDAs_MageKnight1_Enemy[] = {
    { .x = 10, .y = 8, .flags = 0x10, .b = -1, }
};
static const struct REDA REDAs_MageKnight2_Enemy[] = {
    { .x = 16, .y = 8, .flags = 0x10, .b = -1, }
};
static const struct REDA REDAs_Vigarde_Enemy[] = {
    { .x = 13, .y = 6, .flags = 0xC, .b = 0xFFFF, }
};
static const struct REDA REDAs_Lyon_Enemy[] = {
    { .x = 12, .y = 7, .flags = 0xC, .b = 0xFFFF, }
};
static const struct REDA REDAs_Valter_Enemy[] = {
    { .x = 10, .y = 5, .b = -1, },
    { .x = 9,  .y = 5, .b = 1, .flags = 0x10, .delayFrames = 16, }
};
static const struct REDA REDAs_Wyvern1_Enemy[] = {
    { .x = 11, .y = 4, .b = -1, .delayFrames = 16, }
};
static const struct REDA REDAs_Wyvern2_Enemy[] = {
    { .x = 11, .y = 6, .b = -1, .delayFrames = 16, }
};
static const struct REDA REDAs_ONeill_Enemy[] = {
    { .x = 10, .y = 8, .b = -1, .delayFrames = 0, }
};
static const struct REDA REDAs_Fighter1_Enemy[] = {
    { .x = 8, .y = 6, .b = -1, .delayFrames = 16, }
};
static const struct REDA REDAs_Fighter2_Enemy[] = {
    { .x = 9, .y = 6, .b = -1, .delayFrames = 16, }
};