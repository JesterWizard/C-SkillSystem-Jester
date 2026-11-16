#include "common-chax.h"
#include "kernel-lib.h"
#include "main.h"

#define CGC_TILE_COUNT 640               // 32x20 8bpp tiles
#define CGC_SOLID_MAGIC 0x53534743       // "CGSS", Tools/gallery/png2cg.py's pack_solid

/**
 * The solid-tile format: "CGSS", a bit per tile (set: the tile is one colour), a colour byte per such tile, padding
 * to 4, then the other tiles LZ77 compressed. Those are decompressed to the start of vram, then moved to their places
 * from the last tile back (a tile only ever moves forward), filling in the solid ones.
 */
static void CGC_LoadSolidTileGfx(const u8 *src, u32 *vram)
{
	const u8 *bits = src + 4;
	const u8 *colours = bits + CGC_TILE_COUNT / 8;
	int solid = 0, from, k, j;

	for (k = 0; k < CGC_TILE_COUNT; k++)
		if (bits[k >> 3] & (1 << (k & 7)))
			solid++;

	from = CGC_TILE_COUNT - solid - 1;
	if (from >= 0)
		Decompress(src + ((4 + CGC_TILE_COUNT / 8 + solid + 3) & ~3), vram);

	for (k = CGC_TILE_COUNT - 1; k >= 0; k--) {
		u32 *dst = vram + k * 16;

		if (bits[k >> 3] & (1 << (k & 7))) {
			u32 fill = colours[--solid] * 0x01010101;

			for (j = 0; j < 16; j++)
				dst[j] = fill;
		} else {
			if (from != k) {
				const u32 *tile = vram + from * 16;

				for (j = 0; j < 16; j++)
					dst[j] = tile[j];
			}
			from--;
		}
	}
}

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
  if (*(const u32 *)bgData->gfx == CGC_SOLID_MAGIC)
    CGC_LoadSolidTileGfx(bgData->gfx, (u32 *)0x6004000);
  else
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