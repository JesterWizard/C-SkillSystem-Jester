#pragma once

#include "common-chax.h"
#include "kernel-lib.h"

#define CHAX_MAX_LEVEL \
	MIN(gpKernelDesignerConfig->max_level, UNIT_LEVEL_MAX_RE)

#define CHAX_MAX_RECORD_LEVEL \
	MIN(gpKernelDesignerConfig->max_level_record, UNIT_RECORDED_LEVEL_MAX)

extern const u8 MetisTomeGrowthBonus;

/* Growth getter */
int GetUnitHpGrowth(struct Unit *unit);
int GetUnitPowGrowth(struct Unit *unit);
int GetUnitMagGrowth(struct Unit *unit);
int GetUnitSklGrowth(struct Unit *unit);
int GetUnitSpdGrowth(struct Unit *unit);
int GetUnitLckGrowth(struct Unit *unit);
int GetUnitDefGrowth(struct Unit *unit);
int GetUnitResGrowth(struct Unit *unit);

int GetUnitJobBasedHpGrowth(struct Unit *unit);
int GetUnitJobBasedPowGrowth(struct Unit *unit);
int GetUnitJobBasedMagGrowth(struct Unit *unit);
int GetUnitJobBasedSklGrowth(struct Unit *unit);
int GetUnitJobBasedSpdGrowth(struct Unit *unit);
int GetUnitJobBasedLckGrowth(struct Unit *unit);
int GetUnitJobBasedDefGrowth(struct Unit *unit);
int GetUnitJobBasedResGrowth(struct Unit *unit);

/* Job growth */
struct JobGrowthEnt { s8 st[UNIT_STATUS_MAX - 1]; };
// extern struct JobGrowthEnt const gJobGrowthList[0x100];
extern struct JobGrowthEnt const *const gpJobGrowthList;

int GetUnitHpGrowthJobBonus(int status, struct Unit *unit);
int GetUnitPowGrowthJobBonus(int status, struct Unit *unit);
int GetUnitMagGrowthJobBonus(int status, struct Unit *unit);
int GetUnitSklGrowthJobBonus(int status, struct Unit *unit);
int GetUnitSpdGrowthJobBonus(int status, struct Unit *unit);
int GetUnitLckGrowthJobBonus(int status, struct Unit *unit);
int GetUnitDefGrowthJobBonus(int status, struct Unit *unit);
int GetUnitResGrowthJobBonus(int status, struct Unit *unit);

/* Stat points (indexed by pid - 1) */
#define LVUP_STAT_POINTS_AMT 50
extern u8 gLvupStatPoints[LVUP_STAT_POINTS_AMT];

int GetLvupStatPoints(u8 pid);
void AddLvupStatPoints(u8 pid, int amt);
void ResetLvupStatPoints(void);
void SaveLvupStatPoints(u8 *dst, const u32 size);
void LoadLvupStatPoints(u8 *src, const u32 size);

u8 AllocateCommandUsability(const struct MenuItemDef *def, int number);
u8 AllocateCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem);
