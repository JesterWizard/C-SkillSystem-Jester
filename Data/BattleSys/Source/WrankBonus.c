#include "common-chax.h"
#include "wrank-bonus.h"
#include "constants/texts.h"

struct WrankRtextConfEnt const gWrankRtextConf[] = {
	{ MSG_0561, ITYPE_SWORD },
	{ MSG_0562, ITYPE_LANCE },
	{ MSG_0563, ITYPE_AXE },
	{ MSG_0564, ITYPE_BOW },
	{ MSG_0565, ITYPE_ANIMA },
	{ MSG_0566, ITYPE_LIGHT },
	{ MSG_0567, ITYPE_DARK },
	{ MSG_0568, ITYPE_STAFF },
	{ MSG_WRANK_KNIFE_MASTERY, ITYPE_KNIFE },
	{}
};

struct WrankBonusConfEnt const gWrankBonusConf[] = {
	{
		.wtype = ITYPE_SWORD,
		.wrank = WPN_LEVEL_D,
		.bonus = {
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_SWORD,
		.wrank = WPN_LEVEL_C,
		.bonus = {
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_SWORD,
		.wrank = WPN_LEVEL_B,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_SWORD,
		.wrank = WPN_LEVEL_A,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_SWORD,
		.wrank = WPN_LEVEL_S,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 5,
			[BATTLE_STATUS_CRIT] = 5,
			[BATTLE_STATUS_SILENCER] = 0,
		},
	},
	{
		.wtype = ITYPE_LANCE,
		.wrank = WPN_LEVEL_D,
		.bonus = {
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_LANCE,
		.wrank = WPN_LEVEL_C,
		.bonus = {
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_LANCE,
		.wrank = WPN_LEVEL_B,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_LANCE,
		.wrank = WPN_LEVEL_A,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_LANCE,
		.wrank = WPN_LEVEL_S,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 5,
			[BATTLE_STATUS_CRIT] = 5,
			[BATTLE_STATUS_SILENCER] = 0,
		},
	},
	{
		.wtype = ITYPE_AXE,
		.wrank = WPN_LEVEL_D,
		.bonus = {
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_AXE,
		.wrank = WPN_LEVEL_C,
		.bonus = {
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_AXE,
		.wrank = WPN_LEVEL_B,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_AXE,
		.wrank = WPN_LEVEL_A,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_AXE,
		.wrank = WPN_LEVEL_S,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 5,
			[BATTLE_STATUS_CRIT] = 5,
			[BATTLE_STATUS_SILENCER] = 0,
		},
	},
	{
		.wtype = ITYPE_BOW,
		.wrank = WPN_LEVEL_D,
		.bonus = {
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_BOW,
		.wrank = WPN_LEVEL_C,
		.bonus = {
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_BOW,
		.wrank = WPN_LEVEL_B,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_BOW,
		.wrank = WPN_LEVEL_A,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_BOW,
		.wrank = WPN_LEVEL_S,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 5,
			[BATTLE_STATUS_CRIT] = 5,
			[BATTLE_STATUS_SILENCER] = 0,
		},
	},
	{
		.wtype = ITYPE_ANIMA,
		.wrank = WPN_LEVEL_D,
		.bonus = {
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_ANIMA,
		.wrank = WPN_LEVEL_C,
		.bonus = {
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_ANIMA,
		.wrank = WPN_LEVEL_B,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_ANIMA,
		.wrank = WPN_LEVEL_A,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_ANIMA,
		.wrank = WPN_LEVEL_S,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 5,
			[BATTLE_STATUS_CRIT] = 5,
			[BATTLE_STATUS_SILENCER] = 0,
		},
	},
	{
		.wtype = ITYPE_LIGHT,
		.wrank = WPN_LEVEL_D,
		.bonus = {
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_LIGHT,
		.wrank = WPN_LEVEL_C,
		.bonus = {
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_LIGHT,
		.wrank = WPN_LEVEL_B,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_LIGHT,
		.wrank = WPN_LEVEL_A,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_LIGHT,
		.wrank = WPN_LEVEL_S,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 5,
			[BATTLE_STATUS_CRIT] = 5,
			[BATTLE_STATUS_SILENCER] = 0,
		},
	},
	{
		.wtype = ITYPE_DARK,
		.wrank = WPN_LEVEL_D,
		.bonus = {
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_DARK,
		.wrank = WPN_LEVEL_C,
		.bonus = {
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_DARK,
		.wrank = WPN_LEVEL_B,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_DARK,
		.wrank = WPN_LEVEL_A,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 0,
		},
	},
	{
		.wtype = ITYPE_DARK,
		.wrank = WPN_LEVEL_S,
		.bonus = {
			[BATTLE_STATUS_ATK] = 0,
			[BATTLE_STATUS_HIT] = 5,
			[BATTLE_STATUS_CRIT] = 5,
			[BATTLE_STATUS_SILENCER] = 0,
		},
	},
	{}
};
