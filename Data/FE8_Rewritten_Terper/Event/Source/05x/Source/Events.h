static const EventScr EventScr_Beginning[] = {
    ASMC(HandleCh5xUnits_Start)
    MUSC(SONG_SOLVE_THE_RIDDLE)
    LOMA(CHAPTER_07) // Renvall Castle 
    MOVE_CAMERA_TO_POSITION_CENTER(10,4)
    FADE_TO_BLACK(16)
    BROWNBOXTEXT(0x657, 2, 2) // Renvall
    HIGHLIGHT_COORDINATES(9, 4, 60)
    FADE_FROM_BLACK(16)
    LOMA(CHAPTER_08) // Inside Renvall Castle
    MOVE_CAMERA_TO_POSITION_CENTER(11, 4)
    LOAD_WAIT(CH5X_TIRADO_SOLDIERS)
    FADE_TO_BLACK(16)
    LOAD_WAIT(CH5X_VALTER_1)
    MOVE_WAIT(16, CHARACTER_VALTER, 10, 4)
    MOVE_WAIT(16, CHARACTER_VALTER, 9, 3)
    MOVE_WAIT(16, CHARACTER_VALTER, 10, 2)
    MOVE_WAIT(16, CHARACTER_TIRADO, 10, 3)
    HIGHLIGHT_CHARACTER(CHARACTER_TIRADO, 60)
    TEXT(Chapter_05x_Pre_Scene_01_Convo_01)
    FADE_FROM_BLACK(16)
    MUSC(SONG_NONE)
    CLEAR_ALL_UNITS
    LOMA(CHAPTER_06) // Victims of war map
    MOVE_CAMERA_TO_POSITION_CENTER(20, 12)
    LOAD_WAIT(CH5X_EPHRAIM_SOLDIERS_1)
    FADE_TO_BLACK(16)
    HIGHLIGHT_CHARACTER(CHARACTER_EPHRAIM, 60)
    SET_BACKGROUND(0x22) // Woods
    MUSC(SONG_TENSION)
    TEXT(Chapter_05x_Pre_Scene_02_Convo_01)
    FADE_FROM_BLACK(16)
    CLEAR_ALL_UNITS
    LOMA(CHAPTER_07) // Renvall
    MOVE_CAMERA_TO_POSITION_CENTER(9, 7)
    FADE_TO_BLACK(16)
    LOAD_WAIT(CH5X_EPHRAIM_SOLDIERS_2)
    HIGHLIGHT_CHARACTER(CHARACTER_EPHRAIM, 60)
    TEXT_BG(0x21, Chapter_05x_Pre_Scene_03_Convo_01)
    MOVE(0, CHARACTER_EPHRAIM, 9, 4)
    STAL(8)
    MOVE(0, CHARACTER_FORDE, 9, 5)
    MOVE(0, CHARACTER_KYLE, 8, 5)
    MOVE(0, CHARACTER_ORSON_CH5X, 8, 6)
    STAL(8)
    FADE_FROM_BLACK(16)
    ENUN
    CLEAR_ALL_UNITS
    LOMA(CHAPTER_05X) // Inside Renvall Castle - Ephraim
    MOVE_CAMERA_TO_POSITION_CENTER(14, 7)
    LOAD_WAIT_PERSIST(CH5X_ENEMY_UNITS)
    FADE_TO_BLACK(16)
    HIGHLIGHT_CHARACTER(CHARACTER_ZONTA, 60)
    TEXT(Chapter_05x_Pre_Scene_04_Convo_01)
    MOVE_CAMERA_TO_POSITION(0, 18)
    LOAD_WAIT_PERSIST(CH5X_PLAYER_UNITS)
    HIGHLIGHT_CHARACTER(CHARACTER_EPHRAIM, 60)
    SET_BACKGROUND(0x10) // Inside Castle Walls
    TEXT(Chapter_05x_Pre_Scene_05_Convo_01)
    MUSC(SONG_NONE)
    ENDA
};

static const EventScr EventScr_Ending[] = {
    ASMC(HandleCh5xUnits_End)
    SET_BACKGROUND(0x10)
    MUSC(SONG_NONE)
    TEXT(Chapter_05x_Post_Scene_01_Convo_01)
    FADE_FROM_BLACK(16)
    CLEAR_ALL_UNITS
    CLEAN
    MOVE_CAMERA_TO_POSITION_CENTER(13, 9)
    LOAD_WAIT(CH5X_EPHRAIM_SOLDIERS_3)
    FADE_TO_BLACK(16)
    HIGHLIGHT_CHARACTER(CHARACTER_ORSON_CH5X, 60)
    TEXT(Chapter_05x_Post_Scene_01_Convo_02)
    FADE_FROM_BLACK(16)
    CLEAR_ALL_UNITS
    LOMA(CHAPTER_07) // Renvall
    MOVE_CAMERA_TO_POSITION_CENTER(10, 4)
    LOAD_WAIT(CH5X_VALTER_SOLDIERS)
    FADE_TO_BLACK(16)
    MAKE_CAMERA_FOLLOW_MOVING_UNITS
    LOAD_WAIT(CH5X_EPHRAIM_SOLDIERS_4)
    STOP_CAMERA_FOLLOW_MOVING_UNITS
    MUSC(SONG_RAID)
    LOAD_WAIT(CH5X_VALTER_WYVERNS)
    LOAD_WAIT(CH5X_VALTER_2)
    HIGHLIGHT_CHARACTER(CHARACTER_VALTER, 60)
    SET_BACKGROUND(0x21)
    TEXT(Chapter_05x_Post_Scene_02_Convo_01)
    FADE_FROM_BLACK(16)
    MUSC(SONG_NONE)
    WmEvtSetUnitOnNode(WM_MU_0, WM_NODE_Serafew)
    // WM_SETDESTINATION(WM_NODE_Serafew)
    NEXT_CHAPTER_WITH_MAP(CHAPTER_06) // Chapter 6 - Victims of war
    ENDA
};

/**
 * Misc events
 */

static const EventScr EventScr_GiveScroll_1[] = {
#if defined(SID_KillStreak) && (COMMON_SKILL_VALID(SID_KillStreak))
    GIVE_SKILL_SCROLL_TO(SID_KillStreak, 0xFFFF)
#endif
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
    DOOR(23, 12, EVFLAG_TMP(11))
    DOOR(4, 10, EVFLAG_TMP(12))
    CHEST(ITEM_ELIXIR, 4, 8, EVFLAG_TMP(13))
    CHEST(ITEM_NONE, 5, 8, EVFLAG_TMP(14))
    SEIZE(13, 7)
    END_MAIN
};

static const EventListScr EventListScr_Misc[] = {
    CAUSE_GAME_OVER_IF_LORD_DIES
    AFEV(EVFLAG_TMP(15), EventScr_GiveScroll_1, EVFLAG_TMP(14))
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