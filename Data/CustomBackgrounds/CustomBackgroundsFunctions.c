#include "common-chax.h"
#include "jester_headers/custom-structs.h"

//! FE8U = 0x0800E7D0
LYN_REPLACE_CHECK(EventShowTextBgDirect);
u8 EventShowTextBgDirect(u8 mode, u16 bgIndex)
{
    BG_SetPosition(BG_0, 0, 0);
    BG_SetPosition(BG_1, 0, 0);
    BG_SetPosition(BG_2, 0, 0);
    BG_SetPosition(BG_3, 0, 0);

    switch (mode)
    {

        case EVSUBCMD_TEXTSTART:
            return EVC_ERROR;

        case EVSUBCMD_TUTORIALTEXTBOXSTART:
            return EVC_ERROR;

        case EVSUBCMD_SOLOTEXTBOXSTART:
            return EVC_ERROR;

        case EVSUBCMD_0x1A25:
            return EVC_ERROR;

        case EVSUBCMD_REMOVEPORTRAITS:
            // Randomize background (for support viewers)
            if (bgIndex == BG_RANDOM)
                bgIndex = NextRN_N(BG_BLANK);

            // Loading Background Tile Graphics

            Decompress(gConvoBackgroundData_NEW[bgIndex].gfx, (void *)(VRAM + GetBackgroundTileDataOffset(3)));

            // Loading Background Tile Arrangement

            CallARM_FillTileRect(
                gBG3TilemapBuffer, gConvoBackgroundData_NEW[bgIndex].tsa,
                0x8000 // base palette is bg palette 8
            );

            // Loading Background Palettes

            ApplyPalettes(gConvoBackgroundData_NEW[bgIndex].pal, 8, 8);

            BG_EnableSyncByMask(BG3_SYNC_BIT);
            EnablePaletteSync();

            gPaletteBuffer[0] = 0;

            return EVC_ADVANCE_YIELD;

        case EVSUBCMD_0x1A22:
            DisplayCGfx(gBG3TilemapBuffer, GetBackgroundTileDataOffset(BG_3), 8, 8, bgIndex);

            BG_EnableSyncByMask(BG3_SYNC_BIT);
            EnablePaletteSync();

            return EVC_ADVANCE_YIELD;

        default:
            return EVC_ADVANCE_YIELD;

    } // switch (mode)
}

//! FE8U = 0x0800EC50
LYN_REPLACE_CHECK(sub_800EC50);
void sub_800EC50(struct ConvoBackgroundFadeProc * proc)
{
    switch (proc->unkType)
    {
        case 0:
        case 3:
        case 4:
        case 5:
            while (1)
            {
            } // oh

        case 1:
            if (proc->bgIndex == BG_RANDOM)
                proc->bgIndex = NextRN_N(BG_BLANK);

            // Loading Background Tile Graphics

            Decompress(
                gConvoBackgroundData_NEW[proc->bgIndex].gfx, (void *)(VRAM + GetBackgroundTileDataOffset(BG_2)));

            // Loading Background Tile Arrangement

            CallARM_FillTileRect(
                gBG2TilemapBuffer, gConvoBackgroundData_NEW[proc->bgIndex].tsa,
                0 // base palette is bg palette 0
            );

            // Loading Background Palettes

            ApplyPalettes(gConvoBackgroundData_NEW[proc->bgIndex].pal, 0, 6);

            BG_EnableSyncByMask(BG2_SYNC_BIT);
            EnablePaletteSync();

            gPaletteBuffer[0] = 0;

            break;

        case 2:
            DisplayCGfx(gBG2TilemapBuffer, GetBackgroundTileDataOffset(BG_2), 0, 6, proc->bgIndex);

            BG_EnableSyncByMask(BG2_SYNC_BIT);
            EnablePaletteSync();

            break;
    }

    SetDispEnable(FALSE, FALSE, TRUE, TRUE, TRUE);
}

//! FE8U = 0x0800ED50
LYN_REPLACE_CHECK(sub_800ED50);
void sub_800ED50(struct ConvoBackgroundFadeProc * proc)
{
    switch (proc->unkType)
    {
        case 0:
        case 3:
        case 4:
        case 5:
            while (1)
            {
            } // oh

        case 1:
            if (proc->bgIndex == BG_RANDOM)
                proc->bgIndex = NextRN_N(BG_BLANK);

            // Loading Background Tile Graphics

            Decompress(
                gConvoBackgroundData_NEW[proc->bgIndex].gfx, (void *)(VRAM + GetBackgroundTileDataOffset(BG_3)));

            // Loading Background Tile Arrangement

            CallARM_FillTileRect(
                gBG3TilemapBuffer, gConvoBackgroundData_NEW[proc->bgIndex].tsa,
                0x8000 // base palette is bg palette 8
            );

            // Loading Background Palettes

            ApplyPalettes(gConvoBackgroundData_NEW[proc->bgIndex].pal, 8, 6);

            BG_EnableSyncByMask(BG3_SYNC_BIT);
            EnablePaletteSync();

            gPaletteBuffer[0] = 0;

            break;

        case 2:
            DisplayCGfx(gBG3TilemapBuffer, GetBackgroundTileDataOffset(BG_3), 8, 6, proc->bgIndex);

            BG_EnableSyncByMask(BG3_SYNC_BIT);
            EnablePaletteSync();

            break;
    }

    SetDispEnable(FALSE, FALSE, TRUE, TRUE, TRUE);
}