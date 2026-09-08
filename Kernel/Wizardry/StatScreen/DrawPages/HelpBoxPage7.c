#include "common-chax.h"
#include "stat-screen.h"
#include "constants/texts.h"

static _DECL_INFO RText_Promotion1, RText_Promotion2, RText_Promotion3;
static _DECL_INFO RText_Skill1, RText_Skill2, RText_Skill3, RText_Skill4, RText_Skill5, RText_Skill6, RText_Skill7, RText_Skill8, RText_Skill9;
static _DECL_INFO RText_Name, RText_Class, RText_Level, RText_Exp, RText_Hp;

_DECL_INFO *const RTextPagePromotions = &RText_Promotion1;

/* Promotions */
static _DECL_INFO RText_Promotion1 = {
    &RText_Promotion3, &RText_Promotion2, &RText_Name, &RText_Skill1,
	0x6C, 0x20, 0,
	HbRedirect_SSItem, GetPromotedUnitDescId
};

static _DECL_INFO RText_Promotion2 = {
    &RText_Promotion1, &RText_Promotion3, &RText_Name, &RText_Skill4,
	0x6C, 0x48, 0,
	HbRedirect_SSItem, GetPromotedUnitDescId
};

static _DECL_INFO RText_Promotion3 = {
    &RText_Promotion2, &RText_Promotion1, &RText_Name, &RText_Skill7,
	0x6C, 0x70, 0,
	HbRedirect_SSItem, GetPromotedUnitDescId
};

/* Skills */
static _DECL_INFO RText_Skill1 = {
    &RText_Skill7, &RText_Skill4, &RText_Promotion1, &RText_Skill2,
	0x90, 0x28, 0,
	HbRedirect_SSItem, GetPromotedUnitSkillId
};
static _DECL_INFO RText_Skill2 = {
    &RText_Skill8, &RText_Skill5, &RText_Skill1, &RText_Skill3,
	0xA0, 0x28, 0,
	HbRedirect_SSItem, GetPromotedUnitSkillId
};
static _DECL_INFO RText_Skill3 = {
    &RText_Skill9, &RText_Skill6, &RText_Skill2, &RText_Name,
	0xB0, 0x28, 0,
	HbRedirect_SSItem, GetPromotedUnitSkillId
};

static _DECL_INFO RText_Skill4 = {
    &RText_Skill1, &RText_Skill7, &RText_Promotion2, &RText_Skill5,
	0x90, 0x50, 0,
	HbRedirect_SSItem, GetPromotedUnitSkillId
};
static _DECL_INFO RText_Skill5 = {
    &RText_Skill2, &RText_Skill8, &RText_Skill4, &RText_Skill6,
	0xA0, 0x50, 0,
	HbRedirect_SSItem, GetPromotedUnitSkillId
};
static _DECL_INFO RText_Skill6 = {
    &RText_Skill3, &RText_Skill9, &RText_Skill5, &RText_Name,
	0xB0, 0x50, 0,
	HbRedirect_SSItem, GetPromotedUnitSkillId
};

static _DECL_INFO RText_Skill7 = {
    &RText_Skill4, &RText_Skill1, &RText_Promotion3, &RText_Skill8,
	0x90, 0x78, 0,
	HbRedirect_SSItem, GetPromotedUnitSkillId
};
static _DECL_INFO RText_Skill8 = {
    &RText_Skill5, &RText_Skill2, &RText_Skill7, &RText_Skill9,
	0xA0, 0x78, 0,
	HbRedirect_SSItem, GetPromotedUnitSkillId
};
static _DECL_INFO RText_Skill9 = {
    &RText_Skill6, &RText_Skill3, &RText_Skill8, &RText_Name,
	0xB0, 0x78, 0,
	HbRedirect_SSItem, GetPromotedUnitSkillId
};

/* Page left */
static _DECL_INFO RText_Name = {
	&RText_Hp, &RText_Class, NULL, &RText_Promotion1,
	0x18, 0x50, 0,
	NULL, HbPopulate_SSCharacter
};

static _DECL_INFO RText_Class = {
	&RText_Name, &RText_Level, NULL, &RText_Promotion2,
	0x06, 0x68, 0,
	NULL, HbPopulate_SSClass
};

static _DECL_INFO RText_Level = {
	&RText_Class, &RText_Hp, NULL, &RText_Exp,
	0x06, 0x78, 0x542,
	NULL, NULL
};

static _DECL_INFO RText_Exp = {
	&RText_Class, &RText_Hp, &RText_Level, &RText_Promotion2,
	0x26, 0x78, 0x543,
	NULL, NULL
};

static _DECL_INFO RText_Hp = {
	&RText_Level, &RText_Name, NULL, &RText_Promotion3,
	0x06, 0x88, 0x544,
	NULL, NULL
};
