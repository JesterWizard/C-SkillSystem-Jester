#include "../../_shared/worldmap-include.h"

const EventScr EventScrWM_PrologueSkip[] = {
    EVBIT_MODIFY(0x1)
    WmEvtNOFADE // ENOSUPP in EAstdlib
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
    WmEvtNOFADE // ENOSUPP in EAstdlib
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
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0001)
    TEXTCONT
    TEXTEND

    // For some 800 years, a quiet peace reigned in the absence of the terrible darkness.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0002)
    TEXTCONT
    TEXTEND

    // The Sacred Stones have been passed from generation to generation.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0003)
    TEXTCONT
    TEXTEND

    // Nations have been built around their power and their legacy.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0004)
    TEXTCONT
    TEXTEND

    WM_MOVECAM2(0, 0, 0, 24, 60, 0)
    STAL(60)
    WM_SHOWPORTRAIT(0, 0x0051, 0x02BC, 0)
    STAL(6)
    STAL(26)
    WM_HIGHLIGHT(WM_NATION_Renais)

    // The kingdom of Renais, ruled by Fado, the peerless Warrior King.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0005)
    TEXTCONT
    TEXTEND

    STAL(30)
    WM_HIGHLIGHTCLEAR1(WM_NATION_Renais)
    WM_HIGHLIGHTCLEAR2(WM_NATION_Renais)
    WM_CLEARPORTRAIT(0, 0x0100, 0)
    STAL(32)
    WM_MOVECAM2(0, 24, 0, -8, 60, 0)
    STAL(60)
    WM_SHOWPORTRAIT(0, 0x0052, 0x02BC, 0)
    STAL(6)
    STAL(26)
    WM_HIGHLIGHT(WM_NATION_Frelia)

    // The kingdom of Frelia, ruled by Hayden, the venerable Sage King.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0006)
    TEXTCONT
    TEXTEND

    STAL(30)
    WM_HIGHLIGHTCLEAR1(WM_NATION_Frelia)
    WM_HIGHLIGHTCLEAR2(WM_NATION_Frelia)
    WM_CLEARPORTRAIT(0, 0x0100, 0)
    STAL(32)
    WM_MOVECAM2(0, -8, 0, 30, 60, 0)
    STAL(60)
    WM_SHOWPORTRAIT(0, 0x0056, 0x0534, 0)
    STAL(6)
    STAL(26)
    WM_HIGHLIGHT(WM_NATION_Jehanna)

    // The kingdom of Jehanna, ruled by Ismaire, Queen of the White Dunes.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0007)
    TEXTCONT
    TEXTEND

    STAL(30)
    WM_HIGHLIGHTCLEAR1(WM_NATION_Jehanna)
    WM_HIGHLIGHTCLEAR2(WM_NATION_Jehanna)
    WM_CLEARPORTRAIT(0, 0x0200, 0)
    STAL(32)
    WM_MOVECAM2(0, 30, 0, -8, 60, 0)
    STAL(60)
    WM_SHOWPORTRAIT(0, 0x0053, 0x0534, 0)
    STAL(6)
    STAL(26)
    WM_HIGHLIGHT(WM_NATION_Rausten)

    // The theocracy of Rausten, ruled by Mansel, the Divine Emperor.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0008)
    TEXTCONT
    TEXTEND

    STAL(30)
    WM_HIGHLIGHTCLEAR1(WM_NATION_Rausten)
    WM_HIGHLIGHTCLEAR2(WM_NATION_Rausten)
    WM_CLEARPORTRAIT(0, 0x0200, 0)
    STAL(32)
    WM_MOVECAM2(0, -8, 0, 48, 60, 0)
    STAL(60)
    WM_SHOWPORTRAIT(0, 0x0040, 0x02BC, 0)
    STAL(6)
    STAL(26)
    WM_HIGHLIGHT(WM_NATION_Grado)

    // The Grado Empire, ruled by Vigarde, the stalwart Silent Emperor.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0009)
    TEXTCONT
    TEXTEND

    STAL(30)
    WM_HIGHLIGHTCLEAR1(WM_NATION_Grado)
    WM_HIGHLIGHTCLEAR2(WM_NATION_Grado)
    WM_CLEARPORTRAIT(0, 0x0100, 0)
    
    // These five countries house the power of the Sacred Stones.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0010)
    TEXTCONT
    TEXTEND

    STAL(30)
    WM_MOVECAM2(0, 48, 0, 0, 60, 0)
    STAL(60)
    WM_SHOWPORTRAIT(0, 0x0054, 0x02BC, 0)
    STAL(6)
    STAL(26)
    WM_HIGHLIGHT(WM_NATION_Carcino)
    // They are joined by the emerging mercantile republic of Carcino.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0011)
    TEXTCONT
    TEXTEND

    STAL(30)
    WM_HIGHLIGHTCLEAR1(WM_NATION_Carcino)
    WM_HIGHLIGHTCLEAR2(WM_NATION_Carcino)
    WM_CLEARPORTRAIT(0, 0x0100, 0)
    STAL(32)

    // Though peace reigns, the harmony is fragile.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0012)
    TEXTCONT
    TEXTEND

    // For months, rumors of Grado's military expansions have reached neighboring courts.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0013)
    TEXTCONT
    TEXTEND

    // Renais, once a close ally of Grado, has grown cautious, strengthening its borders.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0014)
    TEXTCONT
    TEXTEND

    // Preparing for any eventuality.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0015)
    TEXTCONT
    TEXTEND

    // It is now the year 803...
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0016)
    TEXTCONT
    TEXTEND

    // In an instant, the whole of Magvel is threatened by a devastating betrayal.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0017)
    TEXTCONT
    TEXTEND

    STAL(30)
    WM_MOVECAM2(0, 0, 0, 48, 60, 0)
    STAL(60)
    WM_SHOWPORTRAIT(0, 0x0040, 0x02BC, 0)
    STAL(6)
    STAL(26)
    WM_HIGHLIGHT(WM_NATION_Grado)

    // The Grado Empire, the largest of the Sacred Stone nations,
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0018)
    TEXTCONT
    TEXTEND

    // has invaded the kingdom of Renais under orders from Emperor Vigarde.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0019)
    TEXTCONT
    TEXTEND

    WM_HIGHLIGHTCLEAR1(WM_NATION_Grado)
    WM_HIGHLIGHTCLEAR2(WM_NATION_Grado)
    WM_CLEARPORTRAIT(0, 0x0100, 0)
    STAL(32)
    WM_MOVECAM2(0, 48, 0, 24, 60, 0)
    STAL(60)
    WM_SHOWPORTRAIT(0, 0x0051, 0x02BC, 0)
    STAL(6)
    STAL(26)
    WM_HIGHLIGHT(WM_NATION_Renais)

    // Despite its vigilance, the sheer scale of the operation catches Renais on the backfoot.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0020)
    TEXTCONT
    TEXTEND

    // Leaving it unable to mount a sufficient resistance.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0021)
    TEXTCONT
    TEXTEND

    WM_HIGHLIGHTCLEAR1(WM_NATION_Renais)
    WM_HIGHLIGHTCLEAR2(WM_NATION_Renais)
    WM_CLEARPORTRAIT(0, 0x0100, 0)
    STAL(32)
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
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0022)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITES(WM_MU_ANY)
    WM_REMSPRITE(WM_MU_2)
    WM_REMSPRITE(WM_MU_3)
    WM_REMSPRITE(WM_MU_4)
    STAL(30)
    WM_SHOWPORTRAIT(0, 0x0014, 0x0534, 0)
    STAL(6)
    STAL(60)
    PUTSPRITE(WM_MU_2, CLASS_EPHRAIM_LORD, WM_FACTION_BLUE, WM_NODE_AdlasPlains)
    WM_PUTMOVINGSPRITE(WM_MU_2, 0, 0x5c, 0x64, 0x5c, 0x6c, 180, 0x3, 16)
    
    // Compounding King Fado's worries, his son, Prince Ephraim, has gone missing.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0023)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITES(WM_MU_ANY)
    WM_REMSPRITE(WM_MU_2)
    WM_CLEARPORTRAIT(0, 0x0200, 0)
    STAL(46)
    PUTSPRITE(WM_MU_6, CLASS_EIRIKA_LORD, WM_FACTION_BLUE, WM_NODE_RenaisCastle)
    PUTSPRITE(WM_MU_5, CLASS_PEER, WM_FACTION_BLUE, WM_NODE_RenaisCastle)
    PUTSPRITE(WM_MU_2, CLASS_GENERAL, WM_FACTION_RED, WM_NODE_GradoKeep)
    PUTSPRITE(WM_MU_3, CLASS_MAGE_KNIGHT_F, WM_FACTION_RED, WM_NODE_GradoKeep)
    PUTSPRITE(WM_MU_4, CLASS_WYVERN_KNIGHT, WM_FACTION_RED, WM_NODE_GradoKeep)
    WM_PUTMOVINGSPRITE(WM_MU_2, 0, 0x84, 0x84, 0x6c, 0x5c, 210, 0x1, 0)
    WM_PUTMOVINGSPRITE(WM_MU_3, 0, 0x73, 0x92, 0x5b, 0x56, 210, 0x1, 0)
    WM_PUTMOVINGSPRITE(WM_MU_4, 0, 0x95, 0x92, 0x7d, 0x56, 210, 0x1, 0)
    
    // Grado's momentum carries its armies to the gates of Castle Renais itself.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0024)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITES(WM_MU_ANY)
    STAL(26)
    WM_PUTSPRITE(WM_MU_6, 0x63, 0x45)
    WM_PUTSPRITE(WM_MU_5, 0x6c, 0x4c)
    WM_FADEINSPRITE(WM_MU_6, 60)
    WM_FADEINSPRITE(WM_MU_5, 60)

    // Renais will fall... It is inevitable.
    SOUN(SONG_VOICE_CH00_PROLOGUE_LINE_0025)
    TEXTCONT
    TEXTEND

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
