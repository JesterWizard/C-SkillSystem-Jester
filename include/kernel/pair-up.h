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
bool PairUp_CanTransfer(struct Unit *leader, struct Unit *target);
bool PairUp_Transfer(struct Unit *leader, struct Unit *target);

int PairUp_GetStatBonus(const struct Unit *unit, int stat);
int PairUp_RescueStatScale(int status, const struct Unit *unit, int stat);

u8 PairUp_Usability(const struct MenuItemDef *def, int number);
u8 Shelter_Usability(const struct MenuItemDef *def, int number);
u8 PairUp_OnSelected(struct MenuProc *menu, struct MenuItemProc *item);
u8 Shelter_OnSelected(struct MenuProc *menu, struct MenuItemProc *item);
u8 PairUp_TransferUsability(const struct MenuItemDef *def, int number);
u8 PairUp_TransferEffect(struct MenuProc *menu, struct MenuItemProc *item);
u8 PairUp_SwitchUsability(const struct MenuItemDef *def, int number);
u8 PairUp_SwitchEffect(struct MenuProc *menu, struct MenuItemProc *item);

