#include "common-chax.h"
#include "map-task.h"

const struct MapTaskConf gMapTaskConfs[] = {
	[MTSKCONF_HPBAR]        = {  0,  9, gObject_16x8 },
	[MTSKCONF_COMM_BUFF]    = { -1,  8, gObject_8x8 },
	[MTSKCONF_COMM_DEBUFF]  = { -1,  8, gObject_8x8 },
	[MTSKCONF_RESCUING]     = {  9,  7, gObject_8x8 },
	[MTSKCONF_BOSS]         = {  9,  7, gObject_8x8 },
	[MTSKCONF_PROTECTION]   = {  9,  7, gObject_8x8 },
	[MTSKCONF_TALK_1]   	= {  -6, -14, gObject_32x8 },
	[MTSKCONF_TALK_2]   	= {  -6, -6, gObject_32x8 },
	[MTSKCONF_DROP_ITEM]	= {  9, 7, gObject_8x8 },
	[MTSKCONF_WARNING]		= {  3, 2, gObject_8x8 },
	[MTSKCONF_STATUS_SLOW]  = {  0, -4, gObject_8x8 },
	[MTSKCONF_STATUS_BREAK] = {  0, -4, gObject_8x8 },
	[MTSKCONF_STATUS_TOXIC] = {  0, -4, gObject_8x8 },
	[MTSKCONF_STATUS_HIDE]  = {  0, -4, gObject_8x8 },
	[MTSKCONF_STATUS_DECOY] = {  0, -4, gObject_8x8 },
	[MTSKCONF_STATUS_DOOM]  = {  0, -4, gObject_8x8 },
	[MTSKCONF_STATUS_REPLICATE]  = {  0, -4, gObject_8x8 },
	[MTSKCONF_STATUS_EFFECTIVE]  = {  0, -4, gObject_8x8 },
	[MTSKCONF_STATUS_SHOCK]  = {  0, -4, gObject_8x8 },
	[MTSKCONF_STATUS_PANIC] = {  0, -4, gObject_8x8 },
	// [MTSKCONF_STATUS_ENFEEBLE] = {  0, -4, gObject_8x8 },
	// [MTSKCONF_STATUS_PROVIDENCE] = {  0, -4, gObject_8x8 },
	// [MTSKCONF_STATUS_BOUND] = {  0, -4, gObject_8x8 },
	// [MTSKCONF_STATUS_HEX] = {  0, -4, gObject_8x8 },
};

const u8 gHpBarChr[] = {
	      0x74, 0x74,
	0x56, 0x54, 0x52,
	0x36, 0x34, 0x32,
	0x16, 0x14, 0x12
};
