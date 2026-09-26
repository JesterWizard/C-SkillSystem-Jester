#include "global.h"
#include "proc.h"
#include "mapanim.h"
#include "face.h"
#include "hardware.h"
#include "bmunit.h"
#include "bmmind.h"
#include "classchg.h"

extern const u8 PromotionOnMaxLevelEnabled;
extern const struct ProcCmd ProcScr_PromoHandler[];

/**
 * Vanilla ManimLevelUp_ScrollOut.
 * When the window finishes scrolling out, an unpromoted unit at the
 * vanilla level cap opens the promotion screen.
 * item_slot is forced to -1 so ExecClassChgReal does not spend a use
 * of whatever inventory slot the battle left in gActionData.
 */
void ManimLevelUp_ScrollOut_Promo(struct ManimLevelUpProc *proc)
{
	struct Unit *unit;
	struct ProcPromoHandler *promo;

	proc->y_scroll_offset -= 8;

	BG_SetPosition(BG_0, 0, proc->y_scroll_offset);
	BG_SetPosition(BG_1, 0, proc->y_scroll_offset);

	gFaces[0]->yPos = 32 - proc->y_scroll_offset;

	if (proc->y_scroll_offset > -144)
		return;

	if (PromotionOnMaxLevelEnabled) {
		unit = gManimSt.actor[proc->actor_id].unit;
		if (unit->level == UNIT_LEVEL_MAX && !(UNIT_CATTRIBUTES(unit) & CA_PROMOTED)) {
			gActionData.subjectIndex = unit->index;
			StartBmPromotion(proc);
			promo = Proc_Find(ProcScr_PromoHandler);
			if (promo)
				promo->item_slot = -1;
			GetUnit(gActionData.subjectIndex)->level = 1;
			GetUnit(gActionData.subjectIndex)->exp = 0;
			gActionData.subjectIndex = 0;
		}
	}

	Proc_Break(proc);
}
