#pragma once

#include "common-chax.h"

enum PairUpStat {
	PAIR_UP_STAT_POW,
	PAIR_UP_STAT_MAG,
	PAIR_UP_STAT_SKL,
	PAIR_UP_STAT_SPD,
	PAIR_UP_STAT_LCK,
	PAIR_UP_STAT_DEF,
	PAIR_UP_STAT_RES,
	PAIR_UP_STAT_MOV,
};

bool PairUp_IsPaired(const struct Unit *unit);
bool PairUp_IsLeader(const struct Unit *unit);
bool PairUp_IsSupport(const struct Unit *unit);
struct Unit *PairUp_GetPartner(const struct Unit *unit);
struct Unit *PairUp_GetLeader(const struct Unit *unit);
struct Unit *PairUp_GetSupport(const struct Unit *unit);

bool PairUp_CanPair(struct Unit *support, struct Unit *leader);
bool PairUp_Attach(struct Unit *support, struct Unit *leader);
bool PairUp_Separate(struct Unit *leader, int x, int y);
bool PairUp_Switch(struct Unit *leader);

int PairUp_RescueStatScale(int status, const struct Unit *unit, int stat);

