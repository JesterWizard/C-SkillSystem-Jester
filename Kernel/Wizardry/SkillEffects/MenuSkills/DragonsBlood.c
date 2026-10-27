#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "event-rework.h"
#include "debuff.h"
#include "constants/skills.h"
#include "constants/texts.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
#define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

/*
 * Dragon's Blood: a unit standing on a Dragon Vein (TRAP_DRAGON_VEIN) can
 * activate it. The vein's effect hits the whole map, then the vein is gone.
 */

#if defined(SID_DragonsBlood) && (COMMON_SKILL_VALID(SID_DragonsBlood))

static struct Trap *GetDragonVeinUnderUnit(struct Unit *unit)
{
	return GetTypedTrapAt(unit->xPos, unit->yPos, TRAP_DRAGON_VEIN);
}

u8 DragonsBlood_Usability(const struct MenuItemDef *def, int number)
{
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if (!GetDragonVeinUnderUnit(gActiveUnit))
		return MENU_NOTSHOWN;

	return MENU_ENABLED;
}

u8 DragonsBlood_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
		return MENU_ACT_SND6B;
	}

	gActionData.unk08 = SID_DragonsBlood;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

	PlaySoundEffect(0x6A);
	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static bool IsDragonVeinTarget(struct Unit *unit)
{
	if (!UNIT_IS_VALID(unit) || unit->curHP <= 0)
		return false;

	return !(unit->state & (US_DEAD | US_NOT_DEPLOYED | US_BIT16));
}

static void DragonVein_Apply(ProcPtr proc)
{
	struct Trap *trap = GetDragonVeinUnderUnit(gActiveUnit);
	int effect, param, i;

	if (!trap)
		return;

	effect = trap->extra;
	param = (u8)trap->data[TRAP_EXTDATA_DRAGON_VEIN_PARAM];
	RemoveTrap(trap);

	if (effect == DRAGON_VEIN_MAPCHANGE) {
		TriggerMapChanges(param, true, proc);
		RefreshUnitSprites();
		return;
	}

	for (i = 1; i < 0xC0; i++) {
		struct Unit *unit = GetUnit(i);
		bool ally;
		int hp;

		if (!IsDragonVeinTarget(unit))
			continue;

		ally = AreUnitsAllied(gActiveUnit->index, unit->index);

		switch (effect) {
		case DRAGON_VEIN_HEAL:
			if (ally)
				AddUnitHp(unit, param ? param : GetUnitMaxHp(unit));
			break;

		case DRAGON_VEIN_DAMAGE:
			if (ally)
				break;

			hp = unit->curHP - (param ? param : 10);
			SetUnitHp(unit, hp < 1 ? 1 : hp);
			break;

		case DRAGON_VEIN_STATUS:
			if (!ally && param > 0 && param < NEW_UNIT_STATUS_MAX)
				SetUnitStatusIndex(unit, param);
			break;
		}
	}

	RefreshUnitSprites();
}

static const EventScr EventScr_DragonVein[] = {
	SVAL(EVT_SLOT_B, SID_DragonsBlood)
	CALL(EventScr_MuSkillAnim)
	STAL(1)
	EvtStartEarthQuake(0, 1)
	STAL(40)
	ASMC(DragonVein_Apply)
	STAL(40)
	EvtEndEarthQuake
	STAL(10)
	NOFADE
	ENDA
};

bool Action_DragonsBlood(ProcPtr parent)
{
	ClearBg0Bg1();
	KernelCallEvent(EventScr_DragonVein, EV_EXEC_CUTSCENE, parent);
	return true;
}

#endif
