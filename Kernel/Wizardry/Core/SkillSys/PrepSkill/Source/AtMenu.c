#include "common-chax.h"
#include "prep-skill.h"
#include "kernel-lib.h"
#include "uichapterstatus.h"
#include "jester_headers/procs.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/miscellaneous.h"
#include "constants/texts.h"
#include "uiconfig.h"
#include "prepscreen.h"
#include "uichapterstatus.h"
#include "unitlistscreen.h"

void PrepScreenMenu_OnEquip(struct ProcAtMenu *proc)
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
	proc->state = 8;
	Proc_Goto(proc, 0xA);
}

void PrepScreenMenu_OnInfuse(struct ProcAtMenu* proc) 
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    proc->state = 14; // Sets the index AtMenu_StartSubMenu will look for
    Proc_Goto(proc, 0xA); // Sets a fade out and then calls AtMenu_StartSubMenu in ProcScr_AtMenu
}

void PrepScreenMenu_OnAugury(struct ProcAtMenu* proc) 
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    proc->state = 5;
    // CallSomeSoundMaybe(SONG_BONDS, 0x100, 0x100, 0x20, 0);
    Proc_Goto(proc, 0xA);
}

void PrepScreenMenu_OnBEXP(struct ProcAtMenu* proc) 
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    proc->state = 6;
    Proc_Goto(proc, 0xA);
}

LYN_REPLACE_CHECK(PrepScreenMenu_OnCheckMap);
void PrepScreenMenu_OnCheckMap(struct ProcAtMenu* proc) 
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    proc->state = 7;
    Proc_Goto(proc, 0x5);
}

void PrepScreenMenu_BaseConversations(struct ProcAtMenu* proc) 
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    proc->state = 9;
    Proc_Goto(proc, 0xA);
}

/* This handles where each menu option should go when selected */
LYN_REPLACE_CHECK(AtMenu_StartSubmenu);
void AtMenu_StartSubmenu(struct ProcAtMenu * proc)
{
    sub_8095C2C(proc);

    switch (proc->state) {

    case 1: /* Pick Units */
        Proc_StartBlocking(ProcScr_PrepUnitScreen, proc);
        break;

    case 2: /* Items */
        StartPrepItemScreen(proc);
        break;

    case 3: /* Save */
        StartPrepSaveScreen(proc);
        break;

    case 4: /* Support */
        StartFortuneSubMenu(2, proc);
        break;

    case 5: /* Augury */
        if (gpKernelDesignerConfig->prep_menu_augury == true)
            Proc_StartBlocking(PREEXT_Procs_Augury, proc);
        else
            StartChapterStatusScreen_FromPrep(proc);

        break;

    case 6: /* Bonus EXP */
        StartChapterStatusScreen_FromPrep(proc);
        break;

    case 7: /* Chapter Status */
        StartChapterStatusScreen_FromPrep(proc);
        break;

    case 8: /* Skills */
        StartPrepEquipScreen(proc); 
        break;

    case 9: /* Base Conversations */
        StartChapterStatusScreen_FromPrep(proc);
        break;

    case 14: /* Infuse */
        StartInfuseScreen(proc);
        break;

    default:
        break;
    }

    Proc_Break(proc);
};

LYN_REPLACE_CHECK(PrepMenu_OnInit);
void PrepMenu_OnInit(struct ProcPrepMenu * proc)
{
    int i;
    for (i = 0; i < PREP_MENU_MAX_COUNT; i++)
        proc->cmds[i] = 0;

    proc->cur_index = 0;
    proc->max_index = 0;

    ResetSysHandCursor(proc);

    // JESTER - Was originally at 0x600, but seems to be overlapping with some map icons, so switching to 0x740
    DisplaySysHandCursorTextShadow(0x740, 1);

    proc->on_PressB = 0;
    proc->on_PressStart = 0;
    proc->on_End = 0;
    proc->do_help = 0;

    firstVisibleIndex = 0;

    StartMenuScrollBar(proc);
    PutMenuScrollBarAt(3, 65); // x and y

    // JESTER - This was originally at 0x200, but seems to be overlapping some map icons, so switching to 0xE00
    InitMenuScrollBarImg(0xE00, 2); 
}

LYN_REPLACE_CHECK(PrepMenu_CtrlLoop);
void PrepMenu_CtrlLoop(struct ProcPrepMenu *proc)
{
    struct ProcPrepMenuItem* cmd;
    int index = proc->cur_index;
    int xPos = (proc->xPos + 1) * 8 + 4;

    int visibleX = (proc->xPos + 1) * 8 + (4 - firstVisibleIndex);
    int visibleY = (proc->yPos + 1) * 8 + (proc->cur_index - firstVisibleIndex) * 16;
    ShowSysHandCursor(xPos, visibleY, 0x6, 0x400);

    cmd = proc->cmds[proc->cur_index];

    if (proc->do_help) {
        if ((R_BUTTON | B_BUTTON) & gKeyStatusPtr->newKeys) {
            CloseHelpBox();
            proc->do_help = 0;
            return;
        }
    } else {
        if (R_BUTTON & gKeyStatusPtr->newKeys) {
            if (cmd->msg_rtext) {
                StartHelpBox(visibleX, visibleY, cmd->msg_rtext);
                proc->do_help = 1;
            }
            return;
        }

        if (A_BUTTON & gKeyStatusPtr->newKeys) {
            if ((1 & cmd->color) || (NULL == cmd->effect)) {
                PlaySoundEffect(SONG_6C);
                return;
            } else {
                cmd->effect(proc->proc_parent);
                PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
                return;
            }
        }

        if (B_BUTTON & gKeyStatusPtr->newKeys) {
            if (proc->on_PressB) {
                if (proc->on_PressB(proc->proc_parent)) {
                    PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
                    return;
                } else {
                    PlaySoundEffect(SONG_6C);
                    return;
                }
            }
            return;
        }

        if (START_BUTTON & gKeyStatusPtr->newKeys) {
            if (proc->on_PressStart) {
                if (proc->on_PressStart(proc->proc_parent)) {
                    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
                    Proc_Goto(proc, 0x0);
                    return;
                } else {
                    PlaySoundEffect(SONG_6C);
                    return;
                }
            }
            return;
        }
    }

    if (DPAD_UP & gKeyStatusPtr->repeatedKeys) {
        if (proc->cur_index)
            proc->cur_index = proc->cur_index - 1;
        else if (DPAD_UP & gKeyStatusPtr->newKeys) // Allows looping of cursor when at top
            proc->cur_index = proc->max_index - 1;
    }

    if (DPAD_DOWN & gKeyStatusPtr->repeatedKeys) {
        if (proc->cur_index < (proc->max_index - 1)) // Need this to start moving the UI hand cursor at all
            proc->cur_index = proc->cur_index + 1;
        else if (DPAD_DOWN & gKeyStatusPtr->newKeys) // Allows looping of cursor when at bottom
            proc->cur_index = 0;
    }

    if (index != proc->cur_index) {
        PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);

        if (proc->do_help) {
            StartHelpBox((proc->xPos + 1) * 8 + 4,
                         (proc->yPos + 1) * 8 + proc->cur_index * 16,
                         (cmd = proc->cmds[proc->cur_index])->msg_rtext);
        }
    }

    if (proc->cur_index < firstVisibleIndex) {
        firstVisibleIndex = proc->cur_index;
    }
    if (proc->cur_index >= firstVisibleIndex + PREP_MENU_VISIBLE_COUNT - 1) {
        firstVisibleIndex = proc->cur_index - PREP_MENU_VISIBLE_COUNT + 1;
    }

    /* This is what's causing the additional menu items to persist when switching to the view map screen */
    /* As a stop gap measure it now won't display on the other menu now if it has 4 or less options */
    if (proc->max_index > 4)
        SetPrepScreenMenuPosition(1, 6);

    UpdateMenuScrollBarConfig(
        (u8)proc->max_index,                 // Height of scrollbar (should be set to max)
        (u16)firstVisibleIndex*16,           // currentSegment: which is topmost, every segment is in batches of 16 pixels
        (u16)proc->max_index,                // totalRows: total number of items - should be actual max
        (u8)PREP_MENU_VISIBLE_COUNT          // visibleRows: window size
    );
}

LYN_REPLACE_CHECK(AtMenu_Reinitialize);
void AtMenu_Reinitialize(struct ProcAtMenu* proc)
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
        InitText(&gPrepMainMenuTexts[i + 5], 0xE); // Initialize text space for menu section descriptions
    for (i = 0; i < 4; i++)
        InitText(&gPrepMainMenuTexts[i + 1], 0x8); // NOT Menu section titles
    InitText(&gPrepMainMenuTexts[0], 0xA);

    /* "Preparations" */
    Decompress(gUnknown_08A1A4C8, (void*)0x6014800);
    /* "Menu", "Start" button */
    Decompress(gUnknown_08A1D510, (void*)0x6016000);
    ApplyPalettes(Pal_SysBrownBox, 0x19, 2);
    
    /* Chapter text */
    sub_8095C50(0x7000, 0x6);
    ApplyPalette(gUnknown_08A1D4C8, 0x14);
    EnablePaletteSync();

    gLCDControlBuffer.bg0cnt.priority = 0;
    gLCDControlBuffer.bg1cnt.priority = 2;
    gLCDControlBuffer.bg2cnt.priority = 1;
    gLCDControlBuffer.bg3cnt.priority = 3;

    gLCDControlBuffer.dispcnt.win0_on = 0;
    gLCDControlBuffer.dispcnt.win1_on = 0;
    gLCDControlBuffer.dispcnt.objWin_on  = 0;

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

    switch ((proc->unk_35)) {
        case 0:
            ParsePrepMenuDescTexts(MSG_PREP_SCREEN_DESC_UNITS);
            break;
        case 1:
            ParsePrepMenuDescTexts(MSG_PREP_SCREEN_DESC_ITEMS);
            break;
        case 2:
            ParsePrepMenuDescTexts(MSG_PREP_SCREEN_DESC_SAVE);
            break;
        case 3:
            ParsePrepMenuDescTexts(MSG_PREP_SCREEN_DESC_SUPPORT);
            break;
        case 4:
            if (gpKernelDesignerConfig->prep_menu_augury == true)
                ParsePrepMenuDescTexts(MSG_PREP_SCREEN_DESC_AUGURY);

            break;
        case 5:
            ParsePrepMenuDescTexts(MSG_PREP_SCREEN_DESC_BEXP);
            break;
        case 6:
            ParsePrepMenuDescTexts(MSG_PREP_SCREEN_DESC_START_MAP);
            break;
        case 7:
            ParsePrepMenuDescTexts(MSG_PREP_SCREEN_DESC_SKILLS);
            break;
        case 8:
            ParsePrepMenuDescTexts(MSG_PREP_SCREEN_DESC_BASE_CONVERSATIONS);
            break;
        case 9:
            ParsePrepMenuDescTexts(MSG_PREP_SCREEN_DESC_BASE_CONVERSATIONS);
            break;
        case 13:
            ParsePrepMenuDescTexts(MSG_PREP_SCREEN_DESC_INFUSE);
            break;
        default:
            break;
    }
    
    DrawPrepMenuDescTexts();
}

LYN_REPLACE_CHECK(sub_8095C00);
void sub_8095C00(int msg, ProcPtr parent)
{
    struct ProcPrepMenuDesc * proc;

    proc = Proc_Find(ProcScr_PrepMenuDescHandler);
    if (proc)
        Proc_End(proc);
    
    proc = Proc_Start(ProcScr_PrepMenuDescHandler, parent);

    /* So this is actually what replaces the prep menu descriptions when moving the cursor up and down */
    switch (GetActivePrepMenuItemIndex()) {
        case 0:
            proc->msg = MSG_PREP_SCREEN_DESC_UNITS;
            break;
        case 1:
            proc->msg = MSG_PREP_SCREEN_DESC_ITEMS;
            break;
        case 2:
            proc->msg = MSG_PREP_SCREEN_DESC_SAVE;
            break;
        case 3:
            proc->msg = MSG_PREP_SCREEN_DESC_SUPPORT;
            break;
        case 4:
            if (gpKernelDesignerConfig->prep_menu_augury == true)
                proc->msg = MSG_PREP_SCREEN_DESC_AUGURY;
                
            break;
        case 5:
            proc->msg = MSG_PREP_SCREEN_DESC_BEXP;
            break;
        case 6:
            proc->msg = MSG_PREP_SCREEN_DESC_START_MAP;
            break;
        case 7:
            proc->msg = MSG_PREP_SCREEN_DESC_SKILLS;
            break;
        case 8: 
            proc->msg = MSG_PREP_SCREEN_DESC_CHECK_MAP;
            break;
        case 9:
            proc->msg = MSG_PREP_SCREEN_DESC_BASE_CONVERSATIONS;
            break;
        case 13:
            proc->msg = MSG_PREP_SCREEN_DESC_INFUSE;
            break;
        default:
            proc->msg = msg;
            break;
    }
}

LYN_REPLACE_CHECK(SetPrepScreenMenuPosition);
void SetPrepScreenMenuPosition(int x, int y)
{
    int i;

    struct ProcPrepMenu *proc;

    struct ProcPrepMenuItem *cmd;
    proc = Proc_Find(ProcScr_PrepMenu);

    if (proc) {
        proc->xPos = x;
        proc->yPos = y;

        if (proc->max_index > 1) {
            // Loop through the *visible slots* on the screen (0 to PREP_MENU_VISIBLE_COUNT)
    
            for (i = 0; i < PREP_MENU_VISIBLE_COUNT; i++) {
                // Calculate the actual index of the menu item that should be in this visible slot
                int actualItemIndex = firstVisibleIndex + i;

                if (actualItemIndex < proc->max_index) {
                    cmd = proc->cmds[actualItemIndex]; // Get the menu item data

                    ClearText(&cmd->text); // Clear previous text for this item
                    PutDrawText( // Draw the text for the menu item
                        &cmd->text,
                        TILEMAP_LOCATED(gBG0TilemapBuffer, x + 2, y + 2 * i + 1), // Y position based on visible slot 'i'
                        1 & cmd->color,
                        0, 0,
                        GetStringFromIndex(cmd->msg)
                    );
                }
            }
        }
        BG_EnableSyncByMask(0x1);
    }
}

LYN_REPLACE_CHECK(SetPrepScreenMenuItem);
void SetPrepScreenMenuItem(int index, const void* func, int color, int msg, int msg_rtext)
{
    int i;
    // int max_index; // Not used?
    // struct ProcPrepMenuItem* cmd; // Not used?

    struct ProcPrepMenu *proc;

    proc = Proc_Find(ProcScr_PrepMenu);

    if (proc) {
        for (i = 0; i < proc->max_index; i++) {
            if (!proc->cmds[i])
                continue;

            if (proc->cmds[i]->index == index) {
                proc->cmds[i]->effect = func;
                proc->cmds[i]->color = color;
                proc->cmds[i]->msg = msg;
                proc->cmds[i]->msg_rtext = msg_rtext;
                return;
            }
        }

        i = proc->max_index;

        proc->cmds[i] = Proc_Start(ProcScr_PrepScreenMenuDummyItem, proc);
        proc->cmds[i]->index = index;
    	proc->cmds[i]->effect = func;
    	proc->cmds[i]->color = color;
    	proc->cmds[i]->msg = msg;
    	proc->cmds[i]->msg_rtext = msg_rtext;
        InitText(&proc->cmds[i]->text, 7);
        proc->max_index++;
    }
}

LYN_REPLACE_CHECK(InitPrepScreenMainMenu);
void InitPrepScreenMainMenu(struct ProcAtMenu* proc)
{
    StartPrepScreenMenu(proc);

    if (!CheckInLinkArena()) 
    {
        SetPrepScreenMenuItem(PREP_MAINMENU_UNIT, PrepScreenMenu_OnPickUnits, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_UNITS, 0);
        SetPrepScreenMenuItem(PREP_MAINMENU_ITEM, PrepScreenMenu_OnItems, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_ITEMS, 0);

        if (CanPrepScreenSave())
            SetPrepScreenMenuItem(PREP_MAINMENU_SAVE, PrepScreenMenu_OnSave, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_SAVE, 0);
        else
            SetPrepScreenMenuItem(PREP_MAINMENU_SAVE, PrepScreenMenu_OnSave, TEXT_COLOR_SYSTEM_GRAY, MSG_PREP_SCREEN_TITLE_SAVE, 0);

        SetPrepScreenMenuItem(PREP_MAINMENU_INFUSE, PrepScreenMenu_OnInfuse, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_INFUSE, 0);
        SetPrepScreenMenuItem(PREP_MAINMENU_SUPPORT, PrepScreenMenu_OnSupport, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_SUPPORT, 0);

        if (gpKernelDesignerConfig->prep_menu_augury == true)
        {
            if (gPlaySt.chapterIndex == CHAPTER_L_4)
                SetPrepScreenMenuItem(PREP_MAINMENU_AUGURY, PrepScreenMenu_OnAugury, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_AUGURY, 0);
            else
                SetPrepScreenMenuItem(PREP_MAINMENU_AUGURY, PrepScreenMenu_OnAugury, TEXT_COLOR_SYSTEM_GRAY, MSG_PREP_SCREEN_TITLE_AUGURY, 0);
        }
            
        // SetPrepScreenMenuItem(PREP_MAINMENU_BONUS_EXP, PrepScreenMenu_OnBEXP, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_BEXP, 0);
        SetPrepScreenMenuItem(PREP_MAINMENU_SKILLS, PrepScreenMenu_OnEquip, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_SKILLS, 0);
        SetPrepScreenMenuItem(PREP_MAINMENU_CHECKMAP, PrepScreenMenu_OnCheckMap, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_CHECK_MAP, 0);
        // SetPrepScreenMenuItem(PREP_MAINMENU_BASE_CONVERSATIONS, PrepScreenMenu_OnCheckMap, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_BASE_CONVERSATIONS, 0);
    } 
    else 
    {
        SetPrepScreenMenuItem(PREP_MAINMENU_UNIT, PrepScreenMenu_OnPickUnits, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_UNITS, 0);
        SetPrepScreenMenuItem(PREP_MAINMENU_ITEM, PrepScreenMenu_OnItems, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_ITEMS, 0);
        SetPrepScreenMenuItem(PREP_MAINMENU_UNK3, sub_8095284, TEXT_COLOR_SYSTEM_WHITE, 0x75C, 0);
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