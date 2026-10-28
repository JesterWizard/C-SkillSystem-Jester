#pragma once

#include "common-chax.h"

/* Targets kept per weapon by AiCollectOffensiveTargets */
#define AI_TARGET_LIST_MAX 0x40

/*
 * The list lives in EWRAM (include/link/config-memmap.s), not on the stack:
 * the battle simulation that follows runs close to the end of the stack.
 */
extern u16 sAiTargetList[AI_TARGET_LIST_MAX];

/* AiCollectOffensiveTargets entries hold (distance << 8) | unit id */
#define AI_TARGET_UID(entry) ((entry) & 0xFF)

int AiCollectOffensiveTargets(s8 (*isEnemy)(struct Unit *unit), u16 item, int move_distance, u16 *list);

/*
 * The battle simulation is the deepest point of the stack, and runs only a
 * few bytes above the sound mixer code in IWRAM (0x03006CB0-0x03007708).
 * The tile search returns before the caller simulates, so it adds no depth.
 */
bool AiFindBestTileNearTarget(struct AiCombatSimulationSt *st, int tx, int ty, int range);
