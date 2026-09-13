#include "../../_shared/worldmap-include.h"

const EventScr EventScrWM_Renval_CH5[] = {
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_Renvall2)
    WM_SETCAMTONODE(WM_NODE_AdlasPlains)
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    WM_MOVESPRITETO(WM_MU_0, 0x0004, WM_NODE_Renvall2, WM_NODE_AdlasPlains, -2, 0)
    WM_WAITFORSPRITES(WM_MU_0)
    WM_MOVESPRITETO(WM_MU_0, 0x0004, WM_NODE_AdlasPlains, WM_NODE_Serafew, -2, 0)
    WM_WAITFORSPRITES(WM_MU_0)
    WM_SETUNITONNODE(WM_MU_0, WM_NODE_Serafew)
    WM_SETNEXTSTORYNODE(WM_NODE_Serafew)
    SKIPWN
    ENDA
};

const EventScr EventScrWM_Ch5_SET_NODE[] = {
    CHECK_EVENTID(136)
    BEQ(0x1, EVT_SLOT_C, EVT_SLOT_0)
    CALL(EventScrWM_Renval_CH5)
    GOTO(0x2)
LABEL(0x1)
    WM_OPEN_MAP(CHARACTER_EIRIKA, WM_NODE_ZahaWoods)
    WM_REVEAL_DEST(WM_NODE_Serafew, WM_PATH_04)
    WM_MAKELORDVISIBLE(WM_MU_0)
    EVBIT_MODIFY(0x1)
    CALL(EventScr_RemoveBGIfNeeded)
LABEL(0x2)
    ENDA
};

const EventScr EventScrWM_Ch5_TRAVEL_TO_NODE[] = {
    CHECK_EVENTID(136)
    BEQ(0x1, EVT_SLOT_C, EVT_SLOT_0)
    CALL(EventScrWM_Intermission_Renvall_To_Serafew)
    GOTO(0x2)
LABEL(0x1)
    MUSCFAST(0x7fff)
    MUSC(SONG_THE_BEGINNING)
    STAL(6)
    ASMC(ReduceBGMVolume)
    WM_MOVECAM(-1, -1, 26, 112, 60, 0)
    WM_WAITFORCAM
    CALL(EventScr_WM_FadeCommon)
    STAL(60)
    WM_TEXTSTART
    WM_SHOWDRAWNMAP(0, 0, 0x3)
    STAL(120)
    PUTSPRITE(WM_MU_2, CLASS_REVENANT, WM_FACTION_RED, WM_NODE_BorderMulan)
    PUTSPRITE(WM_MU_3, CLASS_REVENANT, WM_FACTION_RED, WM_NODE_BorderMulan)
    PUTSPRITE(WM_MU_4, CLASS_REVENANT, WM_FACTION_RED, WM_NODE_BorderMulan)
    PUTSPRITE(WM_MU_5, CLASS_REVENANT, WM_FACTION_RED, WM_NODE_BorderMulan)
    PUTSPRITE(WM_MU_6, CLASS_REVENANT, WM_FACTION_RED, WM_NODE_BorderMulan)
    WM_PUTSPRITE(WM_MU_2, 0x28, 0x5e)
    WM_PUTSPRITE(WM_MU_3, 0x4e, 0x3d)
    WM_PUTSPRITE(WM_MU_4, 0x89, 0x4e)
    WM_PUTSPRITE(WM_MU_5, 0xaa, 0x2d)
    WM_PUTSPRITE(WM_MU_6, 0xbf, 0x5b)
    WM_FADEINSPRITE(WM_MU_2, 60)
    WM_FADEINSPRITE(WM_MU_3, 60)
    WM_FADEINSPRITE(WM_MU_4, 60)
    WM_FADEINSPRITE(WM_MU_5, 60)
    WM_FADEINSPRITE(WM_MU_6, 60)
    WM_TEXT(Chapter_05_WM, 0)

    // An ominous feeling of dread washes over the continent...
    WM_VOICE_LINE(SONG_VOICE_CH05_LINE_0001)

    WM_WAITFORSPRITELOAD
    WM_FADEOUTSPRITE(WM_MU_2, 60)
    WM_FADEOUTSPRITE(WM_MU_3, 60)
    WM_FADEOUTSPRITE(WM_MU_4, 60)
    WM_FADEOUTSPRITE(WM_MU_5, 60)
    WM_FADEOUTSPRITE(WM_MU_6, 60)
    WM_WAITFORSPRITELOAD
    WM_REMSPRITE(WM_MU_2)
    WM_REMSPRITE(WM_MU_3)
    WM_REMSPRITE(WM_MU_4)
    WM_REMSPRITE(WM_MU_5)
    WM_REMSPRITE(WM_MU_6)
    WM_WAITFORFXCLEAR1
    WM_FADEOUTSPRITE(WM_MU_0, 1)
    WM_WAITFORFXCLEAR2
    WM_SHOW_FACE_WAIT(0, Overworld_Eirika, WM_FACE_RIGHT, 46)

    // Eirika's journey to find her brother Ephraim, grows ever more perilous.
    WM_VOICE_LINE(SONG_VOICE_CH05_LINE_0002)

    // Still, she presses on despite the danger, without rest.
    WM_VOICE_LINE(SONG_VOICE_CH05_LINE_0003)

    PUTSPRITE(WM_MU_2, CLASS_ARMOR_KNIGHT, WM_FACTION_RED, WM_NODE_Serafew)
    WM_FADEINSPRITE(WM_MU_2, 60)

    // However, a glimmer of hope now exists.
    WM_VOICE_LINE(SONG_VOICE_CH05_LINE_0004)

    // Having reunited Renais' former famed military commander: Garcia, with his son: Ross.
    WM_VOICE_LINE(SONG_VOICE_CH05_LINE_0005)

    // A temporary alliance has been born as the group now travels to Serafew.
    WM_VOICE_LINE(SONG_VOICE_CH05_LINE_0006)

    // A bustling border town located between Renais and Grado.
    WM_VOICE_LINE(SONG_VOICE_CH05_LINE_0007)

    // The people of both countries have long used the town as a meeting place.
    WM_VOICE_LINE(SONG_VOICE_CH05_LINE_0008)

    WM_WAITFORSPRITELOAD
    STAL(14)
    WM_FADEOUTSPRITE(WM_MU_2, 60)
    WM_MOVESPRITETO(WM_MU_0, Overworld_Eirika, WM_NODE_ZahaWoods, WM_NODE_Serafew, -4, 0) // Eirika
    WM_WAITFORSPRITELOAD

    // It once stood as testimony to the harmony the nations have shared these many years.
    WM_VOICE_LINE(SONG_VOICE_CH05_LINE_0009)

    // Now however, it exists as a grim reflection of a friendship ruined...
    WM_VOICE_LINE(SONG_VOICE_CH05_LINE_0010)

    WM_WAITFORSPRITES(WM_MU_ANY)
    STAL(20)

    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    WM_REMSPRITE(WM_MU_2)
LABEL(0x2)
    ENDA
};
