#include "../../_shared/worldmap-include.h"

const EventScr EventScrWM_Ch7_SET_NODE[] = {
    WM_OPEN_MAP(CHARACTER_EIRIKA, WM_NODE_AdlasPlains)
    WM_REVEAL_DEST(WM_NODE_Renvall1, WM_PATH_06)
    WM_CLOSE_SET_NODE()
};

const EventScr EventScrWM_Ch7_TRAVEL_TO_NODE[] = {
    MUSCFAST(0x7fff)
    MUSC(SONG_THE_BEGINNING)
    STAL(6)
    ASMC(ReduceBGMVolume)
    WM_MOVECAM(-1, -1, 84, 152, 60, 0)
    WM_WAITFORCAM
    CALL(EventScr_WM_FadeCommon)
    WM_TEXTSTART
    WM_TEXT(Chapter_07_WM, 0)

    // Rumors say that Ephraim has been defeated and taken prisoner.
    WM_VOICE_LINE(SONG_VOICE_CH07_LINE_0001)

    WM_SHOW_FACE_WAIT(0, 0x0002, WM_FACE_RIGHT, 46) // Eirika

    // Now Eirika must find a way to rescue her brother from beneath enemy eyes.
    WM_VOICE_LINE(SONG_VOICE_CH07_LINE_0002)

    WM_MOVESPRITETO(WM_MU_0, 0x0002, WM_NODE_AdlasPlains, WM_NODE_Renvall1, -8, 0)

    // Eirika's company rushes toward Castle Renvall, where Ephraim is being held.
    WM_VOICE_LINE(SONG_VOICE_CH07_LINE_0003)

    // Surrounded by lakes, Renvall is a natural fortress, all but unassailable.
    WM_VOICE_LINE(SONG_VOICE_CH07_LINE_0004)

    // A lone bridge, held by the enemy, is its only entrance.
    WM_VOICE_LINE(SONG_VOICE_CH07_LINE_0005)

    // Eirika has only one choice.
    WM_VOICE_LINE(SONG_VOICE_CH07_LINE_0006)

    // She resolves to confront Grado's soldiers head on...
    WM_VOICE_LINE(SONG_VOICE_CH07_LINE_0007)

    WM_WAITFORSPRITES(WM_MU_ANY)
    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    WM_REMSPRITE(WM_MU_2)
    ENDA
};
