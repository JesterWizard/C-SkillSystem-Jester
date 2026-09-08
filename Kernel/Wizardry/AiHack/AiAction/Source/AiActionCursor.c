#include "common-chax.h"
#include "kernel/realtime-battle.h"
#include "vanilla.h"

LYN_REPLACE_CHECK(AiRefreshMap);
void AiRefreshMap(void)
{
	gActiveUnit = GetUnit(gActionData.subjectIndex);
	if (!UNIT_IS_VALID(gActiveUnit))
		return;

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

/*
 * Vanilla hides AI MUs that start/end in fog during the red phase. Real-time
 * mode reuses that red-faction path while the player still owns the map, so
 * the acting enemy must always get a MU and a non-blocking camera snap.
 */
LYN_REPLACE_CHECK(CpPerform_MoveCameraOntoUnit);
void CpPerform_MoveCameraOntoUnit(struct CpPerformProc *proc)
{
	proc->isUnitVisible = 1;

	if (!UNIT_IS_VALID(gActiveUnit)) {
		proc->isUnitVisible = 0;
		return;
	}

	if (IsRealtimeBattleActive()) {
		RealtimeBattle_SnapCameraOntoPosition(gActiveUnit->xPos, gActiveUnit->yPos);
		return;
	}

	if ((gPlaySt.chapterVisionRange != 0) && (gPlaySt.faction == FACTION_RED)) {
		if ((gBmMapFog[gActiveUnit->yPos][gActiveUnit->xPos] != 0)
			|| (gBmMapFog[gAiDecision.yMove][gAiDecision.xMove] != 0)) {
			EnsureCameraOntoPosition(proc, gActiveUnit->xPos, gActiveUnit->yPos);
		} else {
			proc->isUnitVisible = 0;

			if (gAiDecision.actionId == AI_ACTION_PILLAGE)
				EnsureCameraOntoPosition(proc, gAiDecision.xMove, gAiDecision.yMove);
		}
	} else {
		EnsureCameraOntoPosition(proc, gActiveUnit->xPos, gActiveUnit->yPos);
	}
}

LYN_REPLACE_CHECK(CpPerform_MoveCameraOntoTarget);
void CpPerform_MoveCameraOntoTarget(struct CpPerformProc *proc)
{
	struct Unit *unit;

	int x = 0;
	int y = 0;

	if (gActionData.unitActionType == UNIT_ACTION_TRAPPED)
		return;

	if (!UNIT_IS_VALID(gActiveUnit))
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
			if (!UNIT_IS_VALID(unit))
				return;
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
		if (!UNIT_IS_VALID(unit))
			return;

		x = unit->xPos;
		y = unit->yPos;

		break;

	case AI_ACTION_REFRESH:
		unit = GetUnit(gAiDecision.targetId);
		if (!UNIT_IS_VALID(unit))
			return;

		x = unit->xPos;
		y = unit->yPos;

		break;

	case AI_ACTION_TALK:
		unit = GetUnit(gAiDecision.yTarget);
		if (!UNIT_IS_VALID(unit))
			return;

		x = unit->xPos;
		y = unit->yPos;

		break;

	case AI_ACTION_STAFF:
		if (gAiDecision.targetId == 0)
			return;

		unit = GetUnit(gAiDecision.targetId);
		if (!UNIT_IS_VALID(unit))
			return;

		x = unit->xPos;
		y = unit->yPos;

		break;

	case CONFIG_AI_ACTION_EXPA_Rescue:
		if (gAiDecision.targetId == 0)
			return;

		unit = GetUnit(gAiDecision.targetId);
		if (!UNIT_IS_VALID(unit))
			return;

		x = unit->xPos;
		y = unit->yPos;

		break;

	case CONFIG_AI_ACTION_EXPA_Drop:
		x = gAiDecision.xTarget;
		y = gAiDecision.yTarget;

		break;
	}

	if (IsRealtimeBattleActive()) {
		RealtimeBattle_SnapCameraOntoPosition(x, y);
		return;
	}

	EnsureCameraOntoPosition(proc, x, y);

	StartAiTargetCursor(x * 16, y * 16, 2, proc);
}
