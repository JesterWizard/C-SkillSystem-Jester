#include "../../_shared/worldmap-include.h"

const EventScr EventScrWM_Ch2_SET_NODE[] = {
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_CastleFrelia)
    WM_CENTERCAMONLORD(WM_MU_0)
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    STAL(20)
    WM_LOADLOCATION3(WM_NODE_Ide)
    WM_SETDESTINATION(WM_NODE_Ide)
    //WM_CREATENEXTDESTINATION
    WM_WAITFORFX
    STAL(40)
    WM_DRAWPATH(WM_PATH_01)
    STAL(70)
    MUSC(SONG_THE_BEGINNING)
    TUTORIAL_CHECK(EventScr_Tutorial_SKILL_SCROLL)
    WM_MAKELORDVISIBLE(WM_MU_0)
    EVBIT_MODIFY(0x1)
    CALL(EventScr_RemoveBGIfNeeded)
    ENDA
};

const EventScr EventScrWM_Ch2_TRAVEL_TO_NODE[] = {
    MUSCFAST(0x7fff)
    MUSC(SONG_THE_BEGINNING)
    STAL(6)
    ASMC(ReduceBGMVolume)
    WM_MOVECAM(-1, -1, 0, 32, 45, 0)
    WM_WAITFORCAM
    CALL(EventScr_WM_FadeCommon)
    WM_TEXTSTART
    WM_SHOWPORTRAIT(0, 0x0052, 0x0534, 0) // HAYDEN
    STAL(6)
    WM_TEXT(Chapter_02_WM, 0)

    // King Hayden apologizes for being unable to spare soldiers
    SOUN(SONG_VOICE_CH02_LINE_0001)
    TEXTCONT
    TEXTEND

    WM_CLEARPORTRAIT(0, 0x0634, 0)
    STAL(60)
    WM_SHOWPORTRAIT(0, 0x002C, 0x0534, 0) // TANA
    STAL(6)
    STAL(46)
    WM_SHOWPORTRAIT(1, 0x0007, 0x02BC, 0) // MOULDER
    STAL(6)

    // However, he does provide Eirika with his trusted vassals and his daughter.
    SOUN(SONG_VOICE_CH02_LINE_0002)
    TEXTCONT
    TEXTEND

    WM_CLEARPORTRAIT(0, 0x0634, 0)
    STAL(46)
    WM_CLEARPORTRAIT(1, 0x01BC, 0)
    STAL(60)
    WM_SHOWPORTRAIT(0, 0x0002, 0x02BC, 0) //EIRIKA
    STAL(6)

    // Eirika is grateful for the king's aid.
    SOUN(SONG_VOICE_CH02_LINE_0003)
    TEXTCONT
    TEXTEND

    STAL(46)
    WM_CLEARPORTRAIT(0, 0x01BC, 0)
    STAL(46)
    WM_SHOWDRAWNMAP(0, -8, 0x3)
    STAL(120)
    WM_MOVECAM2(0, -8, 0, 48, 70, 0)
    STAL(85)
    WM_PLACEDOT(0, 0, WM_NODE_Renvall2, 1)

    // Chasing rumors of her brother, she sets out for Grado.
    SOUN(SONG_VOICE_CH02_LINE_0004)
    TEXTCONT
    TEXTEND

    STAL(30)
    WM_FXCLEAR1(-0x1)
    WM_FXCLEAR2(-0x1)
    WmEvtRemoveBigMap // WM_WAITFORFXCLEAR1
    WmEvtWaitBigMapRemove // WM_WAITFORFXCLEAR2

    // The group's first stop is the remote village of Ide.
    SOUN(SONG_VOICE_CH02_LINE_0005)
    TEXTCONT
    TEXTEND

    WM_MOVESPRITETO(WM_MU_0, 0x0002, WM_NODE_CastleFrelia, WM_NODE_Ide, 200, 30)

    // Which was, until very recently a part of Renais.
    SOUN(SONG_VOICE_CH02_LINE_0006)
    TEXTCONT
    TEXTEND

    // Here, Eirika sees the devastation of Renais with her own eyes
    SOUN(SONG_VOICE_CH02_LINE_0007)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITES(WM_MU_ANY)
    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    ENDA
};
