#pragma once

#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"

/**
 * We use unit->_u3A and unit->_u3B
 * These 16 bits is saved in suspand data
 */

enum unit_expa_sus_bitfile_idx {

#if defined(SID_Teleportation) && (COMMON_SKILL_VALID(SID_Teleportation))
    UES_BIT_TELEPORTATION_SKILL_USED,
#endif
#if defined(SID_Capture) && (COMMON_SKILL_VALID(SID_Capture))
    UES_BIT_CAPTURE_SKILL_USED,
#endif
#if defined(SID_Turncoat) && (COMMON_SKILL_VALID(SID_Turncoat))
    UES_BIT_CHANGED_FACTIONS,
#endif
#if defined(SID_Resurrection) && (COMMON_SKILL_VALID(SID_Resurrection))
    UES_BIT_RESURRECTION_SKILL_USED,
#endif
#if defined(SID_ShadowFlash) && (COMMON_SKILL_VALID(SID_ShadowFlash))
    UES_BIT_SHADOWFLASH_SKILL_USED,
#endif
#if defined(SID_TwinCrests) && (COMMON_SKILL_VALID(SID_TwinCrests))
    UES_BIT_MOVE_AGAIN_SKILL_USED,
#endif
#if defined(SID_DeathBlight) && (COMMON_SKILL_VALID(SID_DeathBlight))
    UES_BIT_DEATHBLIGHT_SKILL_USED,
#endif
#if defined(SID_LoadstarRush) && (COMMON_SKILL_VALID(SID_LoadstarRush))
    UES_BIT_LOADSTAR_RUSH_SKILL_USED,
#endif
#if defined(SID_WyvernCrash) && (COMMON_SKILL_VALID(SID_WyvernCrash))
    UES_BIT_WYVERN_CRASH_SKILL_USED,
#endif
#if defined(SID_Sellsword) && (COMMON_SKILL_VALID(SID_Sellsword))
    UES_BIT_SELLSWORD_SKILL_USED,
#endif
#if defined(SID_Switcher) && (COMMON_SKILL_VALID(SID_Switcher)) // This is a special case, we use the bit to determine if doubling should be allowed
    UES_BIT_SWITCHER_SKILL_USED,
#endif
#if defined(SID_GrimReaper) && (COMMON_SKILL_VALID(SID_GrimReaper))
    UES_BIT_GRIM_REAPER_SKILL_USED,
#endif
#if defined(SID_ResolvedHeart) && (COMMON_SKILL_VALID(SID_ResolvedHeart))
    UES_BIT_RESOLVED_HEART_SKILL_USED,
#endif
#if defined(SID_Bide) && (COMMON_SKILL_VALID(SID_Bide))
    UES_BIT_BIDE_SKILL_USED,
#endif
#if defined(SID_Protect) && (COMMON_SKILL_VALID(SID_Protect))
    UES_BIT_PROTECT_SKILL_USED,
#endif
#if defined(SID_GorillaTactics) && (COMMON_SKILL_VALID(SID_GorillaTactics))
	UES_BIT_GORILLA_TACTICS_SKILL_USED,
#endif

};

void SetBitUES_BU(struct BattleUnit * bu, int bit);
void SetBitUES(struct Unit *unit, int bit);
void ClearBitUES(struct Unit *unit, int bit);
bool CheckBitUES(struct Unit *unit, int bit);

void ResetUnitsExpaSus(void);

/**
 * Demo unit expa
 */
struct DemoUnitExpa {
	u8 _pad_[4];
};

extern struct DemoUnitExpa sDemoUnitExpaAlly[CONFIG_UNIT_AMT_ALLY];
extern struct DemoUnitExpa sDemoUnitExpaEnemy[CONFIG_UNIT_AMT_ENEMY + 1];
extern struct DemoUnitExpa sDemoUnitExpaNpc[CONFIG_UNIT_AMT_NPC];
extern struct DemoUnitExpa sDemoUnitExpaBattle[2];
extern struct DemoUnitExpa *const sDemoUnitExpaPool[0x100];
extern struct DemoUnitExpa *const *const gpDemoUnitExpaPool;

struct DemoUnitExpa *GetDemoUnitExpa(struct Unit *unit);
