#include "../../_shared/worldmap-include.h"

const EventScr EventScrWM_Ch6_SET_NODE[] = {
    WM_OPEN_MAP(CHARACTER_EIRIKA, WM_NODE_Serafew)
    WM_REVEAL_DEST(WM_NODE_AdlasPlains, WM_PATH_05)
    WM_CLOSE_SET_NODE()
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
    WM_SHOW_FACE_WAIT(0, 0x0002, WM_FACE_RIGHT, 46) // Eirika
    WM_TEXT(Chapter_06_WM, 0)

    // Eirika suppresses her feelings of unease and continues to search for her brother.
    WM_VOICE_LINE(SONG_VOICE_CH06_LINE_0001)

    // Passing through Serafew, the group crosses into the Grado Empire.
    WM_VOICE_LINE(SONG_VOICE_CH06_LINE_0002)

    WM_MOVESPRITETO(WM_MU_0, 0x0002, WM_NODE_Serafew, WM_NODE_AdlasPlains, -4, 0)

    // From here forward, there is no doubt that they are walking into hostile territory.
    WM_VOICE_LINE(SONG_VOICE_CH06_LINE_0003)

    // In hopes of avoiding unnecessary trouble, Eirika and company adjust course slightly.
    WM_VOICE_LINE(SONG_VOICE_CH06_LINE_0004)

    WM_WAITFORSPRITES(WM_MU_ANY)

    // Aiming to pass through seemingly empty fields as they gradually make their way south.
    WM_VOICE_LINE(SONG_VOICE_CH06_LINE_0005)

    // Grado's machinations, however, will soon be laid bare before her.
    WM_VOICE_LINE(SONG_VOICE_CH06_LINE_0006)

    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    ENDA
};
