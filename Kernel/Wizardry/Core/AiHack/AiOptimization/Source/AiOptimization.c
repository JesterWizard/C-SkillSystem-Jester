#include "common-chax.h"
#include "skill-system.h"
#include "battle-system.h"
#include "kernel-lib.h"
#include "debuff.h"
#include "constants/skills.h"
#include "weapon-range.h"
#include "status-getter.h"
#include "gaiden-magic.h"
#include "jester_headers/custom-functions.h"
#include "enemy-fog-vision.h"

extern void CpDecide_Main(ProcPtr proc);
extern void DecideHealOrEscape(void);
extern bool AiTryDoMenuSkills(void);
extern bool AiTryDoDanceCommand(void);
extern void AiPhaseMarkUnitPerformed(u8 uid);
extern bool Generic_CanUnitBeOnPos(struct Unit *unit, s8 x, s8 y, int x2, int y2);
extern s8 AiIsUnitEnemy(struct Unit *unit);

static const int sAdjTileOffsets[4][2] = {
	{-1, 0},
	{1, 0},
	{0, -1},
	{0, 1},
};

static struct Unit *AiGetAdjacentRescueTarget(void)
{
	int i;
	struct Unit *bestTarget = NULL;
	int bestHp = 0;

	for (i = 0; i < 4; ++i)
	{
		int x = gActiveUnit->xPos + sAdjTileOffsets[i][0];
		int y = gActiveUnit->yPos + sAdjTileOffsets[i][1];
		struct Unit *unit;

		if (x < 0 || y < 0 || x >= gBmMapSize.x || y >= gBmMapSize.y)
			continue;

		unit = GetUnit(gBmMapUnit[y][x]);
		if (!unit || !unit->pCharacterData)
			continue;

		if (!AreUnitsAllied(gActiveUnit->index, unit->index))
			continue;

		if (unit->state & (US_RESCUING | US_RESCUED | US_DEAD | US_HIDDEN))
			continue;

		if (GetUnitCurrentHp(unit) * 2 >= GetUnitMaxHp(unit))
			continue;

		if (!CanUnitRescue(gActiveUnit, unit))
			continue;

		if (!bestTarget || GetUnitCurrentHp(unit) < bestHp)
		{
			bestTarget = unit;
			bestHp = GetUnitCurrentHp(unit);
		}
	}

	return bestTarget;
}

static int AiGetNearestEnemyDistanceToTile(int x, int y)
{
	int i;
	int bestDistance = 0x7FFF;
	bool found = false;

#ifdef CONFIG_FOURTH_ALLEGIANCE
	for (i = 1; i < 0xD0; ++i)
#else
	for (i = 1; i < 0xC0; ++i)
#endif
	{
		struct Unit *unit = GetUnit(i);
		int distance;

		if (!UNIT_IS_VALID(unit))
			continue;

		if (!AiIsUnitEnemy(unit))
			continue;

		distance = RECT_DISTANCE(x, y, unit->xPos, unit->yPos);
		if (!found || distance < bestDistance)
		{
			bestDistance = distance;
			found = true;
		}
	}

	return found ? bestDistance : 0x7FFF;
}

static bool AiFindBestDropTile(struct Unit *carriedUnit, int carrierX, int carrierY, struct Vec2 *out, int *outScore)
{
	int i;
	int bestScore = -1;
	bool found = false;

	for (i = 0; i < 4; ++i)
	{
		int x = carrierX + sAdjTileOffsets[i][0];
		int y = carrierY + sAdjTileOffsets[i][1];
		int score;

		if (x < 0 || y < 0 || x >= gBmMapSize.x || y >= gBmMapSize.y)
			continue;

		if (!Generic_CanUnitBeOnPos(carriedUnit, x, y, -1, -1))
			continue;

		score = AiGetNearestEnemyDistanceToTile(x, y);
		if (!found || score > bestScore)
		{
			bestScore = score;
			out->x = x;
			out->y = y;
			found = true;
		}
	}

	if (found && outScore)
		*outScore = bestScore;

	return found;
}

static bool AiFindFarthestCarryPosition(struct Unit *carriedUnit, struct Vec2 *movePos, struct Vec2 *dropPos, bool *outHasDrop)
{
	int x;
	int y;
	int bestMoveScore = -1;
	int bestDropScore = -1;
	bool found = false;
	bool bestHasDrop = false;
	struct Vec2 bestDrop = { 0, 0 };

	for (y = 0; y < gBmMapSize.y; ++y)
	{
		for (x = 0; x < gBmMapSize.x; ++x)
		{
			int moveScore;
			int dropScore = -1;
			struct Vec2 dropCandidate;
			bool hasDrop;

			if ((s8)gBmMapMovement[y][x] < 0)
				continue;

			if (!Generic_CanUnitBeOnPos(gActiveUnit, x, y, -1, -1))
				continue;

			moveScore = AiGetNearestEnemyDistanceToTile(x, y);
			hasDrop = AiFindBestDropTile(carriedUnit, x, y, &dropCandidate, &dropScore);

			if (!found
				|| moveScore > bestMoveScore
				|| (moveScore == bestMoveScore && hasDrop && !bestHasDrop)
				|| (moveScore == bestMoveScore && hasDrop == bestHasDrop && dropScore > bestDropScore))
			{
				found = true;
				bestMoveScore = moveScore;
				bestHasDrop = hasDrop;
				bestDropScore = dropScore;
				movePos->x = x;
				movePos->y = y;

				if (hasDrop)
					bestDrop = dropCandidate;
			}
		}
	}

	if (!found)
		return false;

	if (bestHasDrop)
		*dropPos = bestDrop;

	if (outHasDrop)
		*outHasDrop = bestHasDrop;

	return true;
}

static bool AiTryRescueWeakAdjacentAlly(void)
{
	struct Unit *target = AiGetAdjacentRescueTarget();

	if (!target)
		return false;

	AiSetDecision(
		gActiveUnit->xPos,
		gActiveUnit->yPos,
		CONFIG_AI_ACTION_EXPA_Rescue,
		target->index,
		0,
		target->xPos,
		target->yPos);

	return true;
}

static bool AiTryRescueCarryDrop(void)
{
	struct Unit *carriedUnit;
	struct Vec2 dropPos = { 0, 0 };
	struct Vec2 movePos = { 0, 0 };
	bool canDrop = false;

	if (!(gActiveUnit->state & US_RESCUING))
		return false;

	carriedUnit = GetUnit(gActiveUnit->rescue);
	if (!carriedUnit || !carriedUnit->pCharacterData)
		return false;

	if (AiFindFarthestCarryPosition(carriedUnit, &movePos, &dropPos, &canDrop) == true)
	{
		if (canDrop == true)
		{
			AiSetDecision(
				movePos.x,
				movePos.y,
				CONFIG_AI_ACTION_EXPA_Drop,
				0,
				0,
				dropPos.x,
				dropPos.y);
			return true;
		}

		AiSetDecision(
			movePos.x,
			movePos.y,
			AI_ACTION_NONE,
			0,
			0,
			0,
			0);

		return true;
	}

	{
		u8 aiFlags = gActiveUnit->aiFlags;
		bool didEscape;

		gActiveUnit->aiFlags |= AI_UNIT_FLAG_3;
		didEscape = (AiTryMoveTowardsEscape() == TRUE);
		gActiveUnit->aiFlags = aiFlags;

		if (didEscape == true)
			return true;
	}

	return false;
}

// Function to find an adjacent ally with higher HP and swap positions with the defender
void SwapDefenderWithAllyIfNecessary(struct Unit* defender) {
    int adjLookup[4][2] = {
        {-1, 0}, // Left
        {1, 0},  // Right
        {0, -1}, // Up
        {0, 1}   // Down
    };

    struct Unit* adjacentAlly = NULL;
    int defenderHP = defender->curHP;
    int x = defender->xPos;
    int y = defender->yPos;

    // Iterate over adjacent positions
    for (int i = 0; i < 4; ++i) {
        int adjX = x + adjLookup[i][0];
        int adjY = y + adjLookup[i][1];

        if (adjX < 0 || adjY < 0 || adjX >= gBmMapSize.x || adjY >= gBmMapSize.y)
            continue;

        struct Unit* unit = GetUnit(gBmMapUnit[adjY][adjX]);
        if (unit && AreUnitsAllied(defender->index, unit->index) && unit->curHP > defenderHP) {
            adjacentAlly = unit;
            defenderHP = unit->curHP;
        }
    }

    // Swap positions if a suitable ally is found
    if (adjacentAlly) {
        int tempX = defender->xPos;
        int tempY = defender->yPos;
        defender->xPos = adjacentAlly->xPos;
        defender->yPos = adjacentAlly->yPos;
        adjacentAlly->xPos = tempX;
        adjacentAlly->yPos = tempY;

        // Update the positions in the map
        gBmMapUnit[defender->yPos][defender->xPos] = defender->index;
        gBmMapUnit[adjacentAlly->yPos][adjacentAlly->xPos] = adjacentAlly->index;

        // Update the AI decision to target the new defender
        gAiDecision.targetId = adjacentAlly->index;
    }
}

LYN_REPLACE_CHECK(CpDecide_Main);
void CpDecide_Main(ProcPtr proc)
{
next_unit:
    gAiState.decideState = 0;

    if (*gAiState.unitIt)
    {
        gAiState.unk7C = 0;

        gActiveUnitId = *gAiState.unitIt;
        gActiveUnit = GetUnit(gActiveUnitId);

        if (gActiveUnit->state & (US_DEAD | US_UNSELECTABLE) || !gActiveUnit->pCharacterData)
        {
            gAiState.unitIt++;
            goto next_unit;
        }

        do
        {
            RefreshEntityBmMaps();
            BuildEnemyFogVision();
            RenderBmMap();
            RefreshUnitSprites();

            AiUpdateNoMoveFlag(gActiveUnit);

            gAiState.combatWeightTableId = (gActiveUnit->ai_config & AI_UNIT_CONFIG_COMBATWEIGHT_MASK) >> AI_UNIT_CONFIG_COMBATWEIGHT_SHIFT;

            gAiState.dangerMapFilled = FALSE;
            AiInitDangerMap();

            AiClearDecision();
            AiDecideMainFunc();

#if (defined(SID_Guardian) && COMMON_SKILL_VALID(SID_Guardian))
            if (SkillTester(GetUnit(gAiDecision.targetId), SID_Guardian))
            {
                // Assuming gAiDecision.targetId is the defender's unit ID
                struct Unit* defender = GetUnit(gAiDecision.targetId);
                if (defender && AreUnitsAllied(defender->index, gActiveUnit->index) == FALSE) {
                    SwapDefenderWithAllyIfNecessary(defender);
                }
            }
#endif

            gActiveUnit->state |= US_HAS_MOVED_AI;

            if (!gAiDecision.actionPerformed ||
                (gActiveUnit->xPos == gAiDecision.xMove && gActiveUnit->yPos == gAiDecision.yMove && gAiDecision.actionId == AI_ACTION_NONE))
            {
                // Ignoring actions that are just moving to the same square

                gAiState.unitIt++;
                Proc_Goto(proc, 0);
            }
            else
            {
                AiPhaseMarkUnitPerformed(gActiveUnitId);
                gAiState.unitIt++;
                Proc_StartBlocking(gProcScr_CpPerform, proc);
            }
        } while (0);
    }
    else
    {
        Proc_End(proc);
    }
}

#define LOCAL_TRACE 0

struct AiSimuSlotEnt {
	u8 slot, action_type;
	u16 item;
};

extern u8 sAiSimuSlotBuf[0x100]; // I think it cannot overflow?
struct AiSimuSlotEnt *const gpAiSimuSlotBuf = (void *)sAiSimuSlotBuf;

void CollectAiSimuSlots(struct Unit *unit, struct AiSimuSlotEnt *buf)
{
	int i;
	struct AiSimuSlotEnt *it = buf;

	CpuFastFill(0, gpAiSimuSlotBuf, sizeof(sAiSimuSlotBuf));

	for (i = 0; i < UNIT_ITEM_COUNT; i++) {
		u16 item = unit->items[i];

		if (item == 0)
			break;

		if (ITEM_INDEX(item) == ITEM_NIGHTMARE)
			continue;

		if (CanUnitUseWeapon(unit, item)) {
			it->slot = i;
			it->item = item;
			it->action_type = AI_ACTION_COMBAT;

			it++;
		}
	}

	/* Gaiden B.Mag */
	if (gpKernelDesignerConfig->gaiden_magic && gpKernelDesignerConfig->gaiden_magic_ai_use) {
		struct GaidenMagicList *gmaglist = GetGaidenMagicList(unit);

		for (i = 0; i < GAIDEN_MAGIC_LIST_LEN; i++) {
			u16 item = gmaglist->bmags[i];

			if (item == 0)
				break;

			if (CanUnitUseGaidenMagic(unit, item) && (unit->curHP > GetGaidenWeaponHpCost(unit, item))) {
				it->slot = CHAX_BUISLOT_GAIDEN_BMAG1 + i;
				it->item = item;
				it->action_type = AI_ACTION_COMBAT;

				it++;
			}
		}
	}

	/* terminator */
	it->item = 0;
}

void CollectAiSimuStaffSlots(struct Unit *unit, struct AiSimuSlotEnt *buf)
{
	int i;
	struct AiSimuSlotEnt *it = buf;

	CpuFastFill(0, gpAiSimuSlotBuf, sizeof(sAiSimuSlotBuf));

	for (i = 0; i < UNIT_ITEM_COUNT; i++) {
		u16 item = unit->items[i];

		if (item == 0)
			break;

		if (GetItemAttributes(item) & IA_STAFF) {
			it->slot = i;
			it->item = item;
			it++;
		}
	}

	/* Gaiden w.Mag */
	if (gpKernelDesignerConfig->gaiden_magic && gpKernelDesignerConfig->gaiden_magic_ai_use) {
		struct GaidenMagicList *gmaglist = GetGaidenMagicList(unit);

		for (i = 0; i < GAIDEN_MAGIC_LIST_LEN; i++) {
			u16 item = gmaglist->wmags[i];

			if (item == 0)
				break;

			if (!(GetItemAttributes(item) & IA_STAFF))
				continue;

			if (CanUnitUseGaidenMagic(unit, item) && (unit->curHP > GetGaidenWeaponHpCost(unit, item))) {
				it->slot = CHAX_BUISLOT_GAIDEN_WMAG1 + i;
				it->item = item;
				it++;
			}
		}
	}
}

LYN_REPLACE_CHECK(AiAttemptOffensiveAction);
s8 AiAttemptOffensiveAction(s8 (*isEnemy)(struct Unit *unit))
{
	enum { TARGET_COUNT_TRIGLEVEL = 5, };

	struct AiCombatSimulationSt tmpResult = {0};
	struct AiCombatSimulationSt finalResult = {0};

	struct AiSimuSlotEnt *it, *final_slot = NULL;

	u8 UNIT_AMOUNT = 0xC0;

#ifdef CONFIG_FOURTH_ALLEGIANCE
	UNIT_AMOUNT = 0xD0;
#endif

	int target_count = 0;

	int uid;
	bool ret = 0;

	if (gActiveUnit->state & US_IN_BALLISTA) {
		BmMapFill(gBmMapMovement, -1);
		gBmMapMovement[gActiveUnit->yPos][gActiveUnit->xPos] = 0;

		if (GetRiddenBallistaAt(gActiveUnit->xPos, gActiveUnit->yPos) != 0)
			goto try_ballist_combat;

		TryRemoveUnitFromBallista(gActiveUnit);
	} else {
		if (UNIT_CATTRIBUTES(gActiveUnit) & CA_STEAL) {
			if (GetUnitItemCount(gActiveUnit) < UNIT_ITEM_COUNT) {
				GenerateUnitMovementMap(gActiveUnit);
				MarkMovementMapEdges();

				if (AiAttemptStealActionWithinMovement() == 1)
					return 0;
			}
		}

		if (gAiState.flags & AI_FLAG_STAY) {
			BmMapFill(gBmMapMovement, -1);
			gBmMapMovement[gActiveUnit->yPos][gActiveUnit->xPos] = 0;
		} else
			GenerateUnitMovementMap(gActiveUnit);

		if (UnitHasMagicRank(gActiveUnit))
			GenerateMagicSealMap(-1);
	}

	SetWorkingBmMap(gBmMapRange);

	CollectAiSimuSlots(gActiveUnit, gpAiSimuSlotBuf);

	for (it = gpAiSimuSlotBuf; it->item != 0; it++) {
		int move_distance;

		u16 item = it->item;

		LTRACEF("[%d] item=0x%04X, slot=0x%02X", it - gpAiSimuSlotBuf, it->item, it->slot);

		tmpResult.itemSlot = it->slot;

		move_distance = MovGetter(gActiveUnit) + GetItemMaxRangeRework(item, gActiveUnit);

		for (uid = 1; uid < UNIT_AMOUNT; uid++) {
			struct Unit *unit = GetUnit(uid);

			if (!UNIT_IS_VALID(unit))
				continue;

			if (unit->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
				continue;

			if (gpKernelDesignerConfig->calculate_map_range_faster == true)
			{
				if (move_distance < RECT_DISTANCE(unit->xPos, unit->yPos, gActiveUnit->xPos, gActiveUnit->yPos))
					continue;
			}
			else
			{
				if (!AiReachesByBirdsEyeDistance(gActiveUnit, unit, item))
					continue;
			}

#if defined(SID_ShadePlus) && (COMMON_SKILL_VALID(SID_ShadePlus))
			if (SkillTesterPlus(unit, SID_ShadePlus))
				continue;
#endif

#if defined(SID_Shade) && (COMMON_SKILL_VALID(SID_Shade))
			/* Shade skill may make unit avoid to be target */
			if (SkillTester(unit, SID_Shade))
				if (Roll1RN(unit->lck))
					continue;
#endif

            if (GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_HIDE)
                continue;

#if defined(SID_Rampage) && (COMMON_SKILL_VALID(SID_Rampage))
            if (SkillTester(gActiveUnit, SID_Rampage))
            {
                if (AreUnitsAllied(gActiveUnit->index, unit->index))
                    continue;
                if (gActiveUnit->index == unit->index)
                    continue;
                if (!EnemyFogVisionCanSeeUnit(unit))
                    continue;
            }
            else
            {
                if (!isEnemy(unit))
                    continue;
            }
#else
            if (!isEnemy(unit))
                continue;
#endif

			AiFillReversedAttackRangeMap(unit, item);

			tmpResult.targetId = unit->index;

			ret = AiSimulateBestBattleAgainstTarget(&tmpResult);
			if (!ret)
				continue;

			if (tmpResult.score >= finalResult.score) {
				finalResult = tmpResult;
				finalResult.itemSlot = it->slot;
				final_slot = it;

				LTRACEF("Find slot: %d", it - gpAiSimuSlotBuf);
			}

			if (gpKernelDesignerConfig->calculate_map_range_faster == true)
			{
				if (++target_count >= TARGET_COUNT_TRIGLEVEL)
					break;
			}
		}
	}

try_ballist_combat:
	if (UNIT_CATTRIBUTES(gActiveUnit) & CA_BALLISTAE) {
		ret = AiAttemptBallistaCombat(isEnemy, &tmpResult);
		if (ret == 1)
			if (tmpResult.score >= finalResult.score) {
				finalResult = tmpResult;
				final_slot = NULL;
			}
	}

	if ((finalResult.score != 0) || (finalResult.targetId != 0)) {
		int action_type = AI_ACTION_COMBAT;

		if (final_slot)
			action_type = final_slot->action_type;

		LTRACEF("Set decision: x=%d, y=%d, action=%d, slot=%d",
				finalResult.xMove, finalResult.yMove, action_type, finalResult.itemSlot);

		AiSetDecision(finalResult.xMove, finalResult.yMove, action_type, finalResult.targetId, finalResult.itemSlot, 0, 0);

		if ((s8)finalResult.itemSlot != -1)
			TryRemoveUnitFromBallista(gActiveUnit);

		return ret;
	}

	return ret;
}

/**
 * Add unit to AI list
 */
extern void DecideScriptA(void);
extern void DecideScriptB(void);
extern void CpOrderBerserkInit(ProcPtr proc);

LYN_REPLACE_CHECK(DecideScriptA);
void DecideScriptA(void)
{
    int i = 0;

    if (UNIT_IS_GORGON_EGG(gActiveUnit))
        return;

    if (gAiState.flags & AI_FLAG_BERSERKED)
    {
        AiDoBerserkAction();
        return;
    }
    else
    {
#if (defined(SID_Rampage) && (COMMON_SKILL_VALID(SID_Rampage)))
        if (SkillTester(gActiveUnit, SID_Rampage))
        {
            NoCashGBAPrint("Decide script A check");
            AiDoBerserkAction();
            return;
        }
#endif
    }

    for (i = 0; i < 0x100; ++i)
    {
        if (AiTryExecScriptA() == TRUE)
            return;
    }

	if (AiTryDoDanceCommand() == TRUE)
		return;

	if (AiTryDoMenuSkills() == TRUE)
		return;

    AiExecFallbackScriptA();
}

LYN_REPLACE_CHECK(DecideScriptB);
void DecideScriptB(void)
{
    int i = 0;

    if ((gActiveUnit->state & US_IN_BALLISTA) && (GetRiddenBallistaAt(gActiveUnit->xPos, gActiveUnit->yPos) != NULL))
        return;

    if (gAiState.flags & AI_FLAG_BERSERKED)
    {
        AiDoBerserkMove();
        return;
    }
    else
    {
#if (defined(SID_Rampage) && (COMMON_SKILL_VALID(SID_Rampage)))
        if (SkillTester(gActiveUnit, SID_Rampage))
        {
            AiDoBerserkAction();
            return;
        }
#endif
    }

    for (i = 0; i < 0x100; ++i)
    {
        if (AiTryExecScriptB() == TRUE)
            return;
    }

	if (AiTryDoDanceCommand() == TRUE)
		return;

	if (AiTryDoMenuSkills() == TRUE)
		return;

    AiExecFallbackScriptB();
}

LYN_REPLACE_CHECK(DecideHealOrEscape);
void DecideHealOrEscape(void)
{
	if (gAiState.flags & AI_FLAG_BERSERKED)
		return;

	if (gpKernelDesignerConfig->rescue_drop_ai_use == true) {
		if (gActiveUnit->state & US_RESCUING)
		{
			if (AiTryRescueCarryDrop() == true)
				return;
		}
		else if (AiTryRescueWeakAdjacentAlly() == true)
		{
			return;
		}
	}

	if (AiUpdateGetUnitIsHealing(gActiveUnit) == TRUE)
	{
		struct Vec2 vec2;

		if (AiTryHealSelf() == TRUE)
			return;

		if ((gActiveUnit->aiFlags & AI_UNIT_FLAG_3) && (AiTryMoveTowardsEscape() == TRUE))
		{
			AiTryDanceOrStealAfterMove();
			return;
		}

		if (AiTryGetNearestHealPoint(&vec2) == TRUE)
		{
			AiTryMoveTowards(vec2.x, vec2.y, 0, 0, 1);

			if (gAiDecision.actionPerformed == TRUE)
				AiTryActionAfterMove();

			if (gAiDecision.actionPerformed == TRUE)
				return;
		}
	}
	else
	{
		if ((gActiveUnit->aiFlags & AI_UNIT_FLAG_3) && (AiTryMoveTowardsEscape() == TRUE))
		{
			AiTryDanceOrStealAfterMove();
			return;
		}
	}

	if (AiTryDoDanceCommand() == TRUE)
		return;

	if (AiTryDoMenuSkills() == TRUE)
		return;
}


LYN_REPLACE_CHECK(AiTryDoStaff);
bool AiTryDoStaff(s8 (*isEnemy)(struct Unit *unit))
{
	struct AiSimuSlotEnt *it;
	u8 exp = 0;

	if (GetUnitStatusIndex(gActiveUnit) == UNIT_STATUS_SILENCED)
		return gAiDecision.actionPerformed;

	CollectAiSimuStaffSlots(gActiveUnit, gpAiSimuSlotBuf);

	for (it = gpAiSimuSlotBuf; it->item != 0; it++) {
		int index;
		u16 item = it->item;

		if (GetItemRequiredExp(item) < exp)
			continue;

		index = GetAiStaffFuncIndex(item);
		if (index != -1) {
			sAiStaffFuncLut[index].func(it->slot, isEnemy);

			if (gAiDecision.actionPerformed != false)
				exp = GetItemRequiredExp(item);
		}
	}

	if (AiTryDoMenuSkills() == TRUE)
		return true;

	return gAiDecision.actionPerformed;
}
