#include "common-chax.h"
#include "utf8.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "jester_headers/procs.h"
#include "jester_headers/macros.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/custom-functions.h"

#define BASE_VISIBLE_COUNT 5

u8 MapMenu_IsBiographyCommandAvailable(const struct MenuItemDef* def, int number) {
    return MENU_ENABLED;
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
};

static int NumberOfCharacterBiographies() {
    int lengthOfList = 0;
    for (int i = 0; i < (int)ARRAY_COUNT(gCharacterBiographies); i++)
        lengthOfList += 1;

    return lengthOfList;
}

static void DrawBaseConversations(int x, int y) {

    for (int i = 0; i < BASE_VISIBLE_COUNT; i++) 
    {
        ClearText(&PrepItemSuppyTexts.th[i+2]);

        PutDrawText(&PrepItemSuppyTexts.th[i+2], TILEMAP_LOCATED(gBG0TilemapBuffer, x, y + (i * 2)), 
                    TEXT_COLOR_SYSTEM_WHITE, 0, 0, GetStringFromIndex(GetCharacterData(gCharacterBiographies[i].characterId)->nameTextId));
    
        PutDrawText(&PrepItemSuppyTexts.th[i+2], TILEMAP_LOCATED(gBG0TilemapBuffer, x, y + (i * 2)), 
                    TEXT_COLOR_SYSTEM_GOLD, 40, 0, Utf8ToNarrowFonts(gCharacterBiographies[i].subtitle));
    }

    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void Biography_Init(struct ProcPrepUnit * proc)
{
    gCharacterBiographyPage = 0;

    StartBgm(SONG_LAUGHTER, 0);

    ClearBg0Bg1();
    SetupBackgrounds(NULL);
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

    UpdateMenuScrollBarConfig(10, gTopVisibleListIndex * 16, 10, 9);
    DrawBaseConversations(8, 8);
}

static void MainKeyHandler_Biography(struct ProcPrepUnit * proc) 
{
    bool hasScrolled = false;
    u16 keys = gKeyStatusPtr->newKeys;

    if (keys & A_BUTTON) {
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
        UpdateMenuScrollBarConfig(10, gTopVisibleListIndex * 16, 10, 9);
    }
}

static void SetIndexes(struct ProcPrepUnit * proc) 
{
    gEventSlots[EVT_SLOT_2] = gCharacterBiographies[gCharacterBiographyListNumber].entries[gCharacterBiographyPage].backgroundId;
    gEventSlots[EVT_SLOT_3] = gCharacterBiographies[gCharacterBiographyListNumber].entries[gCharacterBiographyPage].textId;
    gCharacterBiographyPage++;
}

static EventScr const EventScr_CharacterBiographies[] = {
	EVBIT_MODIFY(0x4) // No skipping
    FADE_FROM_BLACK(16)
    ASMC(SetIndexes)
	TEXT_CG_BIOGRAPHY
    ASMC(SetIndexes)
	TEXT_CG_BIOGRAPHY
    ASMC(SetIndexes)
	TEXT_CG_BIOGRAPHY
    ASMC(SetIndexes)
	TEXT_CG_BIOGRAPHY
	ENDA
};

void CallBiographyEvent(struct ProcPrepUnit * proc) {
    /* Set the song based on the selection */
    gCharacterBiographyListNumber = proc->list_num_cur;
    if (gCharacterBiographies[proc->list_num_cur].songId == 0)
        StartBgm(NextRN_N(0x46), 0);
    else
        StartBgm(gCharacterBiographies[proc->list_num_cur].songId, 0);

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
    PROC_CALL(DisablePrepScreenDisplay),
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),
    PROC_CALL_ARG(NewFadeIn, 16),
    PROC_WHILE(FadeInExists),
    PROC_CALL(CallBiographyEvent),
    PROC_WHILE(EventEngineExists), 
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),
    PROC_GOTO(PL_MAP_MENU_BIOGRAPHY_INIT),

PROC_LABEL(PL_MAP_MENU_BIOGRAPHY_PRESS_B),
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),
    PROC_CALL(Biography_RestoreMapGraphics),
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