#include "common-chax.h"
#include "item-sys.h"
#include "battle-system.h"
#include "constants/items.h"
#include "constants/texts.h"
#include "bmitemuse.h"
#include "bmtarget.h"
#include "menu_def.h"
#include "mapanim.h"
#include "proc.h"
#include "vanilla.h"

static void AgainStaff_Exec(ProcPtr proc)
{
	struct Unit *unit = GetUnit(gActionData.subjectIndex);
	struct Unit *target = GetUnit(gActionData.targetIndex);

	if (!UNIT_IS_VALID(unit) || !UNIT_IS_VALID(target))
		return;

	BattleInitItemEffect(unit, gActionData.itemSlotIndex);
	BattleInitItemEffectTarget(target);
	BattleApplyItemEffect(proc);

	target->state &= ~(US_UNSELECTABLE | US_HAS_MOVED | US_HAS_MOVED_AI);

	RefreshEntityBmMaps();
	RenderBmMap();
	RefreshUnitSprites();
}

static void AgainStaff_Anim(ProcPtr proc)
{
	PlaySoundEffect(0x269);
	Proc_StartBlocking(ProcScr_DanceringAnim, proc);

	BG_SetPosition(
		BG_0,
		-SCREEN_TILE_IX(gActiveUnit->xPos - 1),
		-SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

static bool AgainStaff_IsAnimRunning(ProcPtr proc)
{
	return Proc_Exists(ProcScr_DanceringAnim);
}

STATIC_DECLAR const struct ProcCmd ProcScr_AgainStaff[] = {
	PROC_CALL(AgainStaff_Exec),
	PROC_CALL(AgainStaff_Anim),
	PROC_WHILE(AgainStaff_IsAnimRunning),
	PROC_END,
};

bool IER_Usability_Again(struct Unit *unit, int item)
{
	if (unit->state & US_CANTOING)
		return false;

	return HasSelectTarget(unit, MakeTargetListForRefresh);
}

void IER_Effect_Again(struct Unit *unit, int item)
{
	gActionData.unk08 = ITEM_STAFF_AGAIN;
	gActionData.subjectIndex = unit->index;
	SetStaffUseAction(unit);

	MakeTargetListForRefresh(unit);
	StartSubtitleHelp(
		NewTargetSelection_Specialized(&gSelectInfo_Dance, StaffSelectOnSelect),
		GetStringFromIndex(MSG_ITEM_AGAIN_STAFF_USEDESC));
}

void IER_Action_Again(ProcPtr proc, struct Unit *unit, int item)
{
	Proc_StartBlocking(ProcScr_AgainStaff, proc);
}