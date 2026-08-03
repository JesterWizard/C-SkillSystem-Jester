.include "macros.inc"

/* common */
dat 0x08017d64, gpCharacterData
dat 0x030005D0, gPopupInst
dat 0x030005F0, gpPopupUnit
dat 0x030005F4, gPopupItem
dat 0x030005F8, gPopupNumber

/* IconDisplay */
dat 0x02026A90, gIconReSts @ DrawnIconLookupTable

/* VeslyAchievements */
dat 0x08017AB8, classTablePoin
dat 0x080AECA8, sSoundRoom
dat 0x0802AEC4, sBattleHitArray
dat 0x0802B90A, BattleHitArrayWidth

/* 0x02026AD0 - 0x02026E30 is free (via: _kernel_malloc3) */
