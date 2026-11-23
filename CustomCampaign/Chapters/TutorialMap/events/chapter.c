#include "common-chax.h"
#include "constants/texts.h"

/*
 * Extras > Tutorials' map (Documentation/Features/Tutorials.md, "The maps"): vanilla's chapter row 0x43 (TUTORIAL_MAP),
 * a filler row with no title that nothing uses. The map is vanilla Chapter 4's (without its tile changes); each
 * tutorial puts its own units on it.
 */
const struct ROMChapterData ChapterTutorialMap = {
    .map = {
       .obj1Id = 1,
       .obj2Id = 0,
       .paletteId = 2,
       .tileConfigId = 3,
       .mainLayerId = 0x15,   /* Chapter 4's map: 15x15, plains and forests round a river */
       .objAnimId = 5,
       .paletteAnimId = 0,
       .changeLayerId = 0,
    },
    .initialFogLevel = 0,
    .hasPrepScreen = FALSE,
    .chapTitleId = 0x43,   /* the chapter-name routine (Chname) draws this row's chapTitleTextId */
    .chapTitleIdInHectorStory = 0,
    .chapTitleTextId = MSG_TUTMAP_TITLE,
    .initialPosX = 7,
    .initialPosY = 5,
    .initialWeather = WEATHER_FINE,
    .battleTileSet = 0,
    .easyModeLevelMalus = 0,
    .difficultModeLevelBonus = 0,
    .normalModeLevelMalus = 0,
    .mapBgmIds = {
        [MAP_BGM_BLUE] = SONG_DISTANT_ROADS,
        [MAP_BGM_RED] = SONG_SHADOW_OF_THE_ENEMY,
        [MAP_BGM_GREEN] = SONG_SHADOW_OF_THE_ENEMY,
        [MAP_BGM_BLUE_HECTOR] = 13,
        [MAP_BGM_RED_HECTOR] = 21,
        [MAP_BGM_GREEN_HECTOR] = 15,
        [MAP_BGM_BLUE_GREEN_ALT] = 65535,
        [MAP_BGM_RED_ALT] = 65535,
        [MAP_BGM_PROLOGUE_LYN] = 11,
        [MAP_BGM_PROLOGUE] = 65535,
        [MAP_BGM_PROLOGUE_HECTOR] = 65535,
    },
    .prepScreenNumber = 0,
    .mapCrackedWallHeath = 50,
    .mapEventDataId = 0xF2,
    .gmapEventId = 0,
    .victorySongEnemyThreshold = SONG_THE_VALIANT,
    .fadeToBlack = TRUE,
    .statusObjectiveTextId = 0x1A2,
    .goalWindowTextId = 414,
    /*
     * Not GOAL_TYPE_DEFEAT_ALL: with no enemy left, vanilla's phase change (RunPhaseSwitchEvents) ends the chapter
     * (CallEndEvent), and its fade out and in came before the lesson's own way back (Staff EXP's map has no enemy). A
     * tutorial ends with its lesson (Tutorials/Source/TutorialGuides.c); the goal window shows the text alone.
     */
    .goalWindowDataType = GOAL_TYPE_SPECIAL,
    .goalWindowEndTurnNumber = 0,
    .protectCharacterIndex = 0,
    .destPosX = 255,
    .destPosY = 0,
    .unk91 = 29,
    .unk92 = 0,
    .unk93 = 0,
};
