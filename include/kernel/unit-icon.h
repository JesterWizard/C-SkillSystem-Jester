#pragma once

#include "common-chax.h"

/*
 * Repointed standing map sprite table (vanilla unit_icon_wait_table).
 *
 * Vanilla code masks the id with 0x7F, so the table has 0x80 entries.
 * Ids 0x00-0x6A are the vanilla sprites; the custom sprites start at 0x6B.
 * The table is in Data/UnitIcon/UnitIconWait.c.
 */
#define UNIT_ICON_WAIT_AMT 0x80

extern const UnitIconWait gUnitIconWaitTable[UNIT_ICON_WAIT_AMT];

/* Vanilla sprites used by trap code */
#define SMS_ID_LIGHT_RUNE 0x66

/* Trap sprites */
#define SMS_ID_UNLIT_TORCH 0x6B
#define SMS_ID_TELEPORT_TILE 0x6C
#define SMS_ID_GRASS_TILE 0x6D
#define SMS_ID_BOULDER_TILE 0x6E
#define SMS_ID_SPIN_TILE_RIGHT 0x6F
#define SMS_ID_SPIN_TILE_LEFT 0x70
#define SMS_ID_SPIN_TILE_UP 0x71
#define SMS_ID_SPIN_TILE_DOWN 0x72
#define SMS_ID_HELLSCAPE_VOID 0x73
#define SMS_ID_CALTROPS 0x74
/* Collapsing floor: intact, cracked, hole (follows COLLAPSING_FLOOR_*) */
#define SMS_ID_COLLAPSING_FLOOR 0x75
#define SMS_ID_HEAL_TILE 0x78
#define SMS_ID_DRAGON_VEIN 0x79
#define SMS_ID_LIT_TORCH 0x7A
