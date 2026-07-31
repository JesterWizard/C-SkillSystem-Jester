#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "ai-hack.h"

#define LOCAL_TRACE 0

void AiPhaseClearPerformedFlags(void)
{
	int i;

	for (i = 0; i < 7; i++)
		sAiPhasePerformedBits[i] = 0;
}

void AiPhaseMarkUnitPerformed(u8 uid)
{
	if (uid >= 0xE0)
		return;

	sAiPhasePerformedBits[uid >> 5] |= 1u << (uid & 0x1F);
}

bool AiPhaseDidUnitPerform(u8 uid)
{
	if (uid >= 0xE0)
		return false;

	return (sAiPhasePerformedBits[uid >> 5] >> (uid & 0x1F)) & 1;
}

static const s8 sAdjTileOffsets[4][2] = {
	{-1, 0},
	{1, 0},
	{0, -1},
	{0, 1},
};

static bool AiDance_CanActiveUnitDance(void)
{
	if (UNIT_CATTRIBUTES(gActiveUnit) & (CA_DANCE | CA_PLAY))
		return true;

#if defined(SID_Dance) && (COMMON_SKILL_VALID(SID_Dance))
	if (SkillTester(gActiveUnit, SID_Dance))
		return true;
#endif

	return false;
}

static bool AiDance_IsValidTarget(struct Unit *unit)
{
	if (!UNIT_IS_VALID(unit))
		return false;

	if (unit->index == gActiveUnit->index)
		return false;

	if (!AreUnitsAllied(gActiveUnit->index, unit->index))
		return false;

	if (unit->state & (US_HIDDEN | US_DEAD | US_RESCUED))
		return false;

	/* already refreshed and waiting in the pool again */
	if (!(unit->state & US_HAS_MOVED_AI))
		return false;

	/* only worth refreshing an ally who actually performed an action
	 * this phase, not one who just waited in place */
	if (!AiPhaseDidUnitPerform(unit->index))
		return false;

	/* dancers cannot refresh each other, avoiding dance chains */
	if (UNIT_CATTRIBUTES(unit) & (CA_DANCE | CA_PLAY))
		return false;

	return true;
}

/**
 * Put a unit back to the front of the pool of units who haven't
 * moved yet, so that CpDecide_Main gives it another decision this phase.
 */
static void AiDance_RequeueTarget(u8 uid)
{
	u8 *pos = gAiState.unitIt;
	u8 *term = pos;
	u8 *it;

	if (uid == 0)
		return;

	while (*term != 0)
		term++;

	/* already waiting in the remaining pool */
	for (it = pos; it < term; it++)
		if (*it == uid)
			return;

	/* no room left to keep the list terminated */
	if (term >= gAiState.units + sizeof(gAiState.units) - 1)
		return;

	for (it = term + 1; it > pos; it--)
		*it = *(it - 1);

	*pos = uid;
}

bool AiTryDoDanceCommand(void)
{
	int x, y, i;
	int bestScore = -1;
	u8 bestX = 0, bestY = 0, bestTarget = 0;

	if (gAiDecision.actionPerformed)
		return true;

	if (gpKernelDesignerConfig->ai_dance_use != true)
		return false;

	if (!AiDance_CanActiveUnitDance())
		return false;

	if (gAiState.flags & AI_FLAG_STAY) {
		BmMapFill(gBmMapMovement, -1);
		gBmMapMovement[gActiveUnit->yPos][gActiveUnit->xPos] = 0;
	} else {
		GenerateUnitMovementMap(gActiveUnit);
	}

	for (y = 0; y < gBmMapSize.y; y++) {
		for (x = 0; x < gBmMapSize.x; x++) {
			if ((s8)gBmMapMovement[y][x] < 0)
				continue;

			if (gBmMapUnit[y][x] != 0 && gBmMapUnit[y][x] != gActiveUnit->index)
				continue;

			for (i = 0; i < 4; i++) {
				int nx = x + sAdjTileOffsets[i][0];
				int ny = y + sAdjTileOffsets[i][1];
				struct Unit *unit;
				int score;

				if (nx < 0 || ny < 0 || nx >= gBmMapSize.x || ny >= gBmMapSize.y)
					continue;

				if (gBmMapUnit[ny][nx] == 0)
					continue;

				unit = GetUnit(gBmMapUnit[ny][nx]);

				if (!AiDance_IsValidTarget(unit))
					continue;

				/* prefer highest level target, then the shortest move
				 * (map stores remaining mov: higher means closer) */
				score = unit->level * 0x100 + gBmMapMovement[y][x];

				if (score > bestScore) {
					bestScore = score;
					bestX = x;
					bestY = y;
					bestTarget = unit->index;
				}
			}
		}
	}

	if (bestTarget == 0)
		return false;

	LTRACEF("uid=%x dance target=%x at x=%d, y=%d",
			gActiveUnit->index & 0xFF, bestTarget, bestX, bestY);

	AiSetDecision(
		bestX,
		bestY,
		AI_ACTION_REFRESH,
		bestTarget,
		0,
		GetUnit(bestTarget)->xPos,
		GetUnit(bestTarget)->yPos);

	return true;
}

/* AiActionConf::exec */
void AiAction_RefreshDance(struct CpPerformProc *proc)
{
	/* The unit struct coords are only committed at CpPerform_Cleanup,
	 * so move them now or the dance animation plays at the pre-move position */
	gActiveUnit->xPos = gAiDecision.xMove;
	gActiveUnit->yPos = gAiDecision.yMove;

	gActionData.subjectIndex = gActiveUnitId;
	gActionData.targetIndex = gAiDecision.targetId;
	gActionData.unitActionType = UNIT_ACTION_DANCE;

	ApplyUnitAction(proc);

	AiDance_RequeueTarget(gAiDecision.targetId);
}
