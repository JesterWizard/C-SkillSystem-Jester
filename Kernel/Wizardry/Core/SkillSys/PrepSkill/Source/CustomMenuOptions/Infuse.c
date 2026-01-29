#include "common-chax.h"
#include "utf8.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "popup.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-structs.h"

/*
** This is my shopping list:
** - Add dragon stone shard graphic
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
    [ITEM_SWORD_IRON]   = { ITEM_LIGHT_PURGE, 2 },
    [ITEM_SWORD_RAPIER] = { ITEM_ANIMA_THUNDER, 4 },
    [ITEM_VULNERARY] = {ITEM_ELIXIR, 3}
};

struct PopupInstruction const InfusedPopup[] = {
    POPUP_SOUND(SONG_SE_UPDATE),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_SPACE(3),
    POPUP_MSG(MSG_INFUSED),
    POPUP_SPACE(3),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
    POPUP_ITEM_STR,
    POPUP_SPACE(1),
    POPUP_ITEM_ICON,
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_SPACE(1),
    POPUP_MSG(0x022),                   /* .[.] */
    POPUP_END
};

/* Helper function */
void drawInfuseSprites(void)
{
    /* Display down arrow */
    PutSprite(1, 40, 96, gObject_16x32,  OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x259));

    /* UI Line 1 - parts 1, 2, 3 */
    PutSprite(1, 14, 69, gObject_32x32,  OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x2E0));
    PutSprite(1, 46, 69, gObject_32x32,  OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x2E4));
    PutSprite(1, 78, 69, gObject_32x32,  OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x2E8));
    
    /* UI Line 2 - parts 1, 2, 3 */
    PutSprite(1, 14, 132, gObject_32x32, OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x2E0));
    PutSprite(1, 46, 132, gObject_32x32, OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x2E4));
    PutSprite(1, 78, 132, gObject_32x32, OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x2E8));
}

void InfuseSpriteWorker(ProcPtr proc) {
    drawInfuseSprites();
}

void displayScrollBackground_INFUSE(void)
{
    SetTextFont(NULL);
    TileMap_FillRect(gBG0TilemapBuffer + 0x34, 12, 1, 0);

    InitText(&PrepItemSuppyTexts.th[0], 0xA);
    InitText(&PrepItemSuppyTexts.th[2], 0xA);
    InitText(&PrepItemSuppyTexts.th[3], 0xC);
    InitText(&PrepItemSuppyTexts.th[4], 0x4);
    InitText(&PrepItemSuppyTexts.th[5], 0x4);

    PutDrawText(&PrepItemSuppyTexts.th[0], TILEMAP_LOCATED(gBG0TilemapBuffer, 6, 2), 0, 2, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_SELECT_WEAPON)));
    PutFaceChibi(FID_SUPPLY + 1, TILEMAP_LOCATED(gBG0TilemapBuffer, 1, 1), 0x270, 2, 0);

    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 3, 5), TEXT_COLOR_SYSTEM_WHITE, gInfuseMenuArray[0]);

    /* Decompress graphics for down arrow */
    Decompress(Gfx_Down_Arrow, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x6014B20, 2, 4);

    Decompress(Gfx_UI_Frame_One_Line_1, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x6015C00, 4, 4);
    Decompress(Gfx_UI_Frame_One_Line_2, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x6015C80, 4, 4);
    Decompress(Gfx_UI_Frame_One_Line_3, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x6015D00, 4, 4);

    /* Draw dragon egg icon */
    DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 13), GetItemIconId(0xAA), 0x4000);
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

    gInfuseMenuArray[0] = 7;

    gLCDControlBuffer.dispcnt.mode = 0;

    SetupBackgrounds(NULL);

    BG_Fill(BG_GetMapBuffer(0), 0);
    BG_Fill(BG_GetMapBuffer(1), 0);
    BG_Fill(BG_GetMapBuffer(2), 0);

    gLCDControlBuffer.bg0cnt.priority = 0;
    gLCDControlBuffer.bg1cnt.priority = 2;
    gLCDControlBuffer.bg2cnt.priority = 1;
    gLCDControlBuffer.bg3cnt.priority = 3;

    ResetFaces();
    ResetText();
    ResetIconGraphics_();
    LoadUiFrameGraphics();
    LoadObjUIGfx();

    InitSpriteTextFont(&PrepItemSuppyTexts.font, (void*)0x06011000, 0xb);
    ApplyPalette(Pal_Text, 0x1B);
    InitSpriteText(&PrepItemSuppyTexts.th[0xf]);
    SetTextFont(&PrepItemSuppyTexts.font);
    SetTextFontGlyphs(0);
    SpriteText_DrawBackgroundExt(&PrepItemSuppyTexts.th[0xf], 0);
    Text_InsertDrawString(&PrepItemSuppyTexts.th[0xf], 0, TEXT_COLOR_SYSTEM_WHITE, "Yes");
    Text_InsertDrawString(&PrepItemSuppyTexts.th[0xf], 0x40, TEXT_COLOR_SYSTEM_WHITE, "No");

    BG_SetPosition(0, 0, 0);
    BG_SetPosition(1, 0, 0);
    BG_SetPosition(2, 0, proc->yOffsetPerPage[proc->currentPage] - 40);

    LoadHelpBoxGfx((void*)0x06012000, -1);
    LoadIconPalettes(4);

    RestartMuralBackground();

    PutImg_PrepItemUseUnk(0x5000, 5);
    PutImg_PrepPopupWindow(0x800, 10);

    /* Load Unit's 5 item menu and convoy menu together */
    Decompress(gUnknown_08A1B9EC, gGenericBuffer);
    CallARM_FillTileRect(gBG1TilemapBuffer, gGenericBuffer, 0x1000);

    /* This is used to fill in the area used by the first set of TSA above */
    TileMap_FillRect(gBG1TilemapBuffer + (0x8 * 32), 14, 12, 0);

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
    StartParallelWorker(InfuseSpriteWorker, proc);

    return;
}

/* Redraw the current owner name when switching left and right in the supply without overwriting part of the minimug graphic in the top left */
void sub_809F150_INFUSE(struct PrepItemListProc * proc)
{
    ResetIconGraphics_();

    SomethingPrepListRelated(proc->unit, proc->currentPage, 3);
    sub_809F370(proc);

    sub_809D300(PrepItemSuppyTexts.th + 7, gBG2TilemapBuffer + 0xF, proc->yOffsetPerPage[proc->currentPage] >> 4, proc->unit);

    BG_EnableSyncByMask(5);

    StartParallelFiniteLoop(PrepItemList_DrawCurrentOwnerText_INFUSE, 2, proc);

    /* Draw dragon egg icon */
    DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 13), GetItemIconId(0xAA), 0x4000);

    if (proc->unk_36 == 0) {
        return;
    }

    if (gUnknown_02012F56 != 0) {
        int item = gPrepScreenItemList[proc->idxPerPage[proc->currentPage]].item;
        StartItemHelpBox(
            0x80,
            proc->idxPerPage[proc->currentPage] * 16 + 40 - proc->yOffsetPerPage[proc->currentPage],
            item
        );
        proc->unk_36 = 1;
    } else {
        CloseHelpBox();
        proc->unk_36 = 0xff;
    }

    return;
}

//! FE8U = 0x0809F218
void PrepItemList_SwitchPageLeft_INFUSE(struct PrepItemListProc * proc)
{

    int x = 0;

    int four = 4;

    proc->unk_32++;

    if (proc->unk_32 < four) {
        int tmp = (((4 - proc->unk_32) * 0x60 * (4 - proc->unk_32)) / (four * four));
        x = tmp - 0x60;
    }

    if (proc->unk_32 == four) {
        if (proc->currentPage == 0) {
            proc->currentPage = 8;
        } else {
            proc->currentPage--;
        }
        sub_809F150_INFUSE(proc);
    }

    if (proc->unk_32 >= four) {
        int tmp = four - (proc->unk_32 - four);
        x = (tmp * 0x60 * tmp) / (four * four);
    }

    BG_SetPosition(2, (x & 0xff), proc->yOffsetPerPage[proc->currentPage] - 40);

    if (proc->unk_32 == four * 2) {
        Proc_Goto(proc, 1);
    }

    return;
}

//! FE8U = 0x0809F2C4
void PrepItemList_SwitchPageRight_INFUSE(struct PrepItemListProc* proc) {

    int x = 0;

    int four = 4;

    proc->unk_32++;

    if (proc->unk_32 < four) {
        int tmp = (((4 - proc->unk_32) * 0x60 * (4 - proc->unk_32)) / (four * four));
        x = 0x60 - tmp;
    }

    if (proc->unk_32 == four) {
        if (proc->currentPage == 8) {
            proc->currentPage = 0;
        } else {
            proc->currentPage++;
        }
        sub_809F150_INFUSE(proc);
    }

    if (proc->unk_32 >= four) {
        int tmp = four - (proc->unk_32 - four);
        x = -((tmp * 0x60 * tmp) / (four * four));
    }

    BG_SetPosition(2, (x & 0xff), proc->yOffsetPerPage[proc->currentPage] - 40);

    if (proc->unk_32 == four * 2) {
        Proc_Goto(proc, 1);
    }

    return;
}

//! FE8U = 0x0809F3F4
void PrepItemList_ScrollVertical_INFUSE(struct PrepItemListProc * proc, int amount)
{
    ResetIconGraphics_();

    sub_809D418(gBG2TilemapBuffer + 0xF, proc->yOffsetPerPage[proc->currentPage] >> 4);

    /* Draw dragon egg icon */
    DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 13), GetItemIconId(0xAA), 0x4000);

    BG_EnableSyncByMask(5);

    if (amount < 0) {
        sub_809D47C(PrepItemSuppyTexts.th + 7, gBG2TilemapBuffer + 0xF, (proc->yOffsetPerPage[proc->currentPage] >> 4) - 1,  proc->unit);
    }

    if (amount > 0) {
        sub_809D47C(PrepItemSuppyTexts.th + 7, gBG2TilemapBuffer + 0xF, (proc->yOffsetPerPage[proc->currentPage] >> 4) + 7, proc->unit);
    }

    proc->yOffsetPerPage[proc->currentPage] += amount;

    BG_SetPosition(2, 0, proc->yOffsetPerPage[proc->currentPage] - 40);

    return;
}

void PerformInfusion(struct PrepItemListProc* proc, int idx, u8 target, u8 cost) {
    // 1. Deduct Shards
    gInfuseMenuArray[0] -= cost;

    struct PrepScreenItemListEnt* ent = &gPrepScreenItemList[idx];
    u16 newItem = target | (GetItemMaxUses(target) << 8);

    // 2. Update ACTUAL game data
    if (ent->pid == 0) {
        gConvoyItemArray[ent->itemSlot] = newItem;
    } else {
        struct Unit* unit = GetUnitFromCharId(ent->pid);
        unit->items[ent->itemSlot] = newItem;
    }

    // 3. Force Prep Screen cache refresh
    SomethingPrepListRelated(proc->unit, proc->currentPage, 3);

    // 4. Feedback
    PlaySoundEffect(0x5A); 
    // Update the Shard count number
    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 3, 5), TEXT_COLOR_SYSTEM_WHITE, gInfuseMenuArray[0]);

    // 5. REPAIR THE UI (The critical part)
    // Instead of DrawPrepScreenItemIcons (which causes the ghosts), 
    // we call your custom refresh function.
    sub_809F150_INFUSE(proc); 

    // 6. Refresh the Infuse Boxes immediately
    // This forces the Loop_MainKeyHandler to notice the "new" item 
    // and redraw the box contents in the next frame.
    gInfuseMenuArray[1] = -1; 
    
    // 7. Popup
    SetPopupItem(target);
    // NewPopup_Simple(InfusedPopup, 0x60, 0x00, proc);

    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT);
}

void PrepItemList_Loop_MainKeyHandler_INFUSE(struct PrepItemListProc * proc)
{
    int idx = proc->idxPerPage[proc->currentPage];
    u16 item = gPrepScreenItemList[idx].item;
    u8 itemId = ITEM_INDEX(item);
    u8 target = gInfusionLookupTable[itemId].targetItemId;
    u8 cost = gInfusionLookupTable[itemId].cost;

    // Forces redraw every frame by resetting the "previous index" tracker
    gInfuseMenuArray[1] = -1; 

    // --- 1. INITIAL DRAWING LOGIC (Exact Restoration) ---
    if (idx != gInfuseMenuArray[1]) {
        u16 item = gPrepScreenItemList[idx].item;
        u8 itemId = ITEM_INDEX(item);
        u8 target = gInfusionLookupTable[itemId].targetItemId;

        ClearText(&PrepItemSuppyTexts.th[2]);
        ClearText(&PrepItemSuppyTexts.th[3]);

        if (gUnknown_02012F56 == 0)
        {
            /* Draw selected item name */
            PutDrawText(&PrepItemSuppyTexts.th[2], TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 9), TEXT_COLOR_SYSTEM_GRAY, 4, 0, "Nothing");
            PutDrawText(&PrepItemSuppyTexts.th[3], TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 17), TEXT_COLOR_SYSTEM_GRAY, 2, 0, "No fusable target");
        }
        else
        {
            ClearText(&PrepItemSuppyTexts.th[2]);

            /* Draw selected item icon */
            DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 9), GetItemIconId(item), 0x4000);
            /* Draw selected item name */
            PutDrawText(&PrepItemSuppyTexts.th[2], TILEMAP_LOCATED(gBG0TilemapBuffer, 4, 9), 0, 2, 0, GetItemName(item));
            /* Draw selected item durability */
            PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 12, 9), TEXT_COLOR_SYSTEM_BLUE, ITEM_USES(item));

            /* Draw the fuse item cost */
            PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 9, 13), TEXT_COLOR_SYSTEM_WHITE, gInfusionLookupTable[itemId].cost);

            if (target != 0) {
                /* Draw fused item icon */
                DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 17), GetItemIconId(target), 0x4000);
                /* Draw fused item name */
                PutDrawText(&PrepItemSuppyTexts.th[3], TILEMAP_LOCATED(gBG0TilemapBuffer, 4, 17), TEXT_COLOR_SYSTEM_GREEN, 2, 0, GetItemName(target));
                /* Draw fused item durability */
                PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 12, 17), TEXT_COLOR_SYSTEM_BLUE, GetItemMaxUses(target));
            } 
            else {
                /* Draw fallback text */
                PutDrawText(&PrepItemSuppyTexts.th[3], TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 17), TEXT_COLOR_SYSTEM_GRAY, 2, 0, "No fusable target");
            }   
        }

        BG_EnableSyncByMask(BG0_SYNC_BIT);
        gInfuseMenuArray[1] = idx;
    }

    // --- 2. INPUT HANDLING (Main Loop) ---
    if ((proc->yOffsetPerPage[proc->currentPage] & 0xf) == 0) {
        if ((proc->unk_36 == 0) || (proc->unk_36 == 0xff)) {
            
            // R-Button Help
            if (gKeyStatusPtr->newKeys & R_BUTTON) {
                if (gUnknown_02012F56 == 0) {
                    PlaySoundEffect(SONG_6C);
                    return;
                } else {
                    // Determine which item to show based on state
                    u16 helpItem;
                    int helpX;
                    int helpY;
                    
                    if (gInfuseMenuArray[4] == 1) {
                        // We're in infuse state, show the TARGET item
                        helpItem = target;
                        helpX = 20;  // X position of the infuse box 
                        helpY = 125; // Y position of the infuse box
                    } else {
                        // Normal state, show the selected list item
                        helpItem = gPrepScreenItemList[proc->idxPerPage[proc->currentPage]].item;
                        helpX = 0x80;
                        helpY = proc->idxPerPage[proc->currentPage] * 16 + 40 - proc->yOffsetPerPage[proc->currentPage];
                    }
                    
                    StartItemHelpBox(helpX, helpY, helpItem);
                    proc->unk_36 = 1;
                    return;
                }
            }

            // A-Button Logic (States 0, 1, and 2)
            if (gKeyStatusPtr->newKeys & A_BUTTON) {
                if (gUnknown_02012F56 == 0) {
                    PlaySoundEffect(SONG_6C);
                    return;
                }

                // State 1: Highlighted Infuse UI -> Open Confirmation
                if (gInfuseMenuArray[4] == 1) {
                    if (target == 0) {
                        PlaySoundEffect(SONG_6C);
                        return;
                    }
                    gInfuseMenuArray[4] = 2;
                    gInfuseMenuArray[5] = 0; // Default to Yes
                    ClearText(&PrepItemSuppyTexts.th[0]);
                    PutDrawText(&PrepItemSuppyTexts.th[0], TILEMAP_LOCATED(gBG0TilemapBuffer, 6, 2), TEXT_COLOR_SYSTEM_WHITE, 2, 0, "Infuse weapon?");
                    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
                    StartParallelWorker(PutGiveTakeBoxSprites, proc);
                    EndUiCursorHand();
                    ShowSysHandCursor(68, 36, 0x4, 0x000); // Priority adjusted per original
                    BG_EnableSyncByMask(7);
                    return;
                }

                // State 2: Confirmation Box Open -> Perform Action
                if (gInfuseMenuArray[4] == 2) {
                    if (gInfuseMenuArray[5] == 0) {
                        if (gInfuseMenuArray[0] >= cost)
                        {
                            PerformInfusion(proc, idx, target, cost);
                            PlaySoundEffect(0x5A);
                        }
                        else
                        {
                            ClearText(&PrepItemSuppyTexts.th[0]);
                            PutDrawText(&PrepItemSuppyTexts.th[0], TILEMAP_LOCATED(gBG0TilemapBuffer, 6, 2), TEXT_COLOR_SYSTEM_GRAY, 2, 0, "Not enough...");     
                            PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
                            return;
                        }

                    } else {
                        PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
                    }
                    goto EXIT_SUB_MENU;
                }

                // Default State: List -> Infuse UI
                gInfuseMenuArray[2] = 0x80;
                gInfuseMenuArray[3] = proc->idxPerPage[proc->currentPage] * 16 + 40 - proc->yOffsetPerPage[proc->currentPage];
                gInfuseMenuArray[4] = 1;
                EndUiCursorHand();
                ShowSysHandCursor(14, 135, 0xB, 0x800);
                PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
                return;
            }

            // B-Button Logic
            if (gKeyStatusPtr->newKeys & B_BUTTON) {
                if (gInfuseMenuArray[4] > 0) {
                    goto EXIT_SUB_MENU;
                }
                SetPrimaryHBlankHandler(NULL);
                Proc_Goto(proc, 9);
                StartPrepAtMenuWithConfig();
                PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
                proc->unk_36 = 0;
                return;
            }

            // Yes/No Selection Toggle (Up/Down)
            if (gInfuseMenuArray[4] == 2 && (gKeyStatusPtr->newKeys & (DPAD_UP | DPAD_DOWN))) {
                gInfuseMenuArray[5] ^= 1;
                PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
                int cursorY = (gInfuseMenuArray[5] == 0) ? 36 : 52;
                ShowSysHandCursor(68, cursorY, 0x4, 0x000);
                return;
            }

            // DPAD Left/Right Page Switching (Restored to Original)
            if (gKeyStatusPtr->repeatedKeys & DPAD_LEFT && gInfuseMenuArray[4] == 0) {
                SetUiSpinningArrowFastMaybe(0);
                PlaySoundEffect(SONG_SE_SYS_CURSOR_LR1);
                Proc_Goto(proc, 3);
                proc->unk_32 = 0;
                PrepItemList_SwitchPageLeft_INFUSE(proc);
                return;
            }
            if (gKeyStatusPtr->repeatedKeys & DPAD_RIGHT && gInfuseMenuArray[4] == 0) {
                SetUiSpinningArrowFastMaybe(1);
                PlaySoundEffect(SONG_SE_SYS_CURSOR_LR1);
                Proc_Goto(proc, 4);
                proc->unk_32 = 0;
                PrepItemList_SwitchPageRight_INFUSE(proc);
                return;
            }

            // Scrolling Logic
            proc->scrollAmount = (gKeyStatusPtr->heldKeys & L_BUTTON) ? 8 : 4;

            if ((gKeyStatusPtr->repeatedKeys & DPAD_UP && gInfuseMenuArray[4] != 1) ||
                ((gKeyStatusPtr->heldKeys & DPAD_UP) && (proc->scrollAmount == 8) && gInfuseMenuArray[4] != 1)) {
                if (proc->idxPerPage[proc->currentPage] != 0) proc->idxPerPage[proc->currentPage]--;
                ClearText(&PrepItemSuppyTexts.th[3]);
            }

            if ((gKeyStatusPtr->repeatedKeys & DPAD_DOWN && gInfuseMenuArray[4] != 1) ||
                ((gKeyStatusPtr->heldKeys & DPAD_DOWN) && (proc->scrollAmount == 8) && gInfuseMenuArray[4] != 1)) {
                if (proc->idxPerPage[proc->currentPage] < gUnknown_02012F56 - 1) proc->idxPerPage[proc->currentPage]++;
                ClearText(&PrepItemSuppyTexts.th[3]);
            }

        } else {
            // Help Box Navigation
            if (gKeyStatusPtr->newKeys & (R_BUTTON | B_BUTTON)) {
                CloseHelpBox();
                proc->unk_36 = 0;
                return;
            }
        }
    } else {
        // Vertical Scroll position updates
        if ((proc->idxPerPage[proc->currentPage] * 16 + 40 - proc->yOffsetPerPage[proc->currentPage]) < 0x38) {
            proc->yOffsetPerPage[proc->currentPage] -= proc->scrollAmount;
        }
        if ((proc->idxPerPage[proc->currentPage] * 16 + 40 - proc->yOffsetPerPage[proc->currentPage]) > 0x78) {
            proc->yOffsetPerPage[proc->currentPage] += proc->scrollAmount;
        }
        BG_SetPosition(2, 0, proc->yOffsetPerPage[proc->currentPage] - 40);
    }

    // --- 3. CURSOR & SCREEN REFRESH ---
    if (idx != proc->idxPerPage[proc->currentPage]) {
        u16 item = gPrepScreenItemList[proc->idxPerPage[proc->currentPage]].item;
        PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);

        if (gPrepScreenItemList[proc->idxPerPage[proc->currentPage]].pid != gPrepScreenItemList[idx].pid) {
            PrepItemList_DrawCurrentOwnerText_INFUSE(proc);
        }

        int yPos = proc->idxPerPage[proc->currentPage] * 16 + 40 - proc->yOffsetPerPage[proc->currentPage];
        if (yPos < 0x38 && proc->idxPerPage[proc->currentPage] != 0) {
            if (proc->unk_36 != 0) StartItemHelpBox(0x80, yPos + 16, item);
            PrepItemList_ScrollVertical_INFUSE(proc, -proc->scrollAmount);
        } else if (yPos > 0x78 && proc->idxPerPage[proc->currentPage] != gUnknown_02012F56 - 1) {
            if (proc->unk_36 != 0) StartItemHelpBox(0x80, yPos - 0x10, item);
            PrepItemList_ScrollVertical_INFUSE(proc, +proc->scrollAmount);
        } else {
            if (proc->unk_36 != 0) StartItemHelpBox(0x80, yPos, item);
            ShowSysHandCursor(0x80, yPos, 0xB, 0x800);
        }
    }

    return;

EXIT_SUB_MENU:
    gInfuseMenuArray[4] = 0;
    Proc_End(GetParallelWorker(PutGiveTakeBoxSprites));
    EndUiCursorHand();
    ShowSysHandCursor(gInfuseMenuArray[2], gInfuseMenuArray[3], 0xB, 0x800);
    ClearText(&PrepItemSuppyTexts.th[0]);
    PutDrawText(&PrepItemSuppyTexts.th[0], TILEMAP_LOCATED(gBG0TilemapBuffer, 6, 2), 0, 2, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_SELECT_WEAPON)));
    
    // --- ADD THESE LINES TO RESTORE ICON VRAM ---
    // ResetIconGraphics_(); 
    // SomethingPrepListRelated(proc->unit, proc->currentPage, 3);
    // sub_809D300(
    //     PrepItemSuppyTexts.th + 7, 
    //     gBG2TilemapBuffer + 0xF, 
    //     proc->yOffsetPerPage[proc->currentPage] >> 4, 
    //     proc->unit
    // );
    sub_809F150_INFUSE(proc);

    // --------------------------------------------

    PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
    return;
}

struct ProcCmd const ProcScr_PrepItemListScreen_INFUSE[] = {
    PROC_SLEEP(0),
    PROC_CALL(PrepItemList_Init),

PROC_LABEL(0),
    PROC_CALL(PrepItemList_InitGfx_INFUSE),

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
    PROC_SLEEP(0),

    PROC_GOTO(0),

PROC_LABEL(7),
    PROC_CALL(PrepItemList_SwitchToUnitInventory),
    PROC_REPEAT(PrepItemList_Loop_UnitInvKeyHandler),

    PROC_GOTO(1),

PROC_LABEL(3),
    PROC_REPEAT(PrepItemList_SwitchPageLeft_INFUSE),

    // fallthrough

PROC_LABEL(4),
    PROC_REPEAT(PrepItemList_SwitchPageRight_INFUSE),

    // fallthrough

PROC_LABEL(8),
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),

    // fallthrough

PROC_LABEL(9),
    PROC_CALL(PrepItemList_OnEnd),

    PROC_END,
};