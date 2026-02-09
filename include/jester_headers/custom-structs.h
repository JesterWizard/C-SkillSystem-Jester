struct DefeatTalkEntNew {
             u16 pidA;
             u16 pidB;
    /* 02 */ u8 route;
    /* 03 */ u8 chapter;
    /* 04 */ u16 flag;
    /* 06 */ u16 msg;
    /* 08 */ EventScr * event;
};

extern struct DefeatTalkEntNew* GetDefeatTalkEntry_NEW(u16);

/* START - Page 7 - Unit promotions list */
typedef struct
{
    int classId;     // Promotion class
    int skills[3];   // Up to 3 skills for this promotion
} PromotionEntry;

typedef struct
{
    int key;                       // Unit ID
    PromotionEntry promotions[3];  // Up to 3 promotions
} UnitPromotions;

#define UNIT_PROMOS(unit_id, ...) \
    { .key = (unit_id), .promotions = { __VA_ARGS__ } }
#define PROMO(class_id, ...) \
    { .classId = (class_id), .skills = { __VA_ARGS__ } }

extern const UnitPromotions unit_promotions[];
/* END - Page 7 - Unit promotions list */

// 139-175 (0x8B-0xAF) : probably free
// 176-178 (0xB0-0xB2) : free
// 180-235 (0xB4-0xEB) : Used by Guide (see [FE8] "Help" Command & Permanent Event IDs)
// 236-239 (0xEC-0xEF) : free
// 240-295 (0xF0-0x127) : Used to mark guide entries as read

/* Global flags use */
enum {
    GLOBAL_FLAG_BASE_CHAPTER_INTRO_SKIP = 0xEC,
    GLOBAL_FLAG_TIME_RAN_OUT = 0xED,
};

extern const EventListScr EventScr_MapSupportConversation_NEW[];


struct ChapterTimerProc
{
    PROC_HEADER;
    int frameClock;
};

void DrawTimeHMS(struct Text *text, int x, int seconds);
void StartChapterTimer(int seconds);
void ChapterTimer_OnTick(struct ChapterTimerProc *proc);
void GoalDisplay_Loop(struct PlayerInterfaceProc *proc);

extern u16 gChapterTimerSeconds;

// 0x0 - gInfuseDragonGlass
// 0x1 - gInfuseLastIndex
// 0x2 - gInfuseHandOriginX
// 0x3 - gInfuseHandOriginY
// 0x4 - gInfuseHandMode
// 0x5 - gInfuseSelectedOption
extern u8 gInfuseMenuArray[6];

enum {
    PL_INFUSE_INIT = 0,
    PL_INFUSE_SHOW_CURSOR = 1,
    PL_INFUSE_IDLE = 2,
    PL_INFUSE_PRESS_LEFT = 3,
    PL_INFUSE_PRESS_RIGHT = 4,
    PL_INFUSE_REFRESH_VIEW = 6,
    PL_INFUSE_SHOW_INVENTORY = 7,
    PL_INFUSE_PRESS_B = 8,
    PL_INFUSE_END = 9
};

enum {
    INFUSE_STATE_LIST = 0,
    INFUSE_STATE_INFUSE_UI = 1,
    INFUSE_STATE_CONFIRM = 2
};

struct InfuseRecipe {
    u8 targetItemId;
    u8 cost;
};

extern const struct InfuseRecipe gInfusionLookupTable[256];

extern u16 gBEXP_Amount;
extern u8 gBEXP_TopVisibleIndex;

enum {
    PL_BEXP_INIT = 0,
    PL_BEXP_SHOW_CURSOR = 1,
    PL_BEXP_IDLE = 2,
    PL_BEXP_PRESS_LEFT = 3,
    PL_BEXP_PRESS_RIGHT = 4,
    PL_BEXP_PRESS_R = 5,
    PL_BEXP_REFRESH_VIEW = 6,
    PL_BEXP_SHOW_INVENTORY = 7,
    PL_BEXP_PRESS_B = 8,
    PL_BEXP_END = 9
};
