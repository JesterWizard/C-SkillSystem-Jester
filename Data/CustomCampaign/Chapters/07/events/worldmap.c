#include "../../_shared/worldmap-include.h"

const EventScr EventScrWM_Ch7_SET_NODE[] = {
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_AdlasPlains)
    WM_CENTERCAMONLORD(WM_MU_0)
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    STAL(20)
    WM_LOADLOCATION3(WM_NODE_Renvall1)
    WM_SETDESTINATION(WM_NODE_Renvall1)
    // WM_CREATENEXTDESTINATION
    WM_WAITFORFX
    STAL(40)
    WM_DRAWPATH(WM_PATH_06)
    STAL(70)
    WM_MAKELORDVISIBLE(WM_MU_0)
    EVBIT_MODIFY(0x1)
    CALL(EventScr_RemoveBGIfNeeded)
    ENDA
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
    SOUN(SONG_VOICE_CH07_LINE_0001)
    TEXTCONT
    TEXTEND

    WM_SHOWPORTRAIT(0, 0x0002, 0x0534, 0) // Eirika
    STAL(6)
    STAL(46)

    // Now Eirika must find a way to rescue her brother from beneath enemy eyes.
    SOUN(SONG_VOICE_CH07_LINE_0002)
    TEXTCONT
    TEXTEND

    WM_MOVESPRITETO(WM_MU_0, 0x0002, WM_NODE_AdlasPlains, WM_NODE_Renvall1, -8, 0)
    
    // Eirika's company rushes toward Castle Renvall, where Ephraim is being held.
    SOUN(SONG_VOICE_CH07_LINE_0003)
    TEXTCONT
    TEXTEND

    // Surrounded by lakes, Renvall is a natural fortress, all but unassailable.
    SOUN(SONG_VOICE_CH07_LINE_0004)
    TEXTCONT
    TEXTEND

    // A lone bridge, held by the enemy, is its only entrance.
    SOUN(SONG_VOICE_CH07_LINE_0005)
    TEXTCONT
    TEXTEND

    // Eirika has only one choice.
    SOUN(SONG_VOICE_CH07_LINE_0006)
    TEXTCONT
    TEXTEND

    // She resolves to confront Grado's soldiers head on...
    SOUN(SONG_VOICE_CH07_LINE_0007)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITES(WM_MU_ANY)
    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    WM_REMSPRITE(WM_MU_2)
    ENDA
};
