#pragma once

#include "common-chax.h"

struct UnitInfoWindowProc;

#define PAIR_UP_UNIT_ID_MAX 0x100

#define PAIR_UP_STATE_PARTNER_MASK 0x00FF
#define PAIR_UP_STATE_LEADER        0x0100

enum PairUpAction {
	PAIR_UP_ACTION_ATTACH,
	PAIR_UP_ACTION_SWITCH,
	PAIR_UP_ACTION_SEPARATE,
	PAIR_UP_ACTION_TRANSFER,
};

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

enum PairUpDualSupportStat {
	PAIR_UP_DUAL_SUPPORT_HIT,
	PAIR_UP_DUAL_SUPPORT_AVOID,
	PAIR_UP_DUAL_SUPPORT_CRIT,
	PAIR_UP_DUAL_SUPPORT_DODGE,
};

struct PairUpClassBonus {
	u8 pow;
	u8 mag;
	u8 skl;
	u8 spd;
	u8 lck;
	u8 def;
	u8 res;
	u8 mov;
};

extern const struct PairUpClassBonus gPairUpClassBonuses[0x80];

extern u16 gPairUpState[PAIR_UP_UNIT_ID_MAX];
extern struct UnitInfoWindowProc *gPairUpPreviewWindows[2];

bool PairUp_IsEnabled(void);
bool PairUp_IsPaired(const struct Unit *unit);
bool PairUp_IsLeader(const struct Unit *unit);
bool PairUp_IsSupport(const struct Unit *unit);
struct Unit *PairUp_GetPartner(const struct Unit *unit);
struct Unit *PairUp_GetLeader(const struct Unit *unit);
struct Unit *PairUp_GetSupport(const struct Unit *unit);
struct Unit *PairUp_GetCombatSupport(struct Unit *leader);

bool PairUp_CanPair(struct Unit *support, struct Unit *leader);
bool PairUp_Attach(struct Unit *support, struct Unit *leader);
bool PairUp_Separate(struct Unit *leader, int x, int y);
bool PairUp_Switch(struct Unit *leader);
bool PairUp_Transfer(struct Unit *leader, struct Unit *newLeader);
void PairUp_MarkPairActed(struct Unit *leader);

void PairUp_ClearUnit(struct Unit *unit);
void PairUp_ResetAll(void);
void PairUp_Reconcile(void);
void PairUp_RebuildMap(void);
int PairUp_OnClearUnit(struct Unit *unit);
int PairUp_OnCopyUnit(struct Unit *from, struct Unit *to);
int PairUp_OnUnitKill(struct Unit *unit);

int PairUp_GetSupportLevel(struct Unit *leader, struct Unit *support);
int PairUp_GetSupportRank(struct Unit *leader, struct Unit *support);
int PairUp_GetStatBonus(struct Unit *leader, struct Unit *support, int stat);
int PairUp_GetLeadStatBonus(struct Unit *leader, int stat);
int PairUp_GetDualSupportRank(struct Unit *leader);
int PairUp_GetDualSupportBonus(int rank, int stat);
int PairUp_GetDualStrikeChance(struct Unit *leader);
int PairUp_GetDualGuardChance(struct Unit *leader, int weapon);

void PairUp_SaveState(u8 *dst, const u32 size);
void PairUp_LoadState(u8 *src, const u32 size);

u8 PairUp_MenuUsability(const struct MenuItemDef *def, int number);
u8 PairUp_MenuOnSelected(struct MenuProc *menu, struct MenuItemProc *item);
u8 PairUp_SwitchUsability(const struct MenuItemDef *def, int number);
u8 PairUp_SwitchOnSelected(struct MenuProc *menu, struct MenuItemProc *item);
u8 PairUp_SeparateUsability(const struct MenuItemDef *def, int number);
u8 PairUp_SeparateOnSelected(struct MenuProc *menu, struct MenuItemProc *item);
u8 PairUp_TransferUsability(const struct MenuItemDef *def, int number);
u8 PairUp_TransferOnSelected(struct MenuProc *menu, struct MenuItemProc *item);
bool ActionPairUp(ProcPtr proc);
