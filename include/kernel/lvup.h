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

/* Class growths: gpKernelDesignerConfig->class_growths */
enum class_growths_mode {
	CLASS_GROWTHS_OFF = 0,
	CLASS_GROWTHS_ADD = 1,     /* personal + class */
	CLASS_GROWTHS_AVERAGE = 2, /* (personal + class) / 2 */
};

enum class_growth_stat {
	CLASS_GROWTH_HP,
	CLASS_GROWTH_POW,
	CLASS_GROWTH_MAG,
	CLASS_GROWTH_SKL,
	CLASS_GROWTH_SPD,
	CLASS_GROWTH_LCK,
	CLASS_GROWTH_DEF,
	CLASS_GROWTH_RES,

	CLASS_GROWTH_MAX
};

static inline bool ClassGrowthPreviewEnabled(void)
{
	return gpKernelDesignerConfig->class_growths != CLASS_GROWTHS_OFF && !KernelLvupHidesGrowthDisplay();
}

/* Personal growth merged with the class growth, before skill/status bonuses */
int GetClassGrowthBase(int personal, int job);
int GetUnitBaseHpGrowth(struct Unit *unit);
int GetUnitBasePowGrowth(struct Unit *unit);
int GetUnitBaseMagGrowth(struct Unit *unit);
int GetUnitBaseSklGrowth(struct Unit *unit);
int GetUnitBaseSpdGrowth(struct Unit *unit);
int GetUnitBaseLckGrowth(struct Unit *unit);
int GetUnitBaseDefGrowth(struct Unit *unit);
int GetUnitBaseResGrowth(struct Unit *unit);

int GetUnitGrowthByStat(struct Unit *unit, int stat);
void GetClassChangeGrowthDiffs(struct Unit *unit, int jid, int diffs[CLASS_GROWTH_MAX]);
extern const char *const gClassGrowthStatNames[CLASS_GROWTH_MAX];

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

void StartLvupStatPointsMenu(struct Unit *unit, struct BattleUnit *bu, ProcPtr parent);
