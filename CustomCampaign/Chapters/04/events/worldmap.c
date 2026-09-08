#include "../../_shared/worldmap-include.h"

const EventScr EventScrWM_Ch4_SET_NODE[] = {
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_BorgoRidge)
    WM_CENTERCAMONLORD(WM_MU_0)
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    STAL(20)
    WM_LOADLOCATION3(WM_NODE_ZahaWoods)
    WM_SETDESTINATION(WM_NODE_ZahaWoods)
    //WM_CREATENEXTDESTINATION
    WM_WAITFORFX
    STAL(40)
    WM_DRAWPATH(WM_PATH_03)
    STAL(70)
    WM_MAKELORDVISIBLE(WM_MU_0)
    EVBIT_MODIFY(0x1)
    CALL(EventScr_RemoveBGIfNeeded)
    ENDA
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
    WM_SHOWPORTRAIT(0, 0x0002, 0x0534, 0)  // Eirika
    STAL(6)
    STAL(46)
    WM_TEXT(Chapter_04_WM, 0)

    // With her bracelet recovered, Eirika sets out with renewed determination.
    SOUN(SONG_VOICE_CH04_LINE_0001)
    TEXTCONT
    TEXTEND

    WM_MOVESPRITETO(WM_MU_0, 0x0000, WM_NODE_BorgoRidge, WM_NODE_ZahaWoods, -8, 0)

    // Passing through the ancient forest of Za'ha brings them close to Grado's border.
    SOUN(SONG_VOICE_CH04_LINE_0002)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITES(WM_MU_ANY)
    PUTSPRITE(WM_MU_2, CLASS_REVENANT, WM_FACTION_RED, WM_NODE_BorderMulan)
    PUTSPRITE(WM_MU_3, CLASS_REVENANT, WM_FACTION_RED, WM_NODE_BorderMulan)
    WM_PUTSPRITE(WM_MU_2, 0x8c, 0xa8)
    WM_PUTSPRITE(WM_MU_3, 0x9e, 0xa8)
    WM_FADEINSPRITE(WM_MU_2, 60)
    WM_FADEINSPRITE(WM_MU_3, 60)

    // As they near the town of Serafew, Eirika's fears are assuaged by the lack of troops.
    SOUN(SONG_VOICE_CH04_LINE_0003)
    TEXTCONT
    TEXTEND

    // But terrors of old crawl in the shadows of the trees.
    SOUN(SONG_VOICE_CH04_LINE_0004)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITELOAD
    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    WM_REMSPRITE(WM_MU_2)
    WM_REMSPRITE(WM_MU_3)
    ENDA
};
