#include "common-chax.h"
#include "prep-skill.h"
#include "kernel-lib.h"
#include "uichapterstatus.h"
#include "jester_headers/procs.h"
#include "constants/texts.h"
#include "uiconfig.h"
#include "prepscreen.h"
#include "uichapterstatus.h"

void PrepScreenMenu_Augury(struct ProcAtMenu* proc) 
{
    proc->state = 5;
    CallSomeSoundMaybe(SONG_BONDS, 0x100, 0x100, 0x20, 0);
    Proc_Goto(proc, 0xA);
}

void PrepScreenMenu_BEXP(struct ProcAtMenu* proc) 
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    proc->state = 6;
    Proc_Goto(proc, 0xA);
}

LYN_REPLACE_CHECK(PrepScreenMenu_OnBPress);
int PrepScreenMenu_OnBPress(struct ProcAtMenu* proc) {

    if (false != CheckInLinkArena()) {
        Proc_Goto(proc, 0x5);
        return true;
    }
    
    if (false == CanPrepScreenCheckMap())
        return false;

    PrepSpecialChar_BlinkButtonB();
    Proc_Goto(proc, 0x5);
    return true;
}


LYN_REPLACE_CHECK(PrepScreenMenu_OnCheckMap);
void PrepScreenMenu_OnCheckMap(struct ProcAtMenu* proc) 
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    proc->state = 7;
    Proc_Goto(proc, 0x5);
}

void PrepScreenMenu_OnEquip(struct ProcAtMenu * proc)
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    proc->state = 8;
    Proc_Goto(proc, 0x8);
};

void PrepScreenMenu_BaseConversations(struct ProcAtMenu* proc) 
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    proc->state = 9;
    Proc_Goto(proc, 0xA);
}

LYN_REPLACE_CHECK(PrepAtMenu_OnInit);
void PrepAtMenu_OnInit(struct ProcAtMenu * proc)
{
    PrepSetLatestCharId(0);
    proc->xDiff = 0;
    *((u16*)&proc->yDiff) = 0;    /* ? */

    if (CheckInLinkArena())
        proc->max_counter = 5;
    else
        proc->max_counter = GetChapterAllyUnitCount();

    proc->unk_30 = 0;
    proc->unk_31 = 0;
    proc->unk_32 = 0;
    proc->state = 0;
    proc->do_help = 0;
    proc->end_prep = 0;
    proc->cur_cmd = 0;
    proc->hand_pos = 0;
}

LYN_REPLACE_CHECK(PrepMenu_OnInit);
#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
void PrepMenu_OnInit(struct ProcPrepMenu_Scroll * proc)
#else
void PrepMenu_OnInit(struct ProcPrepMenu * proc)
#endif
{
    int i;
    for (i = 0; i < proc->max_index; i++)
        proc->cmds[i] = 0;

    proc->cur_index = 0;
    proc->max_index = 0;

    ResetSysHandCursor(proc);
    DisplaySysHandCursorTextShadow(0x600, 1);

    proc->on_PressB = 0;
    proc->on_PressStart = 0;
    proc->on_End = 0;
    proc->do_help = 0;

#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
    proc->firstVisibleIndex = 0;
#endif

    StartMenuScrollBar(proc);
    PutMenuScrollBarAt(3, 60); // x and y
    InitMenuScrollBarImg(0x200, 2); 
}

LYN_REPLACE_CHECK(SetPrepScreenMenuOnBPress);
void SetPrepScreenMenuOnBPress(const void* func)
{
#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
    struct ProcPrepMenu_Scroll *proc;
#else
    struct ProcPrepMenu *proc;
#endif

    proc = Proc_Find(ProcScr_PrepMenu);

    if (proc)
        proc->on_PressB = func;
}

LYN_REPLACE_CHECK(SetPrepScreenMenuOnStartPress);
void SetPrepScreenMenuOnStartPress(const void* func)
{
#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
    struct ProcPrepMenu_Scroll *proc;
#else
    struct ProcPrepMenu *proc;
#endif

    proc = Proc_Find(ProcScr_PrepMenu);

    if (proc)
        proc->on_PressStart = func;
}

extern struct ProcCmd sProc_Menu[];
LYN_REPLACE_CHECK(PrepMenu_CtrlLoop);
#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
void PrepMenu_CtrlLoop(struct ProcPrepMenu_Scroll *proc)
#else
void PrepMenu_CtrlLoop(struct ProcPrepMenu *proc)
#endif
{
    struct ProcPrepMenuItem* cmd;
    int index = proc->cur_index;
    int xPos = (proc->xPos + 1) * 8 + 4;
    int yPos = (proc->yPos + 1) * 8 + proc->cur_index * 16;

#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
    int visibleX = (proc->xPos + 1) * 8 + (4 - proc->firstVisibleIndex);
    int visibleY = (proc->yPos + 1) * 8 + (proc->cur_index - proc->firstVisibleIndex) * 16;
    ShowSysHandCursor(xPos, visibleY, 0x6, 0x400);
    struct MenuProc* proc2 = Proc_Find(sProc_Menu);
    GetMenuCursorPosition(proc2, &xPos, &yPos);
    ApplyMenuCursorVScroll(proc2, &xPos, &yPos);
#else
    ShowSysHandCursor(xPos, yPos, 0x6, 0x400);
#endif

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
#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
                StartHelpBox(visibleX, visibleY, cmd->msg_rtext);
#else
                StartHelpBox(xPos, yPos, cmd->msg_rtext);
#endif
                proc->do_help = 1;
            }
            return;
        }

        if (A_BUTTON & gKeyStatusPtr->newKeys) {
            if ((1 & cmd->color) || (NULL == cmd->effect)) {
                PlaySoundEffect(SONG_6C);
                return;
            } else {
                Proc_Goto(proc, 0x0);
                cmd->effect(proc->proc_parent);
                PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
                return;
            }
        }

        if (B_BUTTON & gKeyStatusPtr->newKeys) {
            if (proc->on_PressB) {
                if (proc->on_PressB(proc->proc_parent)) {
                    Proc_Goto(proc, 0x0);
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
        // else if (DPAD_UP & gKeyStatusPtr->newKeys) // Allows looking of cursor when at top
        //     proc->cur_index = proc->max_index - 1;
    }

    if (DPAD_DOWN & gKeyStatusPtr->repeatedKeys) {
        if (proc->cur_index < (proc->max_index - 1)) // Need this to start moving the UI hand cursor at all
            proc->cur_index = proc->cur_index + 1;
        // else if (DPAD_DOWN & gKeyStatusPtr->newKeys) // Allows looping of cursor when at bottom
        //     proc->cur_index = 0;
    }

    if (index != proc->cur_index) {
        PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);

        if (proc->do_help) {
            StartHelpBox((proc->xPos + 1) * 8 + 4,
                         (proc->yPos + 1) * 8 + proc->cur_index * 16,
                         (cmd = proc->cmds[proc->cur_index])->msg_rtext);
        }
    }

#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
    if (proc->cur_index < proc->firstVisibleIndex) {
        proc->firstVisibleIndex = proc->cur_index;
    }
    if (proc->cur_index >= proc->firstVisibleIndex + PREP_MENU_VISIBLE_COUNT - 1) {
        proc->firstVisibleIndex = proc->cur_index - PREP_MENU_VISIBLE_COUNT + 1;
    }

    /* This is what's causing the additional menu items to persist when siwtching to the view map screen */
    /* As a stop gap measure it now won't display on the other menu now if it has 4 or less options */
    if (proc->max_index > 4)
        SetPrepScreenMenuPosition(1, 6);

    UpdateMenuScrollBarConfig(
        (u8)proc->max_index,                       // Height of scrollbar (should be set to max)
        (u16)proc->firstVisibleIndex*16,           // currentSegment: which is topmost, every segment is in batches of 16 pixels
        (u16)proc->max_index,                      // totalRows: total number of items - should be actual max
        (u8)PREP_MENU_VISIBLE_COUNT                // visibleRows: window size
    );

#endif
}

LYN_REPLACE_CHECK(PrepMenu_ShowFrozenHand);
#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
void PrepMenu_ShowFrozenHand(struct ProcPrepMenu_Scroll *proc)
#else
void PrepMenu_ShowFrozenHand(struct ProcPrepMenu *proc)
#endif
{
    DisplayFrozenUiHand((proc->xPos + 1) * 8 + 4,
                        (proc->yPos + 1) * 8 + proc->cur_index * 16);
}

LYN_REPLACE_CHECK(PrepMenu_ShowActiveHand);
#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
void PrepMenu_ShowActiveHand(struct ProcPrepMenu_Scroll *proc)
#else
void PrepMenu_ShowActiveHand(struct ProcPrepMenu *proc)
#endif
{    
    ShowSysHandCursor((proc->xPos + 1) * 8 + 4,
                             (proc->yPos + 1) * 8 + proc->cur_index * 16,
                             6, 0x400);
}

LYN_REPLACE_CHECK(PrepMenu_OnEnd);
#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
void PrepMenu_OnEnd(struct ProcPrepMenu_Scroll *proc)
#else
void PrepMenu_OnEnd(struct ProcPrepMenu *proc)
#endif
{
    if (proc->on_End)
        proc->on_End(proc->proc_parent);

    EndMenuScrollBar();
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
            proc->msg = MSG_PREP_SCREEN_DESC_BASE_CONVERSATIONS;
            break;
        default:
            break;
    }
}

//! FE8U = 0x0808E3D4
LYN_REPLACE_CHECK(ChapterStatus_ShowAllLayers);
void ChapterStatus_ShowAllLayers(void)
{
    SetDispEnable(1, 1, 1, 1, 1);
    return;
}

LYN_REPLACE_CHECK(DrawPrepScreenMenuFrameAt);
void DrawPrepScreenMenuFrameAt(int x, int y)
{
    int i;

#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
    struct ProcPrepMenu_Scroll *proc;
#else
    struct ProcPrepMenu *proc;
#endif

    struct ProcPrepMenuItem *cmd;
    proc = Proc_Find(ProcScr_PrepMenu);

    if (proc) {
        proc->xPos = x;
        proc->yPos = y;

        DrawUiFrame2(x, y, 0xA, proc->max_index * 2 + 2, 0);

        if (proc->max_index > 1) {
            for (i = 0; i < proc->max_index; i++) {
                cmd = proc->cmds[i];
                ClearText(&cmd->text);
    
                PutDrawText(
    				&cmd->text,
    				TILEMAP_LOCATED(gBG0TilemapBuffer, x + 2, y + 2 * i + 1),
    				1 & cmd->color,
    				0, 0,
    				GetStringFromIndex(cmd->msg) );
            }
        }

        BG_EnableSyncByMask(0x3);
    }
}

LYN_REPLACE_CHECK(SetPrepScreenMenuPosition);
void SetPrepScreenMenuPosition(int x, int y)
{
    int i;

#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
    struct ProcPrepMenu_Scroll *proc;
#else
    struct ProcPrepMenu *proc;
#endif

    struct ProcPrepMenuItem *cmd;
    proc = Proc_Find(ProcScr_PrepMenu);

    if (proc) {
        proc->xPos = x;
        proc->yPos = y;

        if (proc->max_index > 1) {
            // Loop through the *visible slots* on the screen (0 to PREP_MENU_VISIBLE_COUNT)
    
#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
            for (i = 0; i < PREP_MENU_VISIBLE_COUNT; i++) {
                // Calculate the actual index of the menu item that should be in this visible slot
                int actualItemIndex = proc->firstVisibleIndex + i;

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
#else
            for (i = 0; i < proc->max_index; i++) {
                cmd = proc->cmds[i];
                ClearText(&cmd->text);
    
                PutDrawText(
                    &cmd->text,
                    TILEMAP_LOCATED(gBG0TilemapBuffer, x + 2, y + 2 * i + 1),
                    1 & cmd->color,
                    0, 0,
                    GetStringFromIndex(cmd->msg) );
            }
#endif
        }
        BG_EnableSyncByMask(0x1);
    }
}

LYN_REPLACE_CHECK(SetPrepScreenMenuSelectedItem);
void SetPrepScreenMenuSelectedItem(int index)
{
    int i, cur = 0;

#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
    struct ProcPrepMenu_Scroll *proc;
#else
    struct ProcPrepMenu *proc;
#endif

    proc = Proc_Find(ProcScr_PrepMenu);

    if (proc) {
        for (i = 0; i < proc->max_index; i++) {
            if (!proc->cmds[i])
                continue;

            if (proc->cmds[i]->index == index) {
                proc->cur_index = cur;
                return;
            }
            cur++;
        }
    }
}

LYN_REPLACE_CHECK(GetActivePrepMenuItemIndex);
int GetActivePrepMenuItemIndex()
{
    int i, cur = 0;

#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
    struct ProcPrepMenu_Scroll *proc;
#else
    struct ProcPrepMenu *proc;
#endif

    proc = Proc_Find(ProcScr_PrepMenu);

    if (proc) {
        for (i = 0; i < proc->max_index; i++) {
            if (!proc->cmds[i])
                    continue;
    
            if (proc->cur_index == cur) {
                return proc->cmds[i]->index;
            }
            cur++;
        }
    }
    return 0;
}



LYN_REPLACE_CHECK(SetPrepScreenMenuItem);
void SetPrepScreenMenuItem(int index, const void* func, int color, int msg, int msg_rtext)
{
    int i;
    // int max_index; // Not used?
    // struct ProcPrepMenuItem* cmd; // Not used?

#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
    struct ProcPrepMenu_Scroll *proc;
#else
    struct ProcPrepMenu *proc;
#endif

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
    
        SetPrepScreenMenuItem(PREP_MAINMENU_SUPPORT, PrepScreenMenu_OnSupport, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_SUPPORT, 0);
#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
        SetPrepScreenMenuItem(PREP_MAINMENU_AUGURY, PrepScreenMenu_Augury, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_AUGURY, 0);
        // SetPrepScreenMenuItem(PREP_MAINMENU_BONUS_EXP, PrepScreenMenu_BEXP, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_BEXP, 0);
        SetPrepScreenMenuItem(PREP_MAINMENU_SKILLS, PrepScreenMenu_OnEquip, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_SKILLS, 0);
#endif
        SetPrepScreenMenuItem(PREP_MAINMENU_CHECKMAP, PrepScreenMenu_OnCheckMap, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_CHECK_MAP, 0);
        //SetPrepScreenMenuItem(PREP_MAINMENU_BASE_CONVERSATIONS, PrepScreenMenu_BaseConversations, TEXT_COLOR_SYSTEM_WHITE, MSG_PREP_SCREEN_TITLE_BASE_CONVERSATIONS, 0);
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

#ifdef CONFIG_PREPS_INFUSE
    case 4: /* Infuse */
        StartFortuneSubMenu(2, proc);
        break;
#else
    case 4: /* Support */
        StartFortuneSubMenu(2, proc);
        break;
#endif

#ifdef CONFIG_PREPS_AUGURY
    case 5: /* Augury */
        Proc_StartBlocking(PREEXT_Procs_Augury, proc);
        break;
#endif

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
        // StartChapterStatusScreen_FromPrep(proc);
        break;

    default:
        break;
    }

    Proc_Break(proc);
};


struct ProcCmd const ProcScr_ChapterStatusScreen_FromPrep_NEW[] =
{
    PROC_YIELD,

    PROC_CALL(ChapterStatus_Init),
    PROC_CALL(ChapterStatus_DrawText),
    PROC_YIELD,

    PROC_CALL(ChapterStatus_ShowAllLayers),
    PROC_CALL(FadeInBlackSpeed40),
    PROC_YIELD,

PROC_LABEL(0),
    PROC_REPEAT(ChapterStatus_LoopKeyHandler),

PROC_LABEL(1),
    PROC_CALL(sub_8013F58),
    PROC_YIELD,

    PROC_CALL(EndMuralBackground),
    PROC_CALL(ChapterStatus_OnEnd),

    PROC_END,
};

//! FE8U = 0x0808E79C
LYN_REPLACE_CHECK(StartChapterStatusScreen_FromPrep);
void StartChapterStatusScreen_FromPrep(ProcPtr parent)
{
    struct ChapterStatusProc * proc = Proc_StartBlocking(ProcScr_ChapterStatusScreen_FromPrep_NEW, parent);
    proc->unk_3f = 1;

    return;
};

//! FE8U = 0x08033548
LYN_REPLACE_CHECK(PrepHelpPrompt_Init);
void PrepHelpPrompt_Init(struct ProcPrepSallyCursor * proc)
{
    StartHelpPromptSprite(170, 140, 2, proc);
    Decompress(Img_PrepHelpButtonSprites, (void *)(OBJ_VRAM1 + 0x3000));
    proc->unk_58 = 0;
    return;
}

//! FE8U = 0x08033574
LYN_REPLACE_CHECK(PrepHelpPrompt_Loop);
void PrepHelpPrompt_Loop(void)
{
    PutSprite(4, 100, 140, gObject_32x16, OAM2_CHR(0x38B) + OAM2_PAL(2));
    PutSprite(4, 132, 140, gObject_32x16, OAM2_CHR(0x38F) + OAM2_PAL(2));
    PutSprite(4, 164, 140, gObject_16x16, OAM2_CHR(0x393) + OAM2_PAL(2));
    PutSprite(4, 16, 140, gObject_32x16, OAM2_CHR(0x395) + OAM2_PAL(2));
    PutSprite(4, 48, 140, gObject_32x16, OAM2_CHR(0x399) + OAM2_PAL(2));
    PutSprite(4, 80, 140, gObject_8x16, OAM2_CHR(0x39D) + OAM2_PAL(2));
}

//! FE8U = 0x08033648
LYN_REPLACE_CHECK(PrepScreenProc_StartMapMenu);
void PrepScreenProc_StartMapMenu(struct ProcPrepSallyCursor * proc)
{
    LoadHelpBoxGfx(0, -1);
    ResetText();
    EndPlayerPhaseSideWindows();
    HideMoveRangeGraphics();

    StartPrepScreenMenu(proc);

    SetPrepScreenMenuItem(1, PrepMapMenu_OnViewMap, TEXT_COLOR_SYSTEM_WHITE, 0x590, 0x5BB);

    SetPrepScreenMenuItem(
        2, PrepMapMenu_OnFormation, (PrepGetDeployedUnitAmt() ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY),
        0x591, 0x5BC);

    SetPrepScreenMenuItem(8, PrepMapMenu_OnOptions, TEXT_COLOR_SYSTEM_WHITE, 0x592, 0x5BD);

    if (CanPrepScreenSave())
    {
        SetPrepScreenMenuItem(9, PrepMapMenu_OnSave, TEXT_COLOR_SYSTEM_WHITE, 0x579, 0x5BE);
    }
    else
    {
        SetPrepScreenMenuItem(9, PrepMapMenu_OnSave, TEXT_COLOR_SYSTEM_GRAY, 0x579, 0x5BE);
    }

    /* JESTER - 
    ** This turns off the other two button icons in the prep screen
    ** This is to combat a problem where leftover graphics from this screeb
    ** display in the prep menu and on the map.
    */ 
    // StartPrepHelpPrompt(proc);
    SetPrepScreenMenuOnBPress(PrepMapMenu_OnBPress);
    SetPrepScreenMenuOnStartPress(PrepMapMenu_OnStartPress);
    SetPrepScreenMenuOnEnd(PrepMapMenu_OnEnd);
    DrawPrepScreenMenuFrameAt(10, 2);

    SetPrepScreenMenuSelectedItem(proc->unk_58);
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

    return;
}

//! FE8U = 0x080333A4
LYN_REPLACE_CHECK(PrepMapMenu_OnStartPress);
bool PrepMapMenu_OnStartPress(ProcPtr proc)
{
    if (PrepGetDeployedUnitAmt() == 0)
    {
        return false;
    }

    Proc_Goto(proc, 55);

    return true;
}

//! FE8U = 0x080333C4
LYN_REPLACE_CHECK(PrepMapMenu_OnBPress);
bool PrepMapMenu_OnBPress(ProcPtr proc)
{
    Proc_Goto(proc, 51);
    return true;
}

LYN_REPLACE_CHECK(SetPrepScreenMenuOnEnd);
void SetPrepScreenMenuOnEnd(const void * func)
{
    
#ifdef CONFIG_EXPANDED_PREP_MENU_OPTIONS
    struct ProcPrepMenu_Scroll *proc;
#else
    struct ProcPrepMenu *proc;
#endif

    proc = Proc_Find(ProcScr_PrepMenu);

    if (proc)
        proc->on_End = func;
}