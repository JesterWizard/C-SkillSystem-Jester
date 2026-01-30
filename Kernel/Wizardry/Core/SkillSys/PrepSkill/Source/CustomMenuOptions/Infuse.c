#include "common-chax.h"
#include "utf8.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "popup.h"
#include "prep-skill.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-structs.h"

// A simpler struct for the mapping
STATIC_DECLAR struct InfuseRecipe {
    u8 targetItemId;
    u8 cost;
};

// Allocate a table for all 256 possible items
// Initializing with [256] ensures O(1) access via itemId index
STATIC_DECLAR const struct InfuseRecipe gInfusionLookupTable[256] = {
// Swords
[ITEM_SWORD_IRON]        = { ITEM_SWORD_STEEL,      1 },
[ITEM_SWORD_SLIM]        = { ITEM_SWORD_RAPIER,     2 },
[ITEM_SWORD_STEEL]       = { ITEM_SWORD_SILVER,     3 },
[ITEM_SWORD_SILVER]      = { ITEM_SWORD_BRAVE,      6 },
[ITEM_SWORD_POISON]      = { ITEM_SWORD_KILLER,     4 },
[ITEM_SWORD_RAPIER]      = { ITEM_SWORD_ARMORSLAYER,5 },
[ITEM_SWORD_KILLER]      = { ITEM_SWORD_SHAMSIR,    6 },
[ITEM_SWORD_ARMORSLAYER] = { ITEM_SWORD_ZANBATO,    5 },
[ITEM_SWORD_LIGHTBRAND]  = { ITEM_SWORD_RUNESWORD,  7 },
[ITEM_SWORD_LANCEREAVER] = { ITEM_SWORD_WYRMSLAYER, 6 },

// Blades
[ITEM_BLADE_IRON]        = { ITEM_BLADE_STEEL,      2 },
[ITEM_BLADE_STEEL]       = { ITEM_BLADE_SILVER,     4 },
[ITEM_BLADE_SILVER]      = { ITEM_SWORD_BRAVE,      7 },

// Lances
[ITEM_LANCE_IRON]        = { ITEM_LANCE_STEEL,      1 },
[ITEM_LANCE_SLIM]        = { ITEM_LANCE_JAVELIN,    2 },
[ITEM_LANCE_STEEL]       = { ITEM_LANCE_SILVER,     3 },
[ITEM_LANCE_POISON]      = { ITEM_LANCE_KILLER,     4 },
[ITEM_LANCE_SILVER]      = { ITEM_LANCE_BRAVE,      6 },
[ITEM_LANCE_KILLER]      = { ITEM_LANCE_HORSESLAYER,5 },
[ITEM_LANCE_JAVELIN]     = { ITEM_LANCE_SPEAR,      4 },
[ITEM_LANCE_AXEREAVER]   = { ITEM_LANCE_REGINLEIF,  7 },

// Axes
[ITEM_AXE_IRON]          = { ITEM_AXE_STEEL,        1 },
[ITEM_AXE_STEEL]         = { ITEM_AXE_SILVER,       3 },
[ITEM_AXE_POISON]        = { ITEM_AXE_KILLER,       4 },
[ITEM_AXE_SILVER]        = { ITEM_AXE_BRAVE,        6 },
[ITEM_AXE_KILLER]        = { ITEM_AXE_HALBERD,      5 },
[ITEM_AXE_HALBERD]       = { ITEM_AXE_HAMMER,       4 },
[ITEM_AXE_HANDAXE]       = { ITEM_AXE_TOMAHAWK,     5 },
[ITEM_AXE_SWORDREAVER]   = { ITEM_AXE_SWORDSLAYER,  6 },
[ITEM_AXE_DRAGON]        = { ITEM_AXE_GARM,         9 },

// Bows
[ITEM_BOW_IRON]          = { ITEM_BOW_STEEL,        1 },
[ITEM_BOW_STEEL]         = { ITEM_BOW_SILVER,       3 },
[ITEM_BOW_POISON]        = { ITEM_BOW_KILLER,       4 },
[ITEM_BOW_SILVER]        = { ITEM_BOW_BRAVE,        6 },
[ITEM_BOW_SHORTBOW]      = { ITEM_BOW_LONGBOW,      3 },
[ITEM_BOW_KILLER]        = { ITEM_BOW_BEACONBOW,    7 },

// Ballistae
[ITEM_BALLISTA_REGULAR]  = { ITEM_BALLISTA_LONG,    3 },
[ITEM_BALLISTA_LONG]     = { ITEM_BALLISTA_KILLER,  5 },

// Anima magic
[ITEM_ANIMA_FIRE]        = { ITEM_ANIMA_ELFIRE,     2 },
[ITEM_ANIMA_THUNDER]     = { ITEM_ANIMA_BOLTING,    4 },
[ITEM_ANIMA_ELFIRE]      = { ITEM_ANIMA_FORBLAZE,   6 },
[ITEM_ANIMA_FIMBULVETR]  = { ITEM_ANIMA_EXCALIBUR,  7 },
[ITEM_ANIMA_EXCALIBUR]   = { ITEM_ANIMA_AIRCALIBUR, 8 },

// Light magic
[ITEM_LIGHT_LIGHTNING]   = { ITEM_LIGHT_SHINE,      1 },
[ITEM_LIGHT_SHINE]       = { ITEM_LIGHT_DIVINE,     3 },
[ITEM_LIGHT_DIVINE]      = { ITEM_LIGHT_PURGE,      5 },
[ITEM_LIGHT_AURA]        = { ITEM_LIGHT_LUCE,       6 },
[ITEM_LIGHT_LUCE]        = { ITEM_LIGHT_IVALDI,     9 },

// Dark magic
[ITEM_DARK_FLUX]         = { ITEM_DARK_LUNA,        2 },
[ITEM_DARK_LUNA]         = { ITEM_DARK_NOSFERATU,   4 },
[ITEM_DARK_NOSFERATU]    = { ITEM_DARK_ECLIPSE,     6 },
[ITEM_DARK_FENRIR]       = { ITEM_DARK_GLEIPNIR,    7 },
[ITEM_DARK_GLEIPNIR]     = { ITEM_DARK_NAGLFAR,     9 },

// Staves
[ITEM_STAFF_HEAL]        = { ITEM_STAFF_MEND,       1 },
[ITEM_STAFF_MEND]        = { ITEM_STAFF_RECOVER,    3 },
[ITEM_STAFF_RECOVER]     = { ITEM_STAFF_PHYSIC,     4 },
[ITEM_STAFF_PHYSIC]      = { ITEM_STAFF_FORTIFY,    6 },
[ITEM_STAFF_RESTORE]     = { ITEM_STAFF_BARRIER,   4 },
[ITEM_STAFF_SILENCE]     = { ITEM_STAFF_SLEEP,     4 },
[ITEM_STAFF_SLEEP]       = { ITEM_STAFF_BERSERK,    6 },
[ITEM_STAFF_WARP]        = { ITEM_STAFF_RESCUE,     7 },
[ITEM_STAFF_RESCUE]      = { ITEM_STAFF_LATONA,     9 },

// Consumables & misc
[ITEM_VULNERARY]         = { ITEM_ELIXIR,           3 },
[ITEM_CHESTKEY]          = { ITEM_CHESTKEY_BUNDLE,  2 },
[ITEM_DOORKEY]           = { ITEM_LOCKPICK,         4 },
[ITEM_TORCH]             = { ITEM_LIGHTRUNE,        2 },

// Stat boosters
[ITEM_BOOSTER_HP]        = { ITEM_BOOSTER_DEF,      5 },
[ITEM_BOOSTER_POW]       = { ITEM_BOOSTER_SKL,      4 },
[ITEM_BOOSTER_SPD]       = { ITEM_BOOSTER_MOV,      8 },
[ITEM_BOOSTER_RES]       = { ITEM_BOOSTER_CON,      6 },

// Promotion
[ITEM_HEROCREST]         = { ITEM_MASTERSEAL,       7 },
[ITEM_KNIGHTCREST]       = { ITEM_MASTERSEAL,       7 },
[ITEM_ORIONSBOLT]        = { ITEM_MASTERSEAL,       7 },
[ITEM_GUIDINGRING]       = { ITEM_METISSTOME,       6 },

};

STATIC_DECLAR struct PopupInstruction const InfusedPopup[] = {
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
STATIC_DECLAR void drawInfuseSprites(void)
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

STATIC_DECLAR void InfuseSpriteWorker(ProcPtr proc) {
    drawInfuseSprites();
}

STATIC_DECLAR void drawItems_INFUSE(struct Text * textBase, u16 * tm, int yLines, struct Unit * unit)
{
    int i;

    TileMap_FillRect(tm, 12, 31, 0);

    if (gUnknown_02012F56 == 0) {
        ClearText(textBase);
        Text_InsertDrawString(textBase, 0, TEXT_COLOR_SYSTEM_GRAY, GetStringFromIndex(0x5a8)); // TODO: msgid "Nothing[.]"
        PutText(textBase, tm + 3);
        return;
    }

    for (i = yLines; (i < yLines + 7) && (i < gUnknown_02012F56); i++) {
        struct Text* th = textBase + (i & 7);
        int item = gPrepScreenItemList[i].item;

        ClearText(th);
        Text_InsertDrawString(th, 0, TEXT_COLOR_SYSTEM_WHITE, GetItemName(item));
        DrawIcon(tm + TILEMAP_INDEX(1, i*2 & 0x1f), GetItemIconId(item), 0x4000);
        PutText(th, tm + TILEMAP_INDEX(3, i*2 & 0x1f));
        PutNumberOrBlank(tm + TILEMAP_INDEX(12, i*2 & 0x1f), TEXT_COLOR_SYSTEM_BLUE, GetItemUses(item));
    }

    return;
}

STATIC_DECLAR void displayScrollBackground_INFUSE(void)
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

    /* Decompress graphics for frames to hold the left side items */
    Decompress(Gfx_UI_Frame_One_Line_1, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x6015C00, 4, 4);
    Decompress(Gfx_UI_Frame_One_Line_2, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x6015C80, 4, 4);
    Decompress(Gfx_UI_Frame_One_Line_3, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x6015D00, 4, 4);

    /* Decomoress graphics for numbers */
    Decompress(Gfx_UI_Number_0, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x6017800, 2, 2);
    Decompress(Gfx_UI_Number_1, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x6017840, 2, 2);
    Decompress(Gfx_UI_Number_2, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x6017880, 2, 2);
    Decompress(Gfx_UI_Number_3, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x60178C0, 2, 2);
    Decompress(Gfx_UI_Number_4, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x6017900, 2, 2);
    Decompress(Gfx_UI_Number_5, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x6017940, 2, 2);
    Decompress(Gfx_UI_Number_6, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x6017980, 2, 2);
    Decompress(Gfx_UI_Number_7, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x60179C0, 2, 2);
    Decompress(Gfx_UI_Number_8, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x6017A00, 2, 2);
    Decompress(Gfx_UI_Number_9, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x6017A40, 2, 2);

    /* Draw dragon egg icon */
    DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 13), GetItemIconId(0xAA), 0x4000);
    BG_EnableSyncByMask(BG0_SYNC_BIT);

    return;
}

STATIC_DECLAR void PrepItemList_DrawCurrentOwnerText_INFUSE(struct PrepItemListProc* proc) {
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

STATIC_DECLAR void List_PutHighlightedCategorySprites_INFUSE(struct PrepItemListProc* proc) {
    int x = proc->currentPage * 12 + 124;

    gPaletteBuffer[0x14D] = *(gUnknown_08A1BD60 + (GetGameClock() >> 2 & 0xf));
    EnablePaletteSync();

    PutSprite(4, x, 24, gUnknown_08A19608[proc->currentPage], OAM2_PAL(3) + OAM2_LAYER(3) + OAM2_CHR(0x280));
    PutSprite(4, x, 24, gUnknown_08A195F8, OAM2_PAL(3) + OAM2_LAYER(3) + OAM2_CHR(0x280));

    UpdateMenuScrollBarConfig(0xc, proc->yOffsetPerPage[proc->currentPage], gUnknown_02012F56, 7);

    return;
}

STATIC_DECLAR void PrepItemList_InitGfx_INFUSE(struct PrepItemListProc * proc)
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

    /* The little highlight on selected weapon icons */
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
    drawItems_INFUSE(
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

    StartParallelWorker(List_PutHighlightedCategorySprites_INFUSE, proc);
    
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
STATIC_DECLAR void sub_809F150_INFUSE(struct PrepItemListProc * proc)
{
    ResetIconGraphics_();

    SomethingPrepListRelated(proc->unit, proc->currentPage, 3);
    sub_809F370(proc);

    drawItems_INFUSE(PrepItemSuppyTexts.th + 7, gBG2TilemapBuffer + 0xF, proc->yOffsetPerPage[proc->currentPage] >> 4, proc->unit);

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

STATIC_DECLAR void PrepItemList_SwitchPageLeft_INFUSE(struct PrepItemListProc * proc)
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
        Proc_Goto(proc, PL_INFUSE_SHOW_CURSOR);
    }

    return;
}

STATIC_DECLAR void PrepItemList_SwitchPageRight_INFUSE(struct PrepItemListProc* proc) {

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
        Proc_Goto(proc, PL_INFUSE_SHOW_CURSOR);
    }

    return;
}

STATIC_DECLAR void PrepItemList_ScrollVertical_INFUSE(struct PrepItemListProc * proc, int amount)
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

STATIC_DECLAR void PerformInfusion(struct PrepItemListProc* proc, int idx, u8 target, u8 cost) {
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

STATIC_DECLAR void PrepItemList_Loop_MainKeyHandler_INFUSE(struct PrepItemListProc * proc)
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

            /* Overwrite last durability value and display nothing */
            TileMap_FillRect(gBG0TilemapBuffer + (9 * 32) + 12, 3, 1, 0);
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
            // PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 9, 13), TEXT_COLOR_SYSTEM_WHITE, gInfusionLookupTable[itemId].cost);

            switch (gInfusionLookupTable[itemId].cost)
            {
            case 0:
                PutSprite(1, 70, 102, gObject_16x16,  OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x3C0));
                break;
            case 1:
                PutSprite(1, 70, 102, gObject_16x16,  OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x3C2));
                break;
            case 2:
                PutSprite(1, 70, 102, gObject_16x16,  OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x3C4));
                break;
            case 3:
                PutSprite(1, 70, 102, gObject_16x16,  OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x3C6));
                break;
            case 4:
                PutSprite(1, 70, 102, gObject_16x16,  OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x3C8));
                break;
            case 5:
                PutSprite(1, 70, 102, gObject_16x16,  OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x3CA));
                break;
            case 6:
                PutSprite(1, 70, 102, gObject_16x16,  OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x3CC));
                break;
            case 7:
                PutSprite(1, 70, 102, gObject_16x16,  OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x3CE));
                break;
            case 8:
                PutSprite(1, 70, 102, gObject_16x16,  OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x3D0));
                break;
            case 9:
                PutSprite(1, 70, 102, gObject_16x16,  OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x3D2));
                break;
            }

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
                Proc_Goto(proc, PL_INFUSE_PRESS_B);
              //  StartPrepAtMenuWithConfig();
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
                Proc_Goto(proc, PL_INFUSE_PRESS_LEFT);
                proc->unk_32 = 0;
                PrepItemList_SwitchPageLeft_INFUSE(proc);
                return;
            }
            if (gKeyStatusPtr->repeatedKeys & DPAD_RIGHT && gInfuseMenuArray[4] == 0) {
                SetUiSpinningArrowFastMaybe(1);
                PlaySoundEffect(SONG_SE_SYS_CURSOR_LR1);
                Proc_Goto(proc, PL_INFUSE_PRESS_RIGHT);
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
    
    // --- ADD THESE LINES TO RESTORE ICON VRAM AFTER POPUP IS SHOWN - THIS NEEDS WORKSHOPPING ---
    // ResetIconGraphics_(); 
    // SomethingPrepListRelated(proc->unit, proc->currentPage, 3);
    // drawItems_INFUSE(
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

STATIC_DECLAR void PrepItemList_OnEnd_INFUSE(struct PrepItemListProc * proc)
{
    struct ProcAtMenu *pproc = proc->proc_parent;

    pproc->state = 1;

    EndAllParallelWorkers();
    EndAllProcChildren(proc);
    EndFaceById(0);
    EndMuralBackground_();

    return;
}

STATIC_DECLAR struct ProcCmd const ProcScr_PrepItemListScreen_INFUSE[] = {
    PROC_NAME("PrepItemListScreen_INFUSE"),
    PROC_YIELD,
    PROC_SET_END_CB(PrepItemList_OnEnd_INFUSE),

PROC_LABEL(PL_INFUSE_INIT),
    PROC_CALL(PrepItemList_Init),
    PROC_CALL(PrepItemList_InitGfx_INFUSE),
	PROC_CALL_ARG(NewFadeIn, 0x10),
    PROC_WHILE(FadeInExists),

PROC_LABEL(PL_INFUSE_SHOW_CURSOR),
    PROC_CALL(sub_809F5F4),

PROC_LABEL(PL_INFUSE_IDLE),
    PROC_REPEAT(PrepItemList_Loop_MainKeyHandler_INFUSE),

PROC_LABEL(PL_INFUSE_REFRESH_VIEW),
    PROC_CALL_ARG(NewFadeOut, 0x10),
    PROC_WHILE(FadeOutExists),
    PROC_CALL(PrepItemList_OnEnd_INFUSE),
    PROC_SLEEP(0),
    PROC_GOTO(PL_INFUSE_IDLE),

PROC_LABEL(PL_INFUSE_SHOW_INVENTORY),
    PROC_CALL(PrepItemList_SwitchToUnitInventory),
    PROC_REPEAT(PrepItemList_Loop_UnitInvKeyHandler),
    PROC_GOTO(PL_INFUSE_SHOW_CURSOR),

PROC_LABEL(PL_INFUSE_PRESS_LEFT),
    PROC_REPEAT(PrepItemList_SwitchPageLeft_INFUSE),
    PROC_GOTO(PL_INFUSE_IDLE),

PROC_LABEL(PL_INFUSE_PRESS_RIGHT),
    PROC_REPEAT(PrepItemList_SwitchPageRight_INFUSE),
    PROC_GOTO(PL_INFUSE_IDLE),

PROC_LABEL(PL_INFUSE_PRESS_B),
    PROC_CALL_ARG(NewFadeOut, 0x10),
    PROC_WHILE(FadeOutExists),

PROC_LABEL(PL_INFUSE_END),
    PROC_END
};

void StartInfuseScreen(struct ProcAtMenu *pproc)
{
    Proc_StartBlocking(ProcScr_PrepItemListScreen_INFUSE, pproc);
    return;
}