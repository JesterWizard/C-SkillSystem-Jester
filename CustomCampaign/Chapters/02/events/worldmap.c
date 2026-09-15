#include "../../_shared/worldmap-include.h"

const EventScr EventScrWM_Ch2_SET_NODE[] = {
    WM_OPEN_MAP(CHARACTER_EIRIKA, WM_NODE_CastleFrelia)
    WM_REVEAL_DEST(WM_NODE_Ide)
    MUSC(SONG_THE_BEGINNING)
    TUTORIAL_CHECK(EventScr_Tutorial_SKILL_SCROLL)
    WM_CLOSE_SET_NODE()
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
    WM_SHOW_FACE_WAIT(0, 0x0052, WM_FACE_RIGHT, 46) // HAYDEN
    WM_TEXT(Chapter_02_WM, 0)

    // King Hayden apologizes for being unable to spare soldiers
    WM_VOICE_LINE(SONG_VOICE_CH02_LINE_0001)

    WM_HIDE_FACE_WAIT(0, WM_FACE_SLIDE_RIGHT, 60)
    WM_SHOW_FACE_WAIT(0, 0x002C, WM_FACE_RIGHT, 46) // TANA
    WM_SHOW_FACE(1, 0x0007, WM_FACE_LEFT) // MOULDER

    // However, he does provide Eirika with his trusted vassals and his daughter.
    WM_VOICE_LINE(SONG_VOICE_CH02_LINE_0002)

    WM_HIDE_FACE(0, WM_FACE_SLIDE_RIGHT)
    WM_HIDE_FACE_WAIT(1, WM_FACE_SLIDE_LEFT, 60)
    WM_SHOW_FACE(0, 0x0002, WM_FACE_LEFT) // EIRIKA

    // Eirika is grateful for the king's aid.
    WM_VOICE_LINE(SONG_VOICE_CH02_LINE_0003)

    STAL(46)
    WM_HIDE_FACE(0, WM_FACE_SLIDE_LEFT)
    WM_SHOWDRAWNMAP(0, -8, 0x3)
    STAL(120)
    WM_MOVECAM2(0, -8, 0, 48, 70, 0)
    STAL(85)
    WM_PLACEDOT(0, 0, WM_NODE_Renvall2, 1)

    // Chasing rumors of her brother, she sets out for Grado.
    WM_VOICE_LINE(SONG_VOICE_CH02_LINE_0004)

    STAL(30)
    WM_FXCLEAR1(-0x1)
    WM_FXCLEAR2(-0x1)
    WM_WAITFORFXCLEAR1
    WM_WAITFORFXCLEAR2

    // The group's first stop is the remote village of Ide.
    WM_VOICE_LINE(SONG_VOICE_CH02_LINE_0005)

    WM_MOVESPRITETO(WM_MU_0, 0x0002, WM_NODE_CastleFrelia, WM_NODE_Ide, 200, 30)

    // Which was, until very recently a part of Renais.
    WM_VOICE_LINE(SONG_VOICE_CH02_LINE_0006)

    // Here, Eirika sees the devastation of Renais with her own eyes
    WM_VOICE_LINE(SONG_VOICE_CH02_LINE_0007)

    WM_WAITFORSPRITES(WM_MU_ANY)
    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    ENDA
};
