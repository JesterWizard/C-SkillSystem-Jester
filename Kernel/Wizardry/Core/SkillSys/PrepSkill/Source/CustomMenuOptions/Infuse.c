#include "common-chax.h"
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

void displayScrollBackground_INFUSE(void)
{
    // SetTextFont(NULL);
    TileMap_FillRect(gBG0TilemapBuffer + 0x34, 12, 1, 0);

    // PutDrawText(&PrepItemSuppyTexts.th[0], gBG0TilemapBuffer + 0x34 + 0x6d, 0, 2, 0, "Infuse");
    PutFaceChibi(FID_SUPPLY + 1, gBG0TilemapBuffer + 0x34 - 0x13, 0x270, 2, 0);
    // PutDrawText(&PrepItemSuppyTexts.th[0] + 1, gBG0TilemapBuffer + 0x34 - 1, 0, 4, 0, GetStringFromIndex(0x5a0));

    PutNumber(gBG0TilemapBuffer + 0x34 + 0x6E, TEXT_COLOR_SYSTEM_WHITE, GetConvoyItemCount_());

    BG_EnableSyncByMask(BG0_SYNC_BIT);

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

    StartGreenText(proc);

    StartHelpPromptSprite(195, 147, 9, proc);

    InitText(PrepItemSuppyTexts.th + 0, 6);
    InitText(PrepItemSuppyTexts.th + 1, 5);

    InitText(PrepItemSuppyTexts.th + 15, 4);

    for (i = 0; i < 8; i++) {
        InitTextDb(PrepItemSuppyTexts.th + 7 + i, 7);
    }

    /* Display weapon type graphics at top of convoy on the right */
    StoreConvoyWeaponIconGraphics(0x4000, 6);

    /* Display background for weapon tiles to sit in? */
    sub_809D8D4(gBG0TilemapBuffer + 0x6F, 0x4000, 6);

    Decompress(gUnknown_08A19CCC, (void*)0x06015000);
    ApplyPalette(Pal_SpinningArrow, 0x14);

    StartMenuScrollBarExt(proc, 225, 47, 0x5800, 9);
    sub_8097668();
    SomethingPrepListRelated(proc->unit, proc->currentPage, 3);
    sub_809F5F4(proc);

    sub_809D300(
        PrepItemSuppyTexts.th + 7,
        gBG2TilemapBuffer + 0xF,
        (proc->yOffsetPerPage[proc->currentPage]) >> 4,
        proc->unit
    );

    BG_EnableSyncByMask(4);

    /* Display infuse graphic */
    displayScrollBackground_INFUSE();

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

    /* Displays "Owner" text in top right brown box */
    sub_809EBF0();
    PrepItemList_DrawCurrentOwnerText(proc);

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
    PROC_REPEAT(PrepItemList_Loop_MainKeyHandler),

    // fallthrough

PROC_LABEL(6),
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),

    PROC_CALL(PrepItemList_OnEnd),
    PROC_CALL(PrepItemList_StartTradeScreen),
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