static const EventScr EventScr_Beginning[] = {
    MUSC(SONG_TENSION)
    LOAD_WAIT(CH1_BREGUET_FORCES)
    STAL(15)
    HIGHLIGHT_COORDINATES(2, 2, 60)
    TEXT_BG(0x26, Chapter_01_Scene_01_Convo_01)
    LOAD_WAIT(CH1_FRELIAN_FORCES)
   // MOVE_WAIT(0, 0xC0, 2, 2)
    ERASE(0xC1)
    HIGHLIGHT_CHARACTER(CHARACTER_BREGUET, 60)
    TEXT_BG(0x1C, Chapter_01_Scene_02_Convo_01)
    MOVE_WAIT(0, CHARACTER_BREGUET, 2, 3)
    /* Force set battle-quotes flag to make the following script-battle not to show battle quote */
    ENUT(EVFLAG_BATTLE_QUOTES)

    START_BATTLE
    CRITICAL_HIT(0, 20)
    NORMAL_DAMAGE(1, 0)
    END_ATTACK
    FIGHT(CHARACTER_BREGUET, 0xC0, 0, 0)

    /* Clear battle Quotes flag */
    ENUF(EVFLAG_BATTLE_QUOTES)

    KILL(0xC0)
    ERASE(0xC0)
    HIGHLIGHT_CHARACTER(CHARACTER_BREGUET, 60)
    TEXT(Chapter_01_Scene_02_Convo_02)
    MOVE_WAIT(0, CHARACTER_BREGUET, 2, 2)
    MOVE_POSITION(24, 1, 6, 1, 3)
    MOVE_POSITION(24, 3, 6, 3, 3)
    MOVE_POSITION(24, 1, 8, 9, 5)
    MOVE_POSITION(24, 2, 7, 8, 3)
    MOVE_POSITION(24, 3, 8, 4, 7)
    MOVE_POSITION_WAIT(24, 2, 9, 2, 8)
    STAL(30)
    HIGHLIGHT_COORDINATES(2, 2, 60)
    TEXT_BG(0x26, Chapter_01_Scene_03_Convo_01)
    LOAD_WAIT_PERSIST(CH1_EIRIKA_SETH)
    SET_UNIT_HP(CHARACTER_SETH, 10)
    HIGHLIGHT_CHARACTER(CHARACTER_EIRIKA, 60)
    TEXT(Chapter_01_Scene_04_Convo_01)

   // ASMC(SetGameOptions_Chapter1)

    /**
     * Temporary flag(11) is used for triggering event: EventScr_Ch1_Turn_EnemyReinforceArrive,
     * this flag will be unset by event: EventScr_Ch1_Misc_Area
     */
    ENUT(EVFLAG_TMP(11))

    NOFADE
    ENDA
};

static const EventScr EventScr_Ending[] = {
    MUSC(SONG_VICTORY)
    TEXT_BG(0x26, Chapter_01_Scene_10_Convo_01)
    REMA
    FADE_FROM_BLACK(16)
    NEXT_CHAPTER_WITH_MAP(0x38)
    ENDA
};

/**
 * Misc events
 */
static const EventListScr EventScr_Talk_FRANZ_SETH[] = {
    ConvoEvent(Chapter_01_Scene_06_Convo_01)
};

static const EventListScr EventScr_Talk_FRANZ_EIRIKA[] = {
    ConvoEvent(Chapter_01_Scene_07_Convo_01)
};

static const EventListScr EventScr_FRANZ_RETURNS[] = {
    MUSC(SONG_COMRADES)
    LOAD_WAIT_PERSIST(CH1_REINFORCEMENTS_ALLIES)
    HIGHLIGHT_CHARACTER(CHARACTER_FRANZ, 60)
    TEXT(Chapter_01_Scene_05_Convo_01)
    NOFADE
    ENDA
};

static const EventListScr EventScr_ENEMY_REINFORCEMENTS[] = {
    REDUCE_VOLUME
    LOAD_WAIT_PERSIST(CH1_REINFORCEMENTS_ENEMIES)
    HIGHLIGHT_CHARACTER(CHARACTER_SOLDIER_83, 60)
    TEXT(Chapter_01_Scene_12_Convo_01)
    RESTORE_VOLUME
    NOFADE
    ENDA
};

static const EventListScr EventListScr_HOUSE_TALK_1[] = {
    HouseEvent(Chapter_01_Scene_08_Convo_01, 0x0)
    END_MAIN
};

static const EventListScr EventListScr_HOUSE_TALK_2[] = {
    HouseEvent(Chapter_01_Scene_09_Convo_01, 0x0)
    END_MAIN
};

static const EventListScr EventScr_Misc_Area[] = {
    SVAL(EVT_SLOT_2, CHARACTER_EIRIKA)
    CALL(EventScr_UnTriggerIfNotUnit)   /* This event may directly ENDB if the condition is not matched */

    ENUF(EVFLAG_TMP(11))
    NOFADE
    ENDA
};

/**
 * Event list
 */
static const EventListScr EventListScr_Turn[] = {
    TURN(0x0, EventScr_FRANZ_RETURNS, 2, 2, FACTION_BLUE)
    TURN(EVFLAG_TMP(11), EventScr_ENEMY_REINFORCEMENTS, 1, 255, FACTION_BLUE)
    END_MAIN
};

static const EventListScr EventListScr_Character[] = {
    CharacterEventBothWays(EVFLAG_TMP(9), EventScr_Talk_FRANZ_SETH, CHARACTER_FRANZ, CHARACTER_SETH)
    CharacterEventBothWays(EVFLAG_TMP(10), EventScr_Talk_FRANZ_EIRIKA, CHARACTER_FRANZ, CHARACTER_EIRIKA)
    END_MAIN
};

// Use 0 for the flag if you want the event to be always available
static const EventListScr EventListScr_Location[] = {
    HOUSE(0, EventListScr_HOUSE_TALK_1, 10, 4)
    HOUSE(0, EventListScr_HOUSE_TALK_2, 13, 6)
    SEIZE(2, 2)
    END_MAIN
};

static const EventListScr EventListScr_Misc[] = {
    AREA(EVFLAG_TMP(10), EventScr_Misc_Area, 0, 0, 7, 9)
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