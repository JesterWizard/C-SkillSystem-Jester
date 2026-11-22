#pragma once

#include "common-chax.h"

/*
 * An enemy promoted by killing a unit (promote_enemy_on_kill, BattleHit.c) waits here for its map effect
 * (Kernel/Wizardry/EnemyPromoFx), which runs after the action.
 */
struct EnemyPromoFx {
	u8 uid;       /* the promoted unit's index; 0 = nothing queued */
	u8 old_jid;   /* its class before the promotion */
	u8 _pad[2];
	char cls[0x10];     /* the new class's name, for the popup */
	char old_cls[0x10]; /* the old class's name, for the popup */
};

extern struct EnemyPromoFx gEnemyPromoFx;

bool PostAction_EnemyPromoFx(ProcPtr parent);
void ChapterInit_ResetEnemyPromoFx(ProcPtr proc);
