static const EventScr EventScr_Beginning[] = {
    MUSC(SONG_SOLVE_THE_RIDDLE)
    SET_BACKGROUND(0x1A)
    TEXT(Chapter_06_Pre_Scene_01_Convo_01)
    MUSC(SONG_NONE)
    FADE_FROM_BLACK(16)
    CLEAN
    MAKE_CAMERA_FOLLOW_MOVING_UNITS
    FADE_TO_BLACK(16)
    LOAD_WAIT(CH6_EIRIKA_SETH)
    STOP_CAMERA_FOLLOW_MOVING_UNITS
    MOVE_CAMERA_TO_POSITION(7, 7)
    LOAD_WAIT_PERSIST(CH6_RENAIS_CITIZENS)
    LOAD_WAIT_PERSIST(CH6_ENEMY_UNITS)
    WARP_CHARACTER_TO_COORDINATES(CHARACTER_NOVALA, 5, 8)
    WARP_CHARACTER_TO_COORDINATES(0xF9, 6, 8)
    MOVE_1STEP(0, CHARACTER_EIRIKA, MOVE_LEFT)
    MOVE_1STEP(0, CHARACTER_SETH, MOVE_LEFT)
    HIGHLIGHT_CHARACTER(CHARACTER_NOVALA, 60)
    SET_BACKGROUND(0x1A)
    TEXT_CHANGE_BGM(Chapter_06_Pre_Scene_02_Convo_01, SONG_RAID)
    SET_BACKGROUND(0x1A)
    TEXT(Chapter_06_Pre_Scene_02_Convo_02)
    FADE_FROM_BLACK(16)
    CLEAN
    FADE_TO_BLACK(16)
    MOVE(0, 0xFB, 20, 5)
    WARP_CHARACTER_OUT(CHARACTER_NOVALA)
    WARP_CHARACTER_OUT(0xF9)
    MOVE_CAMERA_TO_POSITION(19, 5)
    WARP_CHARACTER_TO_COORDINATES(CHARACTER_NOVALA, 19, 6)
    WARP_CHARACTER_TO_COORDINATES(0xF9, 20, 6)
    HIGHLIGHT_CHARACTER(CHARACTER_NOVALA, 60)
    TEXT_BG(0x1E, Chapter_06_Pre_Scene_03_Convo_01)
    STAL(60)
    WARP_CHARACTER_OUT(0xF9)
    WARP_CHARACTER_OUT(0xFB)
    MOVE_CAMERA_TO_POSITION(21, 11)
    WARP_CHARACTER_TO_COORDINATES(0xF9, 26, 12)
    WARP_CHARACTER_TO_COORDINATES(0xFB, 25, 12)
    HIGHLIGHT_CHARACTER(0xF9, 60)
    TEXT_BG(0x1E, Chapter_06_Pre_Scene_03_Convo_02)
    FADE_FROM_BLACK(16)
    PREP
    ENDA
};

static const EventScr EventScr_Ending[] = {
    MUSC(SONG_NONE)
    CHECK_ALIVE(0xfa)
    BEQ(0x0, EVT_SLOT_C, EVT_SLOT_0)
    CHECK_ALIVE(0xfb)
    BEQ(0x0, EVT_SLOT_C, EVT_SLOT_0)
    CHECK_ALIVE(0xf9)
    BEQ(0x0, EVT_SLOT_C, EVT_SLOT_0)
    SET_BACKGROUND(0x1E)
    MUSC(SONG_VICTORY)
    TEXTSHOW(Chapter_06_Post_Scene_01_Convo_01)
    TEXTEND
    REMA
    MUSC(SONG_NONE)
    CALL(EventScr_RemoveBGIfNeeded)
    SVAL(EVT_SLOT_3, ITEM_BOOSTER_POW)
    GIVEITEMTO(CHARACTER_EIRIKA)
    FADE_FROM_BLACK(3)

LABEL(0x0)
    REMA
    SET_BACKGROUND(0x20)
    MUSC(SONG_SORROW)
    FADE_TO_BLACK(3)
    TEXT_NO_REMA(Chapter_06_Post_Scene_02_Convo_01)
    FADE_FROM_BLACK(3)
    REMA
    FADE_TO_BLACK(4)
    TEXT_NO_REMA(Chapter_06_Post_Scene_03_Convo_01)
    FADE_FROM_BLACK(4)
    REMA
    FADE_TO_BLACK(4)
    TEXT_NO_REMA(Chapter_06_Post_Scene_04_Convo_01)
    FADE_FROM_BLACK(4)
    REMA
    FADE_TO_BLACK(4)
    TEXT_NO_REMA(Chapter_06_Post_Scene_05_Convo_01)
    MUSC(SONG_NONE)
    FADE_FROM_BLACK(4)
    REMA
    FADE_TO_BLACK(4)
    TEXT_NO_REMA(Chapter_06_Post_Scene_06_Convo_01)
    FADE_FROM_BLACK(2)
    REMA
    STAL(60)
    TEXTCONT
    TEXTEND
    REMA
    NEXT_CHAPTER_WITH_MAP(CHAPTER_07) // Chapter 7 - Waterside Renvall
    ENDA
};

/**
 * Misc events
 */

static const EventListScr EventScr_CH6_TURN_4_REINFORCEMENTS[] = {
    CHECK_TURNS //Store current turn count in slot C
    SVAL(EVT_SLOT_7, 8)
    BNE(0x0, EVT_SLOT_C, EVT_SLOT_7)
    MOVE_CAMERA_TO_POSITION(13, 0)
    LOAD_WAIT_PERSIST(CH6_TURN_4_REINFORCEMENTS)
    GOTO(0x1)

LABEL(0x0)
    CHECK_EVENTID_
    SADD(EVT_SLOT_2, EVT_SLOT_C, EVT_SLOT_0)
    ENUF_SLOT2
    GOTO(0x1)

LABEL(0x1)
    NOFADE
    ENDA
};

static const EventListScr EventListScr_HOUSE_1_TALK[] = {
    HOUSE_EVENT_NO_END(0x0, Chapter_06_In_Scene_House_1)
    CALL(EventScr_RemoveBGIfNeeded) // This is vital, the game crashes without it for this event
    GIVE_ITEM_TO(ITEM_ANTITOXIN, CHARACTER_EVT_ACTIVE)
    NOFADE
    ENDA
};

const EventListScr EventScr_EndAndResetTriggEvent_JESTER[] = {
    CHECK_EVENTID_
    SADD(EVT_SLOT_2, EVT_SLOT_C, EVT_SLOT_0)
    ENUF(-1)

    NOFADE
    ENDB
};


const EventListScr EventScr_UnTriggerIfNotUnit_JESTER[] = {
    CHECK_ACTIVE
    BEQ(0x0, EVT_SLOT_C, EVT_SLOT_2)

    CALL(EventScr_EndAndResetTriggEvent)

LABEL(0x0)
    ENDA
};

static const EventListScr EventScr_CH6_CHECK_AREA[] = {
    CHECK_ALIVE(0xf9)
    BEQ(0x63, EVT_SLOT_C, EVT_SLOT_0)
    CHECK_INAREA(0xf9, 24, 9, 4, 6)
    BEQ(0x4, EVT_SLOT_C, EVT_SLOT_0)
    SVAL(EVT_SLOT_2, 0xb0)
    CALL(EventScr_UnTriggerIfNotUnit_JESTER)
    MUSC(0x18)
    CAMERA_CAHR(0xf9)
    CUMO_CHAR(0xf9)
    STAL(60)
    CURE
    RANDOMNUMBER(2)
    SVAL(EVT_SLOT_7, 0x1)
    BEQ(0x1, EVT_SLOT_C, EVT_SLOT_7)
    SVAL(EVT_SLOT_7, 0x2)
    BEQ(0x2, EVT_SLOT_C, EVT_SLOT_7)
LABEL(0x0)
    SVAL(EVT_SLOT_2, 0x9ec)
    GOTO(0x3)
LABEL(0x1)
    SVAL(EVT_SLOT_2, 0x9ed)
    GOTO(0x3)
LABEL(0x2)
    SVAL(EVT_SLOT_2, 0x9ee)
LABEL(0x3)
    TEXTSTART
    TEXTSHOW(0xffff)
    TEXTEND
    REMA
LABEL(0x4)
    CALL(EventScr_EndAndResetTriggEvent_JESTER)
LABEL(0x63)
    EVBIT_T(7)
    ENDA
};

/**
 * Event list
 */

static const EventListScr EventListScr_Turn[] = {
    TURN(EVFLAG_TMP(7), EventScr_CH6_TURN_4_REINFORCEMENTS, 1, 255, FACTION_BLUE)
    END_MAIN
};

static const EventListScr EventListScr_Character[] = {
    END_MAIN
};

static const EventListScr EventListScr_Location[] = {
    VILLAGE(EVFLAG_TMP(10), EventListScr_HOUSE_1_TALK, 11, 18)
    END_MAIN
};

static const EventListScr EventListScr_Misc[] = {
    DEFEAT_BOSS(EventScr_Ending)
    CAUSE_GAME_OVER_IF_LORD_DIES
    AFEV(EVFLAG_TMP(7), EventScr_CH6_CHECK_AREA, 0)
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