#include "global.h"
#include "bmbattle.h"
#include "bmunit.h"

#define GUARANTEED_LVUP_MAX_RETRIES 10
#define GUARANTEED_LVUP_RETRY_BONUS 10

static void RollBattleUnitStatGains(struct BattleUnit* bu, int growthBonus)
{
	const struct CharacterData* ch = bu->unit.pCharacterData;

	bu->changeHP = GetStatIncrease(ch->growthHP + growthBonus);
	bu->changePow = GetStatIncrease(ch->growthPow + growthBonus);
	bu->changeSkl = GetStatIncrease(ch->growthSkl + growthBonus);
	bu->changeSpd = GetStatIncrease(ch->growthSpd + growthBonus);
	bu->changeDef = GetStatIncrease(ch->growthDef + growthBonus);
	bu->changeRes = GetStatIncrease(ch->growthRes + growthBonus);
	bu->changeLck = GetStatIncrease(ch->growthLck + growthBonus);
}

static int GetTotalStatGains(const struct BattleUnit* bu)
{
	return bu->changeHP + bu->changePow + bu->changeSkl + bu->changeSpd +
		bu->changeDef + bu->changeRes + bu->changeLck;
}

/**
 * Vanilla CheckBattleUnitLevelUp with guaranteed level-up retries:
 * up to 10 full re-rolls at +10% growth when the initial roll gains nothing.
 */
void CheckBattleUnitLevelUp_Guaranteed(struct BattleUnit* bu)
{
	if (!CanBattleUnitGainLevels(bu) || bu->unit.exp < 100)
		return;

	bu->unit.exp -= 100;
	bu->unit.level++;

	if (UNIT_CATTRIBUTES(&bu->unit) & CA_MAXLEVEL10) {
		if (bu->unit.level == 10) {
			bu->expGain -= bu->unit.exp;
			bu->unit.exp = UNIT_EXP_DISABLED;
		}
	} else if (bu->unit.level == 20) {
		bu->expGain -= bu->unit.exp;
		bu->unit.exp = UNIT_EXP_DISABLED;
	}

	{
		int growthBonus = (bu->unit.state & US_GROWTH_BOOST) ? 5 : 0;
		int i;

		RollBattleUnitStatGains(bu, growthBonus);

		for (i = 0; i < GUARANTEED_LVUP_MAX_RETRIES && GetTotalStatGains(bu) == 0; i++)
			RollBattleUnitStatGains(bu, growthBonus + GUARANTEED_LVUP_RETRY_BONUS);
	}

	CheckBattleUnitStatCaps(GetUnit(bu->unit.index), bu);
}
