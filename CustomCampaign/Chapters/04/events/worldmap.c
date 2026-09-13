#include "../../_shared/worldmap-include.h"

const EventScr EventScrWM_Ch4_SET_NODE[] = {
    WM_OPEN_MAP(CHARACTER_EIRIKA, WM_NODE_BorgoRidge)
    WM_REVEAL_DEST(WM_NODE_ZahaWoods, WM_PATH_03)
    WM_CLOSE_SET_NODE()
};

const EventScr EventScrWM_Ch4_TRAVEL_TO_NODE[] = {
    MUSCFAST(0x7fff)
    MUSC(SONG_THE_BEGINNING)
    STAL(6)
    ASMC(ReduceBGMVolume)
    WM_MOVECAM(-1, -1, 22, 94, 60, 0)
    WM_WAITFORCAM
    CALL(EventScr_WM_FadeCommon)
    WM_TEXTSTART
    WM_SHOW_FACE_WAIT(0, 0x0002, WM_FACE_RIGHT, 46)
    WM_TEXT(Chapter_04_WM, 0)

    // With her bracelet recovered, Eirika sets out with renewed determination.
    WM_VOICE_LINE(SONG_VOICE_CH04_LINE_0001)

    WM_MOVESPRITETO(WM_MU_0, 0x0000, WM_NODE_BorgoRidge, WM_NODE_ZahaWoods, -8, 0)

    // Passing through the ancient forest of Za'ha brings them close to Grado's border.
    WM_VOICE_LINE(SONG_VOICE_CH04_LINE_0002)

    WM_WAITFORSPRITES(WM_MU_ANY)
    PUTSPRITE(WM_MU_2, CLASS_REVENANT, WM_FACTION_RED, WM_NODE_BorderMulan)
    PUTSPRITE(WM_MU_3, CLASS_REVENANT, WM_FACTION_RED, WM_NODE_BorderMulan)
    WM_PUTSPRITE(WM_MU_2, 0x8c, 0xa8)
    WM_PUTSPRITE(WM_MU_3, 0x9e, 0xa8)
    WM_FADEINSPRITE(WM_MU_2, 60)
    WM_FADEINSPRITE(WM_MU_3, 60)

    // As they near the town of Serafew, Eirika's fears are assuaged by the lack of troops.
    WM_VOICE_LINE(SONG_VOICE_CH04_LINE_0003)

    // But terrors of old crawl in the shadows of the trees.
    WM_VOICE_LINE(SONG_VOICE_CH04_LINE_0004)

    WM_WAITFORSPRITELOAD
    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    WM_REMSPRITE(WM_MU_2)
    WM_REMSPRITE(WM_MU_3)
    ENDA
};
