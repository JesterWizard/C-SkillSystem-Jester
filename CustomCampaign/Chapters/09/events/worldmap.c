#include "../../_shared/worldmap-include.h"

const EventScr EventScrWM_Ch9_SET_NODE[] = {
    ASMC(SetMode)
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_CastleFrelia)
    WM_CENTERCAMONLORD(WM_MU_0)
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    STAL(26)
    WM_MOVECAM(-1, -1, 32, 0, 90, 0)
    WM_WAITFORCAM
    STAL(6)
    STAL(20)
    WM_LOADLOCATION3(WM_NODE_TowerOfValni)
    WM_WAITFORFX
    STAL(40)
    WM_DRAWPATH(WM_PATH_09)
    STAL(70)
    STAL(20)
    WM_LOADLOCATION3(WM_NODE_PortKiris)
    WM_SETDESTINATION(WM_NODE_PortKiris)
    // WM_CREATENEXTDESTINATION
    WM_WAITFORFX
    STAL(40)
    WM_DRAWPATH(WM_PATH_08)
    STAL(70)
    WmEvtMoveCamToUnit(-1, -1, WM_MU_0, 46, 0) // ENOSUPP in EAstdlib
    STAL(60)
    WM_WAITFORCAM
    MUSC(SONG_TREASURED_MEMORIES)
    // CALL(EventScrWM_08A3A70C)
    EvtTextStartType5 // These are the events for the above call, up to the commented out ENDA
    SVAL(EVT_SLOT_B, 0x54000c)
    // TEXTSHOW(0x8e5)
    // TEXTEND
    // REMA
    ENUT(229)
    ENUT(235)
    // ENDA
    WM_MAKELORDVISIBLE(WM_MU_0)
    EVBIT_MODIFY(0x1)
    CALL(EventScr_RemoveBGIfNeeded)
    ENDA
};

const EventScr EventScrWM_Ch9_TRAVEL_TO_NODE[] = {
    MUSCFAST(0x7fff)
    MUSC(SONG_THE_BEGINNING)
    STAL(6)
    ASMC(ReduceBGMVolume)
    WM_MOVECAM(-1, -1, 64, 0, 60, 0)
    WM_WAITFORCAM
    CALL(EventScr_WM_FadeCommon)
    STAL(60)
    WM_TEXTSTART
    WM_SHOWDRAWNMAP(0, -8, 0x3)
    STAL(120)
    WM_HIGHLIGHT(WM_NATION_Rausten)
    STAL(30)
    WM_TEXT(Chapter_09_WM, 0)

    // The twins choose to travel by sea to the theocracy of Rausten.
    SOUN(SONG_VOICE_CH09_LINE_0001)
    TEXTCONT
    TEXTEND

    WM_HIGHLIGHTCLEAR1(WM_NATION_Rausten)
    WM_HIGHLIGHTCLEAR2(WM_NATION_Rausten)
    WmEvtRemoveBigMap // WM_WAITFORFXCLEAR1
    WmEvtWaitBigMapRemove // WM_WAITFORFXCLEAR2
    WM_MOVESPRITETO(WM_MU_0, 0x0002, WM_NODE_BorderMulan, WM_NODE_PortKiris, -4, 0)

    // Their search for a ship leads them to Port Kiris in Carcino.
    SOUN(SONG_VOICE_CH09_LINE_0002)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITES(WM_MU_ANY)
    WM_MAKELORDDISAPPEAR(WM_MU_0)
    WM_SHOWDRAWNMAP(0, -8, 0x3)
    STAL(120)
    WM_HIGHLIGHT(WM_NATION_Carcino)
    STAL(30)

    // Carcino is a young nation ruled by a council of merchants, and it is an ally of Frelia.
    SOUN(SONG_VOICE_CH09_LINE_0003)
    TEXTCONT
    TEXTEND

    WM_HIGHLIGHTCLEAR1(WM_NATION_Carcino)
    WM_HIGHLIGHTCLEAR2(WM_NATION_Carcino)
    WM_SHOWPORTRAIT(0, 0x0054, 0x02BC, 0)
    STAL(6)
    STAL(46)

    // Carcino council leader Klimt, a staunch opponent of Grado's imperialistic actions,
    // has pledged Carcino's support to Frelia in this conflict.
    SOUN(SONG_VOICE_CH09_LINE_0004)
    TEXTCONT
    TEXTEND

    WM_SHOWPORTRAIT(1, 0x0052, 0x0534, 0)
    STAL(6)
    STAL(46)

    // Prince Innes also intends to pass through Carcino on his way to Jehanna.
    SOUN(SONG_VOICE_CH09_LINE_0005)
    TEXTCONT
    TEXTEND

    WM_CLEARPORTRAIT(0, 0x01BC, 0)
    STAL(46)
    WM_CLEARPORTRAIT(1, 0x0634, 0)
    STAL(46)
    WmEvtRemoveBigMap // WM_WAITFORFXCLEAR1
    WmEvtWaitBigMapRemove // WM_WAITFORFXCLEAR2
    WM_MOVECAM(-1, -1, 98, 28, 70, 10)
    STAL(30)
    WM_SHOWPORTRAIT(0, 0x000F, 0x02BC, 0)
    STAL(6)
    STAL(46)
    PUTSPRITE(WM_MU_2, CLASS_SNIPER, WM_FACTION_BLUE, WM_NODE_PortKiris)
    WM_MOVESPRITETO(WM_MU_2, 0x0003, WM_NODE_PortKiris, WM_NODE_TerazPlateau, 180, 0)
    WM_WAITFORCAM

    // Eirika believes there is nothing to fear in Carcino.
    SOUN(SONG_VOICE_CH09_LINE_0006)
    TEXTCONT
    TEXTEND

    // ...She is wrong.
    SOUN(SONG_VOICE_CH09_LINE_0007)
    TEXTCONT
    TEXTEND

    WM_WAITFORCAM
    WM_WAITFORSPRITES(WM_MU_ANY)
    WM_CLEARPORTRAIT(0, 0x01BC, 0)
    STAL(46)
    WM_REMSPRITE(WM_MU_2)
    STAL(90)
    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    ENDA
};
