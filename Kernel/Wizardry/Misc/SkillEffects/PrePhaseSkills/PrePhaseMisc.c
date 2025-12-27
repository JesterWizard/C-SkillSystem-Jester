#include "common-chax.h"
#include "unit-expa.h"
#include "bwl.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "jester_headers/custom-arrays.h"
#include "jester_headers/custom-functions.h"

bool PrePhsae_ClearMiscUES(ProcPtr proc)
{
	int uid;
	struct Unit* unit;

	for (uid = 1; uid < 0xC0; uid++) {
		unit = GetUnit(uid);

		if (UNIT_IS_VALID(unit)) {

#if defined(SID_Teleportation) && (COMMON_SKILL_VALID(SID_Teleportation))
            ClearBitUES(unit, UES_BIT_TELEPORTATION_SKILL_USED);
#endif
#if defined(SID_Turncoat) && (COMMON_SKILL_VALID(SID_Turncoat))
            ClearBitUES(unit, UES_BIT_CHANGED_FACTIONS);
#endif
#if defined(SID_ShadowFlash) && (COMMON_SKILL_VALID(SID_ShadowFlash))
            ClearBitUES(unit, UES_BIT_SHADOWFLASH_SKILL_USED);
#endif
#if defined(SID_TwinCrests) && (COMMON_SKILL_VALID(SID_TwinCrests))
            ClearBitUES(unit, UES_BIT_MOVE_AGAIN_SKILL_USED);
#endif
#if defined(SID_DeathBlight) && (COMMON_SKILL_VALID(SID_DeathBlight))
            ClearBitUES(unit, UES_BIT_DEATHBLIGHT_SKILL_USED);
#endif
#if defined(SID_Persuade) && (COMMON_SKILL_VALID(SID_Persuade))
            ClearBitUES(unit, UES_BIT_CHANGED_FACTIONS);
#endif
#if defined(SID_Protect) && (COMMON_SKILL_VALID(SID_Protect))
            ClearBitUES(unit, UES_BIT_PROTECT_SKILL_USED);
#endif

            /* We don't clear these bits as they're meant to be once per unit */
            // ClearBitUES(unit, UES_BIT_LOADSTAR_RUSH_SKILL_USED);
            // ClearBitUES(unit, UES_BIT_WYVERN_CRASH_SKILL_USED);
            // ClearBitUES(unit, UES_BIT_RESOLVED_HEART_SKILL_USED);
            // ClearBitUES(unit, UES_BIT_BIDE_SKILL_USED);
		}
	}
	return false;
}

void PrePhase_ApplyMpStartingAmount(ProcPtr proc)
{
	struct Unit* unit;
	int unit_id;
	struct NewBwl* bwl;

	if (gPlaySt.faction == FACTION_BLUE)
	{
		for (int uid = FACTION_BLUE; uid < FACTION_GREEN; uid++) {

			unit = GetUnit(uid);
			unit_id = UNIT_CHAR_ID(unit);
			bwl = GetNewBwl(unit_id);

			if (UNIT_IS_VALID(unit) && bwl != NULL)
			{
				int idleMPGeneration = gMpSystemPInfoConfigList[unit_id].idleGeneration;
				int initialMP =  gMpSystemPInfoConfigList[unit_id].initialMP;
				if (gPlaySt.chapterTurnNumber == 1 && bwl->currentMP == 0)
				{
#if defined(SID_Enlightenment) && (COMMON_SKILL_VALID(SID_Enlightenment))
					if (SkillTester(unit, SID_Enlightenment))
						initialMP = bwl->maxMP;
#endif
					bwl->currentMP = initialMP;
				}

#if defined(SID_MPChanneling) && (COMMON_SKILL_VALID(SID_MPChanneling))
				if (SkillTester(unit, SID_MPChanneling))
					idleMPGeneration *= 2;
#endif

#if defined(SID_Enlightenment) && (COMMON_SKILL_VALID(SID_Enlightenment))
				if (SkillTester(unit, SID_Enlightenment))
					idleMPGeneration = 0;
#endif

					bwl->currentMP += idleMPGeneration;
			}

			// Clamp the value to max MP using a ternary operator
			int maxMP = GetUnitMaxMP(unit);
			bwl->currentMP = (bwl->currentMP > maxMP) ? maxMP : bwl->currentMP;
		}
	}
}