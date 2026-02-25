#include "common-chax.h"
#include "prep-skill.h"
#include "kernel-lib.h"
#include "uichapterstatus.h"
#include "jester_headers/procs.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/miscellaneous.h"
#include "constants/texts.h"
#include "uiconfig.h"
#include "prepscreen.h"
#include "uichapterstatus.h"
#include "unitlistscreen.h"

#define PREP_MENU_VISIBLE_COUNT 5
#define PREP_MENU_MAX_COUNT 8

LYN_REPLACE_CHECK(PrepScreenMenu_OnCheckMap);
void PrepScreenMenu_OnCheckMap(struct ProcAtMenu *proc)
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    proc->state = 5;
    Proc_Goto(proc, 0x5);
}

void PrepScreenMenu_OnBEXP(struct ProcAtMenu *proc)
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    proc->state = 6;
    Proc_Goto(proc, 0xA);
}

void PrepScreenMenu_OnEquip(struct ProcAtMenu *proc)
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    proc->state = 7;
    Proc_Goto(proc, 0xA);
}

void PrepScreenMenu_OnBaseConversations(struct ProcAtMenu *proc)
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    proc->state = 8;
    Proc_Goto(proc, 0xA);
}

void PrepScreenMenu_OnAugury(struct ProcAtMenu *proc)
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    proc->state = 9;
    Proc_Goto(proc, 0xA);
}

void PrepScreenMenu_OnInfuse(struct ProcAtMenu *proc)
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    proc->state = 10;
    Proc_Goto(proc, 0xA);
}

/* -----------------------------------------------------------------------
 * ROM-backed menu item table.
 * Placed after all callbacks it references so no forward declarations are
 * needed.  The order of entries defines the display order in the menu.
 * proc->cur_index is a direct index into this table.
 * ----------------------------------------------------------------------- */
struct PrepMenuItem {
    int  index;                          /* PREP_MAINMENU_* constant      */
    void (*effect)(struct ProcAtMenu *); /* callback when A is pressed    */
    int  color;                          /* TEXT_COLOR_SYSTEM_*           */
    int  msg;                            /* title string id               */
    int  msg_desc;                       /* description string id         */
    int  msg_rtext;                      /* R-button help text string id  */
};

static const struct PrepMenuItem gPrepMenuTable[] = {
    { PREP_MAINMENU_UNIT,               PrepScreenMenu_OnPickUnits,         TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_UNITS,              MSG_PREP_SCREEN_DESC_UNITS,              0 },
    { PREP_MAINMENU_ITEM,               PrepScreenMenu_OnItems,             TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_ITEMS,              MSG_PREP_SCREEN_DESC_ITEMS,              0 },
    { PREP_MAINMENU_SAVE,               PrepScreenMenu_OnSave,              TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_SAVE,               MSG_PREP_SCREEN_DESC_SAVE,               0 },
    { PREP_MAINMENU_INFUSE,             PrepScreenMenu_OnInfuse,            TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_INFUSE,             MSG_PREP_SCREEN_DESC_INFUSE,             0 },
    { PREP_MAINMENU_AUGURY,             PrepScreenMenu_OnAugury,            TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_AUGURY,             MSG_PREP_SCREEN_DESC_AUGURY,             0 },
    { PREP_MAINMENU_BONUS_EXP,          PrepScreenMenu_OnBEXP,              TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_BEXP,               MSG_PREP_SCREEN_DESC_BEXP,               0 },
    { PREP_MAINMENU_SKILLS,             PrepScreenMenu_OnEquip,             TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_SKILLS,             MSG_PREP_SCREEN_DESC_SKILLS,             0 },
    { PREP_MAINMENU_BASE_CONVERSATIONS, PrepScreenMenu_OnBaseConversations, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_BASE_CONVERSATIONS, MSG_PREP_SCREEN_DESC_BASE_CONVERSATIONS, 0 },
    { PREP_MAINMENU_SUPPORT,            PrepScreenMenu_OnSupport,           TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_SUPPORT,            MSG_PREP_SCREEN_DESC_SUPPORT,            0 },
    { PREP_MAINMENU_CHECKMAP,           PrepScreenMenu_OnCheckMap,          TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_CHECK_MAP,          MSG_PREP_SCREEN_DESC_CHECK_MAP,          0 },
};

#define PREP_MENU_TABLE_SIZE (int)(sizeof(gPrepMenuTable) / sizeof(gPrepMenuTable[0]))
struct CheckMapMenuItem {
    int msg;        /* title string id               */
    int msg_rtext;  /* R-button help text string id  */
};

const struct CheckMapMenuItem gCheckMapMenuTable[] = {
    { MSG_0590, MSG_05BB }, /* View Map  */
    { MSG_0591, MSG_05BC }, /* Formation */
    { MSG_0592, MSG_05BD }, /* Options   */
    { MSG_05D1, MSG_05BE }, /* Save      */
};

#define CHECK_MAP_MENU_TABLE_SIZE (int)(sizeof(gCheckMapMenuTable) / sizeof(gCheckMapMenuTable[0]))

LYN_REPLACE_CHECK(AtMenu_StartSubmenu);
void AtMenu_StartSubmenu(struct ProcAtMenu *proc)
{
    sub_8095C2C(proc);

    switch (proc->state) {
        case PREP_MAINMENU_UNIT + 1:               Proc_StartBlocking(ProcScr_PrepUnitScreen, proc); break;
        case PREP_MAINMENU_ITEM + 1:               StartPrepItemScreen(proc);                        break;
        case PREP_MAINMENU_SAVE + 1:               StartPrepSaveScreen(proc);                        break;
        case PREP_MAINMENU_SUPPORT + 1:            StartFortuneSubMenu(2, proc);                     break;
        case PREP_MAINMENU_CHECKMAP + 1:           StartChapterStatusScreen_FromPrep(proc);          break;
        case PREP_MAINMENU_BONUS_EXP + 1:          StartBEXPScreen_FromPrep(proc);                   break;
        case PREP_MAINMENU_SKILLS + 1:             StartPrepEquipScreen(proc);                       break;
        case PREP_MAINMENU_BASE_CONVERSATIONS + 1: StartBaseScreen_FromPrep(proc);                   break;
        case PREP_MAINMENU_AUGURY + 1:             StartAuguryScreen_FromPrep(proc);                 break;
        case PREP_MAINMENU_INFUSE + 1:             StartInfuseScreen_FromPrep(proc);                 break;
        default: break;
    }

    Proc_Break(proc);
}

LYN_REPLACE_CHECK(PrepMenu_OnInit);
void PrepMenu_OnInit(struct ProcPrepMenu *proc)
{
    proc->cur_index = 0;
    proc->max_index = 0;

    ResetSysHandCursor(proc);
    DisplaySysHandCursorTextShadow(0x740, 1);

    proc->on_PressB     = 0;
    proc->on_PressStart = 0;
    proc->on_End        = 0;
    proc->do_help       = 0;

    gTopVisibleListIndex = 0;

    StartMenuScrollBar(proc);
    PutMenuScrollBarAt(3, 65);
    InitMenuScrollBarImg(0xE00, 2);
}

LYN_REPLACE_CHECK(PrepMenu_CtrlLoop);
void PrepMenu_CtrlLoop(struct ProcPrepMenu *proc)
{
    int index    = proc->cur_index;
    int xPos     = (proc->xPos + 1) * 8 + 4;
    int visibleY = (proc->yPos + 1) * 8 + (proc->cur_index - gTopVisibleListIndex) * 16;

    ShowSysHandCursor(xPos, visibleY, 0x6, 0x400);

    if (proc->do_help) {
        if ((R_BUTTON | B_BUTTON) & gKeyStatusPtr->newKeys) {
            CloseHelpBox();
            proc->do_help = 0;
            return;
        }
    } else {
        if (R_BUTTON & gKeyStatusPtr->newKeys) {
            int rtext = 0;

            if (proc->max_index <= CHECK_MAP_MENU_TABLE_SIZE) {
                if (proc->cur_index < CHECK_MAP_MENU_TABLE_SIZE)
                    rtext = gCheckMapMenuTable[proc->cur_index].msg_rtext;
            } else {
                if (proc->cur_index < PREP_MENU_TABLE_SIZE)
                    rtext = gPrepMenuTable[proc->cur_index].msg_rtext;
            }

            if (rtext) {
                StartHelpBox((proc->xPos + 1) * 8 + 4, visibleY, rtext);
                proc->do_help = 1;
            }
            return;
        }

        if (A_BUTTON & gKeyStatusPtr->newKeys) {
            
            if (proc->max_index <= CHECK_MAP_MENU_TABLE_SIZE) 
            {
                if (proc->cur_index < CHECK_MAP_MENU_TABLE_SIZE)
                {
                    switch (proc->cur_index)
                    {
                        case 0: PrepMapMenu_OnViewMap(Proc_Find(gProcScr_SALLYCURSOR)); return;
                        case 1: PrepMapMenu_OnFormation(Proc_Find(gProcScr_SALLYCURSOR)); return;
                        case 2: PrepMapMenu_OnOptions(Proc_Find(gProcScr_SALLYCURSOR)); return;
                        case 3: PrepMapMenu_OnSave(Proc_Find(gProcScr_SALLYCURSOR)); return;
                        default: return;
                    }
                }
            } 
            else 
            {    
                if (proc->cur_index < PREP_MENU_TABLE_SIZE) {
                    const struct PrepMenuItem *entry = &gPrepMenuTable[proc->cur_index];
                    if (1 & entry->color) {
                        PlaySoundEffect(SONG_6C);
                        return;
                    }
                    if (entry->effect) {
                        PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
                        entry->effect((struct ProcAtMenu *)proc->proc_parent);
                        return;
                    }
                }
            }
        }

        if (B_BUTTON & gKeyStatusPtr->newKeys) {
            if (proc->on_PressB) {
                if (proc->on_PressB(proc->proc_parent))
                {
                    PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
                }
                else
                    PlaySoundEffect(SONG_6C);
            }
            return;
        }

        if (START_BUTTON & gKeyStatusPtr->newKeys) {
            if (proc->on_PressStart) {
                if (proc->on_PressStart(proc->proc_parent)) {
                    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
                    Proc_Goto(proc, 0x0);
                } else {
                    PlaySoundEffect(SONG_6C);
                }
            }
            return;
        }
    }

    if (DPAD_UP & gKeyStatusPtr->repeatedKeys) {
        if (proc->cur_index)
            proc->cur_index--;
        else if (DPAD_UP & gKeyStatusPtr->newKeys)
            proc->cur_index = proc->max_index - 1;
    }

    if (DPAD_DOWN & gKeyStatusPtr->repeatedKeys) {
        if (proc->cur_index < (proc->max_index - 1))
            proc->cur_index++;
        else if (DPAD_DOWN & gKeyStatusPtr->newKeys)
            proc->cur_index = 0;
    }

    if (index != proc->cur_index) {
        PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);

        if (proc->do_help) {
            int rtext = 0;
            int newVisibleY = (proc->yPos + 1) * 8 + (proc->cur_index - gTopVisibleListIndex) * 16;

            if (proc->max_index <= CHECK_MAP_MENU_TABLE_SIZE) {
                if (proc->cur_index < CHECK_MAP_MENU_TABLE_SIZE)
                    rtext = gCheckMapMenuTable[proc->cur_index].msg_rtext;
            } else {
                if (proc->cur_index < PREP_MENU_TABLE_SIZE)
                    rtext = gPrepMenuTable[proc->cur_index].msg_rtext;
            }

            if (rtext)
                StartHelpBox((proc->xPos + 1) * 8 + 4, newVisibleY, rtext);
        }
    }

    if (proc->cur_index < gTopVisibleListIndex)
        gTopVisibleListIndex = proc->cur_index;

    if (proc->cur_index >= gTopVisibleListIndex + PREP_MENU_VISIBLE_COUNT - 1)
        gTopVisibleListIndex = proc->cur_index - PREP_MENU_VISIBLE_COUNT + 1;

    if (proc->max_index > 4)
        SetPrepScreenMenuPosition(1, 6);

    UpdateMenuScrollBarConfig(8, (u16)gTopVisibleListIndex * 16, (u16)proc->max_index, (u8)PREP_MENU_VISIBLE_COUNT);
}

LYN_REPLACE_CHECK(AtMenu_Reinitialize);
void AtMenu_Reinitialize(struct ProcAtMenu *proc)
{
    int i;

    SetupBackgrounds(gBgConfig_ItemUseScreen);
    ResetText();
    LoadUiFrameGraphics();
    LoadHelpBoxGfx(NULL, 0xE);
    SetDispEnable(0, 0, 0, 0, 0);
    LoadObjUIGfx();
    ResetUnitSprites();

    MakePrepUnitList();
    PrepAutoCapDeployUnits(proc);
    ReorderPlayerUnitsBasedOnDeployment();

    BG_Fill(gBG0TilemapBuffer, 0);
    BG_Fill(gBG1TilemapBuffer, 0);
    BG_Fill(gBG2TilemapBuffer, 0);

    for (i = 0; i < 5; i++)
        InitText(&gPrepMainMenuTexts[i + 5], 0xE);

    for (i = 0; i < 4; i++)
        InitText(&gPrepMainMenuTexts[i + 1], 0x8);

    InitText(&gPrepMainMenuTexts[0], 0xA);

    /* "Preparations" */
    Decompress(gUnknown_08A1A4C8, (void *)0x6014800);
    /* "Menu", "Start" button */
    Decompress(gUnknown_08A1D510, (void *)0x6016000);
    ApplyPalettes(Pal_SysBrownBox, 0x19, 2);

    /* Chapter text */
    sub_8095C50(0x7000, 0x6);
    ApplyPalette(gUnknown_08A1D4C8, 0x14);
    EnablePaletteSync();

    gLCDControlBuffer.bg0cnt.priority = 0;
    gLCDControlBuffer.bg1cnt.priority = 2;
    gLCDControlBuffer.bg2cnt.priority = 1;
    gLCDControlBuffer.bg3cnt.priority = 3;

    gLCDControlBuffer.dispcnt.win0_on   = 0;
    gLCDControlBuffer.dispcnt.win1_on   = 0;
    gLCDControlBuffer.dispcnt.objWin_on = 0;

    BG_SetPosition(0, 0, 0);
    BG_SetPosition(1, 0, 0);
    BG_SetPosition(2, 0, 0);

    InitPrepScreenMainMenu(proc);
    BG_EnableSyncByMask(0xF);
    SetDefaultColorEffects();

    StartPrepSpecialCharEffect(proc);
    RestartMuralBackground();
    ApplyPalettes(gUiFramePaletteB, 0x2, 3);

    if (CheckInLinkArena()) {
        Decompress(gUnknown_08A1B698, gGenericBuffer);
        CallARM_FillTileRect(TILEMAP_LOCATED(gBG1TilemapBuffer, 1, 5), gGenericBuffer, 0x1000);
    } else {
        /* Chapter objective tile background */
        Decompress(gUnknown_08A1B658, gGenericBuffer);
        CallARM_FillTileRect(TILEMAP_LOCATED(gBG1TilemapBuffer, 0x10, 2), gGenericBuffer, 0x1000);
        /* Options titles and descriptions tile background */
        Decompress(gUnknown_08A1B698, gGenericBuffer);
        CallARM_FillTileRect(TILEMAP_LOCATED(gBG1TilemapBuffer, 1, 6), gGenericBuffer, 0x1000);
    }

    Prep_DrawChapterGoal(0x5800, 0xB);
    NewSysBlackBoxHandler(proc);
    SysBlackBoxSetGfx(0x6800);
    proc->unk_35 = GetActivePrepMenuItemIndex();

    {
        struct ProcPrepMenu *menuProc = Proc_Find(ProcScr_PrepMenu);
        int slot = menuProc ? menuProc->cur_index : 0;
        if (slot < PREP_MENU_TABLE_SIZE)
            ParsePrepMenuDescTexts(gPrepMenuTable[slot].msg_desc);
    }

    DrawPrepMenuDescTexts();
}

/* -----------------------------------------------------------------------
 * sub_8095C00 — updates the description text when the cursor moves.
 * Reads proc->cur_index directly as a gPrepMenuTable index.
 * ----------------------------------------------------------------------- */
LYN_REPLACE_CHECK(sub_8095C00);
void sub_8095C00(int msg, ProcPtr parent)
{
    struct ProcPrepMenuDesc *proc;
    struct ProcPrepMenu     *menuProc;
    int slot;

    proc = Proc_Find(ProcScr_PrepMenuDescHandler);
    if (proc)
        Proc_End(proc);

    proc = Proc_Start(ProcScr_PrepMenuDescHandler, parent);

    menuProc = Proc_Find(ProcScr_PrepMenu);
    slot     = menuProc ? menuProc->cur_index : 0;

    if (slot < PREP_MENU_TABLE_SIZE)
        proc->msg = gPrepMenuTable[slot].msg_desc;
    else
        proc->msg = msg;
}

LYN_REPLACE_CHECK(SetPrepScreenMenuPosition);
void SetPrepScreenMenuPosition(int x, int y)
{
    int i;
    struct ProcPrepMenu *proc = Proc_Find(ProcScr_PrepMenu);

    if (proc) {
        proc->xPos = x;
        proc->yPos = y;

        if (proc->max_index > 1) {
            for (i = 0; i < PREP_MENU_VISIBLE_COUNT; i++) {
                int slot = gTopVisibleListIndex + i;

                if (slot >= proc->max_index)
                    break;

                ClearText(&gPrepMainMenuTexts[i]);
                PutDrawText(
                    &gPrepMainMenuTexts[i],
                    TILEMAP_LOCATED(gBG0TilemapBuffer, x + 2, y + 2 * i + 1),
                    TEXT_COLOR_SYSTEM_WHITE,
                    0, 0,
                    GetStringFromIndex(gPrepMenuTable[slot].msg)
                );
            }
        }

        BG_EnableSyncByMask(0x1);
    }
}

/* -----------------------------------------------------------------------
 * SetPrepScreenMenuItem — stubbed.
 * All item data lives in gPrepMenuTable.  This function's only remaining
 * job is to bump proc->max_index so the menu knows how many items to
 * scroll through.  All other parameters are ignored.
 * ----------------------------------------------------------------------- */
LYN_REPLACE_CHECK(SetPrepScreenMenuItem);
void SetPrepScreenMenuItem(int index, const void *func, int color, int msg, int msg_rtext)
{
    struct ProcPrepMenu *proc = Proc_Find(ProcScr_PrepMenu);

    if (proc)
        proc->max_index++;

    (void)index; (void)func; (void)color; (void)msg; (void)msg_rtext;
}

LYN_REPLACE_CHECK(InitPrepScreenMainMenu);
void InitPrepScreenMainMenu(struct ProcAtMenu *proc)
{
    StartPrepScreenMenu(proc);

    if (!CheckInLinkArena()) {
        SetPrepScreenMenuItem(PREP_MAINMENU_UNIT, NULL, TEXT_COLOR_SYSTEM_WHITE, 0, 0);
        SetPrepScreenMenuItem(PREP_MAINMENU_ITEM, NULL, TEXT_COLOR_SYSTEM_WHITE, 0, 0);

        if (CanPrepScreenSave())
            SetPrepScreenMenuItem(PREP_MAINMENU_SAVE, NULL, TEXT_COLOR_SYSTEM_WHITE, 0, 0);
        else
            SetPrepScreenMenuItem(PREP_MAINMENU_SAVE, NULL, TEXT_COLOR_SYSTEM_GRAY,  0, 0);

        if (gpKernelDesignerConfig->prep_menu_infuse == true)
            SetPrepScreenMenuItem(PREP_MAINMENU_INFUSE, NULL, TEXT_COLOR_SYSTEM_WHITE, 0, 0);

        if (gpKernelDesignerConfig->prep_menu_augury == true)
            SetPrepScreenMenuItem(PREP_MAINMENU_AUGURY, NULL, TEXT_COLOR_SYSTEM_WHITE, 0, 0);

        if (gpKernelDesignerConfig->prep_menu_bexp == true)
            SetPrepScreenMenuItem(PREP_MAINMENU_BONUS_EXP, NULL, TEXT_COLOR_SYSTEM_WHITE, 0, 0);

        if (gpKernelDesignerConfig->prep_menu_skills == true)
            SetPrepScreenMenuItem(PREP_MAINMENU_SKILLS, NULL, TEXT_COLOR_SYSTEM_WHITE, 0, 0);

        if (gpKernelDesignerConfig->prep_menu_base_conversations == true)
            SetPrepScreenMenuItem(PREP_MAINMENU_BASE_CONVERSATIONS, NULL, TEXT_COLOR_SYSTEM_WHITE, 0, 0);

        SetPrepScreenMenuItem(PREP_MAINMENU_SUPPORT,  NULL, TEXT_COLOR_SYSTEM_WHITE, 0, 0);
        SetPrepScreenMenuItem(PREP_MAINMENU_CHECKMAP, NULL, TEXT_COLOR_SYSTEM_WHITE, 0, 0);
    } else {
        SetPrepScreenMenuItem(PREP_MAINMENU_UNIT, NULL, TEXT_COLOR_SYSTEM_WHITE, 0, 0);
        SetPrepScreenMenuItem(PREP_MAINMENU_ITEM, NULL, TEXT_COLOR_SYSTEM_WHITE, 0, 0);
    }

    SetPrepScreenMenuOnBPress(PrepScreenMenu_OnBPress);
    SetPrepScreenMenuOnStartPress(PrepScreenMenu_OnStartPress);

    TileMap_FillRect(gBG0TilemapBuffer, 0xC, 0x13, 0);
    TileMap_FillRect(gBG1TilemapBuffer, 0xC, 0x13, 0);

    if (CheckInLinkArena())
        SetPrepScreenMenuPosition(1, 5);
    else
        SetPrepScreenMenuPosition(1, 6);

    SetPrepScreenMenuSelectedItem(proc->cur_cmd);
}

/* -----------------------------------------------------------------------
 * SetPrepScreenMenuSelectedItem — stubbed.
 * cur_index is a direct index into gPrepMenuTable; no PREP_MAINMENU_*
 * lookup is required.  cur_index is already zeroed in PrepMenu_OnInit.
 * ----------------------------------------------------------------------- */
LYN_REPLACE_CHECK(SetPrepScreenMenuSelectedItem);
void SetPrepScreenMenuSelectedItem(int index)
{
    (void)index;
}

/* -----------------------------------------------------------------------
 * GetActivePrepMenuItemIndex
 * Returns the PREP_MAINMENU_* index of the entry at the current cursor
 * position by reading directly from gPrepMenuTable.
 * ----------------------------------------------------------------------- */
LYN_REPLACE_CHECK(GetActivePrepMenuItemIndex);
int GetActivePrepMenuItemIndex(void)
{
    struct ProcPrepMenu *proc = Proc_Find(ProcScr_PrepMenu);

    if (proc && proc->cur_index < PREP_MENU_TABLE_SIZE)
        return gPrepMenuTable[proc->cur_index].index;

    return 0;
}

LYN_REPLACE_CHECK(DrawPrepScreenMenuFrameAt);
void DrawPrepScreenMenuFrameAt(int x, int y)
{
    int i;
    struct ProcPrepMenu *proc = Proc_Find(ProcScr_PrepMenu);

    if (proc) {
        proc->xPos = x;
        proc->yPos = y;

        DrawUiFrame2(x, y, 0xA, proc->max_index * 2 + 2, 0);

        if (proc->max_index > 1) {
            for (i = 0; i < proc->max_index; i++) {
                if (i >= CHECK_MAP_MENU_TABLE_SIZE)
                    break;

                ClearText(&gPrepMainMenuTexts[i]);
                PutDrawText(
                    &gPrepMainMenuTexts[i],
                    TILEMAP_LOCATED(gBG0TilemapBuffer, x + 2, y + 2 * i + 1),
                    TEXT_COLOR_SYSTEM_WHITE,
                    0, 0,
                    GetStringFromIndex(gCheckMapMenuTable[i].msg)
                );
            }
        }
        BG_EnableSyncByMask(0x3);
    }
}