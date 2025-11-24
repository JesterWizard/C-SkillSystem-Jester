/**
 * Main events
 */
static const EventScr EventScr_Beginning[] = {
    MUSC(SONG_LEGACY)
    TEXT_BG_HIDE_MAP(0x18, Chapter_03_Scene_01_Convo_01)
    MUSC(SONG_TENSION)
    TEXT_BG_HIDE_MAP(0x1D, Chapter_03_Scene_02_Convo_01)
    FADE_FROM_BLACK(16)
    CLEAN
    LOAD_WAIT_PERSIST(CH3_BANDITS)
    FADE_TO_BLACK(16)
    LOAD_WAIT_PERSIST(CH3_PLAYER_UNITS)
    HIGHLIGHT_CHARACTER(CHARACTER_NEIMI, 60)
    TEXT(Chapter_03_Scene_03_Convo_01)
    MOVE_CAMERA_TO_UNIT(CHARACTER_BAZBA)
    MUSC(SONG_SHADOW_OF_THE_ENEMY)
    HIGHLIGHT_CHARACTER(CHARACTER_BAZBA, 60)
    TEXT(Chapter_03_Scene_03_Convo_02)
    NOFADE
    ENDA
};

static const EventScr EventScr_Ending[] = {
    MUSC(SONG_VICTORY)
    TEXT_BG_HIDE_MAP(0x28, Chapter_03_Scene_04_Convo_01)
/* If Colm wasn't recruited, then have him join */
    CHECK_ALLEGIANCE(CHARACTER_COLM)
	SVAL(EVT_SLOT_1, 0x1)
	BEQ(0x1, EVT_SLOT_C, EVT_SLOT_1)
    GOTO(0x2)
LABEL(0x1)
    FADE_FROM_BLACK(16)
    CHANGE_TO_BLUE(CHARACTER_COLM)
    GOTO(0x2)
LABEL(0x2)
    TEXT_BG_HIDE_MAP(0x29, Chapter_03_Scene_05_Convo_01)
    SOUN(SONG_NONE)
    FADE_FROM_BLACK(2)
    CLEAR_ALL_UNITS
    LOMA(OUTSIDE_GRADO_CASTLE)
    FADE_TO_BLACK(16)
    BROWNBOXTEXT(LOCATION_Grado_Keep, 2, 2)
    HIGHLIGHT_COORDINATES(10, 4, 60)
    TEXT_BG_HIDE_MAP(0xC, Chapter_03_Scene_06_Convo_01)
    FADE_FROM_BLACK(16)
    CLEAR_ALL_UNITS // CLEAN
    LOMA(INSIDE_GRADO_CASTLE) // This is broken - https://github.com/JesterWizard/C-SkillSystem-Mokha/issues/307
    MOVE_CAMERA_TO_POSITION_CENTER(14,2)
    LOAD_WAIT(CH3_GRADO_1)
    FADE_TO_BLACK(16)
    MUSC(SONG_SOLVE_THE_RIDDLE)
    HIGHLIGHT_CHARACTER(CHARACTER_VIGARDE, 60)
    TEXT(Chapter_03_Scene_07_Convo_01)
    SOUN(SONG_SE_UNLOCKING_DOOR)
    TILECHANGE_COORDINATES(14, 8)
    MOVE_TWICE_WAIT(36, CHARACTER_DUESSEL, 14, 4, 12, 4)
    MOVE_1STEP_WAIT(36, CHARACTER_GLEN, MOVE_LEFT)
    MOVE_1STEP_WAIT(36, CHARACTER_SELENA, MOVE_RIGHT)
    LOAD_WAIT(CH3_GRADO_2)
    HIGHLIGHT_CHARACTER(CHARACTER_VIGARDE, 60)
    TEXT(Chapter_03_Scene_07_Convo_02)
    FADE_FROM_BLACK(16)
    NEXT_CHAPTER_WITH_MAP(0x4) // Chapter 4 - Ancient Horrors
    ENDA
};

/**
 * Misc events
 */
static const EventListScr EventScr_COLM_APPEARS[] = {
    CHECK_TURNS //Store current turn count in slot C
    SVAL(EVT_SLOT_7, 1)
    BNE(0x0, EVT_SLOT_C, EVT_SLOT_7)
    MOVE_CAMERA_TO_POSITION_CENTER(0, 0)
    STAL(15)
    LOAD_WAIT_PERSIST(CH3_COLM)
    MUSC(SONG_BINDING_VOW)
    HIGHLIGHT_CHARACTER(CHARACTER_COLM, 60)
    TEXT(Chapter_03_Colm_Appears)
    MOVE_CLOSEST_ENUN(0, CHARACTER_COLM, 2, 4)
    GOTO(0x1)

LABEL(0x0)
    CHECK_EVENTID_
    SADD(EVT_SLOT_2, EVT_SLOT_C, EVT_SLOT_0)
    ENUF_SLOT2

LABEL(0x1)
    NOFADE
    ENDA
};

static const EventListScr EventScr_ROSS_APPEARS[] = {
    CHECK_TURNS //Store current turn count in slot C
    SVAL(EVT_SLOT_7, 3)
    BNE(0x0, EVT_SLOT_C, EVT_SLOT_7)
    STAL(15)
    MOVE_CAMERA_TO_POSITION_CENTER(3, 11)
    LOAD_WAIT_PERSIST(CH3_ROSS)
    MUSC(SONG_BINDING_VOW)
    HIGHLIGHT_CHARACTER(CHARACTER_ROSS, 30)
    TEXT(Chapter_03_Ross_Appears)
    GOTO(0x1)

LABEL(0x0)
    CHECK_EVENTID_
    SADD(EVT_SLOT_2, EVT_SLOT_C, EVT_SLOT_0)
    ENUF_SLOT2

LABEL(0x1)
    NOFADE
    ENDA
};

static const EventListScr EventScr_Talk_ROSS_EIRIKA[] = {
    CHANGE_MUSIC_SAVE_PREVIOUS_MUSIC(SONG_COMRADES)
    STAL(33)
    TEXT(Chapter_03_Ross_Eirika)
    RESTORE_PREVIOUS_MUSIC
    // CHANGE_TO_BLUE(CHARACTER_ROSS)
    NOFADE
    ENDA
};

static const EventListScr EventScr_Talk_COLM_NEIMI[] = {
    CHANGE_MUSIC_SAVE_PREVIOUS_MUSIC(SONG_COMRADES)
    STAL(33)
    TEXT(Chapter_03_Colm_Neimi)
    RESTORE_PREVIOUS_MUSIC
    CHANGE_TO_BLUE(CHARACTER_COLM)
    NOFADE
    ENDA
};

/**
 * Event list
 */

static const EventListScr EventListScr_Turn[] = {
    TURN(EVFLAG_TMP(7), EventScr_COLM_APPEARS, 1, 255, FACTION_BLUE)
    TURN(EVFLAG_TMP(8), EventScr_ROSS_APPEARS, 1, 255, FACTION_BLUE)
    END_MAIN
};

static const EventListScr EventListScr_Character[] = {
    CharacterEventBothWays(EVFLAG_TMP(9), EventScr_Talk_COLM_NEIMI, CHARACTER_COLM, CHARACTER_NEIMI)
    CharacterEventBothWays(EVFLAG_TMP(10), EventScr_Talk_ROSS_EIRIKA, CHARACTER_ROSS, CHARACTER_EIRIKA)
    END_MAIN
};

static const EventListScr EventListScr_Location[] = {
    DOOR(2, 3, EVFLAG_TMP(11))
    DOOR(10, 5, EVFLAG_TMP(12))
    DOOR(6, 10, EVFLAG_TMP(13))
    CHEST(ITEM_BOW_BEACONBOW, 6, 12, EVFLAG_TMP(14))
    CHEST(ITEM_AXE_FIENDCLEAVER, 10, 3, EVFLAG_TMP(15))
    CHEST(ITEM_LANCE_BRIGHTLANCE, 8, 3, EVFLAG_TMP(16))
    CHEST(ITEM_SWORD_SHADOWKILLR, 6, 3, EVFLAG_TMP(17))
    SEIZE(14, 1)
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