#include "common-chax.h"
#include "debuff.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "constants/skills.h"

#define LOCAL_TRACE 0
void StartStatusHealEffect(struct Unit *unit, ProcPtr proc);

LYN_REPLACE_CHECK(StatusDecayDisplay_Display);
void StatusDecayDisplay_Display(struct UnknownBMUSAilmentProc *proc)
{
	struct SelectTarget *target = GetTarget(proc->unk_4C);
	struct Unit *unit = GetUnit(target->uid);
	int status = GetUnitStatusIndex(unit);

	proc->unk_58 = status;

	SetUnitStatus(unit, 0);

	if (gpDebuffInfos[status].tick_type == STATUS_DEBUFF_TICK_ON_ENEMY)
		StartStatusHealEffect(GetUnit(target->uid), proc);
}

LYN_REPLACE_CHECK(TickActiveFactionTurn);
void TickActiveFactionTurn(void)
{
	int displayMapChange = FALSE;

	InitTargets(0, 0);

	FOR_UNITS_ONMAP_FACTION(gPlaySt.faction, unit, {
		if (unit->state & (US_UNAVAILABLE | US_RESCUED))
			continue;

		if (unit->barrierDuration != 0)
			unit->barrierDuration--;

		if (unit->torchDuration != 0) {
			unit->torchDuration--;
			displayMapChange = TRUE;
		}
	})

	if (displayMapChange) {
		RenderBmMapOnBg2();
		RefreshEntityBmMaps();
		RenderBmMap();
		NewBMXFADE(TRUE);
		RefreshUnitSprites();
	}

	#define DEC_STATUS(unit) \
		do { \
			int _status = GetUnitStatusIndex(unit); \
			int _duration = GetUnitStatusDuration(unit); \
			if (_status != 0 && _duration != 0) { \
				if (_status != UNIT_STATUS_RECOVER) \
					TryTickUnitStatusDuration(unit); \
				if (GetUnitStatusDuration(unit) == 0) \
					AddTarget(unit->xPos, unit->yPos, unit->index, 0); \
				LTRACEF("[uid=0x%02X, pid=0x%02X] status=%d, dura=%d", unit->index & 0xFF, UNIT_CHAR_ID(unit), _status, _duration); \
			} \
		} while (0)

	if (gPlaySt.faction == FACTION_BLUE) {
		/* Blue buff */
		FOR_UNITS_ONMAP_FACTION(FACTION_BLUE, unit, {
			/* Doom's last turn is left to the poison display, which kills the unit */
			bool doomStatus = false;

			if (GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_DOOM) 
				doomStatus = true;

			if (!(doomStatus && GetUnitStatusDuration(unit) == 1) &&
			    gpDebuffInfos[GetUnitStatusIndex(unit)].tick_type == STATUS_DEBUFF_TICK_ON_ALLY)
			{
				DEC_STATUS(unit);

#if (defined(SID_ShedSkin) && COMMON_SKILL_VALID(SID_ShedSkin))
                if (SkillTester(unit, SID_ShedSkin))
                    DEC_STATUS(unit);
#endif
			}

            if (GetUnitStatusDuration(unit) == 0 && doomStatus == true)
                UnitKill(unit);

			TickUnitStatDebuff(unit, STATUS_DEBUFF_TICK_ON_ALLY);
		})

		/* Red debuff */
		FOR_UNITS_ONMAP_FACTION(FACTION_RED, unit, {
			/* Doom's last turn is left to the poison display, which kills the unit */
			bool doomStatus = false;

			if (GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_DOOM) 
				doomStatus = true;

			if (!(doomStatus && GetUnitStatusDuration(unit) == 1) &&
			    gpDebuffInfos[GetUnitStatusIndex(unit)].tick_type == STATUS_DEBUFF_TICK_ON_ENEMY)
			{
				DEC_STATUS(unit);

#if (defined(SID_ShedSkin) && COMMON_SKILL_VALID(SID_ShedSkin))
                if (SkillTester(unit, SID_ShedSkin))
                    DEC_STATUS(unit);
#endif
			}

            if (GetUnitStatusDuration(unit) == 0 && doomStatus == true)
                UnitKill(unit);

			TickUnitStatDebuff(unit, STATUS_DEBUFF_TICK_ON_ENEMY);
		})
	} else if (gPlaySt.faction == FACTION_RED) {
		/* Red buff */
		FOR_UNITS_ONMAP_FACTION(FACTION_RED, unit, {
			/* Doom's last turn is left to the poison display, which kills the unit */
			bool doomStatus = false;

			if (GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_DOOM) 
				doomStatus = true;

			if (!(doomStatus && GetUnitStatusDuration(unit) == 1) &&
			    gpDebuffInfos[GetUnitStatusIndex(unit)].tick_type == STATUS_DEBUFF_TICK_ON_ALLY)
			{
				DEC_STATUS(unit);

#if (defined(SID_ShedSkin) && COMMON_SKILL_VALID(SID_ShedSkin))
                if (SkillTester(unit, SID_ShedSkin))
                    DEC_STATUS(unit);
#endif
			}

            if (GetUnitStatusDuration(unit) == 0 && doomStatus == true)
                UnitKill(unit);

			TickUnitStatDebuff(unit, STATUS_DEBUFF_TICK_ON_ALLY);
		})

		/* Blue debuff */
		FOR_UNITS_ONMAP_FACTION(FACTION_BLUE, unit, {
			/* Doom's last turn is left to the poison display, which kills the unit */
			bool doomStatus = false;

			if (GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_DOOM) 
				doomStatus = true;

			if (!(doomStatus && GetUnitStatusDuration(unit) == 1) &&
			    gpDebuffInfos[GetUnitStatusIndex(unit)].tick_type == STATUS_DEBUFF_TICK_ON_ENEMY)
			{
				DEC_STATUS(unit);

#if (defined(SID_ShedSkin) && COMMON_SKILL_VALID(SID_ShedSkin))
                if (SkillTester(unit, SID_ShedSkin))
                    DEC_STATUS(unit);
#endif
			}

            if (GetUnitStatusDuration(unit) == 0 && doomStatus == true)
                UnitKill(unit);

			TickUnitStatDebuff(unit, STATUS_DEBUFF_TICK_ON_ENEMY);
		})

		/* Green debuff */
		FOR_UNITS_ONMAP_FACTION(FACTION_GREEN, unit, {
		  	bool doomStatus = false;

			if (GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_DOOM) 
				doomStatus = true;

			if (!(doomStatus && GetUnitStatusDuration(unit) == 1) &&
			    gpDebuffInfos[GetUnitStatusIndex(unit)].tick_type == STATUS_DEBUFF_TICK_ON_ENEMY)
			{
				DEC_STATUS(unit);

#if (defined(SID_ShedSkin) && COMMON_SKILL_VALID(SID_ShedSkin))
                if (SkillTester(unit, SID_ShedSkin))
                    DEC_STATUS(unit);
#endif
			}

            if (GetUnitStatusDuration(unit) == 0 && doomStatus == true)
                UnitKill(unit);

			TickUnitStatDebuff(unit, STATUS_DEBUFF_TICK_ON_ENEMY);
		})
	} else if (gPlaySt.faction == FACTION_GREEN) {
		/* Green buff */
		FOR_UNITS_ONMAP_FACTION(FACTION_GREEN, unit, {
			/* Doom's last turn is left to the poison display, which kills the unit */
			bool doomStatus = false;

			if (GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_DOOM) 
				doomStatus = true;
			
			if (!(doomStatus && GetUnitStatusDuration(unit) == 1) &&
			    gpDebuffInfos[GetUnitStatusIndex(unit)].tick_type == STATUS_DEBUFF_TICK_ON_ALLY)
			{
				DEC_STATUS(unit);

#if (defined(SID_ShedSkin) && COMMON_SKILL_VALID(SID_ShedSkin))
                if (SkillTester(unit, SID_ShedSkin))
                    DEC_STATUS(unit);
#endif
			}

            if (GetUnitStatusDuration(unit) == 0 && doomStatus == true)
                UnitKill(unit);

			TickUnitStatDebuff(unit, STATUS_DEBUFF_TICK_ON_ALLY);
		})
	}
}

/*
 * Phase-start poison damage: vanilla's 1-3 for Poison; Toxic, which takes
 * 10% of the unit's current HP per turn it has had the status (10%, 20%,
 * 30%), at least 1; and Doom on its last turn, all the unit's HP.
 */
LYN_REPLACE_CHECK(MakePoisonDamageTargetList);
void MakePoisonDamageTargetList(int faction)
{
	int i;

	InitTargets(0, 0);

	for (i = faction + 1; i < faction + 0x40; i++) {
		struct Unit *unit = GetUnit(i);
		int status, damage;

		if (!UNIT_IS_VALID(unit))
			continue;

		if (unit->state & (US_HIDDEN | US_UNAVAILABLE))
			continue;

		status = GetUnitStatusIndex(unit);

		if (status == NEW_UNIT_STATUS_DOOM && GetUnitStatusDuration(unit) == 1) {
			/*
			 * Doom's last turn: the poison display deals the killing damage, so the
			 * unit dies the usual way (death animation, quote, defeat checks)
			 * before the status runs out below.
			 */
			damage = unit->curHP;
		} else if (status == UNIT_STATUS_POISON) {
			damage = NextRN_N(3) + 1;
		} else if (status == NEW_UNIT_STATUS_TOXIC_POISON) {
			int elapsed = gpDebuffInfos[status].duration - GetUnitStatusDuration(unit) + 1;

			if (elapsed < 1)
				elapsed = 1;

			damage = Div(unit->curHP * 10 * elapsed, 100);

			if (damage < 1)
				damage = 1;
		} else {
			continue;
		}

		AddTarget(unit->xPos, unit->yPos, unit->index, damage);
	}
}
