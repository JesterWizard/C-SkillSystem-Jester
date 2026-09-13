#include "../../_shared/worldmap-include.h"

const EventScr EventScrWM_PrologueSkip[] = {
    EVBIT_MODIFY(0x1)
    WM_NOFADE
    SKIPWN
    WM_FXCLEAR1(-0x1)
    WM_FXCLEAR2(-0x1)
    WM_REMSPRITE(WM_MU_2)
    WM_REMSPRITE(WM_MU_3)
    WM_REMSPRITE(WM_MU_4)
    WM_REMSPRITE(WM_MU_5)
    WM_REMSPRITE(WM_MU_6)
    ENDA
};

const EventScr EventScrWM_Prologue_SET_NODE[] = {
    EVBIT_MODIFY(0x1)
    WM_NOFADE
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_BorderMulan)
    WM_CENTERCAMONLORD(WM_MU_0)
    MUSCFAST(0x7fff)
    STAL(32)
    MUSC(SONG_THE_BEGINNING)
    STAL(8)
    ASMC(ReduceBGMVolume)
    WM_SHOWDRAWNMAP(0, 0, 0x10)
    STAL(2)
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    STAL(60)
    WM_SHOWTEXTWINDOW(40, 0x0001)
    WM_WAITFORTEXT
    WM_TEXTSTART
    WM_TEXT(Chapter_00_WM, 0)

    // The continent of Magvel.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0001)

    // For some 800 years, a quiet peace reigned in the absence of the terrible darkness.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0002)

    // The Sacred Stones have been passed from generation to generation.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0003)

    // Nations have been built around their power and their legacy.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0004)

    WM_MOVECAM2(0, 0, 0, 24, 60, 0)
    STAL(60)
    WM_NATION_ON(0x0051, WM_FACE_LEFT, WM_NATION_Renais)

    // The kingdom of Renais, ruled by Fado, the peerless Warrior King.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0005)

    STAL(30)
    WM_NATION_OFF(WM_NATION_Renais, WM_FACE_CLEAR_LEFT)
    WM_MOVECAM2(0, 24, 0, -8, 60, 0)
    STAL(60)
    WM_NATION_ON(0x0052, WM_FACE_LEFT, WM_NATION_Frelia)

    // The kingdom of Frelia, ruled by Hayden, the venerable Sage King.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0006)

    STAL(30)
    WM_NATION_OFF(WM_NATION_Frelia, WM_FACE_CLEAR_LEFT)
    WM_MOVECAM2(0, -8, 0, 30, 60, 0)
    STAL(60)
    WM_NATION_ON(0x0056, WM_FACE_RIGHT, WM_NATION_Jehanna)

    // The kingdom of Jehanna, ruled by Ismaire, Queen of the White Dunes.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0007)

    STAL(30)
    WM_NATION_OFF(WM_NATION_Jehanna, WM_FACE_CLEAR_RIGHT)
    WM_MOVECAM2(0, 30, 0, -8, 60, 0)
    STAL(60)
    WM_NATION_ON(0x0053, WM_FACE_RIGHT, WM_NATION_Rausten)

    // The theocracy of Rausten, ruled by Mansel, the Divine Emperor.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0008)

    STAL(30)
    WM_NATION_OFF(WM_NATION_Rausten, WM_FACE_CLEAR_RIGHT)
    WM_MOVECAM2(0, -8, 0, 48, 60, 0)
    STAL(60)
    WM_NATION_ON(0x0040, WM_FACE_LEFT, WM_NATION_Grado)

    // The Grado Empire, ruled by Vigarde, the stalwart Silent Emperor.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0009)

    STAL(30)
    WM_NATION_OFF_CORE(WM_NATION_Grado, WM_FACE_CLEAR_LEFT)

    // These five countries house the power of the Sacred Stones.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0010)

    STAL(30)
    WM_MOVECAM2(0, 48, 0, 0, 60, 0)
    STAL(60)
    WM_NATION_ON(0x0054, WM_FACE_LEFT, WM_NATION_Carcino)

    // They are joined by the emerging mercantile republic of Carcino.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0011)

    STAL(30)
    WM_NATION_OFF(WM_NATION_Carcino, WM_FACE_CLEAR_LEFT)

    // Though peace reigns, the harmony is fragile.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0012)

    // For months, rumors of Grado's military expansions have reached neighboring courts.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0013)

    // Renais, once a close ally of Grado, has grown cautious, strengthening its borders.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0014)

    // Preparing for any eventuality.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0015)

    // It is now the year 803...
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0016)

    // In an instant, the whole of Magvel is threatened by a devastating betrayal.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0017)

    STAL(30)
    WM_MOVECAM2(0, 0, 0, 48, 60, 0)
    STAL(60)
    WM_NATION_ON(0x0040, WM_FACE_LEFT, WM_NATION_Grado)

    // The Grado Empire, the largest of the Sacred Stone nations,
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0018)

    // has invaded the kingdom of Renais under orders from Emperor Vigarde.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0019)

    WM_NATION_OFF(WM_NATION_Grado, WM_FACE_CLEAR_LEFT)
    WM_MOVECAM2(0, 48, 0, 24, 60, 0)
    STAL(60)
    WM_NATION_ON(0x0051, WM_FACE_LEFT, WM_NATION_Renais)

    // Despite its vigilance, the sheer scale of the operation catches Renais on the backfoot.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0020)

    // Leaving it unable to mount a sufficient resistance.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0021)

    WM_NATION_OFF(WM_NATION_Renais, WM_FACE_CLEAR_LEFT)
    WM_MOVECAM2(0, 24, 0, 40, 52, 0)
    STAL(62)
    WM_PLACEDOT(0, 0, WM_NODE_RenaisCastle, 1)
    WM_PLACEDOT(0, 1, WM_NODE_GradoKeep, 1)
    STAL(60)
    PUTSPRITE(WM_MU_2, CLASS_SOLDIER, WM_FACTION_RED, WM_NODE_GradoKeep)
    PUTSPRITE(WM_MU_3, CLASS_SOLDIER, WM_FACTION_RED, WM_NODE_GradoKeep)
    PUTSPRITE(WM_MU_4, CLASS_SOLDIER, WM_FACTION_RED, WM_NODE_GradoKeep)
    WM_PUTMOVINGSPRITE(WM_MU_2, 0, 0x73, 0x84, 0x55, 0x41, 210, 0x3, 10)
    WM_PUTMOVINGSPRITE(WM_MU_3, 0, 0x84, 0x84, 0x76, 0x57, 170, 0x3, 10)
    WM_PUTMOVINGSPRITE(WM_MU_4, 0, 0x95, 0x84, 0x86, 0x64, 150, 0x3, 10)
    STAL(20)

    // Grado's forces move quickly, seizing one territory after another.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0022)

    WM_WAITFORSPRITES(WM_MU_ANY)
    WM_REMSPRITE(WM_MU_2)
    WM_REMSPRITE(WM_MU_3)
    WM_REMSPRITE(WM_MU_4)
    STAL(30)
    WM_SHOW_FACE_WAIT(0, 0x0014, WM_FACE_RIGHT, 60)
    PUTSPRITE(WM_MU_2, CLASS_EPHRAIM_LORD, WM_FACTION_BLUE, WM_NODE_AdlasPlains)
    WM_PUTMOVINGSPRITE(WM_MU_2, 0, 0x5c, 0x64, 0x5c, 0x6c, 180, 0x3, 16)
    
    // Compounding King Fado's worries, his son, Prince Ephraim, has gone missing.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0023)

    WM_WAITFORSPRITES(WM_MU_ANY)
    WM_REMSPRITE(WM_MU_2)
    WM_HIDE_FACE(0, WM_FACE_CLEAR_RIGHT)
    PUTSPRITE(WM_MU_6, CLASS_EIRIKA_LORD, WM_FACTION_BLUE, WM_NODE_RenaisCastle)
    PUTSPRITE(WM_MU_5, CLASS_PEER, WM_FACTION_BLUE, WM_NODE_RenaisCastle)
    PUTSPRITE(WM_MU_2, CLASS_GENERAL, WM_FACTION_RED, WM_NODE_GradoKeep)
    PUTSPRITE(WM_MU_3, CLASS_MAGE_KNIGHT_F, WM_FACTION_RED, WM_NODE_GradoKeep)
    PUTSPRITE(WM_MU_4, CLASS_WYVERN_KNIGHT, WM_FACTION_RED, WM_NODE_GradoKeep)
    WM_PUTMOVINGSPRITE(WM_MU_2, 0, 0x84, 0x84, 0x6c, 0x5c, 210, 0x1, 0)
    WM_PUTMOVINGSPRITE(WM_MU_3, 0, 0x73, 0x92, 0x5b, 0x56, 210, 0x1, 0)
    WM_PUTMOVINGSPRITE(WM_MU_4, 0, 0x95, 0x92, 0x7d, 0x56, 210, 0x1, 0)
    
    // Grado's momentum carries its armies to the gates of Castle Renais itself.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0024)

    WM_WAITFORSPRITES(WM_MU_ANY)
    STAL(26)
    WM_PUTSPRITE(WM_MU_6, 0x63, 0x45)
    WM_PUTSPRITE(WM_MU_5, 0x6c, 0x4c)
    WM_FADEINSPRITE(WM_MU_6, 60)
    WM_FADEINSPRITE(WM_MU_5, 60)

    // Renais will fall... It is inevitable.
    WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0025)

    WM_WAITFORSPRITELOAD
    WM_REMOVETEXT
    STAL(2)
    FADI(16)

    SKIPWN
    WM_FXCLEAR1(-0x1)
    WM_FXCLEAR2(-0x1)
    WM_REMSPRITE(WM_MU_2)
    WM_REMSPRITE(WM_MU_3)
    WM_REMSPRITE(WM_MU_4)
    WM_REMSPRITE(WM_MU_5)
    WM_REMSPRITE(WM_MU_6)
    ENDA
};

const EventScr EventScrWM_Prologue_TRAVEL_TO_NODE[] = {
    EVBIT_MODIFY(0x1)
    ENUT(137)
    ENDA
};
