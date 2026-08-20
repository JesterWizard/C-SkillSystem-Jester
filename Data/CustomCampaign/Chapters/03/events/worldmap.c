#include "../../_shared/worldmap-include.h"

const EventScr EventScrWM_Ch3_SET_NODE[] = {
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_Ide)
    WM_CENTERCAMONLORD(WM_MU_0)
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    STAL(20)
    WM_LOADLOCATION3(WM_NODE_BorgoRidge)
    WM_SETDESTINATION(WM_NODE_BorgoRidge)
    //WM_CREATENEXTDESTINATION
    WM_WAITFORFX
    STAL(40)
    WM_DRAWPATH(WM_PATH_02)
    STAL(70)
    MUSC(SONG_THE_BEGINNING)
    SVAL(EVT_SLOT_2, EventScrWM_Ch3_BeginningTutorial)
    CALL(EventScr_CallOnTutorialMode)
    WM_MAKELORDVISIBLE(WM_MU_0)
    EVBIT_MODIFY(0x1)
    CALL(EventScr_RemoveBGIfNeeded)
    ENDA
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
    WM_SHOWPORTRAIT(0, 0x0002, 0x0534, 0) // Eirika
    STAL(6)
    STAL(46)
    WM_TEXT(Chapter_03_WM, 0)

    // Eirika and company pursue the thief who stole her bracelet
    SOUN(SONG_VOICE_CH03_LINE_0001)
    TEXTCONT
    TEXTEND

    WM_MAKELORDVISIBLE(WM_MU_0)
    WM_REMSPRITE(WM_MU_3)
    WM_CLEARPORTRAIT(0, 0x0634, 0)
    STAL(46)
    PUTSPRITE(WM_MU_2, CLASS_BRIGAND, WM_FACTION_RED, WM_NODE_BorgoRidge)
    WM_FADEINSPRITE(WM_MU_2, 60)
    WM_SHOWPORTRAIT(0, 0x0033, 0x02BC, 0) // Bazba
    STAL(6)
    STAL(46)

    // Their chase leads them into the mountains,mwhere Bazba's bandits pillage at will.
    SOUN(SONG_VOICE_CH03_LINE_0002)
    TEXTCONT
    TEXTEND

    // Eirika cannot ignore her countrymen's pleas for help.
    SOUN(SONG_VOICE_CH03_LINE_0003)
    TEXTCONT
    TEXTEND

    WM_CLEARPORTRAIT(0, 0x01BC, 0)
    STAL(46)
    WM_FADEOUTSPRITE(WM_MU_2, 60)
    WM_MOVESPRITETO(WM_MU_0, 0x0000, WM_NODE_Ide, WM_NODE_BorgoRidge, -4, 0)
    WM_WAITFORSPRITES(WM_MU_ANY)
    WM_WAITFORSPRITELOAD
    WM_REMSPRITE(WM_MU_2)

    // She leads her group in search of the bandits' stronghold
    SOUN(SONG_VOICE_CH03_LINE_0004)
    TEXTCONT
    TEXTEND

    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    ENDA
};
