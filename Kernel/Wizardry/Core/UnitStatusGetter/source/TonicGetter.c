#include "common-chax.h"
#include "skill-system.h"
#include "status-getter.h"
#include "constants/skills.h"
#include "bwl.h"
#include "unit-expa.h"
#include "debuff.h"

extern int GetTonicStatBonus(struct Unit *unit, int tonicIndex);

int DefGetterTonic(int status, struct Unit *unit)
{
	return status + GetTonicStatBonus(unit, 6);
}

int ResGetterTonic(int status, struct Unit *unit)
{
	return status + GetTonicStatBonus(unit, 7);
}

int MovGetterTonic(int status, struct Unit *unit)
{
	return status + GetTonicStatBonus(unit, 8);
}

int ConGetterTonic(int status, struct Unit *unit)
{
	return status + GetTonicStatBonus(unit, 9);
}