static const EventScr EventScr_Beginning[] = {
    LOAD_WAIT_PERSIST(CH7_ENEMY_UNITS)
    FADE_TO_BLACK(16)
    LOAD_WAIT_DEPLOYMENT_ORDER(CH7_PLAYER_UNITS)
    STAL(15)
    MOVE_CAMERA_TO_POSITION_CENTER(9, 4)
    HIGHLIGHT_COORDINATES(9, 4, 60)
    MOVE_CAMERA_TO_POSITION(0, 21)
    HIGHLIGHT_CHARACTER(CHARACTER_EIRIKA, 60)
    SET_BACKGROUND(0x21)
    TEXT_NO_REMA(Chapter_07_Pre_Scene_01_Convo_01)
    MUSC(SONG_RAY_OF_HOPE)
    TEXT_CONSECUTIVE(Chapter_07_Pre_Scene_Franz)
    TEXT_CONSECUTIVE(Chapter_07_Pre_Scene_Gilliam_Moulder_Tana)
    TEXT_CONSECUTIVE(Chapter_07_Pre_Scene_Ross_Garcia)
    TEXT_CONSECUTIVE(Chapter_07_Pre_Scene_Neimi_Colm)
    TEXT_CONSECUTIVE(Chapter_07_Pre_Scene_Lute_Artur)
    TEXT_CONSECUTIVE(Chapter_07_Pre_Scene_Joshua)
    TEXT_CONSECUTIVE(Chapter_07_Pre_Scene_Natasha)
    TEXT_CONSECUTIVE(Chapter_07_Pre_Scene_02_Convo_01)
    PREP
    FADE_FROM_BLACK(16)
    ENDA
};

static const EventScr EventScr_Ending[] = {
    FADE_FROM_BLACK(16)
    LOMA(0x44)
    CLEAR_ALL_UNITS
    FADE_TO_BLACK(16)
    MUSC(SONG_SE_RIVER)
    LOAD_WAIT(CH7_EIRIKA_SETH)
    HIGHLIGHT_CHARACTER(CHARACTER_EIRIKA, 60)
    TEXT(Chapter_07_Post_Scene_01_Convo_01)
    MOVE_1STEP(0, CHARACTER_SETH, MOVE_RIGHT)
    MOVE_1STEP(0, CHARACTER_EIRIKA, MOVE_LEFT)
    LOAD_WAIT(CH7_ORSON)
    HIGHLIGHT_CHARACTER(CHARACTER_ORSON, 60)
    REDUCE_VOLUME
    TEXT_BG(0x10, Chapter_07_Post_Scene_02_Convo_01)
    MOVE_1STEP(0, CHARACTER_ORSON, MOVE_LEFT)
    ENUN
    MOVE(0, CHARACTER_ORSON, 9, 0)
    SVAL(EVT_SLOT_D, 0x0)
    SVAL(EVT_SLOT_1, 0x109)
    SENQUEUE1
    SVAL(EVT_SLOT_1, 0x0)
    SENQUEUE1
    SVAL(EVT_SLOT_1, 0x9)
    SENQUEUE1
    SVAL(EVT_SLOT_1, 0x0)
    SENQUEUE1
    MOVE_DEFINED(CHARACTER_EIRIKA)
    SVAL(EVT_SLOT_D, 0x0)
    SVAL(EVT_SLOT_1, 0x10a)
    SENQUEUE1
    SVAL(EVT_SLOT_1, 0x0)
    SENQUEUE1
    SVAL(EVT_SLOT_1, 0xa)
    SENQUEUE1
    SVAL(EVT_SLOT_1, 0x0)
    SENQUEUE1
    MOVE_DEFINED(CHARACTER_SETH)
    STAL(8)
    FADE_FROM_BLACK(16)
    ENUN
    NEXT_CHAPTER_WITH_MAP(CHAPTER_08)
    ENDA
};

/**
 * Misc events
 */

static const EventListScr EventListScr_HOUSE_1_TALK[] = {
    HOUSE_EVENT_NO_END(0x0, Chapter_07_In_Scene_House_1)
    CALL(EventScr_RemoveBGIfNeeded) // This is vital, the game crashes without it for this event
    // GIVE_ITEM_TO(ITEM_SWORD_ARMORSLAYER, CHARACTER_EVT_ACTIVE)
    NOFADE
    ENDA
};
static const EventListScr EventListScr_HOUSE_2_TALK[] = {
    HOUSE_EVENT_NO_END(0x0, Chapter_07_In_Scene_House_2)
    CALL(EventScr_RemoveBGIfNeeded) // This is vital, the game crashes without it for this event
    // GIVE_ITEM_TO(ITEM_BOOSTER_DEF, CHARACTER_EVT_ACTIVE)
    NOFADE
    ENDA
};

/**
 * Event list
 */

static const EventListScr EventListScr_Turn[] = {
    END_MAIN
};

static const EventListScr EventListScr_Character[] = {
    END_MAIN
};

static const EventListScr EventListScr_Location[] = {
    VILLAGE(EVFLAG_TMP(10), EventListScr_HOUSE_1_TALK, 1, 15)
    VILLAGE(EVFLAG_TMP(11), EventListScr_HOUSE_2_TALK, 17, 21)
    SEIZE(9, 4)
    END_MAIN
};

static const EventListScr EventListScr_Misc[] = {
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
