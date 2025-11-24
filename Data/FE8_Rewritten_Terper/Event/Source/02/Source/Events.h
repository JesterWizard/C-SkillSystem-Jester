// --- Opening Cinematics ---
static const EventScr EventScr_Beginning[] = {
    MUSC(SONG_ADVANCE)
    TEXT_BG(0x18, Chapter_02_Scene_01_Convo_01)
    FADE_FROM_BLACK(16)
    LOAD_WAIT_PERSIST(CH2_TANA)
    MUSCMID(SONG_NONE)
    LOAD_WAIT_PERSIST(CH2_BANDITS)
    LOAD_WAIT_PERSIST(CH2_BAZBA)
    STAL(30)
    MUSC(SONG_DEFENSE)
    HIGHLIGHT_CHARACTER(CHARACTER_BONE, 60)
    TEXT(Chapter_02_Scene_02_Convo_01)
    MOVE_WAIT(24, CHARACTER_BAZBA, 14, 9)
    ERASE(CHARACTER_BAZBA)
    HIGHLIGHT_COORDINATES(12, 3, 60)
    MUSC(SONG_TENSION)
    TEXT_BG(0x2, Chapter_02_Scene_03_Convo_01)
    HIGHLIGHT_CHARACTER(CHARACTER_BONE, 60)
    TEXT(Chapter_02_Scene_04_Convo_01)
    MOVE_WAIT(24, CHARACTER_BONE, 9, 14)
    MOVE_POSITION_WAIT(0, 12, 5, 12, 3)
    SOUN(SONG_SE_VILLAGE_DESTROYED)
    TILECHANGE_COORDINATES(12, 2)
    SOUN(SONG_SE_WEAPON_BROKE)
    NOTIFY(NOTIFY_village_was_destroyed, SONG_NONE)
    LOAD_WAIT_PERSIST(CH2_ROSS_GARCIA)
    SET_UNIT_HP(CHARACTER_ROSS, 5)
    HIGHLIGHT_CHARACTER(CHARACTER_ROSS, 60)
    TEXT(Chapter_02_Scene_05_Convo_01)
    LOAD_WAIT_PERSIST(CH2_PLAYER_UNITS)
    HIGHLIGHT_CHARACTER(CHARACTER_TANA, 60)
    TEXT(Chapter_02_Scene_06_Convo_01)
    MOVE_WAIT(24, CHARACTER_TANA, 2, 3)
    HIGHLIGHT_CHARACTER(CHARACTER_TANA, 60)
    TEXT(Chapter_02_Scene_06_Convo_02)
    HIGHLIGHT_CHARACTER(CHARACTER_MOULDER, 60)
    TEXT(Chapter_02_Scene_06_Convo_03)
    MOVE_WAIT(24, CHARACTER_TANA, 5, 3)
    NOFADE
    // ASMC(InitShopStock) // Look in External/LimitedShopStock/Installer.event for details
    ENDA
};

static const EventScr EventScr_Ending[] = {
    MUSC(SONG_VICTORY)
    TEXT_BG_HIDE_MAP(0x1D, Chapter_02_Scene_07_Convo_01)
    FADE_FROM_BLACK(16)
    TEXT_BG_HIDE_MAP(0x4, Chapter_02_Scene_08_Convo_01)
    FADE_FROM_BLACK(16)
    MUSCSLOW(SONG_NONE)
    SET_BACKGROUND(0x20)
    TEXT(Chapter_02_Scene_09_Convo_01)
    FADE_TO_WHITE(4)
    SET_BACKGROUND(0x16)
    MUSC(SONG_SE_BIRDS_CHIRPING)
    BROWNBOXTEXT(THREE_YEARS_AGO, 2, 2)
    TEXT(Chapter_02_Scene_10_Convo_01)
    FADE_TO_WHITE(4)
    TEXT_BG_HIDE_MAP(0x20, Chapter_02_Scene_11_Convo_01)
    FADE_FROM_BLACK(16)
    TEXT_BG_HIDE_MAP(0x20, Chapter_02_Scene_12_Convo_01)
    FADE_FROM_BLACK(16)
    NEXT_CHAPTER_WITH_MAP(CHAPTER_03)
    ENDA
};

// --- Talk Scenes ---
static const EventListScr EventScr_Talk_ROSS_EIRIKA[] = {
    CHANGE_MUSIC_SAVE_PREVIOUS_MUSIC(SONG_COMRADES)
    STAL(33)
    TEXT(Chapter_02_Ross_Eirika)
    RESTORE_PREVIOUS_MUSIC
    CHANGE_TO_BLUE(CHARACTER_ROSS)
    NOFADE
    ENDA
};

static const EventListScr EventScr_Talk_ROSS_GARCIA[] = {
    CHANGE_MUSIC_SAVE_PREVIOUS_MUSIC(SONG_COMRADES)
    STAL(33)
    ConvoEvent(Chapter_02_Ross_Garcia)
    RESTORE_PREVIOUS_MUSIC
    NOFADE
    ENDA
};

static const EventListScr EventListScr_VILLAGE_1_TALK[] = {
    CHECK_ACTIVE
    SVAL(EVT_SLOT_1, CHARACTER_EIRIKA)
    BEQ(0x0, EVT_SLOT_C, EVT_SLOT_1)
    GOTO(0x1)

LABEL(0x0)
    HOUSE_EVENT_NO_END(0x2, Chapter_02_Village_01_Eirika)
    GOTO(0x2)

LABEL(0x1)
    HOUSE_EVENT_NO_END(0x2, Chapter_02_Village_01_Generic)
    GOTO(0x3)

LABEL(0x2)
    CALL(EventScr_RemoveBGIfNeeded) // This is vital, the game crashes without it for this event
#if defined(SID_GoldDigger) && (COMMON_SKILL_VALID(SID_GoldDigger))
    GIVE_SKILL_SCROLL_TO(SID_GoldDigger, CHARACTER_EVT_ACTIVE)
#endif
    GOTO(0x4)

LABEL(0x3)
    CALL(EventScr_RemoveBGIfNeeded) // This is vital, the game crashes without it for this event
    GIVE_ITEM_TO(ITEM_REDGEM, CHARACTER_EVT_ACTIVE)
    GOTO(0x4)

LABEL(0x4)
    NOFADE
    ENDA
};

static const EventListScr EventListScr_VILLAGE_2_TALK[] = {
    HOUSE_EVENT_NO_END(0x2, Chapter_02_Village_02)
    CALL(EventScr_RemoveBGIfNeeded)
    GIVE_ITEM_TO(ITEM_ELIXIR, CHARACTER_EVT_ACTIVE)
    NOFADE
    ENDA
};

static const EventListScr EventListScr_VILLAGE_3_TALK[] = {
    HOUSE_EVENT_NO_END(0x2, Chapter_02_Village_03)
    CALL(EventScr_RemoveBGIfNeeded)
    GIVE_ITEM_TO(ITEM_ANTITOXIN, CHARACTER_EVT_ACTIVE)
    NOFADE
    ENDA
};

// --- Shop Events ---
static const u16 ShopList_Event_WeaponShop[] = {
    ITEM_SWORD_SLIM,
    ITEM_SWORD_IRON,
    ITEM_LANCE_SLIM,
    ITEM_LANCE_IRON,
    ITEM_AXE_IRON,
    ITEM_NONE,
};

// --- Turn Events ---
static const EventListScr EventListScr_Turn[] = {
    END_MAIN
};

// - Facilitate Talk Scenes ---
static const EventListScr EventListScr_Character[] = {
    CharacterEventBothWays(EVFLAG_TMP(7), EventScr_Talk_ROSS_EIRIKA, CHARACTER_ROSS, CHARACTER_EIRIKA)
    CharacterEventBothWays(EVFLAG_TMP(8), EventScr_Talk_ROSS_GARCIA, CHARACTER_ROSS, CHARACTER_GARCIA)
    END_MAIN
};

// --- Facilitate Location Scenes ---
static const EventListScr EventListScr_Location[] = {
    VILLAGE(EVFLAG_TMP(9), EventListScr_VILLAGE_1_TALK, 1, 12)
    VILLAGE(EVFLAG_TMP(10), EventListScr_VILLAGE_2_TALK, 7, 2)
    VILLAGE(EVFLAG_TMP(11), EventListScr_VILLAGE_3_TALK, 4, 2)
    ARMORY(ShopList_Event_WeaponShop, 5, 7)
    END_MAIN
};

// --- Other Scenes ---
static const EventListScr EventListScr_Misc[] = {
    DEFEAT_ALL(EventScr_Ending)
    CAUSE_GAME_OVER_IF_LORD_DIES
    END_MAIN
};

static const EventListScr EventListScr_SelectUnit[] = {
    END_MAIN
};

static const EventListScr EventListScr_SelectDestination[] = {
    END_MAIN
};

static const EventListScr EventListScr_UnitMove[] = {
    END_MAIN
};

static void const * const EventListScr_Tutorial[] = {
    NULL
};