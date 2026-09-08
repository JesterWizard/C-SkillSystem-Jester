#include "common-chax.h"
#include "skill-system.h"
#include "status-getter.h"
#include "constants/skills.h"
#include "bwl.h"
#include "unit-expa.h"
#include "debuff.h"
#include "jester_headers/custom-functions.h"

int HPTonic(struct Unit *unit)
{
	return GetTonicStatBonus(unit, 0);
}

int PowTonic(struct Unit *unit)
{
	return GetTonicStatBonus(unit, 1);
}

int MagTonic(struct Unit *unit)
{
	return GetTonicStatBonus(unit, 2);
}

int SklTonic(struct Unit *unit)
{
	return GetTonicStatBonus(unit, 3);
}

int SpdTonic(struct Unit *unit)
{
	return GetTonicStatBonus(unit, 4);
}

int LckTonic(struct Unit *unit)
{
	return GetTonicStatBonus(unit, 5);
}

int DefTonic(struct Unit *unit)
{
	return GetTonicStatBonus(unit, 6);
}

int ResTonic(struct Unit *unit)
{
	return GetTonicStatBonus(unit, 7);
}

int OmniTonic(struct Unit *unit)
{
	return GetTonicStatBonus(unit, 8);
}
