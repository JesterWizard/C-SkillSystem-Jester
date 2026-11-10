#include "common-chax.h"
#include "district.h"

/*
 * World-map district markers (#710): a small gold banner beside each shown node whose district is unlocked, so the
 * player sees where Enter District is. Drawn while the world map is idle (WorldMap_LoopExt), with a slow bob.
 *
 * The tiles and the palette go to slots the idle world map leaves empty (OBJ chr 0x3D0, 0x3D1, 0x3F0 and 0x3F1, a
 * 16x16 sprite in the world map's 2D OBJ mapping; OBJ palette 8: found by dumping OBJ VRAM and palette RAM there).
 * They are written again each frame, so a screen that borrows them in between (a menu, a shop) leaves nothing behind.
 */

#define DISTRICT_MARKER_CHR 0x3D0
#define DISTRICT_MARKER_PAL 8
#define OBJ_CHR_ROW 0x20

/* 4bpp, 16x16 (top-left, top-right, bottom-left, bottom-right): 1 outline, 2 gold, 3 light gold, 4 red */
static const u32 sDistrictMarkerChr[4][8] = {
	{ 0x00000000, 0x00000110, 0x11111310, 0x22222310, 0x33332310, 0x44432310, 0x14432310, 0x14432310 },
	{ 0x00000000, 0x00000000, 0x00011111, 0x01222222, 0x01223333, 0x01223444, 0x01223441, 0x01223441 },
	{ 0x44432310, 0x33332310, 0x22222310, 0x11111310, 0x00000310, 0x00000310, 0x00000310, 0x00000110 },
	{ 0x01223444, 0x01223333, 0x01222222, 0x00011111, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
};

static const u16 sDistrictMarkerPal[16] = {
	0x0000, 0x0888, 0x1ADD, 0x57DF, 0x14F9,
};

/* Up and left of the node's centre (gWMNodeData x, y), clear of the lord standing on it and of the cursor */
#define DISTRICT_MARKER_DX -24
#define DISTRICT_MARKER_DY -24

static void District_LoadWorldMapMarker(void)
{
	u8 *vram = (u8 *)OBJ_VRAM0 + DISTRICT_MARKER_CHR * CHR_SIZE;

	CpuCopy32(sDistrictMarkerChr[0], vram, CHR_SIZE);
	CpuCopy32(sDistrictMarkerChr[1], vram + CHR_SIZE, CHR_SIZE);
	CpuCopy32(sDistrictMarkerChr[2], vram + OBJ_CHR_ROW * CHR_SIZE, CHR_SIZE);
	CpuCopy32(sDistrictMarkerChr[3], vram + (OBJ_CHR_ROW + 1) * CHR_SIZE, CHR_SIZE);
	CpuCopy16(sDistrictMarkerPal, gPaletteBuffer + 0x100 + 0x10 * DISTRICT_MARKER_PAL, sizeof(sDistrictMarkerPal));
	EnablePaletteSync();
}

void District_DrawWorldMapMarkers(void)
{
	const struct DistrictNode *it;
	int x, y, bob;
	bool loaded = false;

	bob = (GetGameClock() >> 5) & 1;

	for (it = gDistrictNodes; it->node != DISTRICT_NODE_END; it++) {
		if (it->node >= NODE_MAX || !(gGMData.nodes[it->node].state & 1) || !District_IsUnlocked(it))
			continue;

		x = gWMNodeData[it->node].x - gGMData.xCamera + DISTRICT_MARKER_DX;
		y = gWMNodeData[it->node].y - gGMData.yCamera + DISTRICT_MARKER_DY - bob;
		if (x < -16 || x >= DISPLAY_WIDTH || y < -16 || y >= DISPLAY_HEIGHT)
			continue;

		if (!loaded) {
			District_LoadWorldMapMarker();
			loaded = true;
		}

		PutSprite(4, OAM1_X(x & 0x1FF), OAM0_Y(y & 0xFF), gObject_16x16,
			  OAM2_PAL(DISTRICT_MARKER_PAL) + OAM2_LAYER(0b01) + OAM2_CHR(DISTRICT_MARKER_CHR));
	}
}
