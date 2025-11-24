static const EventScr EventScr_Beginning[] = {
    LOAD_WAIT(CH8_EIRIKA_SETH_ORSON)
    HIGHLIGHT_CHARACTER(CHARACTER_ORSON, 60)
    SET_BACKGROUND(0x10)
    TEXT_NO_REMA(Chapter_08_Pre_Scene_01_Convo_01)
    CALL(EventScr_TextShowWithFadeIn)
    LOAD_WAIT(CH8_TIRADO_AND_SOLDIERS)
    HIGHLIGHT_CHARACTER(CHARACTER_TIRADO, 60)
    MUSC(SONG_RAID)
    TEXT(Chapter_08_Pre_Scene_01_Convo_02)
    STAL(30)
    CHANGE_TO_RED(CHARACTER_ORSON)
    HIGHLIGHT_CHARACTER(CHARACTER_ORSON, 60)
    MOVE(0x0, CHARACTER_ORSON, 20, 19)
    SVAL(EVT_SLOT_B, 0x100014)
    MOVE_1STEP(0x0, CHAR_EVT_POSITION_AT_SLOTB, FACING_RIGHT)
    ENUN
    MOVE(0x0, CHARACTER_ORSON, 20, 15)
    ENUN
    SVAL(EVT_SLOT_B, 0x100015)
    MOVE_1STEP(0x0, CHAR_EVT_POSITION_AT_SLOTB, FACING_LEFT)
    MOVE(0x0, CHARACTER_ORSON, 19, 10)
    ENUN
    ERASE(CHARACTER_ORSON)
    HIGHLIGHT_CHARACTER(CHARACTER_TIRADO, 60)
    TEXT_BG(0x10, Chapter_08_Pre_Scene_01_Convo_03)
    SVAL(EVT_SLOT_B, 0x100013)
    MOVE_1STEP(0x0, CHAR_EVT_POSITION_AT_SLOTB, FACING_LEFT)
    ENUN
    MOVE(0x0, CHARACTER_TIRADO, 19, 14)
    ENUN
    SVAL(EVT_SLOT_B, 0x100012)
    MOVE_1STEP(0x0, CHAR_EVT_POSITION_AT_SLOTB, FACING_RIGHT)
    ENUN
    MOVE(0x0, CHARACTER_TIRADO, 19, 14)
    ENUN
    TEXT(Chapter_08_Pre_Scene_02_Convo_01)
    SVAL(EVT_SLOT_B, 0x150014)
    SOUN(0xab)
    TILECHANGE(0xffff)
    MOVE(0x0, CHARACTER_TIRADO, 19, 10)
    SVAL(EVT_SLOT_B, 0x100013)
    MOVE(0x10, CHAR_EVT_POSITION_AT_SLOTB, 19, 11)
    SVAL(EVT_SLOT_B, 0x100014)
    MOVE(0x10, CHAR_EVT_POSITION_AT_SLOTB, 20, 11)
    ENUN
    CLEAR_ENEMY_UNITS
    LOAD_WAIT_PERSIST(CH8_ENEMY_UNITS)
    HIGHLIGHT_CHARACTER(CHARACTER_EIRIKA, 60)
    SET_BACKGROUND(0x10)
    TEXT(Chapter_08_Pre_Scene_03_Convo_01)
    PREP
    ENUT(15) // Temporary flag for special reinforcements
    ENDA
};

static const EventScr EventScr_Ending[] = {
    MUSC(SONG_NONE)
    SET_BACKGROUND(0x10)
    TEXT(Chapter_08_Post_Scene_01_Convo_01)
    FADE_FROM_BLACK(12)
    MUSC(SONG_LYON)
    SET_BACKGROUND(0x35)
    FADE_TO_BLACK(3)
    TEXT(Chapter_08_Post_Scene_01_Convo_02)
    FADE_FROM_BLACK(4)
    SET_BACKGROUND(0x10)
    // FADE_TO_BLACK(3)
    TEXT(Chapter_08_Post_Scene_01_Convo_03)
    FADE_FROM_BLACK(1)
    MUSC(SONG_NONE)
    SET_FLAG(0x88) // Flag for New Departure
    NEXT_CHAPTER_WITH_MAP(CHAPTER_05)
    ENDA
};

/**
 * Misc events
 */

static const EventListScr EventScr_CH8_MAGE_REINFORCEMENTS[] = {
    CHECK_EVENTID(0xC)
    BNE(0x0, EVT_SLOT_C, EVT_SLOT_0) // check if flag C has been set
    CHECK_INAREA(-1, 5, 0, 15, 10) // check if active unit stops in this range
    BEQ(0x0, EVT_SLOT_C, EVT_SLOT_0) // check if 
    MOVE_CAMERA_TO_POSITION(2, 10)
    LOAD_WAIT_PERSIST(CH8_MAGE_REINFORCEMENTS)
    ENUF(0xC) // set flag C
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

static const EventListScr EventScr_CH8_TURN_5_REINFORCEMENTS[] = {
    CHECK_TURNS //Store current turn count in slot C
    SVAL(EVT_SLOT_7, 5)
    BNE(0x0, EVT_SLOT_C, EVT_SLOT_7)
    MOVE_CAMERA_TO_POSITION(0, 23)
    LOAD_WAIT_PERSIST(CH8_TURN_5_REINFORCEMENTS)
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

static const EventListScr EventScr_EPHRAIM_ESCAPES[] = {
    CHECK_TURNS //Store current turn count in slot C
    SVAL(EVT_SLOT_7, 2)
    BNE(0x0, EVT_SLOT_C, EVT_SLOT_7)
    MOVE_CAMERA_TO_POSITION(0,23)
    LOAD_WAIT_PERSIST(CH8_EPHRAIM_ESCAPES)
    REVEAL(CHARACTER_EPHRAIM)
    REVEAL(CHARACTER_FORDE)
    REVEAL(CHARACTER_KYLE)
    SVAL(EVT_SLOT_1, 0x1)
    SET_STATE(CHARACTER_EPHRAIM)
    SVAL(EVT_SLOT_1, 0x1)
    SET_STATE(CHARACTER_FORDE)
    SVAL(EVT_SLOT_1, 0x1)
    SET_STATE(CHARACTER_KYLE)
    ASMC(RemoveInventories)
    MUSC(SONG_COMRADES)
    HIGHLIGHT_CHARACTER(CHARACTER_EPHRAIM, 60)
    TEXT(Chapter_08_In_Scene_Ephraim_Escapes)
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

static const EventListScr EventScr_Talk_EIRIKA_EPHRAIM[] = {
    ConvoEvent(Chapter_08_In_Scene_Ephraim_Eirika)
};

static const EventListScr EventScr_Talk_SETH_KYLE[] = {
    ConvoEvent(Chapter_08_In_Scene_Seth_Kyle)
};

static const EventListScr EventScr_Talk_FRANZ_FORDE[] = {
    ConvoEvent(Chapter_08_In_Scene_Forde_Franz)
};


/**
 * Event list
 */

static const EventListScr EventListScr_Turn[] = {
    TURN(EVFLAG_TMP(7), EventScr_EPHRAIM_ESCAPES, 1, 255, FACTION_RED)
    TURN(EVFLAG_TMP(8), EventScr_CH8_MAGE_REINFORCEMENTS, 1, 255, FACTION_BLUE)
    TURN(EVFLAG_TMP(9), EventScr_CH8_TURN_5_REINFORCEMENTS, 1, 255, FACTION_RED)
    END_MAIN
};

static const EventListScr EventListScr_Character[] = {
    CharacterEventBothWays(EVFLAG_TMP(10), EventScr_Talk_EIRIKA_EPHRAIM, CHARACTER_EIRIKA, CHARACTER_EPHRAIM)
    CharacterEventBothWays(EVFLAG_TMP(11), EventScr_Talk_SETH_KYLE, CHARACTER_SETH, CHARACTER_KYLE)
    CharacterEventBothWays(EVFLAG_TMP(12), EventScr_Talk_FRANZ_FORDE, CHARACTER_FRANZ, CHARACTER_FORDE)
    END_MAIN
};

static const EventListScr EventListScr_Location[] = {
    DOOR(1, 5, EVFLAG_TMP(13))
    DOOR(20, 7, EVFLAG_TMP(14))
    CHEST(ITEM_SWORD_SILVER, 1, 3, EVFLAG_TMP(16))
    CHEST(ITEM_MASTERSEAL, 2, 3, EVFLAG_TMP(17))
    CHEST(ITEM_BOOSTER_HP, 19, 4, EVFLAG_TMP(18))
    SEIZE(10, 2)
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