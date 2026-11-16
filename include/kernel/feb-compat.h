#pragma once

#include "common-chax.h"

/** 
 * Individual_animation_ea_2
 */
struct IndividualAnimConf {
	u8 pid, jid;
	u8 _pad_[2];
	const struct BattleAnimDef *anim_conf;
};

const struct IndividualAnimConf *GetPrIndividualAnimConf(void);

/* The unit's battle animations: its row in CustomAnimeTable (character and class), else its class's */
const struct BattleAnimDef *GetUnitBattleAnimDef(struct Unit *unit);

/**
 * Bow2Decrease
 */
bool CheckWeaponCostForMissedBowAttack(struct BattleUnit *actor);

/**
 * Devil
 */
bool RollBaseDevilCheck(struct BattleUnit *bu);
