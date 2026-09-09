#include "common-chax.h"
#include "kernel-lib.h"
#include "player_interface.h"
#include "bmmap.h"
#include "bmlib.h"
#include "bmtrick.h"
#include "fontgrp.h"
#include "constants/terrains.h"

extern const u16 NewTerrainWindow_TSA[];
extern const u16 NewBrokenWall_TSA[];
extern const u16 NewTerrainWindowText_TSA[];

static void PutTerrainBonusDigits(u16 *tm, int value)
{
	StoreNumberStringToSmallBuffer(value);
	PutDigits(tm, (const u8 *)gNumberStr + 7, TILEREF(0x128, 2), 2);
}

LYN_REPLACE_CHECK(DrawTerrainDisplayWindow);
void DrawTerrainDisplayWindow(struct PlayerInterfaceProc *proc)
{
	char *str;
	int num;
	int terrainId = gBmMapTerrain[gBmSt.playerCursor.y][gBmSt.playerCursor.x];
	bool res_window = gpKernelDesignerConfig->res_terrain_window != false;
	int name_y = res_window ? 11 : 12;
	int label_y = res_window ? 13 : 14;
	const u16 *label_tsa = res_window ? NewTerrainWindowText_TSA : Tsa_TerrainMapUi_Labels;
	const u16 *obstacle_tsa = res_window ? NewBrokenWall_TSA : Tsa_TerrainMapUi_ObstacleLabels;
	const u16 *box_tsa = res_window ? NewTerrainWindow_TSA : gTSA_TerrainBox;
	int box_y = res_window ? 10 : 11;

	TileMap_FillRect(gUiTmScratchA + TILEMAP_INDEX(0, 10), 14, 7, 0);
	TileMap_FillRect(gUiTmScratchB + TILEMAP_INDEX(0, 10), 14, 7, 0);

	str = GetTerrainName(terrainId);
	num = GetStringTextCenteredPos(40, str);

	ClearText(proc->texts);
	Text_SetParams(proc->texts, num, TEXT_COLOR_SYSTEM_WHITE);
	Text_DrawString(proc->texts, str);
	PutText(proc->texts, gUiTmScratchA + TILEMAP_INDEX(1, name_y));

	CallARM_FillTileRect(gUiTmScratchA + TILEMAP_INDEX(1, label_y), label_tsa, TILEREF(0x100, 2));

	if (TerrainTable_MovCost_BerserkerNormal[terrainId] > 0) {
		if (res_window) {
			PutTerrainBonusDigits(gUiTmScratchA + TILEMAP_INDEX(5, 13), TerrainTable_Def_Common[terrainId]);
			PutTerrainBonusDigits(gUiTmScratchA + TILEMAP_INDEX(5, 14), TerrainTable_Res_Common[terrainId]);
			PutTerrainBonusDigits(gUiTmScratchA + TILEMAP_INDEX(5, 15), TerrainTable_Avo_Common[terrainId]);
		} else {
			PutTerrainBonusDigits(gUiTmScratchA + TILEMAP_INDEX(5, 14), TerrainTable_Def_Common[terrainId]);
			PutTerrainBonusDigits(gUiTmScratchA + TILEMAP_INDEX(5, 15), TerrainTable_Avo_Common[terrainId]);
		}
	}

	switch (terrainId) {
	case TERRAIN_SNAG:
	case TERRAIN_WALL_DAMAGED:
		CallARM_FillTileRect(gUiTmScratchA + TILEMAP_INDEX(1, label_y), obstacle_tsa, TILEREF(0x100, 2));

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

	CallARM_FillTileRect(gUiTmScratchB + TILEMAP_INDEX(0, box_y), box_tsa, TILEREF(0x0, 1));
}
