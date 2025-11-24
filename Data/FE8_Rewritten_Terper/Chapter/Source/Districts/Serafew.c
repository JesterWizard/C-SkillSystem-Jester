#include "common-chax.h"

#include "constants/texts.h"

const struct ROMChapterData BaseChapter05 = {
    .map = {
       .obj1Id = 0xE,
       .obj2Id= 0,
       .paletteId = 0xF,
       .tileConfigId = 0x10,
       .mainLayerId = 0xCA, // Map pointer. In blocks of 4 per chapter
       .objAnimId = 0x12,
       .paletteAnimId = 0,
       .changeLayerId = 0x5,
    },
    .initialFogLevel = 0,
    .hasPrepScreen = FALSE,
    .chapTitleId = 0x3B, // Chapter 5 - The Empire's Reach
    .chapTitleIdInHectorStory = 0,
    .chapTitleTextId = MSG_BASE_TITLE_SERAFEW,
    .initialPosX = 14,   // Starting position of the camera
    .initialPosY = 20,
    .initialWeather = WEATHER_FINE,
    .battleTileSet = 6,
    .easyModeLevelMalus = 0,
    .difficultModeLevelBonus = 0,
    .normalModeLevelMalus = 1,
    .mapBgmIds = {
        [MAP_BGM_BLUE] = SONG_DISTANT_ROADS,
        [MAP_BGM_RED] = SONG_SHADOW_OF_THE_ENEMY,
        [MAP_BGM_GREEN] = SONG_BINDING_VOW,
        [MAP_BGM_BLUE_HECTOR] = 13,
        [MAP_BGM_RED_HECTOR] = 21,
        [MAP_BGM_GREEN_HECTOR] = 15,
        [MAP_BGM_BLUE_GREEN_ALT] = 65535,
        [MAP_BGM_RED_ALT] = 65535,
        [MAP_BGM_PROLOGUE_LYN] = 11,
        [MAP_BGM_PROLOGUE] = 65535,
        [MAP_BGM_PROLOGUE_HECTOR] = 65535,
    },
    .mapCrackedWallHeath = 50,
    .mapEventDataId = 0xCB,
    .gmapEventId = 0x0,
    .victorySongEnemyThreshold = SONG_THE_VALIANT,
    .fadeToBlack = TRUE,
    .statusObjectiveTextId = CC_DEFEAT_SAAR,
    .goalWindowTextId = 414,
    .goalWindowDataType = GOAL_TYPE_DEFEAT_BOSS,
    .goalWindowEndTurnNumber = 0,
    .protectCharacterIndex = 0,
    .destPosX = 255,
    .destPosY = 0,
    .unk91 = 29,
    .unk92 = 0,
    .unk93 = 0,
};