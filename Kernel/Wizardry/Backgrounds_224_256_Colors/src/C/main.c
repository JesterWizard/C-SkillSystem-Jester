// #include <stdio.h>
#include "main.h"
#include "common-chax.h"

// Set 256-col or 224-col BG.
void CGC_LoadMultiPalBG(struct BGData* bgData, u32 colCount) {
  // Init LCDIO stuff.
  BG_SetPosition(0, 0, 0);
  BG_SetPosition(1, 0, 0);
  BG_SetPosition(2, 0, 0);
  BG_SetPosition(3, 0, 0);
  SetBackgroundMapDataOffset(0, 0xE000);
  SetBackgroundMapDataOffset(1, 0xE800);
  SetBackgroundMapDataOffset(2, 0xF000);
  SetBackgroundMapDataOffset(3, 0xF800);
  SetBackgroundTileDataOffset(0, 0);
  SetBackgroundTileDataOffset(1, 0);
  SetBackgroundTileDataOffset(2, 0);
  SetBackgroundTileDataOffset(3, 0);
  gLCDControlBuffer.bg3cnt.colorMode = 1;                // 256-col mode.
  gLCDControlBuffer.bldcnt.target2_bd_on = false;     // Prevents weird blending into trans colour effect.
  
  // Clear screen entries.
  CpuFill16(0x0, gBG0TilemapBuffer, 0x1800);
  CpuFastFill(0, (void*)0x6000000, 0x20);       // Empty tile.
  
  // Init gfx.
  Decompress(bgData->gfx, (void*)0x6004000);
  for (int i = 0; i < 640; i++)
    gBG3TilemapBuffer[i] = i+256;
  
  // Leave paletteslot 2 and 3 empty for text and chatbubble if 224-col BG.
  if (colCount == 224) {
    CopyToPaletteBuffer(bgData->pal, 0, 0x40);
    CopyToPaletteBuffer(bgData->pal+0x20, 0x80, 0x180);
  }
  else
    CopyToPaletteBuffer(bgData->pal, 0, 0x200);
  
  BG_EnableSyncByMask(0xF);
  EnablePaletteSync();
}