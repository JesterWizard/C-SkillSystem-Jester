#include "../../_shared/worldmap-include.h"

const EventScr EventScrWM_Ch3_SET_NODE[] = {
    WM_OPEN_MAP(CHARACTER_EIRIKA, WM_NODE_Ide)
    WM_REVEAL_DEST(WM_NODE_BorgoRidge)
    MUSC(SONG_THE_BEGINNING)
    SVAL(EVT_SLOT_2, EventScrWM_Ch3_BeginningTutorial)
    CALL(EventScr_CallOnTutorialMode)
    WM_CLOSE_SET_NODE()
};

const EventScr EventScrWM_Ch3_TRAVEL_TO_NODE[] = {
    MUSCFAST(0x7fff)
    MUSC(SONG_THE_BEGINNING)
    STAL(6)
    ASMC(ReduceBGMVolume)
    WM_MOVECAM(-1, -1, 0, 72, 60, 0)
    WM_WAITFORCAM
    CALL(EventScr_WM_FadeCommon)
    WM_TEXTSTART
    PUTSPRITE(WM_MU_3, CLASS_EIRIKA_LORD, WM_FACTION_BLUE, WM_NODE_Ide)
    WM_MAKELORDVISIBLE(WM_MU_3)
    WM_MAKELORDDISAPPEAR(WM_MU_0)
    PUTSPRITE(WM_MU_2, CLASS_THIEF, WM_FACTION_GREEN, WM_NODE_Ide)
    WM_MOVESPRITETO(WM_MU_2, 0x0003, WM_NODE_Ide, WM_NODE_BorgoRidge, -4, 0)
    WM_WAITFORSPRITES(WM_MU_2)
    WM_REMSPRITE(WM_MU_2)
    WM_SHOW_FACE_WAIT(0, 0x0002, WM_FACE_RIGHT, 46) // Eirika
    WM_TEXT(Chapter_03_WM, 0)

    // Eirika and company pursue the thief who stole her bracelet
    WM_VOICE_LINE(SONG_VOICE_CH03_LINE_0001)

    WM_MAKELORDVISIBLE(WM_MU_0)
    WM_REMSPRITE(WM_MU_3)
    WM_HIDE_FACE(0, WM_FACE_SLIDE_RIGHT)
    PUTSPRITE(WM_MU_2, CLASS_BRIGAND, WM_FACTION_RED, WM_NODE_BorgoRidge)
    WM_FADEINSPRITE(WM_MU_2, 60)
    WM_SHOW_FACE_WAIT(0, 0x0033, WM_FACE_LEFT, 46) // Bazba

    // Their chase leads them into the mountains,mwhere Bazba's bandits pillage at will.
    WM_VOICE_LINE(SONG_VOICE_CH03_LINE_0002)

    // Eirika cannot ignore her countrymen's pleas for help.
    WM_VOICE_LINE(SONG_VOICE_CH03_LINE_0003)

    WM_HIDE_FACE(0, WM_FACE_SLIDE_LEFT)
    WM_FADEOUTSPRITE(WM_MU_2, 60)
    WM_MOVESPRITETO(WM_MU_0, 0x0000, WM_NODE_Ide, WM_NODE_BorgoRidge, -4, 0)
    WM_WAITFORSPRITES(WM_MU_ANY)
    WM_WAITFORSPRITELOAD
    WM_REMSPRITE(WM_MU_2)

    // She leads her group in search of the bandits' stronghold
    WM_VOICE_LINE(SONG_VOICE_CH03_LINE_0004)

    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    ENDA
};
