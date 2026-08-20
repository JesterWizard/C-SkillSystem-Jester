#include "../../_shared/worldmap-include.h"

const EventScr EventScrWM_Ch6_SET_NODE[] = {
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_Serafew)
    WM_CENTERCAMONLORD(WM_MU_0)
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    STAL(20)
    WM_LOADLOCATION3(WM_NODE_AdlasPlains)
    WM_SETDESTINATION(WM_NODE_AdlasPlains)
    // WM_CREATENEXTDESTINATION
    WM_WAITFORFX
    STAL(40)
    WM_DRAWPATH(WM_PATH_05)
    STAL(70)
    WM_MAKELORDVISIBLE(WM_MU_0)
    EVBIT_MODIFY(0x1)
    CALL(EventScr_RemoveBGIfNeeded)
    ENDA
};

const EventScr EventScrWM_Ch6_TRAVEL_TO_NODE[] = {
    MUSCFAST(0x7fff)
    MUSC(SONG_THE_BEGINNING)
    STAL(6)
    ASMC(ReduceBGMVolume)
    WM_MOVECAM(-1, -1, 48, 132, 60, 0)
    WM_WAITFORCAM
    CALL(EventScr_WM_FadeCommon)
    WM_TEXTSTART
    WM_SHOWPORTRAIT(0, 0x0002, 0x0534, 0) // Eirika
    STAL(6)
    STAL(46)
    WM_TEXT(Chapter_06_WM, 0)

    // Eirika suppresses her feelings of unease and continues to search for her brother.
    SOUN(SONG_VOICE_CH06_LINE_0001)
    TEXTCONT
    TEXTEND

    // Passing through Serafew, the group crosses into the Grado Empire.
    SOUN(SONG_VOICE_CH06_LINE_0002)
    TEXTCONT
    TEXTEND

    WM_MOVESPRITETO(WM_MU_0, 0x0002, WM_NODE_Serafew, WM_NODE_AdlasPlains, -4, 0)
    
    // From here forward, there is no doubt that they are walking into hostile territory.
    SOUN(SONG_VOICE_CH06_LINE_0003)
    TEXTCONT
    TEXTEND

    // In hopes of avoiding unnecessary trouble, Eirika and company adjust course slightly.
    SOUN(SONG_VOICE_CH06_LINE_0004)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITES(WM_MU_ANY)
    
    // Aiming to pass through seemingly empty fields as they gradually make their way south.
    SOUN(SONG_VOICE_CH06_LINE_0005)
    TEXTCONT
    TEXTEND

    // Grado's machinations, however, will soon be laid bare before her.
    SOUN(SONG_VOICE_CH06_LINE_0006)
    TEXTCONT
    TEXTEND

    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    ENDA
};
