#include "common-chax.h"
#include "kernel-lib.h"

/**
 * Summons can't gain EXP, so after every action pull each summon up to its
 * summoner's current level. GenerateSummonUnitDef stores the summoner's unit
 * index in the summon's staff rank byte.
 */

static bool IsSummonCharId(int pid)
{
	return pid == CHARACTER_SUMMON_EWAN || pid == CHARACTER_SUMMON_KNOLL || pid == CHARACTER_SUMMON_LYON;
}

bool PostAction_SyncSummonLevels(ProcPtr parent)
{
	int i;

	for (i = 1; i < 0xC0; i++) {
		struct Unit *summon = GetUnit(i);
		struct Unit *summoner;
		int levels, old_max_hp;

		if (!UNIT_IS_VALID(summon) || !IsSummonCharId(UNIT_CHAR_ID(summon)))
			continue;

		if (summon->ranks[ITYPE_STAFF] == 0)
			continue;

		summoner = GetUnit(summon->ranks[ITYPE_STAFF]);
		if (!UNIT_IS_VALID(summoner) || (summoner->state & US_DEAD))
			continue;

		levels = summoner->level - summon->level;
		if (levels <= 0)
			continue;

		old_max_hp = summon->maxHP;
		UnitAutolevelCore(summon, UNIT_CLASS_ID(summon), levels);
		UnitCheckStatCaps(summon);

		summon->level = summoner->level;
		summon->curHP += summon->maxHP - old_max_hp;
	}

	return false;
}
