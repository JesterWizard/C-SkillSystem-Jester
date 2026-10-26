#include "global.h"
#include "proc.h"
#include "bmunit.h"
#include "bmbattle.h"
#include "bmmind.h"
#include "classchg.h"

extern const struct ProcCmd ProcScr_PromoHandler[];

#define TRAINEE_PROMOTION_LEVEL 10

/* A trainee (CA_MAXLEVEL10) that levelled up to level 10 or higher in this battle. */
static struct Unit *GetTraineeToPromote(const struct BattleUnit *bu)
{
	struct Unit *unit;

	if (bu->unit.index == 0 || bu->unit.level == bu->levelPrevious)
		return NULL;

	unit = GetUnit(bu->unit.index);

	if (!UNIT_IS_VALID(unit) || unit->curHP == 0)
		return NULL;

	if (unit->state & (US_DEAD | US_NOT_DEPLOYED))
		return NULL;

	if (!(UNIT_CATTRIBUTES(unit) & CA_MAXLEVEL10))
		return NULL;

	if (unit->level < TRAINEE_PROMOTION_LEVEL)
		return NULL;

	return unit;
}

/**
 * Replaces the BATTLE_HandleCombatDeaths step of vanilla sProcScr_CombatAction.
 * Runs the vanilla step, then, once the battle results are applied and the
 * dead are handled, opens the vanilla promotion flow for a trainee that just
 * reached level 10, instead of waiting for the next preparation screen.
 * The promotion is a blocking child of the combat action, which yields on
 * the next script command and ends once the promotion is over.
 */
void BATTLE_HandleCombatDeaths_PromoteTrainees(struct CombatActionProc *proc)
{
	struct Unit *unit;
	struct ProcPromoHandler *promo;
	u8 subject;

	BATTLE_HandleCombatDeaths(proc);

	unit = GetTraineeToPromote(&gBattleActor);
	if (unit == NULL)
		unit = GetTraineeToPromote(&gBattleTarget);
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
