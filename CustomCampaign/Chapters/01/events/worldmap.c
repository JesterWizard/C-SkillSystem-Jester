#include "../../_shared/worldmap-include.h"

const EventScr EventScrWM_Ch1_ENDING[] = {
    CHECK_EVENTID(136)
    BEQ(0x1, EVT_SLOT_C, EVT_SLOT_0)
    CALL(EventScrWM_Intermission_Serafew_To_Frelia)
    GOTO(0x2)

LABEL(0x1)
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_BorderMulan)
    WM_CENTERCAMONLORD(WM_MU_0)
    WM_LOADLOCATION3(WM_NODE_BorderMulan)
    WmEvtSetNodeStateNot2(WM_NODE_BorderMulan) // ENOSUPP in EAstdlib
    WM_SETCAM(0, 12)
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    STAL(20)
    WM_LOADLOCATION3(WM_NODE_CastleFrelia)
    WM_SETDESTINATION(WM_NODE_CastleFrelia)
    // WM_CREATENEXTDESTINATION
    WM_WAITFORFX
    STAL(40)
    WM_DRAWPATH(WM_PATH_00)
    STAL(70)
    MUSC(SONG_TREASURED_MEMORIES)
    STAL(16)
    ASMC(ReduceBGMVolume)
    WM_SHOWTEXTWINDOW(40, 0x0001)
    WM_WAITFORTEXT
    WM_TEXTSTART
    WM_TEXT(Chapter_01_WM, 0)

    // After a fierce battle, Eirika and her companions liberate the captured border castle.
    SOUN(SONG_VOICE_CH01_LINE_0001)
    TEXTCONT
    TEXTEND

    // As she leaves the area with Princess Tana in tow, she cannot help but mourn the lost lives.
    SOUN(SONG_VOICE_CH01_LINE_0002)
    TEXTCONT
    TEXTEND

    SOUN(SONG_VOICE_CH01_LINE_0003)
    TEXTCONT
    TEXTEND

    SOUN(SONG_VOICE_CH01_LINE_0004)
    TEXTCONT
    TEXTEND

    STAL(20)
    WM_MOVESPRITETO(WM_MU_0, 0x0000, WM_NODE_BorderMulan, WM_NODE_CastleFrelia, 200, 40)

    SOUN(SONG_VOICE_CH01_LINE_0005)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITES(WM_MU_ANY)
    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    SKIPWN

LABEL(0x2)
    ENDA
};
