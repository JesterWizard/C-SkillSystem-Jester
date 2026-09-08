#include "common-chax.h"
#include "jester_headers/custom-arrays.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/custom-functions.h"

// Helper function to set both BG layers at once
STATIC_DECLAR void SetBothBGTiles(u16* clearBuffer, u16* tileBuffer, int x, int y, u16 modelTile) {
    u16 offset = TILEMAP_INDEX2(x, y);
    clearBuffer[offset] = 0;
    tileBuffer[offset] = modelTile;
}

// Draw a 2x2 tile block
STATIC_DECLAR void Draw2x2Block(u16* clearBuffer, u16* tileBuffer, int x, int y, const u16* model, const u8* indices) {
    SetBothBGTiles(clearBuffer, tileBuffer, x,     y,     model[indices[0]]);
    SetBothBGTiles(clearBuffer, tileBuffer, x + 1, y,     model[indices[1]]);
    SetBothBGTiles(clearBuffer, tileBuffer, x,     y + 1, model[indices[2]]);
    SetBothBGTiles(clearBuffer, tileBuffer, x + 1, y + 1, model[indices[3]]);
}

// Draw vertical edge tiles
STATIC_DECLAR void DrawVerticalEdges(u16* clearBuffer, u16* tileBuffer, int x, int xMax, int iy, const u16* model, bool isTopRow) {
    if (isTopRow) {
        SetBothBGTiles(clearBuffer, tileBuffer, x,    iy,     model[4]);
        SetBothBGTiles(clearBuffer, tileBuffer, xMax, iy,     model[7]);
        SetBothBGTiles(clearBuffer, tileBuffer, x,    iy + 1, model[8]);
        SetBothBGTiles(clearBuffer, tileBuffer, xMax, iy + 1, model[11]);
    } else {
        SetBothBGTiles(clearBuffer, tileBuffer, x,    iy,     model[8]);
        SetBothBGTiles(clearBuffer, tileBuffer, xMax, iy,     model[7]);
        SetBothBGTiles(clearBuffer, tileBuffer, x,    iy + 1, model[8]);
        SetBothBGTiles(clearBuffer, tileBuffer, xMax, iy + 1, model[11]);
    }
}

// Draw horizontal edge tiles
STATIC_DECLAR void DrawHorizontalEdges(u16* clearBuffer, u16* tileBuffer, int ix, int y, int yMax, const u16* model) {
    SetBothBGTiles(clearBuffer, tileBuffer, ix,     y,    model[2]);
    SetBothBGTiles(clearBuffer, tileBuffer, ix + 1, y,    model[2]);
    SetBothBGTiles(clearBuffer, tileBuffer, ix,     yMax, model[13]);
    SetBothBGTiles(clearBuffer, tileBuffer, ix + 1, yMax, model[14]);
}

// Draw the frame interior (style-dependent)
STATIC_DECLAR void DrawFrameInterior(u16* clearBuffer, u16* tileBuffer, int x, int y, int xMax, int yMax, const u16* model, int style) {
    static const u8 normalBlock[] = {6, 6, 9, 10};
    
    for (int iy = y + 1; iy < yMax; iy += 2) {
        bool isTopRow = (style == 3) && (iy == y + 1);
        
        // Draw center blocks
        for (int ix = x + 1; ix < xMax; ix += 2) {
            if (style == 3 && isTopRow) {
                const u8 topBlock[] = {5, 6, 9, 10};
                Draw2x2Block(clearBuffer, tileBuffer, ix, iy, model, topBlock);
            } else {
                Draw2x2Block(clearBuffer, tileBuffer, ix, iy, model, normalBlock);
            }
        }
        
        // Draw vertical edges
        DrawVerticalEdges(clearBuffer, tileBuffer, x, xMax, iy, model, isTopRow && style == 3);
    }
    
    // Draw horizontal edges (skip top edge for style 3)
    if (style != 3) {
        for (int ix = x + 1; ix < xMax; ix += 2) {
            DrawHorizontalEdges(clearBuffer, tileBuffer, ix, y, yMax, model);
        }
    } else {
        for (int ix = x + 1; ix < xMax; ix += 2) {
            SetBothBGTiles(clearBuffer, tileBuffer, ix,     yMax, model[13]);
            SetBothBGTiles(clearBuffer, tileBuffer, ix + 1, yMax, model[14]);
        }
    }
}

// Draw the four corner tiles
STATIC_DECLAR void DrawCorners(u16* clearBuffer, u16* tileBuffer, int x, int y, int xMax, int yMax, const u16* model, int style) {
    // Always clear corners
    clearBuffer[TILEMAP_INDEX2(x,    yMax)] = 0;
    clearBuffer[TILEMAP_INDEX2(xMax, yMax)] = 0;
    
    // Bottom corners (all styles)
    tileBuffer[TILEMAP_INDEX2(x,    yMax)] = model[12];
    tileBuffer[TILEMAP_INDEX2(xMax, yMax)] = model[15];
    
    // Top corners (skip for style 3)
    if (style != 3) {
        clearBuffer[TILEMAP_INDEX2(x,    y)] = 0;
        clearBuffer[TILEMAP_INDEX2(xMax, y)] = 0;
        
        // Top-left is a 2x2 block
        const u8 topLeftBlock[] = {0, 1, 4, 5};
        Draw2x2Block(clearBuffer, tileBuffer, x, y, model, topLeftBlock);
        
        // Top-right is single tile
        tileBuffer[TILEMAP_INDEX2(xMax, y)] = model[3];
    }
}

static const u16 gUnknown_080DA2F4[] = {
    TILEREF(0x01, BGPAL_WINDOW_FRAME),
    TILEREF(0x02, BGPAL_WINDOW_FRAME),
    TILEREF(0x03, BGPAL_WINDOW_FRAME),
    TILEREF(0x05, BGPAL_WINDOW_FRAME),

    TILEREF(0x07, BGPAL_WINDOW_FRAME),
    TILEREF(0x08, BGPAL_WINDOW_FRAME),
    TILEREF(0x09, BGPAL_WINDOW_FRAME),
    TILEREF(0x0A, BGPAL_WINDOW_FRAME),

    TILEREF(0x06, BGPAL_WINDOW_FRAME),
    TILEREF(0x09, BGPAL_WINDOW_FRAME),
    TILEREF(0x09, BGPAL_WINDOW_FRAME),
    TILEREF(0x0A, BGPAL_WINDOW_FRAME),

    TILEREF(0x1A, BGPAL_WINDOW_FRAME),
    TILEREF(0x1B, BGPAL_WINDOW_FRAME),
    TILEREF(0x1B, BGPAL_WINDOW_FRAME),
    TILEREF(0x21, BGPAL_WINDOW_FRAME),
};

static const u16 gUnknown_080DA314[] = {
    TILEREF(0x01, BGPAL_WINDOW_FRAME),
    TILEREF(0x02, BGPAL_WINDOW_FRAME),
    TILEREF(0x03, BGPAL_WINDOW_FRAME),
    TILEREF(0x05, BGPAL_WINDOW_FRAME),

    TILEREF(0x66, BGPAL_WINDOW_FRAME),
    TILEREF(0x67, BGPAL_WINDOW_FRAME),
    TILEREF(0x67, BGPAL_WINDOW_FRAME),
    TILEREF(0x68, BGPAL_WINDOW_FRAME),

    TILEREF(0x06, BGPAL_WINDOW_FRAME),
    TILEREF(0x09, BGPAL_WINDOW_FRAME),
    TILEREF(0x09, BGPAL_WINDOW_FRAME),
    TILEREF(0x0A, BGPAL_WINDOW_FRAME),

    TILEREF(0x1A, BGPAL_WINDOW_FRAME),
    TILEREF(0x1B, BGPAL_WINDOW_FRAME),
    TILEREF(0x1B, BGPAL_WINDOW_FRAME),
    TILEREF(0x21, BGPAL_WINDOW_FRAME),
};

static const u16 gUnknown_080DA334[] = {
    TILEREF(0x72, BGPAL_WINDOW_FRAME),
    TILEREF(0x73, BGPAL_WINDOW_FRAME),
    TILEREF(0x73, BGPAL_WINDOW_FRAME),
    TILEREF(0x74, BGPAL_WINDOW_FRAME),

    TILEREF(0x75, BGPAL_WINDOW_FRAME),
    TILEREF(0x09, BGPAL_WINDOW_FRAME),
    TILEREF(0x09, BGPAL_WINDOW_FRAME),
    TILEREF(0x76, BGPAL_WINDOW_FRAME),

    TILEREF(0x75, BGPAL_WINDOW_FRAME),
    TILEREF(0x09, BGPAL_WINDOW_FRAME),
    TILEREF(0x09, BGPAL_WINDOW_FRAME),
    TILEREF(0x76, BGPAL_WINDOW_FRAME),

    TILEREF(0x77, BGPAL_WINDOW_FRAME),
    TILEREF(0x78, BGPAL_WINDOW_FRAME),
    TILEREF(0x78, BGPAL_WINDOW_FRAME),
    TILEREF(0x79, BGPAL_WINDOW_FRAME),
};

static const u16 gUnknown_080DA354[] = {
    TILEREF(0x01, BGPAL_WINDOW_FRAME),
    TILEREF(0x02, BGPAL_WINDOW_FRAME),
    TILEREF(0x03, BGPAL_WINDOW_FRAME),
    TILEREF(0x05, BGPAL_WINDOW_FRAME),

    TILEREF(0x0C, BGPAL_WINDOW_FRAME),
    TILEREF(0x0D, BGPAL_WINDOW_FRAME),
    TILEREF(0x0E, BGPAL_WINDOW_FRAME),
    TILEREF(0x0F, BGPAL_WINDOW_FRAME),

    TILEREF(0x0B, BGPAL_WINDOW_FRAME),
    TILEREF(0x0E, BGPAL_WINDOW_FRAME),
    TILEREF(0x0E, BGPAL_WINDOW_FRAME),
    TILEREF(0x0F, BGPAL_WINDOW_FRAME),

    TILEREF(0x1A, BGPAL_WINDOW_FRAME),
    TILEREF(0x1B, BGPAL_WINDOW_FRAME),
    TILEREF(0x1B, BGPAL_WINDOW_FRAME),
    TILEREF(0x21, BGPAL_WINDOW_FRAME),
};

static const u16* sUiFrameModelTilemapLookup[] = {
    gUnknown_080DA2F4,
    gUnknown_080DA334,
    gUnknown_080DA354,
    gUnknown_080DA314,
};

// 0804e368
LYN_REPLACE_CHECK(DrawUiFrame2);
void DrawUiFrame2(int x, int y, int width, int height, int style)
{    
    const u16* model = sUiFrameModelTilemapLookup[style];
    int xMax = x + width  - 1;
    int yMax = y + height - 1;

    if (Proc_Find(ProcScr_PrepItemListScreen_INFUSE))
    {
        DrawFrameInterior(gBG0TilemapBuffer, gBG0TilemapBuffer, x, y, xMax, yMax, model, style);
        DrawCorners(gBG0TilemapBuffer, gBG0TilemapBuffer, x, y, xMax, yMax, model, style);
    }
    else
    {
        DrawFrameInterior(gBG0TilemapBuffer, gBG1TilemapBuffer, x, y, xMax, yMax, model, style);
        DrawCorners(gBG0TilemapBuffer, gBG1TilemapBuffer, x, y, xMax, yMax, model, style); 
    }

    BG_SetPosition(0, 0, 0);
    BG_SetPosition(1, 0, 0);
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

LYN_REPLACE_CHECK(PopupProc_GfxClear);
void PopupProc_GfxClear(struct PopupProc * proc)
{
    TileMap_FillRect(
        TILEMAP_LOCATED(gBG0TilemapBuffer, proc->xTileReal, proc->yTileReal),
        proc->xTileSize, proc->yTileSize, 0);

    if (Proc_Find(ProcScr_PrepItemListScreen_INFUSE))
    {
        // struct PrepItemListProc * proc_infuse = Proc_Find(ProcScr_PrepItemListScreen_INFUSE);

        ShowSysHandCursor(gInfuseMenuArray[2], gInfuseMenuArray[3], 0xB, 0x800);

        // ResetIconGraphics_();
        // drawItems_INFUSE(
        //     PrepItemSuppyTexts.th + 7,
        //     gBG2TilemapBuffer + 0xF,
        //     proc_infuse->yOffsetPerPage[proc_infuse->currentPage] >> 4,
        //     proc_infuse->unit
        // );
        // BG_EnableSyncByMask(BG2_SYNC_BIT);
    }
    else
    {
        TileMap_FillRect(
            TILEMAP_LOCATED(gBG1TilemapBuffer, proc->xTileReal, proc->yTileReal),
            proc->xTileSize, proc->yTileSize, 0);
    }

    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static void PutInfuseWeaponTextSprite(void)
{
    struct PopupProc* proc = Proc_Find(ProcScr_Popup);
    int base_x = ((proc->xTileReal + 1) * 8) + 2;
    PutSpriteExt(4, base_x, 72, gObject_64x32, OAM2_PAL(11) + OAM2_LAYER(0) + OAM2_CHR(0x90));
    PutSpriteExt(4, base_x + 40, 72, gObject_64x32, OAM2_PAL(11) + OAM2_LAYER(0) + OAM2_CHR(0x97));
}

LYN_REPLACE_CHECK(PopupProc_GfxDraw);
void PopupProc_GfxDraw(struct PopupProc * proc)
{
    struct Text th;
    int icon_pos;
    int tile_len;
    int x_pos, y_pos;
    int temp;

    u32 len;

    /* When inside the infuse prep screen, redirect the popup icon to OBJ palette 2
       instead of the default OBJ palette 0.  OBJ palette 0 is used by DrawCostSprite
       (the shard-cost number sprites); clobbering it with the icon palette would make
       those number sprites display with icon colours after the popup closes. */
    if (Proc_Find(ProcScr_PrepItemListScreen_INFUSE))
        proc->iconPalId = 0x12;  /* OBJ palette 2 — not otherwise used in the infuse screen */

    len = ParsePopupInstAndGetLen(proc);
    proc->xGfxSize = len;
    tile_len = (len << 0x10) >> 0x13;

    if (0 != (len & 7))
        tile_len++;

    icon_pos = (tile_len * 8 - proc->xGfxSize) >> 1;

    if (-1 == proc->xTileParam)
        x_pos = ((0x1E - tile_len) >> 1) - 1;
    else
        x_pos = proc->xTileParam;


    if (-1 != proc->yTileParam)
        y_pos = proc->yTileParam;
    else
        y_pos = 8;

    temp = tile_len + 2;
    DrawUiFrame2(x_pos, y_pos, temp, 4, proc->winStyle);

    proc->xTileReal = x_pos;
    proc->yTileReal = y_pos;
    proc->xTileSize = temp;
    proc->yTileSize = 3;
    proc->iconX += icon_pos;

    InitText(&th, tile_len);
    Text_SetColor(&th, proc->textColorId);
    Text_SetCursor(&th, icon_pos);
    GeneratePopupText(proc->pDefinition, th);

    if (0xFFFF != proc->iconId)
        LoadIconObjectGraphics(proc->iconId, proc->iconObjTileId);

    if (Proc_Find(ProcScr_PrepItemListScreen_INFUSE))
    {
        struct PrepItemListProc * procInfuse = Proc_Find(ProcScr_PrepItemListScreen_INFUSE);

        StartParallelWorker(PutInfuseWeaponTextSprite, proc);
        ResetText();
        ResetIconGraphics_();

        drawItems_INFUSE(
            PrepItemSuppyTexts.th + 7,
            gBG2TilemapBuffer + 0xF,
            (procInfuse->yOffsetPerPage[procInfuse->currentPage]) >> 4,
            procInfuse->unit
        );

        /* Draw dragon egg icon */
        DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 13), GetItemIconId(0xAA), 0x4000);

        /* Re-draw BG0 selected-item and infuse-target icons after ResetIconGraphics_()
           resets the tile pool, otherwise their tile references point to stale data. */
        if (gUnknown_02012F56 > 0) {
            int idx = procInfuse->idxPerPage[procInfuse->currentPage];
            u16 selItem = gPrepScreenItemList[idx].item;
            u8 selItemId = ITEM_INDEX(selItem);
            u8 tgt = gInfusionLookupTable[selItemId].targetItemId;
            DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 9),  GetItemIconId(selItem), 0x4000);
            if (tgt != 0)
                DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 17), GetItemIconId(tgt), 0x4000);
        }
    }
    else
    {
        PutText(&th, TILEMAP_LOCATED(gBG0TilemapBuffer, x_pos + 1, y_pos + 1));
        ResetText();
    }

    if (0xFFFF != proc->iconId) {
        struct PopupIconUpdateProc *child =
            Proc_Start(ProcScr_PopupUpdateIcon, proc);

        child->unk_2C = (proc->xTileReal + 1) * 8 + proc->iconX;
        child->unk_30 = (proc->yTileReal + 1) * 8;
        child->unk_4A = proc->iconObjTileId | (proc->iconPalId & 0xf) << 0xC;
    }
}