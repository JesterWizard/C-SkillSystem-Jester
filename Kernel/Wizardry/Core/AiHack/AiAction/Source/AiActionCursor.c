#include "common-chax.h"
#include "kernel/realtime-battle.h"

LYN_REPLACE_CHECK(AiRefreshMap);
void AiRefreshMap(void)
{
	gActiveUnit = GetUnit(gActionData.subjectIndex);

	/*
	 * This is what drags the map cursor onto the acting AI unit. In real-time
	 * mode the player still owns that cursor, so leave it where they put it.
	 */
	if (!IsRealtimeBattleActive())
		SetCursorMapPosition(gAiDecision.xMove, gAiDecision.yMove);

	RenderBmMapOnBg2();
	MoveActiveUnit(gAiDecision.xMove, gAiDecision.yMove);
	RefreshEntityBmMaps();
	RenderBmMap();
	NewBMXFADE(1);
	EndAllMus();
	RefreshEntityBmMaps();
	ShowUnitSprite(gActiveUnit);
	RefreshUnitSprites();
}

LYN_REPLACE_CHECK(CpPerform_MoveCameraOntoTarget);
void CpPerform_MoveCameraOntoTarget(struct CpPerformProc *proc)
{
	struct Unit *unit;

	int x = 0;
	int y = 0;

	if (gActionData.unitActionType == UNIT_ACTION_TRAPPED)
		return;

	switch (gAiDecision.actionId) {
	case AI_ACTION_NONE:
	case AI_ACTION_ESCAPE:
	case AI_ACTION_PILLAGE:
	case AI_ACTION_USEITEM:
	case AI_ACTION_RIDEBALLISTA:
	case AI_ACTION_EXITBALLISTA:
	case AI_ACTION_DKNIGHTMARE:
	case AI_ACTION_DKSUMMON:
	case AI_ACTION_PICK:

#if CHAX
	case CONFIG_AI_ACTION_EXPA_Teleportation:
	case CONFIG_AI_ACTION_EXPA_MenuSkill:
	default:
#endif
		return;

	case AI_ACTION_COMBAT:
		if (gAiDecision.targetId == 0) {
			x = gAiDecision.xTarget;
			y = gAiDecision.yTarget;
		} else {
			unit = GetUnit(gAiDecision.targetId);
			x = unit->xPos;
			y = unit->yPos;
		}

		if (((s8)gAiDecision.itemSlot == -1) && !(gActiveUnit->state & US_IN_BALLISTA)) {
			EndAllMus();

			gActiveUnit->xPos = gAiDecision.xMove;
			gActiveUnit->yPos = gAiDecision.yMove;

			RideBallista(gActiveUnit);

			StartMu(gActiveUnit);
			SetAutoMuDefaultFacing();
		}

		break;

	case AI_ACTION_STEAL:
		unit = GetUnit(gAiDecision.targetId);

		x = unit->xPos;
		y = unit->yPos;

		break;

	case AI_ACTION_REFRESH:
		unit = GetUnit(gAiDecision.targetId);

		x = unit->xPos;
		y = unit->yPos;

		break;

	case AI_ACTION_TALK:
		unit = GetUnit(gAiDecision.yTarget);

		x = unit->xPos;
		y = unit->yPos;

		break;

	case AI_ACTION_STAFF:
		if (gAiDecision.targetId == 0)
			return;

		unit = GetUnit(gAiDecision.targetId);

		x = unit->xPos;
		y = unit->yPos;

		break;

	case CONFIG_AI_ACTION_EXPA_Rescue:
		if (gAiDecision.targetId == 0)
			return;

		unit = GetUnit(gAiDecision.targetId);

		x = unit->xPos;
		y = unit->yPos;

		break;

	case CONFIG_AI_ACTION_EXPA_Drop:
		x = gAiDecision.xTarget;
		y = gAiDecision.yTarget;

		break;
	}

	EnsureCameraOntoPosition(proc, x, y);

	/*
	 * The player keeps control of the map cursor in real-time mode, so the AI
	 * target cursor would fight it for the same sprite.
	 */
	if (!IsRealtimeBattleActive())
		StartAiTargetCursor(x * 16, y * 16, 2, proc);
}
