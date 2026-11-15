#pragma once

#include "common-chax.h"

/*
 * The battle-animation HUD's "x2"/"x4" badge (Kernel/Wizardry/DoubleBadge): how many times each side strikes per
 * exchange, recorded by BattleUnwind (BattleOrder.c) and shown above each side's Hit/Dmg/Crit box.
 */
struct DoubleBadge {
	u8 strikes[2]; /* [0] gBattleActor, [1] gBattleTarget; 0 = can't strike */
	u8 _pad[2];
};

extern struct DoubleBadge gDoubleBadge;

void DoubleBadge_Set(int actor, int target);
void StartDoubleBadge(ProcPtr gauge);
