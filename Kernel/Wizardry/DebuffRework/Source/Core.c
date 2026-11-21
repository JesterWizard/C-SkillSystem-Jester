#include "common-chax.h"
#include "debuff.h"
#include "skill-system.h"
#include "constants/skills.h"

#define LOCAL_TRACE 0

int GetUnitStatusIndex(struct Unit *unit)
{
	return UNIT_STATUS_INDEX(unit);
}

int GetUnitStatusDuration(struct Unit *unit)
{
	int ret = 0;

	if (UNIT_STATUS_INDEX(unit) != 0)
		ret = UNIT_STATUS_DURATION(unit) + 1;

	LTRACEF("[uid=0x%02X, pid=0x%02X] status=%d, dura=%d",
		unit->index & 0xFF, UNIT_CHAR_ID(unit),
		UNIT_STATUS_INDEX(unit), ret);

	return ret;
}

/*
 * Hex halves max HP, and current HP is held to the halved value while it
 * lasts. When Hex ends (it wears off, is cured or replaced), the HP the
 * halving took comes back: 30/30 hexed is 15/15, takes 5 to 10, and is 25/30
 * once Hex is gone.
 */
static void ChangeUnitStatusIndex(struct Unit *unit, int status)
{
	int hexed_max = 0;

	if (UNIT_STATUS_INDEX(unit) == NEW_UNIT_STATUS_HEX && status != NEW_UNIT_STATUS_HEX && unit->curHP > 0)
		hexed_max = GetUnitMaxHp(unit);

	UNIT_STATUS_INDEX(unit) = status;

	if (hexed_max > 0) {
		int full_max = GetUnitMaxHp(unit);
		int hp = unit->curHP + full_max - hexed_max;

		if (hp > full_max)
			hp = full_max;

		if (hp > unit->curHP)
			unit->curHP = hp;
	}
}

void SetUnitStatusIndex(struct Unit *unit, int status)
{
	if (status < NEW_UNIT_STATUS_MAX)
		ChangeUnitStatusIndex(unit, status);
}

void SetUnitStatusDuration(struct Unit *unit, int duration)
{
	if (duration == 0 && gpDebuffInfos[GetUnitStatusIndex(unit)].duration > 0) {
		ChangeUnitStatusIndex(unit, 0);
		UNIT_STATUS_DURATION(unit) = 0;
		return;
	}
	else if (gpDebuffInfos[GetUnitStatusIndex(unit)].duration == 0)
	{
		UNIT_STATUS_DURATION(unit) = 0;
		return;
	}

	/* Two bits hold duration - 1: Residium and Malidiction together can ask for 5 */
	if (duration > NEW_UNIT_STATUS_MAX_DURATION)
		duration = NEW_UNIT_STATUS_MAX_DURATION;

	UNIT_STATUS_DURATION(unit) = duration - 1;
}

int TryTickUnitStatusDuration(struct Unit *unit)
{
	if (UNIT_STATUS_INDEX(unit) == 0)
		return -1;

	LTRACEF("[pid=0x%02X] status=%d, dura=%d",
		UNIT_CHAR_ID(unit), UNIT_STATUS_INDEX(unit), UNIT_STATUS_DURATION(unit));

	if (UNIT_STATUS_DURATION(unit) != 0) {
		UNIT_STATUS_DURATION(unit)--;
	} else {
		ChangeUnitStatusIndex(unit, 0);
		UNIT_STATUS_DURATION(unit) = 0;
	}
	return 0;
}
