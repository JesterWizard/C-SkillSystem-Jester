#include "common-chax.h"

#include "constants/texts.h"

const struct ROMChapterData BaseChapterJehanna = {
    .map = {
       .obj1Id = 0xED,
       .obj2Id= 0,
       .paletteId = 0x43,
       .tileConfigId = 0x8D,
       .mainLayerId = 0xD7, // Map pointer. In blocks of 4 per chapter
       .objAnimId = 0x5,
       .paletteAnimId = 0,
       .changeLayerId = 0,
    },
    .initialFogLevel = 0,
    .hasPrepScreen = FALSE,
    .chapTitleId = 0xF, // Base district title
    .chapTitleIdInHectorStory = 0,
    .chapTitleTextId = MSG_BASE_TITLE_JEHANNA,
    .initialPosX = 11,   // Starting position of the camera
    .initialPosY = 14,
    .initialWeather = WEATHER_FINE,
    .battleTileSet = 9,
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
    .mapEventDataId = 0xCF,
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
