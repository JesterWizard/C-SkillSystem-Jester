#include "common-chax.h"
#include "kernel/realtime-battle.h"

LYN_REPLACE_CHECK(CpPerform_Cleanup);
void CpPerform_Cleanup(struct CpPerformProc *proc)
{
	int cleanupX;
	int cleanupY;

	UpdateAllPhaseHealingAIStatus();

#if !CHAX
	AiRefreshMap();
#else
	gActiveUnit = GetUnit(gActionData.subjectIndex);

	cleanupX = gAiDecision.xMove;
	cleanupY = gAiDecision.yMove;

	if (gActionData.unitActionType == UNIT_ACTION_TRAPPED) {
		cleanupX = gActiveUnit->xPos;
		cleanupY = gActiveUnit->yPos;
	}

	/* Real-time mode leaves the map cursor under the player's control. */
	if (!IsRealtimeBattleActive())
		SetCursorMapPosition(cleanupX, cleanupY);

	RenderBmMapOnBg2();

	MoveActiveUnit(cleanupX, cleanupY);

#if CHAX
	if (gActiveUnit->curHP != 0) {
		switch (gAiDecision.actionId) {
		case CONFIG_AI_ACTION_EXPA_Teleportation:
			gActiveUnit->state &= ~US_UNSELECTABLE;
			break;
		}
	}
#endif

	RefreshEntityBmMaps();
	RenderBmMap();

	NewBMXFADE(1);

	EndAllMus();
	RefreshEntityBmMaps();

	ShowUnitSprite(gActiveUnit);
	RefreshUnitSprites();
#endif

	if (!(gActiveUnit->pCharacterData) || (gActiveUnit->state & (US_HIDDEN | US_DEAD | US_BIT16)))
		Proc_Goto(proc, 1);
}
