#include "global.h"
#include "proc.h"
#include "bmunit.h"
#include "bmbattle.h"
#include "bmmind.h"
#include "classchg.h"

extern const u8 PromotionOnMaxLevelEnabled;
extern const u8 PromotionOnMaxLevelTrainees;
extern const struct ProcCmd ProcScr_PromoHandler[];

#define TRAINEE_PROMOTION_LEVEL 10

/* A unit that levelled up in this battle and should now promote. */
static struct Unit *GetUnitToPromote(const struct BattleUnit *bu)
{
	struct Unit *unit;

	if (bu->unit.index == 0 || bu->unit.level == bu->levelPrevious)
		return NULL;

	unit = GetUnit(bu->unit.index);

	if (!UNIT_IS_VALID(unit) || unit->curHP == 0)
		return NULL;

	if (unit->state & (US_DEAD | US_NOT_DEPLOYED))
		return NULL;

	if (UNIT_CATTRIBUTES(unit) & CA_PROMOTED)
		return NULL;

	if (UNIT_CATTRIBUTES(unit) & CA_MAXLEVEL10) {
		if (PromotionOnMaxLevelTrainees && unit->level >= TRAINEE_PROMOTION_LEVEL)
			return unit;

		return NULL;
	}

	if (PromotionOnMaxLevelEnabled && unit->level == UNIT_LEVEL_MAX)
		return unit;

	return NULL;
}

/**
 * Replaces the BATTLE_HandleCombatDeaths step of vanilla sProcScr_CombatAction.
 * Runs the vanilla step, then, once the battle results are applied to the
 * real units and the dead are handled, opens the vanilla promotion flow for
 * a unit that just reached its level cap.
 * The promotion is a blocking child of the combat action, which yields on
 * the next script command and ends once the promotion is over.
 */
void BATTLE_HandleCombatDeaths_PromoteMaxLevel(struct CombatActionProc *proc)
{
	struct Unit *unit;
	struct ProcPromoHandler *promo;
	u8 subject;

	BATTLE_HandleCombatDeaths(proc);

	unit = GetUnitToPromote(&gBattleActor);
	if (unit == NULL)
		unit = GetUnitToPromote(&gBattleTarget);
	if (unit == NULL)
		return;

	subject = gActionData.subjectIndex;
	gActionData.subjectIndex = unit->index;

	StartBmPromotion(proc);

	/* Do not spend a use of the inventory slot the battle left in gActionData. */
	promo = Proc_Find(ProcScr_PromoHandler);
	if (promo)
		promo->item_slot = -1;

	gActionData.subjectIndex = subject;
}
