#include "common-chax.h"
#include "kernel/realtime-battle.h"

LYN_REPLACE_CHECK(CpPerform_Cleanup);
void CpPerform_Cleanup(struct CpPerformProc *proc)
{
	struct Unit *subject;
	int cleanupX;
	int cleanupY;

	UpdateAllPhaseHealingAIStatus();

#if !CHAX
	AiRefreshMap();
#else
	subject = GetUnit(gActionData.subjectIndex);

	/*
	 * Faction-change post-actions clear the original subject slot and retarget
	 * gActiveUnit. Prefer the living actor when the stored subject is gone.
	 */
	if (!UNIT_IS_VALID(subject) && UNIT_IS_VALID(gActiveUnit))
		subject = gActiveUnit;

	if (!UNIT_IS_VALID(subject)) {
		Proc_Goto(proc, 1);
		return;
	}

	gActiveUnit = subject;
	gActiveUnitId = subject->index;
	gActionData.subjectIndex = subject->index;

	/* Prefer action coords; post-action skills may have rewritten them. */
	cleanupX = gActionData.xMove;
	cleanupY = gActionData.yMove;

	if (gActionData.unitActionType == UNIT_ACTION_TRAPPED) {
		cleanupX = gActiveUnit->xPos;
		cleanupY = gActiveUnit->yPos;
	}

	if (cleanupX < 0 || cleanupY < 0
		|| cleanupX >= gBmMapSize.x || cleanupY >= gBmMapSize.y) {
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
