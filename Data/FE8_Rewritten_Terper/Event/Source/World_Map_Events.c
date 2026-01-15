#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/macros.h"
#include "jester_headers/maps.h"
#include "jester_headers/flags.h"
#include "jester_headers/miscellaneous.h"
#include "jester_headers/event-call.h"
#include "jester_headers/custom-functions.h"
#include "EAstdlib.h"

extern const u16* Events_WM_Beginning[];
extern const u16* Events_WM_ChapterIntro[];

//! FE8U = 0x080BA334
LYN_REPLACE_CHECK(WorldMap_CallBeginningEvent);
void WorldMap_CallBeginningEvent(struct WorldMapMainProc* proc)
{
    int chIndex;
    int node_next;

    Sound_FadeOutBGM(4);

    if ((gGMData.state.bits.monster_merged) || (gPlaySt.chapterStateBits & PLAY_FLAG_POSTGAME))
    {
        sub_80BA008(proc->timer);
    }
    else
    {
        int loc = gGMData.units[0].location;

        gGMData.current_node = loc;
        node_next = WMLoc_GetNextLocId(loc);

        if (node_next > -1)
        {
            chIndex = WMLoc_GetChapterId(node_next);

            gPlaySt.chapterIndex = chIndex;


            if (Events_WM_Beginning[GetROMChapterStruct(chIndex)->gmapEventId] == NULL)
                return;

            ResetGmStoryNode();
            proc->gm_icon->merge_next_node = false;

            /**
             * JESTER - I've resorted to hooking into the WM call function to directly load the
             * WM events I want based on the supplied eventSCR. It's an unfortunate bit of hardcoding
             * I'm looking to remove, but it frees me from having to rely on the list in ASM in vanilla.
             */
            int eventID = GetROMChapterStruct(chIndex)->gmapEventId;

            // NoCashGBAPrintf("SET event id is: %d", eventID);

            switch (eventID) {
            case 55:
                CallEvent((const u16*)EventScrWM_Ch1_ENDING, 0);
                break;
            case 1:
                if (gpKernelDesignerConfig->skip_intro == true)
                    CallEvent((const u16*)EventScrWM_PrologueSkip, 0);
                else
                    CallEvent((const u16*)EventScrWM_Prologue_SET_NODE, 0);

                break;
            case 2:
                break;
            case 3:
                CallEvent((const u16*)EventScrWM_Ch2_SET_NODE, 0);
                break;
            case 4:
                CallEvent((const u16*)EventScrWM_Ch3_SET_NODE, 0);
                break;
            case 5:
                CallEvent((const u16*)EventScrWM_Ch4_SET_NODE, 0);
                break;
            case 6:
                break;
            case 7:
                CallEvent((const u16*)EventScrWM_Ch5_SET_NODE, 0);
                break;
            case 8:
                CallEvent((const u16*)EventScrWM_Ch6_SET_NODE, 0);
                break;
            case 9:
                CallEvent((const u16*)EventScrWM_Ch7_SET_NODE, 0);
                break;
            case 10:
                CallEvent((const u16*)EventScrWM_Ch8_SET_NODE, 0);
                break;
            case 11:
                CallEvent((const u16*)EventScrWM_Ch9_SET_NODE, 0);
                break;
            case 12:
                CallEvent((const u16*)EventScrWM_Ch10_SET_NODE, 0);
                break;
            default:
                CallEvent(Events_WM_Beginning[eventID], 0);
                break;
            }
        }
    }

    StartWMFaceCtrl(proc);
    StartGmapMuEntry(NULL);
}

//! FE8U = 0x080BA3D4
LYN_REPLACE_CHECK(CallChapterWMIntroEvents);
void CallChapterWMIntroEvents(ProcPtr proc)
{
    if (Events_WM_ChapterIntro[GetROMChapterStruct(gPlaySt.chapterIndex)->gmapEventId] != NULL)
    {
        /**
       * JESTER - I've resorted to hooking into the WM call function to directly load the
       * WM events I want based on the supplied eventSCR. It's an unfortunate bit of hardcoding
       * I'm looking to remove, but it frees me from having to rely on the list in ASM in vanilla.
       */
        int eventID = GetROMChapterStruct(gPlaySt.chapterIndex)->gmapEventId;

        // NoCashGBAPrintf("TRAVEL event id is: %d", eventID);

        switch (eventID) {
        case 55:
            break;
        case 1:
            CallEvent((const u16*)EventScrWM_Prologue_TRAVEL_TO_NODE, 0);
            break;
        case 2:
            break;
        case 3:
            CallEvent((const u16*)EventScrWM_Ch2_TRAVEL_TO_NODE, 0);
            break;
        case 4:
            CallEvent((const u16*)EventScrWM_Ch3_TRAVEL_TO_NODE, 0);
            break;
        case 5:
            CallEvent((const u16*)EventScrWM_Ch4_TRAVEL_TO_NODE, 0);
            break;
        case 6:
            break;
        case 7:
            CallEvent((const u16*)EventScrWM_Ch5_TRAVEL_TO_NODE, 0);
            break;
        case 8:
            CallEvent((const u16*)EventScrWM_Ch6_TRAVEL_TO_NODE, 0);
            break;
        case 9:
            CallEvent((const u16*)EventScrWM_Ch7_TRAVEL_TO_NODE, 0);
            break;
        case 10:
            CallEvent((const u16*)EventScrWM_Ch8_TRAVEL_TO_NODE, 0);
            break;
        case 11:
            CallEvent((const u16*)EventScrWM_Ch9_TRAVEL_TO_NODE, 0);
            break;
        case 12:
            CallEvent((const u16*)EventScrWM_Ch10_TRAVEL_TO_NODE, 0);
            break;
        default:
            CallEvent(Events_WM_ChapterIntro[eventID], 0);
            break;
        }
        StartWMFaceCtrl(proc);
        StartGmapMuEntry(NULL);
    }
}

LYN_REPLACE_CHECK(Event97_WmInitNextStoryNode);
//! FE8U = 0x0800C2DC
u8 Event97_WmInitNextStoryNode(struct EventEngineProc* proc)
{
    // struct WorldMapMainProc * worldMapProc;

    int nodeId = WMLoc_GetNextLocId(gGMData.current_node);

    // NoCashGBAPrintf("Next node ID is: %d", nodeId);

    if (nodeId < 0)
    {
        return EVC_ADVANCE_CONTINUE;
    }

    if (EVENT_IS_SKIPPING(proc))
    {
        ResetGmStoryNode();
        gGMData.nodes[nodeId].state |= 1;
        gGMData.nodes[nodeId].state |= 2;

        GM_ICON->nodeId = nodeId;
        GM_ICON->merge_next_node = true;
    }
    else
    {
        if (!(gGMData.nodes[nodeId].state & 1))
        {
            StartGmBaseEntry(nodeId, 0, NULL);
            ResetGmStoryNode();
            gGMData.nodes[nodeId].state |= 2;
        }
    }

    return EVC_ADVANCE_CONTINUE;
};

//! FE8U = 0x08010968
LYN_REPLACE_CHECK(Event3E_PrepScreenCall);
u8 Event3E_PrepScreenCall(struct EventEngineProc* proc)
{
    HideAllUnits();
    ClearFlag(0x84);
    Proc_StartBlocking(gProcScr_SALLYCURSOR, proc);

    return EVC_ADVANCE_YIELD;
}

//! FE8U = 0x080B9B38
LYN_REPLACE_CHECK(WorldMap_CallIntroEvent);
void WorldMap_CallIntroEvent(struct WorldMapMainProc* proc)
{
    GmMu_80BE108(proc->gm_mu, 0, 0);

    if (gGMData.units[0].location[gWMNodeData].placementFlag != GMAP_NODE_PLACEMENT_DUNGEON)
    {
        gPlaySt.chapterIndex = WMLoc_GetChapterId(proc->unk_3e);
        gGMData.state.bits.monster_merged = false;
    }
    else
    {
        gPlaySt.chapterIndex = WMLoc_GetChapterId(gGMData.units[0].location);
    }

    CallChapterWMIntroEvents(proc);

    gGMData.sprite_disp = 0;

    WmRemoveRandomMonsters();
}

const EventScr EventScrWM_Tutorial_SKILL_SCROLL[] = {
    EvtTextStartType5 // ENOSUPP in EAstdlib
    SVAL(EVT_SLOT_B, 0xFFFFFFFF) // Center text box
    TEXTSHOW(Chapter_01_SKILL_SCROLL)
    TEXTEND
    REMA
    ENDA
};

extern struct ProcCmd CONST_DATA gProcScr_OpSubtitle[];

//! FE8U = 0x080C541C
LYN_REPLACE_CHECK(StartIntroMonologue);
void StartIntroMonologue(ProcPtr proc) {

    if (gpKernelDesignerConfig->skip_intro == true)
        return;

    Proc_StartBlocking(gProcScr_OpSubtitle, proc);
    
    return;
}


void ReduceBGMVolume(void)
{
    Sound_SetSEVolume(60);
}

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
    SOUN(0xC0)
    TEXTCONT
    TEXTEND

    // For some 800 years, a quiet peace reigned in the absence of the terrible darkness.
    SOUN(0xC1)
    TEXTCONT
    TEXTEND

    // The Sacred Stones have been passed from generation to generation.
    SOUN(0xC2)
    TEXTCONT
    TEXTEND

    // Nations have been built around their power and their legacy.
    SOUN(0xC3)
    TEXTCONT
    TEXTEND

    WM_MOVECAM2(0, 0, 0, 24, 60, 0)
    STAL(60)
    WM_SHOWPORTRAIT(0, 0x0051, 0x02BC, 0)
    STAL(6)
    STAL(26)
    WM_HIGHLIGHT(WM_NATION_Renais)

    // The kingdom of Renais, ruled by Fado, the peerless Warrior King.
    SOUN(0xC4)
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
    SOUN(0xC5)
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
    SOUN(0xC6)
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
    SOUN(0xD9)
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
    SOUN(0xDA)
    TEXTCONT
    TEXTEND

    STAL(30)
    WM_HIGHLIGHTCLEAR1(WM_NATION_Grado)
    WM_HIGHLIGHTCLEAR2(WM_NATION_Grado)
    WM_CLEARPORTRAIT(0, 0x0100, 0)
    
    // These five countries house the power of the Sacred Stones.
    SOUN(0xDB)
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
    SOUN(0xE0)
    TEXTCONT
    TEXTEND

    STAL(30)
    WM_HIGHLIGHTCLEAR1(WM_NATION_Carcino)
    WM_HIGHLIGHTCLEAR2(WM_NATION_Carcino)
    WM_CLEARPORTRAIT(0, 0x0100, 0)
    STAL(32)

    // Though peace reigns, the harmony is fragile.
    SOUN(0xE1)
    TEXTCONT
    TEXTEND

    // For months, rumors of Grado's military expansions have reached neighboring courts.
    SOUN(0xE4)
    TEXTCONT
    TEXTEND

    // Renais, once a close ally of Grado, has grown cautious, strengthening its borders.
    SOUN(0xE8)
    TEXTCONT
    TEXTEND

    // Preparing for any eventuality.
    SOUN(0xE9)
    TEXTCONT
    TEXTEND

    // It is now the year 803...
    SOUN(0xEC)
    TEXTCONT
    TEXTEND

    // In an instant, the whole of Magvel is threatened by a devastating betrayal.
    SOUN(0xED)
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
    SOUN(0xEE)
    TEXTCONT
    TEXTEND

    // has invaded the kingdom of Renais under orders from Emperor Vigarde.
    SOUN(0x104)
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
    SOUN(0x105)
    TEXTCONT
    TEXTEND

    // Leaving it unable to mount a sufficient resistance.
    SOUN(0x106)
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
    SOUN(0x107)
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
    SOUN(0x10B)
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
    SOUN(0x10C)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITES(WM_MU_ANY)
    STAL(26)
    WM_PUTSPRITE(WM_MU_6, 0x63, 0x45)
    WM_PUTSPRITE(WM_MU_5, 0x6c, 0x4c)
    WM_FADEINSPRITE(WM_MU_6, 60)
    WM_FADEINSPRITE(WM_MU_5, 60)

    // Renais will fall... It is inevitable.
    SOUN(0x10D)
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

const EventScr EventScrWM_Intermission_Renvall_To_Serafew[] = {
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_Renvall1)
    WM_CENTERCAMONLORD(WM_MU_0)
    WmEvtSetNodeStateNot2(WM_NODE_Renvall1) // ENOSUPP in EAstdlib
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    WmEvtSetUnitOnNode(WM_MU_0, WM_NODE_Renvall1) // ENOSUPP in EAstdlib
    WM_MAKELORDVISIBLE(WM_MU_0)
    WM_MOVESPRITETO(WM_MU_0, 0x0004, WM_NODE_Renvall1, WM_NODE_AdlasPlains, -2, 0)
    WM_WAITFORSPRITES(WM_MU_0)
    WM_MOVESPRITETO(WM_MU_0, 0x0004, WM_NODE_AdlasPlains, WM_NODE_Serafew, -2, 0)
    WM_WAITFORSPRITES(WM_MU_0)
    WmEvtSetUnitOnNode(WM_MU_0, WM_NODE_Serafew) // ENOSUPP in EAstdlib
    SKIPWN
    ENDA
};

const EventScr EventScrWM_Intermission_Serafew_To_Frelia[] = {
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_Serafew)
    WM_CENTERCAMONLORD(WM_MU_0)
    WmEvtSetNodeStateNot2(WM_NODE_Serafew) // ENOSUPP in EAstdlib
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    WmEvtSetUnitOnNode(WM_MU_0, WM_NODE_Serafew) // ENOSUPP in EAstdlib
    WM_MAKELORDVISIBLE(WM_MU_0)
    WM_MOVESPRITETO(WM_MU_0, 0x0004, WM_NODE_Serafew, WM_NODE_ZahaWoods, -2, 0)
    WM_WAITFORSPRITES(WM_MU_0)
    WM_MOVESPRITETO(WM_MU_0, 0x0004, WM_NODE_ZahaWoods, WM_NODE_BorgoRidge, -2, 0)
    WM_MOVECAMTO(-1, -1, WM_NODE_CastleFrelia, 120, 0)
    WM_WAITFORSPRITES(WM_MU_0)
    WM_MOVESPRITETO(WM_MU_0, 0x0004, WM_NODE_BorgoRidge, WM_NODE_Ide, -2, 0)
    WM_WAITFORSPRITES(WM_MU_0)
    WM_MOVESPRITETO(WM_MU_0, 0x0004, WM_NODE_Ide, WM_NODE_CastleFrelia, -2, 0)
    WM_WAITFORSPRITES(WM_MU_0)
    WM_WAITFORCAM
    WmEvtSetUnitOnNode(WM_MU_0, WM_NODE_CastleFrelia) // ENOSUPP in EAstdlib
    SKIPWN
    ENDA
};


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
    SOUN(0x115)
    TEXTCONT
    TEXTEND

    // As she leaves the area with Princess Tana in tow, she cannot help but mourn the lost lives.
    SOUN(0x116)
    TEXTCONT
    TEXTEND

    SOUN(0x117)
    TEXTCONT
    TEXTEND

    SOUN(0x118)
    TEXTCONT
    TEXTEND

    STAL(20)
    WM_MOVESPRITETO(WM_MU_0, 0x0000, WM_NODE_BorderMulan, WM_NODE_CastleFrelia, 200, 40)

    SOUN(0x13A)
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
    CALL(EventScrWM_Tutorial_SKILL_SCROLL)
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
    SOUN(0x143)
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
    SOUN(0x144)
    TEXTCONT
    TEXTEND

    WM_CLEARPORTRAIT(0, 0x0634, 0)
    STAL(46)
    WM_CLEARPORTRAIT(1, 0x01BC, 0)
    STAL(60)
    WM_SHOWPORTRAIT(0, 0x0002, 0x02BC, 0) //EIRIKA
    STAL(6)

    // Eirika is grateful for the king's aid.
    SOUN(0x146)
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
    SOUN(0x148)
    TEXTCONT
    TEXTEND

    STAL(30)
    WM_FXCLEAR1(-0x1)
    WM_FXCLEAR2(-0x1)
    WmEvtRemoveBigMap // WM_WAITFORFXCLEAR1
    WmEvtWaitBigMapRemove // WM_WAITFORFXCLEAR2

    // The group's first stop is the remote village of Ide.
    SOUN(0x149)
    TEXTCONT
    TEXTEND

    WM_MOVESPRITETO(WM_MU_0, 0x0002, WM_NODE_CastleFrelia, WM_NODE_Ide, 200, 30)

    // Which was, until very recently a part of Renais.
    SOUN(0x152)
    TEXTCONT
    TEXTEND

    // Here, Eirika sees the devastation of Renais with her own eyes
    SOUN(0x153)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITES(WM_MU_ANY)
    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    ENDA
};

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
    SOUN(0x15C)
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
    SOUN(0x15D)
    TEXTCONT
    TEXTEND

    // Eirika cannot ignore her countrymen's pleas for help.
    SOUN(0x166)
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
    SOUN(0x167)
    TEXTCONT
    TEXTEND

    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    ENDA
};

const EventScr EventScrWM_Ch4_SET_NODE[] = {
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_BorgoRidge)
    WM_CENTERCAMONLORD(WM_MU_0)
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    STAL(20)
    WM_LOADLOCATION3(WM_NODE_ZahaWoods)
    WM_SETDESTINATION(WM_NODE_ZahaWoods)
    //WM_CREATENEXTDESTINATION
    WM_WAITFORFX
    STAL(40)
    WM_DRAWPATH(WM_PATH_03)
    STAL(70)
    WM_MAKELORDVISIBLE(WM_MU_0)
    EVBIT_MODIFY(0x1)
    CALL(EventScr_RemoveBGIfNeeded)
    ENDA
};

const EventScr EventScrWM_Ch4_TRAVEL_TO_NODE[] = {
    MUSCFAST(0x7fff)
    MUSC(SONG_THE_BEGINNING)
    STAL(6)
    ASMC(ReduceBGMVolume)
    WM_MOVECAM(-1, -1, 22, 94, 60, 0)
    WM_WAITFORCAM
    CALL(EventScr_WM_FadeCommon)
    WM_TEXTSTART
    WM_SHOWPORTRAIT(0, 0x0002, 0x0534, 0)  // Eirika
    STAL(6)
    STAL(46)
    WM_TEXT(Chapter_04_WM, 0)

    // With her bracelet recovered, Eirika sets out with renewed determination.
    SOUN(0x170)
    TEXTCONT
    TEXTEND

    WM_MOVESPRITETO(WM_MU_0, 0x0000, WM_NODE_BorgoRidge, WM_NODE_ZahaWoods, -8, 0)

    // Passing through the ancient forest of Za'ha brings them close to Grado's border.
    SOUN(0x171)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITES(WM_MU_ANY)
    PUTSPRITE(WM_MU_2, CLASS_REVENANT, WM_FACTION_RED, WM_NODE_BorderMulan)
    PUTSPRITE(WM_MU_3, CLASS_REVENANT, WM_FACTION_RED, WM_NODE_BorderMulan)
    WM_PUTSPRITE(WM_MU_2, 0x8c, 0xa8)
    WM_PUTSPRITE(WM_MU_3, 0x9e, 0xa8)
    WM_FADEINSPRITE(WM_MU_2, 60)
    WM_FADEINSPRITE(WM_MU_3, 60)

    // As they near the town of Serafew, Eirika's fears are assuaged by the lack of troops.
    SOUN(0x17A)
    TEXTCONT
    TEXTEND

    // But terrors of old crawl in the shadows of the trees.
    SOUN(0x17B)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITELOAD
    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    WM_REMSPRITE(WM_MU_2)
    WM_REMSPRITE(WM_MU_3)
    ENDA
};

const EventScr EventScrWM_Renval_CH5[] = {
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_Renvall2)
    WmEvtSetCamToNode(WM_NODE_AdlasPlains) // ENOSUPP in EAstdlib
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    WM_MOVESPRITETO(WM_MU_0, 0x0004, WM_NODE_Renvall2, WM_NODE_AdlasPlains, -2, 0)
    WM_WAITFORSPRITES(WM_MU_0)
    WM_MOVESPRITETO(WM_MU_0, 0x0004, WM_NODE_AdlasPlains, WM_NODE_Serafew, -2, 0)
    WM_WAITFORSPRITES(WM_MU_0)
    WmEvtSetUnitOnNode(WM_MU_0, WM_NODE_Serafew) // ENOSUPP in EAstdlib
    WmEvtSetNextStoryNode(WM_NODE_Serafew) // ENOSUPP in EAstdlib
    SKIPWN
    ENDA
};

const EventScr EventScrWM_Ch5_SET_NODE[] = {
    CHECK_EVENTID(136)
    BEQ(0x1, EVT_SLOT_C, EVT_SLOT_0)
    CALL(EventScrWM_Renval_CH5)
    GOTO(0x2)
LABEL(0x1)
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_ZahaWoods)
    WM_CENTERCAMONLORD(WM_MU_0)
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    STAL(20)
    WM_LOADLOCATION3(WM_NODE_Serafew)
    WM_SETDESTINATION(WM_NODE_Serafew)
    // WM_CREATENEXTDESTINATION
    WM_WAITFORFX
    STAL(40)
    WM_DRAWPATH(WM_PATH_04)
    STAL(70)
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
    SOUN(0x184)
    TEXTCONT
    TEXTEND

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
    WmEvtRemoveBigMap // WM_WAITFORFXCLEAR1
    WM_FADEOUTSPRITE(WM_MU_0, 1)
    WmEvtWaitBigMapRemove // WM_WAITFORFXCLEAR2
    WM_SHOWPORTRAIT(0, Overworld_Eirika, 0x0534, 0)
    STAL(6)
    STAL(46)

    // Eirika's journey to find her brother Ephraim, grows ever more perilous.
    SOUN(0x185)
    TEXTCONT
    TEXTEND

    // Still, she presses on despite the danger, without rest.
    SOUN(0x18E)
    TEXTCONT
    TEXTEND

    PUTSPRITE(WM_MU_2, CLASS_ARMOR_KNIGHT, WM_FACTION_RED, WM_NODE_Serafew)
    WM_FADEINSPRITE(WM_MU_2, 60)

    // However, a glimmer of hope now exists.
    SOUN(0x18F)
    TEXTCONT
    TEXTEND

    // Having reunited Renais' former famed military commander: Garcia, with his son: Ross.
    SOUN(0x196)
    TEXTCONT
    TEXTEND

    // A temporary alliance has been born as the group now travels to Serafew.
    SOUN(0x197)
    TEXTCONT
    TEXTEND

    // A bustling border town located between Renais and Grado.
    SOUN(0x198)
    TEXTCONT
    TEXTEND

    // The people of both countries have long used the town as a meeting place.
    SOUN(0x199)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITELOAD
    STAL(14)
    WM_FADEOUTSPRITE(WM_MU_2, 60)
    WM_MOVESPRITETO(WM_MU_0, Overworld_Eirika, WM_NODE_ZahaWoods, WM_NODE_Serafew, -4, 0) // Eirika
    WM_WAITFORSPRITELOAD

    // It once stood as testimony to the harmony the nations have shared these many years.
    SOUN(0x1A0)
    TEXTCONT
    TEXTEND

    // Now however, it exists as a grim reflection of a friendship ruined...
    SOUN(0x1A1)
    TEXTCONT
    TEXTEND

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

const EventScr EventScrWM_Ch5x_STUFF[] = {
    FADI(16)
    ENDA
};

const EventScr EventScrWM_Ch6_SET_NODE[] = {
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_Serafew)
    WM_CENTERCAMONLORD(WM_MU_0)
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    STAL(20)
    WM_LOADLOCATION3(WM_NODE_AdlasPlains)
    WM_SETDESTINATION(WM_NODE_AdlasPlains)
    // WM_CREATENEXTDESTINATION
    WM_WAITFORFX
    STAL(40)
    WM_DRAWPATH(WM_PATH_05)
    STAL(70)
    WM_MAKELORDVISIBLE(WM_MU_0)
    EVBIT_MODIFY(0x1)
    CALL(EventScr_RemoveBGIfNeeded)
    ENDA
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
    WM_SHOWPORTRAIT(0, 0x0002, 0x0534, 0) // Eirika
    STAL(6)
    STAL(46)
    WM_TEXT(Chapter_06_WM, 0)

    // Eirika suppresses her feelings of unease and continues to search for her brother.
    SOUN(0x1A2)
    TEXTCONT
    TEXTEND

    // Passing through Serafew, the group crosses into the Grado Empire.
    SOUN(0x1A3)
    TEXTCONT
    TEXTEND

    WM_MOVESPRITETO(WM_MU_0, 0x0002, WM_NODE_Serafew, WM_NODE_AdlasPlains, -4, 0)
    
    // From here forward, there is no doubt that they are walking into hostile territory.
    SOUN(0x1AA)
    TEXTCONT
    TEXTEND

    // In hopes of avoiding unnecessary trouble, Eirika and company adjust course slightly.
    SOUN(0x1AB)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITES(WM_MU_ANY)
    
    // Aiming to pass through seemingly empty fields as they gradually make their way south.
    SOUN(0x1AC)
    TEXTCONT
    TEXTEND

    // Grado's machinations, however, will soon be laid bare before her.
    SOUN(0x1AD)
    TEXTCONT
    TEXTEND

    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    ENDA
};

const EventScr EventScrWM_Ch7_SET_NODE[] = {
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_AdlasPlains)
    WM_CENTERCAMONLORD(WM_MU_0)
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    STAL(20)
    WM_LOADLOCATION3(WM_NODE_Renvall1)
    WM_SETDESTINATION(WM_NODE_Renvall1)
    // WM_CREATENEXTDESTINATION
    WM_WAITFORFX
    STAL(40)
    WM_DRAWPATH(WM_PATH_06)
    STAL(70)
    WM_MAKELORDVISIBLE(WM_MU_0)
    EVBIT_MODIFY(0x1)
    CALL(EventScr_RemoveBGIfNeeded)
    ENDA
};

const EventScr EventScrWM_Ch7_TRAVEL_TO_NODE[] = {
    MUSCFAST(0x7fff)
    MUSC(SONG_THE_BEGINNING)
    STAL(6)
    ASMC(ReduceBGMVolume)
    WM_MOVECAM(-1, -1, 84, 152, 60, 0)
    WM_WAITFORCAM
    CALL(EventScr_WM_FadeCommon)
    WM_TEXTSTART
    WM_TEXT(Chapter_07_WM, 0)
    
    // Rumors say that Ephraim has been defeated and taken prisoner.
    SOUN(0x1B4)
    TEXTCONT
    TEXTEND

    WM_SHOWPORTRAIT(0, 0x0002, 0x0534, 0) // Eirika
    STAL(6)
    STAL(46)

    // Now Eirika must find a way to rescue her brother from beneath enemy eyes.
    SOUN(0x1B5)
    TEXTCONT
    TEXTEND

    WM_MOVESPRITETO(WM_MU_0, 0x0002, WM_NODE_AdlasPlains, WM_NODE_Renvall1, -8, 0)
    
    // Eirika's company rushes toward Castle Renvall, where Ephraim is being held.
    SOUN(0x1B6)
    TEXTCONT
    TEXTEND

    // Surrounded by lakes, Renvall is a natural fortress, all but unassailable.
    SOUN(0x1B7)
    TEXTCONT
    TEXTEND

    // A lone bridge, held by the enemy, is its only entrance.
    SOUN(0x1BE)
    TEXTCONT
    TEXTEND

    // Eirika has only one choice.
    SOUN(0x1BF)
    TEXTCONT
    TEXTEND

    // She resolves to confront Grado's soldiers head on...
    SOUN(0x1C0)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITES(WM_MU_ANY)
    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    WM_REMSPRITE(WM_MU_2)
    ENDA
};

const EventScr EventScrWM_Ch8_SET_NODE[] = {
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_Renvall2)
    WM_CENTERCAMONLORD(WM_MU_0)
    WM_REMOVELOCATION(WM_NODE_Renvall1)
    WM_REMOVEPATH(WM_PATH_06)
    WM_LOADLOCATION2(WM_NODE_Renvall2) // Don't show the blue light effect
    WM_SETDESTINATION(WM_NODE_Renvall2)
    WM_DRAWPATH2(WM_PATH_07)
    WmEvtSetUnitOnNode(WM_MU_0, WM_NODE_Renvall2) // ENOSUPP in EAstdlib
    SKIPWN
    ENDA
};

const EventScr EventScrWM_Ch8_TRAVEL_TO_NODE[] = {
    EVBIT_MODIFY(0x1)
    ENUT(136) // Set departure flag
    ENDA
};

void SetMode() {
    gPlaySt.chapterModeIndex = 3; // Set gameplay mode
    // NoCashGBAPrintf("Current mode is: %d", gPlaySt.chapterModeIndex);
};

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
    SOUN(0x1C1)
    TEXTCONT
    TEXTEND

    WM_HIGHLIGHTCLEAR1(WM_NATION_Rausten)
    WM_HIGHLIGHTCLEAR2(WM_NATION_Rausten)
    WmEvtRemoveBigMap // WM_WAITFORFXCLEAR1
    WmEvtWaitBigMapRemove // WM_WAITFORFXCLEAR2
    WM_MOVESPRITETO(WM_MU_0, 0x0002, WM_NODE_BorderMulan, WM_NODE_PortKiris, -4, 0)

    // Their search for a ship leads them to Port Kiris in Carcino.
    SOUN(0x1C8)
    TEXTCONT
    TEXTEND

    WM_WAITFORSPRITES(WM_MU_ANY)
    WM_MAKELORDDISAPPEAR(WM_MU_0)
    WM_SHOWDRAWNMAP(0, -8, 0x3)
    STAL(120)
    WM_HIGHLIGHT(WM_NATION_Carcino)
    STAL(30)

    // Carcino is a young nation ruled by a council of merchants, and it is an ally of Frelia.
    SOUN(0x1C9)
    TEXTCONT
    TEXTEND

    WM_HIGHLIGHTCLEAR1(WM_NATION_Carcino)
    WM_HIGHLIGHTCLEAR2(WM_NATION_Carcino)
    WM_SHOWPORTRAIT(0, 0x0054, 0x02BC, 0)
    STAL(6)
    STAL(46)

    // Carcino council leader Klimt, a staunch opponent of Grado's imperialistic actions,
    // has pledged Carcino's support to Frelia in this conflict.
    SOUN(0x1CA)
    TEXTCONT
    TEXTEND

    WM_SHOWPORTRAIT(1, 0x0052, 0x0534, 0)
    STAL(6)
    STAL(46)

    // Prince Innes also intends to pass through Carcino on his way to Jehanna.
    SOUN(0x1CB)
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
    SOUN(0x1D2)
    TEXTCONT
    TEXTEND

    // ...She is wrong.
    SOUN(0x1D3)
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

const EventScr EventScrWM_Ch10_SET_NODE[] = {
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_PortKiris)
    WM_CENTERCAMONLORD(WM_MU_0)
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    STAL(20)
    WM_CREATENEXTDESTINATION
    WM_WAITFORFX
    STAL(40)
    WM_DRAWPATH(WM_PATH_0A)
    STAL(70)
    WM_MAKELORDVISIBLE(WM_MU_0)
    EVBIT_MODIFY(0x1)
    CALL(EventScr_RemoveBGIfNeeded)
    ENDA
};

const EventScr EventScrWM_Ch10_TRAVEL_TO_NODE[] = {
    MUSCFAST(0x7fff)
    MUSC(SONG_THE_BEGINNING)
    STAL(6)
    ASMC(ReduceBGMVolume)
    WM_MOVECAM(-1, -1, 86, 20, 60, 0)
    WM_WAITFORCAM
    CALL(EventScr_WM_FadeCommon)
    STAL(60)
    WM_TEXTSTART
    WM_SHOWDRAWNMAP(0, -8, 0x3)
    STAL(120)
    WM_HIGHLIGHT(WM_NATION_Carcino)
    STAL(30)
    WM_TEXT(0x08E7, 0)
    TEXTEND
    WM_HIGHLIGHTCLEAR1(WM_NATION_Carcino)
    WM_HIGHLIGHTCLEAR2(WM_NATION_Carcino)
    WmEvtRemoveBigMap // WM_WAITFORFXCLEAR1
    WmEvtWaitBigMapRemove // WM_WAITFORFXCLEAR2
    WM_SHOWPORTRAIT(0, 0x000F, 0x0534, 0)
    STAL(6)
    STAL(46)
    PUTSPRITE(WM_MU_2, CLASS_SNIPER, WM_FACTION_BLUE, WM_NODE_TerazPlateau)
    WM_FADEINSPRITE(WM_MU_2, 60)
    WM_WAITFORSPRITELOAD
    TEXTCONT
    TEXTEND
    PUTSPRITE(WM_MU_3, CLASS_MERCENARY, WM_FACTION_RED, WM_NODE_BorderMulan)
    PUTSPRITE(WM_MU_4, CLASS_MERCENARY, WM_FACTION_RED, WM_NODE_BorderMulan)
    WM_PUTSPRITE(WM_MU_3, 0xd8, 0x58)
    WM_PUTSPRITE(WM_MU_4, 0xd8, 0x78)
    WM_FADEINSPRITE(WM_MU_3, 60)
    WM_FADEINSPRITE(WM_MU_4, 60)
    WM_WAITFORSPRITELOAD
    TEXTCONT
    TEXTEND
    WM_CLEARPORTRAIT(0, 0x0200, 0)
    WM_FADEOUTSPRITE(WM_MU_2, 60)
    WM_WAITFORSPRITELOAD
    STAL(16)
    WM_FADEOUTSPRITE(WM_MU_3, 60)
    WM_FADEOUTSPRITE(WM_MU_4, 60)
    WM_WAITFORSPRITELOAD
    WM_REMSPRITE(WM_MU_2)
    WM_REMSPRITE(WM_MU_3)
    WM_REMSPRITE(WM_MU_4)
    WM_MOVESPRITETO(WM_MU_0, 0x0000, WM_NODE_PortKiris, WM_NODE_TerazPlateau, -4, 0)
    TEXTCONT
    TEXTEND
    WM_WAITFORSPRITES(WM_MU_ANY)
    STAL(40)
    WM_REMOVETEXT
    STAL(2)
    STAL(20)
    FADI(16)
    ENDA
};