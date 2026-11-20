#include "common-chax.h"
#include "utf8.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "jester_headers/procs.h"
#include "jester_headers/macros.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/custom-functions.h"
#ifdef CONFIG_DISTRICT_GUILD
#include "kernel/guild.h"
#endif

#define BASE_VISIBLE_COUNT 5

u8 MapMenu_IsBiographyCommandAvailable(const struct MenuItemDef* def, int number) {
    if (gpKernelDesignerConfig->map_menu_character_biographies == true)
        return MENU_ENABLED;
    else
        return MENU_NOTSHOWN;
}

int MapMenu_BiographyCommandDraw(struct MenuProc* menu, struct MenuItemProc* menuItem) {
    Text_DrawString(&menuItem->text, Utf8ToNarrowFonts(GetStringFromIndex(menuItem->def->nameMsgId)));
    PutText(&menuItem->text, BG_GetMapBuffer(menu->frontBg) + TILEMAP_INDEX(menuItem->xTile, menuItem->yTile));
    return 0;
}

static const struct CharacterBiography gCharacterBiographies[] =
{
    {
        CHARACTER_EIRIKA, "Restoration Lady", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_EIRIKA_01, 0x3B }, { MSG_HEROES_CARDS_EIRIKA_02, 0x3C }, { MSG_HEROES_CARDS_EIRIKA_03, 0x3D }, { MSG_HEROES_CARDS_EIRIKA_04, 0x3E }}
    },
    {
        CHARACTER_SETH, "The Silver Knight", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_SETH_01, 0xC3 }, { MSG_HEROES_CARDS_SETH_02, 0xC4 }, { MSG_HEROES_CARDS_SETH_03, 0xC5 }, { MSG_HEROES_CARDS_SETH_04, 0xC6 }}
    },
    {
        CHARACTER_FRANZ, "The Faithful", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_FRANZ_01, 0x6B }, { MSG_HEROES_CARDS_FRANZ_02, 0x6C }, { MSG_HEROES_CARDS_FRANZ_03, 0x6D }, { MSG_HEROES_CARDS_FRANZ_04, 0x6E }}
    },
    {
        CHARACTER_GILLIAM, "Wall of Silence", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_GILLIAM_01, 0x77 }, { MSG_HEROES_CARDS_GILLIAM_02, 0x78 }, { MSG_HEROES_CARDS_GILLIAM_03, 0x79 }, { MSG_HEROES_CARDS_GILLIAM_04, 0x7A }}
    },
    {
        CHARACTER_TANA, "Winged Princess", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_TANA_01, 0xCB }, { MSG_HEROES_CARDS_TANA_02, 0xCC }, { MSG_HEROES_CARDS_TANA_03, 0xCD }, { MSG_HEROES_CARDS_TANA_04, 0xCE }}
    },
    {
        CHARACTER_ARTUR, "The Devout", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_ARTUR_01, 0x43 }, { MSG_HEROES_CARDS_ARTUR_02, 0x44 }, { MSG_HEROES_CARDS_ARTUR_03, 0x45 }, { MSG_HEROES_CARDS_ARTUR_04, 0x46 }}
    },
    {
        CHARACTER_AMELIA, "The Hopeful", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_AMELIA_01, 0x3F }, { MSG_HEROES_CARDS_AMELIA_02, 0x40 }, { MSG_HEROES_CARDS_AMELIA_03, 0x41 }, { MSG_HEROES_CARDS_AMELIA_04, 0x42 }}
    },
    {
        CHARACTER_EPHRAIM, "Restoration Lord", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_EPHRAIM_01, 0x5B }, { MSG_HEROES_CARDS_EPHRAIM_02, 0x5C }, { MSG_HEROES_CARDS_EPHRAIM_03, 0x5D }, { MSG_HEROES_CARDS_EPHRAIM_04, 0x5E }}
    },
    {
        CHARACTER_INNES, "Regal Bowman", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_INNES_01, 0x7F }, { MSG_HEROES_CARDS_INNES_02, 0x80 }, { MSG_HEROES_CARDS_INNES_03, 0x81 }, { MSG_HEROES_CARDS_INNES_04, 0x82 }}
    },
    {
        CHARACTER_LUTE, "Prodigy", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_LUTE_01, 0x93 }, { MSG_HEROES_CARDS_LUTE_02, 0x94 }, { MSG_HEROES_CARDS_LUTE_03, 0x95 }, { MSG_HEROES_CARDS_LUTE_04, 0x96 }}
    },
    {
        CHARACTER_NATASHA, "Gentle Cleric", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_NATASHA_01, 0xA7 }, { MSG_HEROES_CARDS_NATASHA_02, 0xA8 }, { MSG_HEROES_CARDS_NATASHA_03, 0xA9 }, { MSG_HEROES_CARDS_NATASHA_04, 0xAA }}
    },
    {
        CHARACTER_CORMAG, "Wyvern Knight", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_CORMAG_01, 0x4F }, { MSG_HEROES_CARDS_CORMAG_02, 0x50 }, { MSG_HEROES_CARDS_CORMAG_03, 0x51 }, { MSG_HEROES_CARDS_CORMAG_04, 0x52 }}
    },
    {
        CHARACTER_FORDE, "Easygoing Knight", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_FORDE_01, 0x63 }, { MSG_HEROES_CARDS_FORDE_02, 0x64 }, { MSG_HEROES_CARDS_FORDE_03, 0x65 }, { MSG_HEROES_CARDS_FORDE_04, 0x66 }}
    },
    {
        CHARACTER_KYLE, "Earnest Knight", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_KYLE_01, 0x8B }, { MSG_HEROES_CARDS_KYLE_02, 0x8C }, { MSG_HEROES_CARDS_KYLE_03, 0x8D }, { MSG_HEROES_CARDS_KYLE_04, 0x8E }}
    },
    {
        CHARACTER_GERIK, "Mercenary Leader", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_GERIK_01, 0x73 }, { MSG_HEROES_CARDS_GERIK_02, 0x74 }, { MSG_HEROES_CARDS_GERIK_03, 0x75 }, { MSG_HEROES_CARDS_GERIK_04, 0x76 }}
    },
    {
        CHARACTER_TETHYS, "Graceful Dancer", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_TETHYS_01, 0xCF }, { MSG_HEROES_CARDS_TETHYS_02, 0xD0 }, { MSG_HEROES_CARDS_TETHYS_03, 0xD1 }, { MSG_HEROES_CARDS_TETHYS_04, 0xD2 }}
    },
    {
        CHARACTER_MARISA, "Crimson Flash", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_MARISA_01, 0x9B }, { MSG_HEROES_CARDS_MARISA_02, 0x9C }, { MSG_HEROES_CARDS_MARISA_03, 0x9D }, { MSG_HEROES_CARDS_MARISA_04, 0x9E }}
    },
    {
        CHARACTER_EWAN, "Playful Pupil", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_EWAN_01, 0x5F }, { MSG_HEROES_CARDS_EWAN_02, 0x60 }, { MSG_HEROES_CARDS_EWAN_03, 0x61 }, { MSG_HEROES_CARDS_EWAN_04, 0x62 }}
    },
    {
        CHARACTER_LARACHEL, "Holy Princess", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_LARACHEL_01, 0x8F }, { MSG_HEROES_CARDS_LARACHEL_02, 0x90 }, { MSG_HEROES_CARDS_LARACHEL_03, 0x91 }, { MSG_HEROES_CARDS_LARACHEL_04, 0x92 }}
    },
    {
        CHARACTER_DOZLA, "Jovial Warrior", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_DOZLA_01, 0x53 }, { MSG_HEROES_CARDS_DOZLA_02, 0x54 }, { MSG_HEROES_CARDS_DOZLA_03, 0x55 }, { MSG_HEROES_CARDS_DOZLA_04, 0x56 }}
    },
    {
        CHARACTER_RENNAC, "Reluctant Rogue", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_RENNAC_01, 0xB3 }, { MSG_HEROES_CARDS_RENNAC_02, 0xB4 }, { MSG_HEROES_CARDS_RENNAC_03, 0xB5 }, { MSG_HEROES_CARDS_RENNAC_04, 0xB6 }}
    },
    {
        CHARACTER_DUESSEL, "Obsidian", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_DUESSEL_01, 0x57 }, { MSG_HEROES_CARDS_DUESSEL_02, 0x58 }, { MSG_HEROES_CARDS_DUESSEL_03, 0x59 }, { MSG_HEROES_CARDS_DUESSEL_04, 0x5A }}
    },
    {
        CHARACTER_MYRRH, "Great Dragon", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_MYRRH_01, 0xA3 }, { MSG_HEROES_CARDS_MYRRH_02, 0xA4 }, { MSG_HEROES_CARDS_MYRRH_03, 0xA5 }, { MSG_HEROES_CARDS_MYRRH_04, 0xA6 }}
    },
    {
        CHARACTER_KNOLL, "Dark Scholar", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_KNOLL_01, 0x87 }, { MSG_HEROES_CARDS_KNOLL_02, 0x88 }, { MSG_HEROES_CARDS_KNOLL_03, 0x89 }, { MSG_HEROES_CARDS_KNOLL_04, 0x8A }}
    },
    {
        CHARACTER_JOSHUA, "Lucky Gambler", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_JOSHUA_01, 0x83 }, { MSG_HEROES_CARDS_JOSHUA_02, 0x84 }, { MSG_HEROES_CARDS_JOSHUA_03, 0x85 }, { MSG_HEROES_CARDS_JOSHUA_04, 0x86 }}
    },
    {
        CHARACTER_SYRENE, "Falcon Knight", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_SYRENE_01, 0xC7 }, { MSG_HEROES_CARDS_SYRENE_02, 0xC8 }, { MSG_HEROES_CARDS_SYRENE_03, 0xC9 }, { MSG_HEROES_CARDS_SYRENE_04, 0xCA }}
    },
    {
        CHARACTER_ROSS, "Eager Trainee", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_ROSS_01, 0xBB }, { MSG_HEROES_CARDS_ROSS_02, 0xBC }, { MSG_HEROES_CARDS_ROSS_03, 0xBD }, { MSG_HEROES_CARDS_ROSS_04, 0xBE }}
    },
    {
        CHARACTER_NEIMI, "Teary Archer", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_NEIMI_01, 0xAB }, { MSG_HEROES_CARDS_NEIMI_02, 0xAC }, { MSG_HEROES_CARDS_NEIMI_03, 0xAD }, { MSG_HEROES_CARDS_NEIMI_04, 0xAE }}
    },
    {
        CHARACTER_COLM, "Brash Thief", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_COLM_01, 0x4B }, { MSG_HEROES_CARDS_COLM_02, 0x4C }, { MSG_HEROES_CARDS_COLM_03, 0x4D }, { MSG_HEROES_CARDS_COLM_04, 0x4E }}
    },
    {
        CHARACTER_GARCIA, "Retired Warrior", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_GARCIA_01, 0x6F }, { MSG_HEROES_CARDS_GARCIA_02, 0x70 }, { MSG_HEROES_CARDS_GARCIA_03, 0x71 }, { MSG_HEROES_CARDS_GARCIA_04, 0x72 }}
    },
    {
        CHARACTER_LYON_CC, "Prince of Grado", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_LYON_01, 0x97 }, { MSG_HEROES_CARDS_LYON_02, 0x98 }, { MSG_HEROES_CARDS_LYON_03, 0x99 }, { MSG_HEROES_CARDS_LYON_04, 0x9A }}
    },
    {
        CHARACTER_ORSON_CC, "Devoted Knight", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_ORSON_01, 0xAF }, { MSG_HEROES_CARDS_ORSON_02, 0xB0 }, { MSG_HEROES_CARDS_ORSON_03, 0xB1 }, { MSG_HEROES_CARDS_ORSON_04, 0xB2 }}
    },
    {
        CHARACTER_GLEN_CC, "Sunstone", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_GLEN_01, 0x7B }, { MSG_HEROES_CARDS_GLEN_02, 0x7C }, { MSG_HEROES_CARDS_GLEN_03, 0x7D }, { MSG_HEROES_CARDS_GLEN_04, 0x7E }}
    },
    {
        CHARACTER_SELENA_CC, "Fluorspar", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_SELENA_01, 0xBF }, { MSG_HEROES_CARDS_SELENA_02, 0xC0 }, { MSG_HEROES_CARDS_SELENA_03, 0xC1 }, { MSG_HEROES_CARDS_SELENA_04, 0xC2 }}
    },
    {
        CHARACTER_VALTER_CC, "Moonstone", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_VALTER_01, 0xD3 }, { MSG_HEROES_CARDS_VALTER_02, 0xD4 }, { MSG_HEROES_CARDS_VALTER_03, 0xD5 }, { MSG_HEROES_CARDS_VALTER_04, 0xD6 }}
    },
    {
        CHARACTER_RIEV_CC, "Fallen Bishop", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_RIEV_01, 0xB7 }, { MSG_HEROES_CARDS_RIEV_02, 0xB8 }, { MSG_HEROES_CARDS_RIEV_03, 0xB9 }, { MSG_HEROES_CARDS_RIEV_04, 0xBA }}
    },
    {
        CHARACTER_CAELLACH_CC, "Tiger Eye", SONG_POWERFUL_FOE,
        {{ MSG_HEROES_CARDS_CAELLACH_01, 0x47 }, { MSG_HEROES_CARDS_CAELLACH_02, 0x48 }, { MSG_HEROES_CARDS_CAELLACH_03, 0x49 }, { MSG_HEROES_CARDS_CAELLACH_04, 0x4A }}
    },
};

/* A biography opens once its character has joined the army (a blue unit, alive or not) */
static bool BiographyUnlocked(int index)
{
    int i;

    for (i = FACTION_BLUE + 1; i < FACTION_BLUE + 0x40; i++) {
        struct Unit *unit = GetUnit(i);

        if (UNIT_IS_VALID(unit) && UNIT_CHAR_ID(unit) == gCharacterBiographies[index].characterId)
            return true;
    }
    return false;
}

/* The list holds the biographies whose character has joined: list row -> gCharacterBiographies index, or -1 */
static int BiographyAtRow(int row)
{
    int i;

    for (i = 0; i < (int)ARRAY_COUNT(gCharacterBiographies); i++) {
        if (!BiographyUnlocked(i))
            continue;
        if (row-- == 0)
            return i;
    }
    return -1;
}

static int NumberOfCharacterBiographies(void)
{
    int i, count = 0;

    for (i = 0; i < (int)ARRAY_COUNT(gCharacterBiographies); i++)
        if (BiographyUnlocked(i))
            count++;
    return count;
}

/* The visible rows: the unlocked biographies from gTopVisibleListIndex on (the list scrolls past BASE_VISIBLE_COUNT) */
static void DrawBaseConversations(int x, int y) {

    for (int row = 0; row < BASE_VISIBLE_COUNT; row++) 
    {
        int i = BiographyAtRow(gTopVisibleListIndex + row);

        ClearText(&PrepItemSuppyTexts.th[row+2]);
        if (i < 0) {
            PutText(&PrepItemSuppyTexts.th[row+2], TILEMAP_LOCATED(gBG0TilemapBuffer, x, y + (row * 2)));
            continue;
        }

        PutDrawText(&PrepItemSuppyTexts.th[row+2], TILEMAP_LOCATED(gBG0TilemapBuffer, x, y + (row * 2)),
                    TEXT_COLOR_SYSTEM_WHITE, 0, 0,
                    GetStringFromIndex(GetCharacterData(gCharacterBiographies[i].characterId)->nameTextId));

        PutDrawText(&PrepItemSuppyTexts.th[row+2], TILEMAP_LOCATED(gBG0TilemapBuffer, x, y + (row * 2)),
                    TEXT_COLOR_SYSTEM_GOLD, 40, 0,
                    Utf8ToNarrowFonts(gCharacterBiographies[i].subtitle));
    }

    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void Biography_Init(struct ProcPrepUnit * proc)
{
    gCharacterBiographyPage = 0;

    StartBgm(SONG_LAUGHTER, 0);

    ClearBg0Bg1();
    SetupBackgrounds(NULL);

    /* SetupBackgrounds lifts the fade: stay black while the list is drawn, NewFadeIn brings it up */
    SetBlendTargetA(1, 1, 1, 1, 1);
    SetBlendDarken(0x10);
    BG_Fill(BG_GetMapBuffer(0), 0);
    BG_Fill(BG_GetMapBuffer(1), 0);
    BG_Fill(BG_GetMapBuffer(2), 0);
    BG_Fill(BG_GetMapBuffer(3), 0);

    /* Rearrangeing background layer priorities so the cursor text shadow can sit between the text and the container frame */
    gLCDControlBuffer.bg0cnt.priority = 0;
    gLCDControlBuffer.bg1cnt.priority = 2;
    gLCDControlBuffer.bg2cnt.priority = 1;
    gLCDControlBuffer.bg3cnt.priority = 3;

    EndGreenText(); // Prevents glowing green text in CGs
    ResetText(); // Fixes the palette of the text
    LoadUiFrameGraphics(); // Fixes the palette of the container frame
    LoadObjUIGfx(); // Fixes the palette of the cursor hand
    LoadIconPalettes(4);
    RestartMuralBackground();
    DrawUiFrame2(6, 7, 18, 12, 0); // Draw the container frame
    SetPrimaryHBlankHandler(PrepItemSupply_OnHBlank); // Draw the transparent background header
    BG_EnableSyncByMask(7);
    StartUiCursorHand(proc);
    ResetSysHandCursor(proc);
    DisplaySysHandCursorTextShadow(0x600, 1);
    ShowSysHandCursor(56, 64 + ((proc->list_num_cur - gTopVisibleListIndex) * 16), 15, 0x800);

    for (int i = 0; i < 8; i++)
        InitText(PrepItemSuppyTexts.th + i, 20);

    sub_8097668();
    BG_EnableSyncByMask(4);

    PutDrawText(&PrepItemSuppyTexts.th[0], TILEMAP_LOCATED(gBG0TilemapBuffer, 6, 3), TEXT_COLOR_SYSTEM_WHITE, 15, 0, "Select a character to view");
    PutDrawText(&PrepItemSuppyTexts.th[1], TILEMAP_LOCATED(gBG0TilemapBuffer, 0, 0), TEXT_COLOR_SYSTEM_WHITE, 4, 0, "Biography");

    StartSysBrownBox(0x0, 0x7080, 0xf, 0xc00, 0x400, proc);
    EnableSysBrownBox(0, -40, -1, 1);
    StartMenuScrollBar(proc); 
    PutMenuScrollBarAt(188, 64); 
    InitMenuScrollBarImg(0x7A60, 2); 

    UpdateMenuScrollBarConfig(10, gTopVisibleListIndex * 16, NumberOfCharacterBiographies(), BASE_VISIBLE_COUNT);
    DrawBaseConversations(8, 8);
}

static void MainKeyHandler_Biography(struct ProcPrepUnit * proc) 
{
    bool hasScrolled = false;
    u16 keys = gKeyStatusPtr->newKeys;

    if (keys & A_BUTTON) {
        if (BiographyAtRow(proc->list_num_cur) < 0) {
            PlaySoundEffect(SONG_6C);
            return;
        }
        PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
        Proc_Goto(proc, PL_MAP_MENU_BIOGRAPHY_EVENT);
        return;
    }

    if (keys & B_BUTTON) {
        SetPrimaryHBlankHandler(NULL);
        Proc_Goto(proc, PL_MAP_MENU_BIOGRAPHY_PRESS_B);
    }

    if (keys & DPAD_UP) {
        if (proc->list_num_cur > 0) {
            proc->list_num_cur--;
            if (proc->list_num_cur < gTopVisibleListIndex)
                gTopVisibleListIndex--;
            hasScrolled = true;
        }
    }

    if (keys & DPAD_DOWN) {
        if (proc->list_num_cur < NumberOfCharacterBiographies() - 1) {
            proc->list_num_cur++;
            if (proc->list_num_cur >= gTopVisibleListIndex + BASE_VISIBLE_COUNT)
                gTopVisibleListIndex++;
            hasScrolled = true;
        }
    }

    if (hasScrolled) {
        ShowSysHandCursor(56, 64 + ((proc->list_num_cur - gTopVisibleListIndex) * 16), 15, 0x800);
        PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
        DrawBaseConversations(8, 8);
        UpdateMenuScrollBarConfig(10, gTopVisibleListIndex * 16, NumberOfCharacterBiographies(), BASE_VISIBLE_COUNT);
    }
}

static void SetIndexes(struct ProcPrepUnit * proc) 
{
    gEventSlots[EVT_SLOT_2] = gCharacterBiographies[gCharacterBiographyListNumber].entries[gCharacterBiographyPage].backgroundId;
    gEventSlots[EVT_SLOT_3] = gCharacterBiographies[gCharacterBiographyListNumber].entries[gCharacterBiographyPage].textId;
    gCharacterBiographyPage++;
}

/* The four pages, one after another (none is locked any more) */
static EventScr const EventScr_CharacterBiographies[] = {
	EVBIT_MODIFY(0x4) // No skipping
    FADE_TO_BLACK(16)
    ASMC(SetIndexes)
	TEXT_CG_BIOGRAPHY
    ASMC(SetIndexes)
	TEXT_CG_BIOGRAPHY
    ASMC(SetIndexes)
	TEXT_CG_BIOGRAPHY
    ASMC(SetIndexes)
	TEXT_CG_BIOGRAPHY
    FADE_TO_BLACK(16)
	ENDA
};

void CallBiographyEvent(struct ProcPrepUnit * proc) {
    /* Set the song based on the selection */
    gCharacterBiographyListNumber = BiographyAtRow(proc->list_num_cur);
    if (gCharacterBiographies[gCharacterBiographyListNumber].songId == 0)
        StartBgm(NextRN_N(0x46), 0);
    else
        StartBgm(gCharacterBiographies[gCharacterBiographyListNumber].songId, 0);

    KernelCallEvent(EventScr_CharacterBiographies, EV_EXEC_QUIET, proc);
}

/* Disable all backgrounds and TSAs before switching to the conversation event */
static void DisablePrepScreenDisplay(struct ProcPrepUnit * proc) {
    SetPrimaryHBlankHandler(NULL);

    ClearBg0Bg1();
    BG_Fill(BG_GetMapBuffer(2), 0);
    BG_Fill(BG_GetMapBuffer(3), 0);
    BG_EnableSyncByMask(0xF);
    
    ResetText();
    EndMenuScrollBar();
    EndSysBrownBox();
    EndMuralBackground_();
    HideSysHandCursor();

    /*
     * BG3's tile 0 is a map tile (grass): the cleared BG3 showed it when the event's first fade began. Blank it; the
     * event's pictures replace BG3's tiles, and leaving reloads the map's.
     */
    CpuFastFill(0, (void *)(VRAM + GetBackgroundTileDataOffset(BG_3)), CHR_SIZE);
}

static void Biography_RestoreMapGraphics(struct ProcPrepUnit * proc) {
    EndMuralBackground_();
    ResetUnitSprites();
    RefreshBMapGraphics();
    EndMenuScrollBar();
    HideSysHandCursor();
    EndSysBrownBox();
    SyncUnitSpriteSheet();
    RefreshUnitSprites();
    InitPlayerPhaseInterface();
}


static void ResetScrollerBarVariables(struct ProcPrepUnit * proc) 
{
    gTopVisibleListIndex = 0;
    gCharacterBiographyListNumber = 0;
    proc->list_num_cur = 0;
}

static void ForceBMapDispResume(void)
{
    while (gBmSt.gameGfxSemaphore)
    {
        BMapDispResume();
    }
}

struct ProcCmd const ProcScr_MenuMap_BIOGRAPHY[] =
{
    PROC_NAME("MapMenu_BIOGRAPHY"),
    PROC_YIELD,
    PROC_CALL(LockGame), // Hide the map cursor
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),
    PROC_CALL(BMapDispSuspend), // Hide the unit map sprites
    PROC_YIELD,
    PROC_CALL(EndPlayerPhaseSideWindows),
    PROC_CALL(ResetScrollerBarVariables),

PROC_LABEL(PL_MAP_MENU_BIOGRAPHY_INIT),
    PROC_CALL(Biography_Init),
    PROC_CALL_ARG(NewFadeIn, 16),
    PROC_WHILE(FadeInExists),

PROC_LABEL(PL_MAP_MENU_BIOGRAPHY_IDLE),
    PROC_REPEAT(MainKeyHandler_Biography),

PROC_LABEL(PL_MAP_MENU_BIOGRAPHY_EVENT),
    /* The list fades out as it is, then is cleared; the event fades its first page in and fades out at its end */
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),
    PROC_CALL(DisablePrepScreenDisplay),
    PROC_YIELD, /* the list's sprites and tiles go before the event's CG mode, which keeps the last sprites */
    PROC_YIELD,
    PROC_CALL(CallBiographyEvent),
    PROC_YIELD, /* the event caller blocks this proc from the next frame: without it the list was drawn again at once */
    PROC_WHILE(EventEngineExists),
    PROC_GOTO(PL_MAP_MENU_BIOGRAPHY_INIT),

PROC_LABEL(PL_MAP_MENU_BIOGRAPHY_PRESS_B),
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),
    PROC_CALL(Biography_RestoreMapGraphics),
    PROC_CALL(StartMapSongBgm), // the map's music again: the list and the biographies play their own
    PROC_CALL(ApplyUnitSpritePalettes), // the list's brown box used OBJ palette 15, the waited units' grey
    PROC_CALL(SetAllUnitNotBackSprite),
    PROC_CALL(ResetUnitSpriteHover),
    PROC_CALL(ForceBMapDispResume),
    PROC_CALL(UnlockGame), // Display the map cursor

PROC_LABEL(PL_MAP_MENU_BIOGRAPHY_END),
    PROC_END
};

u8 MapMenu_BiographyCommand(struct MenuProc* menu, struct MenuItemProc* menuItem) {
    Proc_Start(ProcScr_MenuMap_BIOGRAPHY, PROC_TREE_3);

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}