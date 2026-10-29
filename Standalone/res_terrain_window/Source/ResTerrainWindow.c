#include "global.h"
#include "player_interface.h"
#include "bmmap.h"
#include "bmlib.h"
#include "bmtrick.h"
#include "fontgrp.h"
#include "hardware.h"
#include "functions.h"
#include "constants/terrains.h"

extern const u16 NewTerrainWindow_TSA[];
extern const u16 NewBrokenWall_TSA[];
extern const u16 NewTerrainWindowText_TSA[];

static void PutTerrainBonusDigits(u16 *tm, int value)
{
	StoreNumberStringToSmallBuffer(value);
	PutDigits(tm, (const u8 *)gNumberStr + 7, TILEREF(0x128, 2), 2);
}

/**
 * Replaces vanilla DrawTerrainDisplayWindow (FE8U 0x0808C964): the terrain
 * window gets a taller box with DEF, RES and AVO rows.
 */
void DrawTerrainDisplayWindow_Res(struct PlayerInterfaceProc *proc)
{
	char *str;
	int num;
	int terrainId = gBmMapTerrain[gBmSt.playerCursor.y][gBmSt.playerCursor.x];

	TileMap_FillRect(gUiTmScratchA + TILEMAP_INDEX(0, 10), 14, 7, 0);
	TileMap_FillRect(gUiTmScratchB + TILEMAP_INDEX(0, 10), 14, 7, 0);

	str = GetTerrainName(terrainId);
	num = GetStringTextCenteredPos(40, str);

	ClearText(proc->texts);
	Text_SetParams(proc->texts, num, TEXT_COLOR_SYSTEM_WHITE);
	Text_DrawString(proc->texts, str);
	PutText(proc->texts, gUiTmScratchA + TILEMAP_INDEX(1, 11));

	CallARM_FillTileRect(gUiTmScratchA + TILEMAP_INDEX(1, 13), NewTerrainWindowText_TSA, TILEREF(0x100, 2));

	if (TerrainTable_MovCost_BerserkerNormal[terrainId] > 0) {
		PutTerrainBonusDigits(gUiTmScratchA + TILEMAP_INDEX(5, 13), TerrainTable_Def_Common[terrainId]);
		PutTerrainBonusDigits(gUiTmScratchA + TILEMAP_INDEX(5, 14), TerrainTable_Res_Common[terrainId]);
		PutTerrainBonusDigits(gUiTmScratchA + TILEMAP_INDEX(5, 15), TerrainTable_Avo_Common[terrainId]);
	}

	switch (terrainId) {
	case TERRAIN_SNAG:
	case TERRAIN_WALL_DAMAGED:
		CallARM_FillTileRect(gUiTmScratchA + TILEMAP_INDEX(1, 13), NewBrokenWall_TSA, TILEREF(0x100, 2));

		num = GetObstacleHpAt(gBmSt.playerCursor.x, gBmSt.playerCursor.y);

		if (num == 100)
			CallARM_FillTileRect(gUiTmScratchA + TILEMAP_INDEX(4, 15), Tsa_TerrainMapUi_ObstacleFullHp, TILEREF(0x100, 0));
		else
			PutTerrainBonusDigits(gUiTmScratchA + TILEMAP_INDEX(5, 15), num);

		break;

	case TERRAIN_BALLISTA_REGULAR:
	case TERRAIN_BALLISTA_LONG:
	case TERRAIN_BALLISTA_KILLER:
		CallARM_FillTileRect(gUiTmScratchA + TILEMAP_INDEX(1, 14), gTSA_TerrainBox_Ballistae, TILEREF(0x100, 0));
		PutTerrainBonusDigits(gUiTmScratchA + TILEMAP_INDEX(5, 14),
				     GetObstacleHpAt(gBmSt.playerCursor.x, gBmSt.playerCursor.y));
		break;
	}

	CallARM_FillTileRect(gUiTmScratchB + TILEMAP_INDEX(0, 10), NewTerrainWindow_TSA, TILEREF(0x0, 1));
}
