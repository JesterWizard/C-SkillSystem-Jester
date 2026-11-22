#include "common-chax.h"
#include "stat-screen.h"
#include "constants/texts.h"

/*
 * R text of the Growths page (Kernel/Wizardry/GrowthRadar): each axis label shows the stat's own description, the
 * legend says what each layer is, and the left panel works as on the other pages. Rows: HP; Res, Str; Def, Mag;
 * Lck, Skill; Spd; the legend.
 */
static _DECL_INFO RText_Hp, RText_Pow, RText_Mag, RText_Skl, RText_Spd, RText_Lck, RText_Def, RText_Res;
static _DECL_INFO RText_Growth, RText_Stats;
static _DECL_INFO RText_Name, RText_Class, RText_Level, RText_Exp, RText_PanelHp;

_DECL_INFO *const RTextPageGrowthRadar = &RText_Hp;

static _DECL_INFO RText_Hp = {
	&RText_Growth, &RText_Res, &RText_Res, &RText_Pow,
	0x98, 0x10, 0x544,
	NULL, NULL
};

static _DECL_INFO RText_Res = {
	&RText_Hp, &RText_Def, &RText_Name, &RText_Pow,
	0x68, 0x20, 0x54C,
	NULL, NULL
};

static _DECL_INFO RText_Pow = {
	&RText_Hp, &RText_Mag, &RText_Res, NULL,
	0xD0, 0x20, 0x546,
	NULL, NULL
};

static _DECL_INFO RText_Def = {
	&RText_Res, &RText_Lck, &RText_Class, &RText_Mag,
	0x60, 0x40, 0x54B,
	NULL, NULL
};

static _DECL_INFO RText_Mag = {
	&RText_Pow, &RText_Skl, &RText_Def, NULL,
	0xD8, 0x40, 0x547,
	NULL, NULL
};

static _DECL_INFO RText_Lck = {
	&RText_Def, &RText_Spd, &RText_Level, &RText_Skl,
	0x68, 0x70, 0x54A,
	NULL, NULL
};

static _DECL_INFO RText_Skl = {
	&RText_Mag, &RText_Spd, &RText_Lck, NULL,
	0xD0, 0x70, 0x548,
	NULL, NULL
};

static _DECL_INFO RText_Spd = {
	&RText_Lck, &RText_Growth, &RText_Lck, &RText_Skl,
	0x98, 0x80, 0x549,
	NULL, NULL
};

static _DECL_INFO RText_Growth = {
	&RText_Spd, &RText_Hp, &RText_PanelHp, &RText_Stats,
	0x70, 0x90, MSG_GROWTH_RADAR_HELP_GROWTH,
	NULL, NULL
};

static _DECL_INFO RText_Stats = {
	&RText_Spd, &RText_Hp, &RText_Growth, NULL,
	0xB0, 0x90, MSG_GROWTH_RADAR_HELP_STATS,
	NULL, NULL
};

/* Page left */
static _DECL_INFO RText_Name = {
	&RText_PanelHp, &RText_Class, NULL, &RText_Res,
	0x18, 0x50, 0,
	NULL, HbPopulate_SSCharacter
};

static _DECL_INFO RText_Class = {
	&RText_Name, &RText_Level, NULL, &RText_Def,
	0x06, 0x68, 0,
	NULL, HbPopulate_SSClass
};

static _DECL_INFO RText_Level = {
	&RText_Class, &RText_PanelHp, NULL, &RText_Exp,
	0x06, 0x78, 0x542,
	NULL, NULL
};

static _DECL_INFO RText_Exp = {
	&RText_Class, &RText_PanelHp, &RText_Level, &RText_Lck,
	0x26, 0x78, 0x543,
	NULL, NULL
};

static _DECL_INFO RText_PanelHp = {
	&RText_Level, &RText_Name, NULL, &RText_Growth,
	0x06, 0x88, 0x544,
	NULL, NULL
};
