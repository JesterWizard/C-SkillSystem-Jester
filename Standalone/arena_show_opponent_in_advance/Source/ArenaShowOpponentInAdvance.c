#include "global.h"
#include "bmarena.h"
#include "bmitem.h"
#include "bmlib.h"
#include "hardware.h"
#include "fontgrp.h"
#include "uiutils.h"
#include "scene.h"
#include "variables.h"

static void DrawOpponentDetailsWindow(void)
{
	DrawUiFrame2(7, 9, 0x10, 8, 0);
	SetTextFont(0);
	InitSystemTextFont();

	PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 8, 10), 0, GetStringFromIndex(gMid_Lv));
	PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 12, 10), 2, gArenaState.opponentUnit->level);
	PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 8, 12), 0,
		GetStringFromIndex(gArenaState.opponentUnit->pCharacterData->nameTextId));
	PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 15, 10), 0,
		GetStringFromIndex(gArenaState.opponentUnit->pClassData->nameTextId));
	PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 15, 12), 0, GetItemName(gArenaState.opponentWeapon));

	if (gArenaState.playerPowerWeight - gArenaState.opponentPowerWeight >= 20)
		PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 12, 14), TEXT_COLOR_SYSTEM_GREEN, "Good match");
	else if (gArenaState.opponentPowerWeight - gArenaState.playerPowerWeight <= 20)
		PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 12, 14), TEXT_COLOR_SYSTEM_BLUE, "Okay match");
	else
		PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 12, 14), TEXT_COLOR_SYSTEM_GRAY, "Bad match");
}

void DrawArenaOpponentDetailsText_ShowInAdvance(ProcPtr proc)
{
	(void)proc;
	DrawOpponentDetailsWindow();
}

void ArenaUi_WagerGoldDialogue_ShowInAdvance(ProcPtr proc)
{
	DrawArenaOpponentDetailsText_ShowInAdvance(proc);
	SetTalkNumber(ArenaGetMatchupGoldValue());
	StartArenaDialogue(0x8D2, proc);
}
