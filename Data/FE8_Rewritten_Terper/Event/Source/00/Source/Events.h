static const EventScr EventScr_Beginning[] = {
    LOAD_MAP_XY(17, 13, CHAPTER_16)
    STAL(16)
    LOAD_WAIT(Renais_Castle_Allies)
    FADE_TO_BLACK(16)
    MUSC(SONG_RAID)
    BROWNBOXTEXT(BB_Renais_Castle, 2, 2)
    LOAD_WAIT(Panicked_Soldier_Allies)
    CAMERA(17, 2)
    HIGHLIGHT_CHARACTER(CHARACTER_EPHRAIM, 60)
    TEXT(Chapter_00_Scene_01_Convo_01)
    MOVE_WAIT(0, CHARACTER_EPHRAIM, 13, 11)
    ERASE(CHARACTER_EPHRAIM)
    MOVE_1STEP_WAIT(0, CHARACTER_EIRIKA, MOVE_LEFT)
    HIGHLIGHT_CHARACTER(CHARACTER_EIRIKA, 60)
    TEXT(Chapter_00_Scene_01_Convo_02)
    MOVEONTO_WAIT(0, CHARACTER_SETH, CHARACTER_EIRIKA)
    ERASE(CHARACTER_EIRIKA)
    HIGHLIGHT_CHARACTER(CHARACTER_SETH, 60)
    TEXT(Chapter_00_Scene_01_Convo_03)
    MOVE(0, CHARACTER_SETH, 13, 11)
    MOVE_TWICE_WAIT(0, CHARACTER_FRANZ, 12, 4, 12, 11)
    ERASE(CHARACTER_SETH)
    ERASE(CHARACTER_FRANZ)
    MOVE(0, CHARACTER_MOULDER, 11, 4)
    MOVE(0, CHARACTER_VANESSA, 15, 4)
    ENUN
    MOVE_1STEP(0, CHARACTER_MOULDER, MOVE_RIGHT)
    MOVE_1STEP(0, CHARACTER_VANESSA, MOVE_LEFT)
    ENUN
    LOAD_WAIT(Renais_Castle_Enemy1)
    LOAD_WAIT(Renais_Castle_Enemy2)
    LOAD_WAIT(Renais_Castle_Enemy3)
    HIGHLIGHT_CHARACTER(CHARACTER_FADO, 60)
    TEXT(Chapter_00_Scene_01_Convo_04)
    FADE_FROM_BLACK(2)
    REMA
    SKIP_POINT
    CLEAR_ALL_UNITS

    /* Escape Castle Renais */
    LOAD_MAP_XY(0, 0, OUTSIDE_RENAIS)
    FADE_TO_BLACK(16)
    LOAD_WAIT(Renais_Runaways_Allies)
    HIGHLIGHT_CHARACTER(CHARACTER_SETH, 60)
    TEXT_BG(0x1D, Chapter_00_Scene_02_Convo_01)
    MOVE_THRICE_WAIT(0, CHARACTER_FRANZ, 4, 4, 4, 2, 0, 2)
    ERASE(CHARACTER_FRANZ)
    HIGHLIGHT_CHARACTER(CHARACTER_SETH, 60)
    TEXT_BG(0x1D, Chapter_00_Scene_02_Convo_02)
    MOVE_1STEP(0, CHARACTER_SETH, MOVE_RIGHT)
    MOVE_1STEP(0, CHARACTER_EIRIKA, MOVE_LEFT)
    LOAD_WAIT(Renais_Outside_Castle_Enemy)
    HIGHLIGHT_CHARACTER(CHARACTER_VALTER_PROLOGUE, 60)
    TEXT_BG(0x1D, Chapter_00_Scene_02_Convo_03)
    MOVE_1STEP_WAIT(0, CHARACTER_VALTER_PROLOGUE, MOVE_LEFT)

    /* Battle scene */
    START_BATTLE
    NORMAL_DAMAGE(0, 0)
    NORMAL_DAMAGE(1, 0)
    END_ATTACK
    FIGHT(CHARACTER_SETH, CHARACTER_VALTER_PROLOGUE, 0, false)
    
    /* Escape Valter */
    HIGHLIGHT_CHARACTER(CHARACTER_VALTER_PROLOGUE, 60)
    TEXT(Chapter_00_Scene_02_Convo_04)
    MOVE_1STEP_WAIT(0, CHARACTER_SETH, MOVE_LEFT)
    ERASE(CHARACTER_EIRIKA)
    MOVE_THRICE_WAIT(0, CHARACTER_SETH, 4, 4, 4, 2, 0, 2)
    ERASE(CHARACTER_SETH)
    HIGHLIGHT_CHARACTER(CHARACTER_VALTER_PROLOGUE, 60)
    TEXT(Chapter_00_Scene_02_Convo_05)
    FADE_FROM_BLACK(16)
    REMA
    SKIP_POINT
    CLEAR_ALL_UNITS

    /* Highway map with O'Neil and company */
    LOAD_MAP_XY(0, 0, PROLOGUE)
    FADE_TO_BLACK(16)
    LOAD_WAIT_PERSIST(Eirika_Seth_Allies)
    SET_UNIT_HP(CHARACTER_SETH, 10)
    HIGHLIGHT_CHARACTER(CHARACTER_EIRIKA, 60)
    REDUCE_VOLUME
    TEXT_BG(0x1D, Chapter_00_Scene_03_Convo_01)
    MOVE_WAIT(0, CHARACTER_SETH, 4, 4)
    HIGHLIGHT_CHARACTER(CHARACTER_SETH, 60)
    TEXT(Chapter_00_Scene_03_Convo_02)
    MOVE_WAIT(0, CHARACTER_EIRIKA, 4, 5)
    HIGHLIGHT_CHARACTER(CHARACTER_SETH, 60)
    TEXT(Chapter_00_Scene_03_Convo_03)
    GIVE_ITEM_TO(ITEM_SWORD_RAPIER, CHARACTER_EIRIKA)
    LOAD_WAIT(Grado_Reinforcements_Enemy)
    HIGHLIGHT_CHARACTER(CHARACTER_ONEILL, 60)
    TEXT(Chapter_00_Scene_03_Convo_04)
    ASMC(SetGameOptions)
    NOFADE
    ENDA
};

static const EventScr EventScr_Ending[] = {
    MUSC(SONG_VICTORY)
    TEXT_BG(0x1D, Chapter_00_Scene_03_Convo_06)
    FADE_FROM_BLACK(16)
    NEXT_CHAPTER_WITHOUT_MAP(CHAPTER_01)
    REMA
    ENDA
};

/*
** Misc events
*/

static const EventListScr EventScr_PROLOGUE_TURN_3_WARNING[] = {
    CHECK_TURNS //Store current turn count in slot C
    SVAL(EVT_SLOT_7, 3)
    BNE(0x0, EVT_SLOT_C, EVT_SLOT_7)
    // HIGHLIGHT_CHARACTER(CHARACTER_SETH)
    TEXT(Chapter_00_Seth_Warning)
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

static const EventListScr EventScr_PROLOGUE_TURN_4_REINFORCEMENTS[] = {
    CHECK_TURNS //Store current turn count in slot C
    SVAL(EVT_SLOT_7, 4)
    BNE(0x0, EVT_SLOT_C, EVT_SLOT_7)
    LOAD_WAIT_PERSIST(PROLOGUE_TURN_4_REINFORCEMENTS)
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

static const EventListScr EventListScr_ONeillAttack[] = {
    CHECK_ENEMIES // Check how many enemies remain and save the result in EVT_SLOT_C
    SVAL(EVT_SLOT_7, 1) // Put the value we want to check for in EVT_SLOT_7
    BNE(0x0, EVT_SLOT_C, EVT_SLOT_7)
    MUSC(SONG_TRUTH_DESPAIR_AND_HOPE)
    TEXT(Chapter_00_ONeill_Agro)
    SVAL(EVT_SLOT_1, 0x0)
    CHAI(CHARACTER_ONEILL)
    GOTO(0x1)

// Preserves the flag if the condition isn't met
LABEL(0x0)
    CHECK_EVENTID_
    SADD(EVT_SLOT_2, EVT_SLOT_C, EVT_SLOT_0)
    ENUF_SLOT2

LABEL(0x1)
    NOFADE
    ENDA
};

static const EventListScr EventListScr_OneEnemyLeft[] = {
    CHECK_ENEMIES
    SVAL(EVT_SLOT_7, 1)
    BNE(0x0, EVT_SLOT_C, EVT_SLOT_7)
    HIGHLIGHT_CHARACTER(CHARACTER_EIRIKA, 60)
    TEXT(Chapter_00_Scene_03_Convo_05)
    /* this unsets the event ID so the next turn Oneill will agro (see TURN events) */
    ENUF(EVFLAG_TMP(8))
    GOTO(0x1)

// Preserves the flag if the condition isn't met
LABEL(0x0)
    CHECK_EVENTID_
    SADD(EVT_SLOT_2, EVT_SLOT_C, EVT_SLOT_0)
    ENUF_SLOT2

LABEL(0x1)
    NOFADE
    ENDA
};

static const EventListScr EventScr_Talk_EIRIKA_SETH[] = {
// #ifdef CONFIG_FORGING
//     ASMC(CallForgeMenuASMC)
// #endif

// #ifdef CONFIG_LIGHTS_OUT_GAME
//     PLAY_LIGHTS_OUT_GAME(DIMENSIONS_3x3, ICON_COUNT_2, CAN_SKIP, ITEM_SWORD_AUDHULMA, FLAG_10)
// #endif

// #ifdef CONFIG_VESLY_CREDITS_SEQUENCE
//     FADE_TO_BLACK(16) // slow fade to black (optional)
//     ASMC(StartCreditsProc_ASMC)
// #endif
};

/**
 * Event lists
 */

static const EventListScr EventListScr_Turn[] = {
    TURN(EVFLAG_TMP(8), EventListScr_ONeillAttack, 1, 255, FACTION_RED)
    // TURN(EVFLAG_TMP(9), EventScr_PROLOGUE_TURN_3_WARNING, 1, 255, FACTION_BLUE)
    // TURN(EVFLAG_TMP(10), EventScr_PROLOGUE_TURN_4_REINFORCEMENTS, 1, 255, FACTION_BLUE)
    END_MAIN
};

static const EventListScr EventListScr_Character[] = {
    //CharacterEventBothWays(EVFLAG_TMP(9), EventScr_Talk_EIRIKA_SETH, CHARACTER_EIRIKA, CHARACTER_SETH)
    END_MAIN
};

static const EventListScr EventListScr_Location[] = {
    // ESCAPE(1, 0)
    END_MAIN
};

static const EventListScr EventListScr_Misc[] = {
    DEFEAT_ALL(EventScr_Ending)
    AFEV(EVFLAG_TMP(7), EventListScr_OneEnemyLeft, 0)
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