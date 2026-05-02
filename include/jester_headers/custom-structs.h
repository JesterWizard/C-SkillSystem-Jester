struct DefeatTalkEntNew {
             u16 pidA;
             u16 pidB;
    /* 02 */ u8 route;
    /* 03 */ u8 chapter;
    /* 04 */ u16 flag;
    /* 06 */ u16 msg;
    /* 08 */ EventScr * event;
};

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
    GLOBAL_FLAG_BASE_CONVERSATION_1 = 0x8B,
    GLOBAL_FLAG_BASE_CONVERSATION_2 = 0x8C,
    GLOBAL_FLAG_BASE_CONVERSATION_3 = 0x8D,
    GLOBAL_FLAG_BASE_CONVERSATION_4 = 0x8E,
    GLOBAL_FLAG_BASE_CONVERSATION_5 = 0x8F,

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
    PL_INFUSE_REFRESH_VIEW = 3,
    PL_INFUSE_SHOW_INVENTORY = 4,
    PL_INFUSE_PRESS_LEFT = 5,
    PL_INFUSE_PRESS_RIGHT = 6,
    PL_INFUSE_PRESS_R = 7,
    PL_INFUSE_PRESS_B = 8,
    PL_INFUSE_END = 9
};

enum {
    PL_BASE_CONVERSATIONS_INIT = 0,
    PL_BASE_CONVERSATIONS_IDLE = 1,
    PL_BASE_CONVERSATIONS_EVENT = 2,
    PL_BASE_CONVERSATIONS_PRESS_B = 3,
    PL_BASE_CONVERSATIONS_END = 4
};

enum {
    PL_MAP_MENU_BIOGRAPHY_INIT = 0,
    PL_MAP_MENU_BIOGRAPHY_IDLE = 1,
    PL_MAP_MENU_BIOGRAPHY_EVENT = 2,
    PL_MAP_MENU_BIOGRAPHY_PRESS_B = 3,
    PL_MAP_MENU_BIOGRAPHY_END = 4
};

enum {
    PL_AUGURY_INIT = 0,
    PL_AUGURY_IDLE = 1,
    PL_AUGURY_EVENT = 2,
    PL_AUGURY_PRESS_B = 3,
    PL_AUGURY_END = 4
};

enum {
    INFUSE_STATE_LIST = 0,
    INFUSE_STATE_INFUSE_UI = 1,
    INFUSE_STATE_CONFIRM = 2,
    INFUSE_STATE_POPUP_WAIT = 3
};

struct InfuseRecipe {
    u8 targetItemId;
    u8 cost;
};

extern const struct InfuseRecipe gInfusionLookupTable[256];

extern u16 gBEXP_Total;
extern u8 gTopVisibleListIndex;
extern u8 gBEXP_State;
extern u8 gBEXP_Applied;
extern u16 gBEXP_MapGain;

enum {
    BEXP_STATE_LIST = 0,
    BEXP_STATE_RTEXT = 1,
    BEXP_STATE_APPLY = 2,
};

enum {
    PL_BEXP_INIT = 0,
    PL_BEXP_SHOW_CURSOR = 1,
    PL_BEXP_IDLE = 2,
    PL_BEXP_PRESS_LEFT = 3,
    PL_BEXP_PRESS_RIGHT = 4,
    PL_BEXP_PRESS_R = 5,
    PL_BEXP_REFRESH_VIEW = 6,
    PL_BEXP_LEVELUP = 7,
    PL_BEXP_PRESS_B = 8,
    PL_BEXP_END = 9
};


struct BexpGains {
    u8 normal;
    u8 boss;
};

extern const struct BexpGains gBexpGainConstants;

extern u8 gList_Total;
extern u8 gBaseConversations_Flags[10];

extern u8 gCharacterBiographyPage;
extern u8 gCharacterBiographyListNumber;

extern u8 gEventReplay_SelectedChapter;
extern u8 gEventReplay_ChapterScrollIndex;

enum {
    PL_EVENT_REPLAY_CHAPTER_INIT = 0,
    PL_EVENT_REPLAY_CHAPTER_IDLE = 1,
    PL_EVENT_REPLAY_CHAPTER_TO_CONV = 2,
    PL_EVENT_REPLAY_CONV_INIT = 3,
    PL_EVENT_REPLAY_CONV_IDLE = 4,
    PL_EVENT_REPLAY_PLAY = 5,
    PL_EVENT_REPLAY_CONV_TO_CHAPTER = 6,
    PL_EVENT_REPLAY_EXIT = 7,
    PL_EVENT_REPLAY_END = 8
};

struct Auguries {
    int bodyTextId;
    int songId;
};

typedef struct {
    int chapter_id;   // ID of the chapter
    int time_seconds; // time in seconds
} TimerAmount;

extern const TimerAmount chapter_timers[];

struct WmSkillMenuProc {
	PROC_HEADER;
	/* 29 */ u8 listCursor;
	/* 2A */ u8 listTop;
	/* 2B */ u8 mode;
	/* 2C */ u8 iconCursor;
	/* 2D */ u8 iconCount;
	/* 2E */ u8 iconRows;
	/* 2F */ u8 _pad;
	/* 30 */ u8 unitCount;
	/* 31 */ u8 hoveredSkill;
	/* 32 */ u16 hoveredHelp;
    /* 34 */ u8 startSkillScreen;
};

struct WmSkillMenuTransitionProc {
	PROC_HEADER;
};

#define WM_SKILL_VISIBLE_COUNT 5
#define WM_SKILL_ICON_COLS 4
#define WM_SKILL_ICON_MAX 12

enum {
	WM_SKILL_MODE_LIST = 0,
	WM_SKILL_MODE_SKILL_TAB = 1,
	WM_SKILL_MODE_SKILL_SCREEN = 2,
};

extern u8 gSavedWorldMapUnitId;
extern u8 gSavedWorldMapXCoordiate;
extern u8 gSavedWorldMapYCoordiate;

enum {
	CUSTOM_PROC_PRESS_A = 0,
	CUSTOM_PROC_PRESS_B = 1,
	CUSTOM_PROC_IDLE = 2,
};

// Tutorial events
extern const EventScr EventScr_Tutorial_SKILL_SCROLL[];
extern const EventScr EventScr_Tutorial_BONUS_EXP[];