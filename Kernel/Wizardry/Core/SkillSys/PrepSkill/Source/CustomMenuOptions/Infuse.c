#include "common-chax.h"
#include "utf8.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"

/*
** I've haven't gotten too far with this, I've managed to get a cutom graphic to display
** and a custom value alongside it.
**
** This is my shopping list:
** - Add Dragon Stone Shard resource
** - Add Resource and Infused item boxes
** - Add dragon stone shard graphic
** - Add long black background and text
** - Ensure graphics update as I move up and down the target list
** - When I click A on a resource one of two things should happen:
**   1) I have enough dragon stone shards and a new item is generated
**   2) I do not have enough dragon stone shards and a popup appears telling me as such
**
*/

// A simpler struct for the mapping
struct InfuseRecipe {
    u8 targetItemId;
    u8 cost;
};

// Allocate a table for all 256 possible items
// Initializing with [256] ensures O(1) access via itemId index
const struct InfuseRecipe gInfusionLookupTable[256] = {
    [ITEM_SWORD_IRON]   = { ITEM_SWORD_SLIM, 2 },
    [ITEM_SWORD_RAPIER] = { ITEM_SWORD_POISON, 4 },
    // All other entries will default to {0, 0}
};

void displayScrollBackground_INFUSE(void)
{
    SetTextFont(NULL);
    TileMap_FillRect(gBG0TilemapBuffer + 0x34, 12, 1, 0);

    InitText(&PrepItemSuppyTexts.th[0], 0xA);
    InitText(&PrepItemSuppyTexts.th[2], 0xA);
    InitText(&PrepItemSuppyTexts.th[3], 0xA);

    PutDrawText(&PrepItemSuppyTexts.th[0], TILEMAP_LOCATED(gBG0TilemapBuffer, 6, 2), 0, 2, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_SELECT_WEAPON)));
    PutFaceChibi(FID_SUPPLY + 1, TILEMAP_LOCATED(gBG0TilemapBuffer, 1, 1), 0x270, 2, 0);

    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 3, 5), TEXT_COLOR_SYSTEM_WHITE, 100);

    /* Decompress graphics for down arrow */
    Decompress(Gfx_Down_Arrow, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x6014B20, 2, 4);

    /* Draw dragon egg icon */
    DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, 6, 13), GetItemIconId(0xAA), 0x4000);

    BG_EnableSyncByMask(BG0_SYNC_BIT);

    return;
}

void PrepItemList_DrawCurrentOwnerText_INFUSE(struct PrepItemListProc* proc) {
    int idx = proc->idxPerPage[proc->currentPage];

    TileMap_FillRect(gBG0TilemapBuffer + 0x38, 8, 1, 0);

    ClearText(PrepItemSuppyTexts.th + 1);

    if (gUnknown_02012F56 <= idx) {
        PutDrawText(PrepItemSuppyTexts.th + 1, gBG0TilemapBuffer + 0x38, 1, 0, 0, GetStringFromIndex(0x536));
    } else {
        int pid = gPrepScreenItemList[proc->idxPerPage[proc->currentPage]].pid;

        if (pid == 0) {
            PutDrawText(PrepItemSuppyTexts.th + 1, gBG0TilemapBuffer + 0x38, 3, 0, 0, GetStringFromIndex(0x598)); // TODO: msgid "Supply"
        } else {
            PutDrawText(PrepItemSuppyTexts.th + 1, gBG0TilemapBuffer + 0x38, 0, 0, 0, GetStringFromIndex(GetUnitFromCharId(pid)->pCharacterData->nameTextId));
        }
    }

    BG_EnableSyncByMask(1);

    return;
}

void PrepItemList_InitGfx_INFUSE(struct PrepItemListProc * proc)
{
    int i;

    gLCDControlBuffer.dispcnt.mode = 0;

    SetupBackgrounds(NULL);

    BG_Fill(BG_GetMapBuffer(0), 0);
    BG_Fill(BG_GetMapBuffer(1), 0);
    BG_Fill(BG_GetMapBuffer(2), 0);

    gLCDControlBuffer.bg0cnt.priority = 1;
    gLCDControlBuffer.bg1cnt.priority = 2;
    gLCDControlBuffer.bg2cnt.priority = 0;
    gLCDControlBuffer.bg3cnt.priority = 3;

    ResetFaces();
    ResetText();
    ResetIconGraphics_();
    LoadUiFrameGraphics();
    LoadObjUIGfx();

    BG_SetPosition(0, 0, 0);
    BG_SetPosition(1, 0, 0);
    BG_SetPosition(2, 0, proc->yOffsetPerPage[proc->currentPage] - 40);

    LoadHelpBoxGfx((void*)0x06012000, -1);
    LoadIconPalettes(4);

    RestartMuralBackground();

    PutImg_PrepItemUseUnk(0x5000, 5);

    /* Load Unit's 5 item menu and convoy menu together */
    Decompress(gUnknown_08A1B9EC, gGenericBuffer);
    CallARM_FillTileRect(gBG1TilemapBuffer, gGenericBuffer, 0x1000);

    /* This is used to fill in the area used by the first set of TSA above */
    TileMap_FillRect(gBG1TilemapBuffer + (0x8 * 32), 14, 12, 0);

    /* Now we can start drawing additional TSAs */
    // CallARM_FillTileRect(TILEMAP_LOCATED(gBG1TilemapBuffer, 1, 0x9), gTSA_GoalBox_OneLine,  0x1000);
    // CallARM_FillTileRect(TILEMAP_LOCATED(gBG1TilemapBuffer, 1, 0x10), gTSA_GoalBox_OneLine,  0x1000);
    DrawUiFrame(gBG1TilemapBuffer, 1, 8, 12, 3, 0, 1);
    DrawUiFrame(gBG1TilemapBuffer, 1, 16, 12, 3, 0, 1);

    /* Load top left scroll container */
    Decompress(gUnknown_08A1BCC0, gGenericBuffer);
    CallARM_FillTileRect(gBG1TilemapBuffer, gGenericBuffer, 0x1000);

    BG_EnableSyncByMask(7);
    StartUiCursorHand(proc);
    ResetSysHandCursor(proc);
    DisplaySysHandCursorTextShadow(0x600, 1);

    gLCDControlBuffer.dispcnt.win0_on = 1;
    gLCDControlBuffer.dispcnt.win1_on = 0;
    gLCDControlBuffer.dispcnt.objWin_on = 0;

    gLCDControlBuffer.win0_left = 128;
    gLCDControlBuffer.win0_top = 40;
    gLCDControlBuffer.win0_right = 224;
    gLCDControlBuffer.win0_bottom = 152;

    gLCDControlBuffer.wincnt.win0_enableBg0 = 1;
    gLCDControlBuffer.wincnt.win0_enableBg1 = 1;
    gLCDControlBuffer.wincnt.win0_enableBg2 = 1;
    gLCDControlBuffer.wincnt.win0_enableBg3 = 1;
    gLCDControlBuffer.wincnt.win0_enableObj = 1;

    gLCDControlBuffer.wincnt.wout_enableBg0 = 1;
    gLCDControlBuffer.wincnt.wout_enableBg1 = 1;
    gLCDControlBuffer.wincnt.wout_enableBg2 = 0;
    gLCDControlBuffer.wincnt.wout_enableBg3 = 1;
    gLCDControlBuffer.wincnt.wout_enableObj = 1;

    SetBlendConfig(0, 0, 0, 8);
    StartGreenText(proc);
    StartHelpPromptSprite(195, 147, 9, proc);

    InitText(PrepItemSuppyTexts.th + 0, 6);
    InitText(PrepItemSuppyTexts.th + 1, 5);

    InitText(PrepItemSuppyTexts.th + 15, 4);

    for (i = 0; i < 8; i++) {
        InitTextDb(PrepItemSuppyTexts.th + 7 + i, 7);
    }

    /* Display the transparent black banner behind the text */
    SetPrimaryHBlankHandler(NULL);
    SetPrimaryHBlankHandler(PrepItemSupply_OnHBlank);

    /* Display weapon type graphics at top of convoy on the right */
    StoreConvoyWeaponIconGraphics(0x4000, 6);

    /* Display weapon tiles */
    sub_809D8D4(gBG0TilemapBuffer + 0x6F, 0x4000, 6);

    Decompress(gUnknown_08A19CCC, (void*)0x06015000);
    ApplyPalette(Pal_SpinningArrow, 0x14);

    StartMenuScrollBarExt(proc, 225, 47, 0x5800, 9);
    sub_8097668();
    SomethingPrepListRelated(proc->unit, proc->currentPage, 3);
    sub_809F5F4(proc);

    /* Display weapon texts in prep item list */
    sub_809D300(
        PrepItemSuppyTexts.th + 7,
        gBG2TilemapBuffer + 0xF,
        (proc->yOffsetPerPage[proc->currentPage]) >> 4,
        proc->unit
    );

    BG_EnableSyncByMask(4);

    StartUiSpinningArrows(proc);
    LoadUiSpinningArrowGfx(0, 0x280, 2);
    SetUiSpinningArrowPositions(0x78, 0x18, 0xe9, 0x18);
    SetUiSpinningArrowConfig(3);

    StartParallelWorker(List_PutHighlightedCategorySprites, proc);
    
    StartSysBrownBox(0xd, 0xe00, 0xf, 0xc00, 0x400, proc);

    EnableSysBrownBox(1, 0x98, 6, 2);

    SetBlendConfig(1, 0xe, 4, 0);
    SetBlendTargetA(0, 0, 0, 0, 0);
    SetBlendTargetB(0, 0, 0, 1, 0);

    /* Display infuse graphic */
    displayScrollBackground_INFUSE();

    /* Displays "Owner" text in top right brown box */
    sub_809EBF0();
    PrepItemList_DrawCurrentOwnerText_INFUSE(proc);

    return;
}

void PrepItemList_Loop_MainKeyHandler_INFUSE(struct PrepItemListProc * proc)
{
    int idx = proc->idxPerPage[proc->currentPage];

    /* Display down arrow */
    PutSprite(1, 32, 92, gObject_16x32, TILEREF(0x259, 0x0)); 

    // /* Draw item we're currently selecting in the supply */
    // DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 8), GetItemIconId(gPrepScreenItemList[proc->idxPerPage[proc->currentPage]].item), 0x4000);
    // PutDrawText(&PrepItemSuppyTexts.th[2], TILEMAP_LOCATED(gBG0TilemapBuffer, 4, 8), 0, 2, 0, GetItemName(gPrepScreenItemList[proc->idxPerPage[proc->currentPage]].item));

    // /* Draw its fuse result */
    // DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 17), GetItemIconId(gInfusionLookupTable[gPrepScreenItemList[proc->idxPerPage[proc->currentPage]].item].targetItemId), 0x4000);
    // PutDrawText(&PrepItemSuppyTexts.th[3], TILEMAP_LOCATED(gBG0TilemapBuffer, 4, 17), 0, 2, 0, GetItemName(gInfusionLookupTable[gPrepScreenItemList[proc->idxPerPage[proc->currentPage]].item].targetItemId));

    int lastIdx = -1; // Track the previous item

    // Only redraw the infusion info if the cursor moved
    if (idx != lastIdx) {
        u16 item = gPrepScreenItemList[idx].item;
        u8 itemId = ITEM_INDEX(item);

        /* Draw item we're currently selecting */
        DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 8), GetItemIconId(item), 0x4000);

        /* Draw the fuse item cost */
        PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 8, 13), TEXT_COLOR_SYSTEM_WHITE, gInfusionLookupTable[itemId].cost);
        
        ClearText(&PrepItemSuppyTexts.th[2]); // Clear old name
        PutDrawText(&PrepItemSuppyTexts.th[2], TILEMAP_LOCATED(gBG0TilemapBuffer, 4, 8), 0, 2, 0, GetItemName(item));

        /* Draw its fuse result from O(1) table */
        u8 target = gInfusionLookupTable[itemId].targetItemId;
        if (target != 0) {
            DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 17), GetItemIconId(target), 0x4000);
            
            ClearText(&PrepItemSuppyTexts.th[3]);
            PutDrawText(&PrepItemSuppyTexts.th[3], TILEMAP_LOCATED(gBG0TilemapBuffer, 4, 17), 0, 2, 0, GetItemName(target));
        }
        
        BG_EnableSyncByMask(BG0_SYNC_BIT);
        lastIdx = idx;
    }

    if ((proc->yOffsetPerPage[proc->currentPage] & 0xf) == 0) {
        if ((proc->unk_36 == 0) || (proc->unk_36 == 0xff)) {
            if (gKeyStatusPtr->newKeys & R_BUTTON) {
                if (gUnknown_02012F56 == 0) {
                    PlaySoundEffect(SONG_6C);
                    return;
                } else {
                    int item = gPrepScreenItemList[proc->idxPerPage[proc->currentPage]].item;
                    StartItemHelpBox(
                        0x80,
                        proc->idxPerPage[proc->currentPage] * 16 + 40 - proc->yOffsetPerPage[proc->currentPage],
                        item
                    );
                    proc->unk_36 = 1;
                    return;
                }
            }

            if (gKeyStatusPtr->newKeys & A_BUTTON) {
                if (gUnknown_02012F56 == 0) {
                    PlaySoundEffect(SONG_6C);
                    return;
                }

                if (gPrepScreenItemList[idx].pid == 0) {
                    SetUiCursorHandConfig(
                        0,
                        0x80,
                        proc->idxPerPage[proc->currentPage] * 16 + 40 - proc->yOffsetPerPage[proc->currentPage],
                        2
                    );
                    Proc_Goto(proc, 7);
                    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
                    return;
                } else {
                    Proc_Goto(proc, 6);
                    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
                    return;
                }
            }

            if (gKeyStatusPtr->newKeys & B_BUTTON) {
                SetPrimaryHBlankHandler(NULL);
                Proc_Goto(proc, 9);
                // Proc_Break(proc);
                StartPrepAtMenuWithConfig();
                PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
                proc->unk_36 = 0;
                return;
            }
        } else {
            if (gKeyStatusPtr->newKeys & (R_BUTTON | B_BUTTON)) {
                CloseHelpBox();
                proc->unk_36 = 0;
                return;
            }
        }

        if (gKeyStatusPtr->repeatedKeys & DPAD_LEFT) {
            // SetUiSpinningArrowFastMaybe(0);
            // PlaySoundEffect(SONG_SE_SYS_CURSOR_LR1);
            // Proc_Goto(proc, 3);
            // proc->unk_32 = 0;
            // PrepItemList_SwitchPageLeft(proc);
            return;
        }

        if (gKeyStatusPtr->repeatedKeys & DPAD_RIGHT) {
            // SetUiSpinningArrowFastMaybe(1);
            // PlaySoundEffect(SONG_SE_SYS_CURSOR_LR1);
            // Proc_Goto(proc, 4);
            // proc->unk_32 = 0;
            // PrepItemList_SwitchPageRight(proc);
            return;
        }

        if (gKeyStatusPtr->heldKeys & L_BUTTON) {
            proc->scrollAmount = 8;
        } else {
            proc->scrollAmount = 4;
        }

        if ((gKeyStatusPtr->repeatedKeys & DPAD_UP) ||
            ((gKeyStatusPtr->heldKeys & DPAD_UP) && (proc->scrollAmount == 8))) {
            if (proc->idxPerPage[proc->currentPage] != 0) {
                proc->idxPerPage[proc->currentPage]--;
            }
            ClearText(&PrepItemSuppyTexts.th[3]);
            
        }

        if ((gKeyStatusPtr->repeatedKeys & DPAD_DOWN) ||
            ((gKeyStatusPtr->heldKeys & DPAD_DOWN) && (proc->scrollAmount == 8))) {
            if (proc->idxPerPage[proc->currentPage] < gUnknown_02012F56 - 1) {
                proc->idxPerPage[proc->currentPage]++;
            }
            ClearText(&PrepItemSuppyTexts.th[3]);
        }
    } else {
        if ((proc->idxPerPage[proc->currentPage] * 16 + 40 - proc->yOffsetPerPage[proc->currentPage]) < 0x38) {
            proc->yOffsetPerPage[proc->currentPage] -= proc->scrollAmount;
        }

        if ((proc->idxPerPage[proc->currentPage] * 16 + 40 - proc->yOffsetPerPage[proc->currentPage]) > 0x78) {
            proc->yOffsetPerPage[proc->currentPage] += proc->scrollAmount;
        }

        BG_SetPosition(2, 0, proc->yOffsetPerPage[proc->currentPage] - 40);
    }

    if (idx != proc->idxPerPage[proc->currentPage]) {
        u16 item = gPrepScreenItemList[proc->idxPerPage[proc->currentPage]].item;
        PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);

        if (gPrepScreenItemList[proc->idxPerPage[proc->currentPage]].pid != gPrepScreenItemList[idx].pid) {
            PrepItemList_DrawCurrentOwnerText_INFUSE(proc);
        }

        if ((proc->idxPerPage[proc->currentPage] * 16 + 40 - proc->yOffsetPerPage[proc->currentPage] < 0x38) && (proc->idxPerPage[proc->currentPage] != 0)) {
            if (proc->unk_36 != 0) {
                StartItemHelpBox(
                    0x80,
                    proc->idxPerPage[proc->currentPage] * 16 + 40 - proc->yOffsetPerPage[proc->currentPage] + 16,
                    item
                );
            }

            PrepItemList_ScrollVertical(proc, -proc->scrollAmount);
        } else {
            if ((proc->idxPerPage[proc->currentPage] * 16 + 40 - proc->yOffsetPerPage[proc->currentPage] > 0x78)
                && (proc->idxPerPage[proc->currentPage] != gUnknown_02012F56 - 1)) {

                if (proc->unk_36 != 0) {
                    StartItemHelpBox(
                        0x80,
                        proc->idxPerPage[proc->currentPage] * 16 + 40 - proc->yOffsetPerPage[proc->currentPage] - 0x10,
                        item
                    );
                }
                PrepItemList_ScrollVertical(proc, +proc->scrollAmount);
            } else {
                if (proc->unk_36 != 0) {
                    StartItemHelpBox(
                        0x80,
                        proc->idxPerPage[proc->currentPage] * 16 + 40 - proc->yOffsetPerPage[proc->currentPage],
                        item
                    );
                }

                ShowSysHandCursor(
                    0x80,
                    proc->idxPerPage[proc->currentPage] * 16 + 40 - proc->yOffsetPerPage[proc->currentPage],
                    0xb,
                    0x800
                );
            }
        }
    }

    return;
}

struct ProcCmd const ProcScr_PrepItemListScreen_INFUSE[] = {
    PROC_SLEEP(0),
    PROC_CALL(PrepItemList_Init),

PROC_LABEL(0),
    PROC_CALL(PrepItemList_InitGfx_INFUSE),

    PROC_CALL_ARG(NewFadeIn, 16),
    PROC_WHILE(FadeInExists),

    // fallthrough

PROC_LABEL(1),
    PROC_CALL(sub_809F5F4),

    // fallthrough

PROC_LABEL(2),
    PROC_REPEAT(PrepItemList_Loop_MainKeyHandler_INFUSE),

    // fallthrough

PROC_LABEL(6),
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),

    PROC_CALL(PrepItemList_OnEnd),
    // PROC_CALL(PrepItemList_StartTradeScreen),
    PROC_SLEEP(0),

    PROC_GOTO(0),

PROC_LABEL(7),
    PROC_CALL(PrepItemList_SwitchToUnitInventory),
    PROC_REPEAT(PrepItemList_Loop_UnitInvKeyHandler),

    PROC_GOTO(1),

PROC_LABEL(3),
    PROC_REPEAT(PrepItemList_SwitchPageLeft),

    // fallthrough

PROC_LABEL(4),
    PROC_REPEAT(PrepItemList_SwitchPageRight),

    // fallthrough

PROC_LABEL(8),
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),

    // fallthrough

PROC_LABEL(9),
    PROC_CALL(PrepItemList_OnEnd),

    PROC_END,
};