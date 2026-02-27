#include "common-chax.h"
#include "action-expa.h"
#include "kernel-lib.h"
#include "jester_headers/LimitedShopStock.h"

extern struct KeyStatusBuffer sKeyStatusBuffer;

void SU_SaveShopStock(void* target, int size) {
    WriteAndVerifySramFast(gCurrentShopStocks, target, size);
}

void SU_LoadShopStock(void* source, int size) {
    ReadSramFast(source, gCurrentShopStocks, size);
}

void InitShopStock() {
    int i = 0;
    ShopStockEntry* limitedItems = gShopStockTable[gPlaySt.chapterIndex];
    if (limitedItems) {
        while (limitedItems[i].item) {
            (*gCurrentShopStocks)[i] = limitedItems[i].stock;
            ++i;
        }
        (*gCurrentShopStocks)[i] = 0; //Terminator
    }
    else {
        memset(*gCurrentShopStocks, 0, 1); //Terminator only
    }
}

int GetItemStockEntryNumber(u16 item) {
    u8 itemID = GetItemIndex(item);
    ShopStockEntry* limitedItems = gShopStockTable[gPlaySt.chapterIndex];
    if (limitedItems) {
        for (int i = 0; limitedItems[i].item; ++i) {
            if (limitedItems[i].item == itemID) {
                return i;
            }
        }
    }
    return (-1); //Item is not a stock item
}

int GetItemStock(u16 item) {
    int stockItem = GetItemStockEntryNumber(item);
    if (stockItem != -1) {
        return (*gCurrentShopStocks)[stockItem];
    }
    return -1; 
}

bool IsItemInStock(u16 item) {
    int stockItem = GetItemStockEntryNumber(item);
    if (stockItem != -1) {
        if ((*gCurrentShopStocks)[stockItem]) {
            return TRUE;
        }
        else {
            return FALSE;
        }
    }
    return TRUE;
}

void ReduceItemStock(u16 item) {
    int stockItem = GetItemStockEntryNumber(item);
    if (stockItem != -1 && (*gCurrentShopStocks)[stockItem] > 0) {
        (*gCurrentShopStocks)[stockItem]--;
    }
}

static u8 GetItemNameColor(int item, s8 isUsable)
{
    if (gpKernelDesignerConfig->limited_shop_items == true) {
        int stock = GetItemStock(item);
        
        if (stock == 0)
            return TEXT_COLOR_SYSTEM_GRAY;
        else if (stock > 0)
            return TEXT_COLOR_SYSTEM_GOLD;
        // If stock < 0, it falls through to the standard behavior below
    }

    // Standard behavior (limited stock disabled OR stock < 0)
    return isUsable ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY; 
}

static u8 GetItemStatColor(int item, s8 isUsable)
{
    if (gpKernelDesignerConfig->limited_shop_items == true) {
        int stock = GetItemStock(item);
        
        if (stock == 0)
            return TEXT_COLOR_SYSTEM_GRAY;
        else if (stock > 0)
            return TEXT_COLOR_SYSTEM_BLUE;
        // If stock < 0, it falls through to the standard behavior below
    }

    // Standard behavior (limited stock disabled OR stock < 0)
    return isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY;
}

// --- END NEW COLOR HELPERS ---

void DrawStockedItemLine(struct Text* text, int item, s8 isUsable, u16* mapOut)
{
    int stock = GetItemStock(item);
    u8 nameColor = GetItemNameColor(item, isUsable);
    u8 statColor = GetItemStatColor(item, isUsable);

    Text_SetParams(text, 0, nameColor);
    Text_DrawString(text, GetItemName(item));

    // Name
    if (gpKernelDesignerConfig->limited_shop_items == true)
        PutText(text, mapOut - 1);
    else
        PutText(text, mapOut + 2);

#ifndef CONFIG_INFINITE_DURABILITY

    // Uses
    if (gpKernelDesignerConfig->limited_shop_items == true)
        PutNumberOrBlank(mapOut + 8, statColor, GetItemUses(item));
    else
        PutNumberOrBlank(mapOut + 11, statColor, GetItemUses(item));

    // Stock
    if (gpKernelDesignerConfig->limited_shop_items == true)
    {
        if (stock >= 0)
            PutNumber(mapOut + 17, statColor, stock);
    }

#endif

    // Icon
    if (gpKernelDesignerConfig->limited_shop_items == true)
        DrawIcon(mapOut - 3, GetItemIconId(item), 0x4000);
    else
        DrawIcon(mapOut, GetItemIconId(item), 0x4000);
}

LYN_REPLACE_CHECK(DrawShopItemPriceLine);
void DrawShopItemPriceLine(struct Text* th, int item, struct Unit* unit, u16* dst)
{
    s8 usable = (unit == 0) ? true : IsItemDisplayUsable(unit, item);
    int price = GetItemPurchasePrice(unit, item);

    DrawStockedItemLine(th, item, usable, dst);

    u8 statColor = GetItemStatColor(item, usable);

    if (gpKernelDesignerConfig->limited_shop_items == true)
        PutNumber(dst + 14, statColor, price);
    else
        PutNumber(dst + 17, statColor, price);
}

LYN_REPLACE_CHECK(Shop_Loop_BuyKeyHandler);
void Shop_Loop_BuyKeyHandler(struct ProcShop* proc) {
    u8 head_loc;
    u32 cursor_at_head;
    int price;
    int a;
    int b;

    Shop_TryMoveHandPage();

    BG_SetPosition(2, 0, ShopSt_GetBg2Offset());

    head_loc = proc->head_loc;
    cursor_at_head = ShopSt_GetHeadLoc() != head_loc;

    proc->head_loc = ShopSt_GetHeadLoc();
    proc->hand_loc = ShopSt_GetHandLoc();

    proc->head_idx = proc->head_loc;
    proc->hand_idx = proc->hand_loc;

    a = proc->head_loc * 16;
    b = (proc->hand_loc * 16) - 72;

    int ui_hand_x_position = 56;

    if (gpKernelDesignerConfig->limited_shop_items == true)
        ui_hand_x_position -= 32;

    DisplayUiHand(ui_hand_x_position, a - b);

    /* Reposition helpbox when cursor moves */
    if ((proc->helpTextActive != 0) && (cursor_at_head != 0)) {
        StartItemHelpBox(56, a - b, proc->shopItems[proc->head_loc]);
    }

    DisplayShopUiArrows();

    if (IsShopPageScrolling() != 0)
        return;

    /* === HELPBOX ACTIVE === */
    if (proc->helpTextActive != 0) {
        if (sKeyStatusBuffer.newKeys & (B_BUTTON | R_BUTTON)) {
            proc->helpTextActive = 0;
            CloseHelpBox();
        }
        return;
    }

    /* === OPEN HELPBOX === */
    if (sKeyStatusBuffer.newKeys & R_BUTTON) {
        proc->helpTextActive = 1;
        StartItemHelpBox(56, a - b, proc->shopItems[proc->head_loc]);
        return;
    }

    price = GetItemPurchasePrice(proc->unit, proc->shopItems[proc->head_loc]);

    /* === CONFIRM BUY === */
    if (sKeyStatusBuffer.newKeys & A_BUTTON) {
        if (!IsItemInStock(proc->shopItems[proc->head_loc])) {
            StartShopDialogue(OutOfStockTextBase, proc);
            Proc_Goto(proc, 1);
        }
        else if (price > (int)GetPartyGoldAmount()) {
            StartShopDialogue(0x8B2, proc);
            Proc_Goto(proc, 1);
        }
        else {
            SetTalkNumber(price);
            StartShopDialogue(0x8B5, proc);
            Proc_Break(proc);
        }
        return;
    }

    /* === EXIT SHOP === */
    if (sKeyStatusBuffer.newKeys & B_BUTTON) {
        PlaySFX(0x6B, 0x100, 0, 1);
        Proc_Goto(proc, 7);
        return;
    }
}

LYN_REPLACE_CHECK(HandleShopBuyAction);
void HandleShopBuyAction(struct ProcShop* proc) {
    PlaySeDelayed(0xB9, 8);

    gActionData.unitActionType = UNIT_ACTION_SHOPPED;

    SetPartyGoldAmount(GetPartyGoldAmount() - GetItemPurchasePrice(proc->unit, proc->shopItems[proc->head_loc]));

    if (gpKernelDesignerConfig->limited_shop_items == true)
        ReduceItemStock(proc->shopItems[proc->head_loc]);

    UpdateShopItemCounts(proc);
    DrawShopSoldItems(proc);

    DisplayGoldBoxText(gBG0TilemapBuffer + 0xDB);

    return;
}

LYN_REPLACE_CHECK(Shop_Init);
void Shop_Init(struct ProcShop * proc)
{
    int i;

    if (proc->shopType == SHOP_TYPE_ARMORY)
        StartBgm(SONG_ARMORIES, 0);
    else
        StartBgm(SONG_SHOPS, 0);

    Proc_ForEach(ProcScr_Mu, (ProcFunc) HideMu);

    InitShopScreenConfig();

    if (gpKernelDesignerConfig->limited_shop_items == true)
        InitShopStock();

    gLCDControlBuffer.bg0cnt.priority = 0;
    gLCDControlBuffer.bg1cnt.priority = 2;
    gLCDControlBuffer.bg2cnt.priority = 0;
    gLCDControlBuffer.bg3cnt.priority = 3;

    InitTalk(0x200, 2, 0);

    ResetFaces();

    proc->head_loc = 0;
    proc->head_idx = 0;
    proc->hand_idx = 0;
    proc->hand_loc = 0;
    proc->buy_or_sel = SHOP_ST_BUY;
    proc->helpTextActive = 0;

    UnpackUiVArrowGfx(OBJCHR_SHOP_SPINARROW, OBJPAL_SHOP_SPINARROW);

    StartTalkFace(Shop_GetPortraitIndex(proc), 32, 8, 3, 1);

    Decompress(Tsa_ShopWindows, gGenericBuffer);
    CallARM_FillTileRect(gBG1TilemapBuffer, gGenericBuffer, 0x1000);

    if (gpKernelDesignerConfig->limited_shop_items == true)
        DrawUiFrame2(3, 8, 23, 12, 0);
    else
        DrawUiFrame2(6, 8, 20, 12, 0);

    BG_EnableSyncByMask(BG1_SYNC_BIT);

    StartUiGoldBox(proc);

    for (i = 0; i < SHOP_ITEM_LINE; i++)
        InitText(&gShopItemTexts[i], 20);

    DrawShopSoldItems(proc);

    SetWinEnable(1, 1, 0);
    SetWin0Layers(1, 1, 1, 1, 1);
    SetWin1Layers(1, 1, 0, 1, 1);
    SetWOutLayers(1, 1, 0, 1, 1);

    if (gpKernelDesignerConfig->limited_shop_items == true)
    {
        SetWin0Box(32, 72, 240, 152);
    }
    else
    {
        SetWin0Box(56, 72, 240, 152);
    }

    SetWin1Box(0, 8, 240, 56);

    gLCDControlBuffer.wincnt.win0_enableBlend = 0;
    gLCDControlBuffer.wincnt.win1_enableBlend = 1;
    gLCDControlBuffer.wincnt.wout_enableBlend = 0;

    SetBlendConfig(3, 0, 0, 8);

    SetBlendTargetA(0, 0, 0, 1, 0);
    SetBlendTargetB(0, 0, 0, 0, 0);

    ApplyPalette(Pal_CommGameBgScreenInShop, BGPAL_SHOP_MAINBG);
    Decompress(Img_CommGameBgScreen, (void *)BG_VRAM + GetBackgroundTileDataOffset(BG_3));
    CallARM_FillTileRect(gBG3TilemapBuffer, Tsa_CommGameBgScreenInShop, OBJ_PALETTE(BGPAL_SHOP_MAINBG));

    BG_EnableSyncByMask(BG3_SYNC_BIT);
}