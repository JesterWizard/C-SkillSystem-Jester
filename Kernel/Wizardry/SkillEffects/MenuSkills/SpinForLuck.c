#include "common-chax.h"
#include "skill-system.h"
#include "skill-wheel.h"
#include "debuff.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "constants/gfx.h"
#include "jester_headers/custom-functions.h"

#if defined(SID_SpinForLuck) && (COMMON_SKILL_VALID(SID_SpinForLuck))

enum {
	SPIN_FOR_LUCK_COST = 1000,
};

/* Must match SECTIONS and PAL in Contents/Gfx/Scripts/SpinForLuck/make_wheel.py. */
static const u8 sSpinForLuckStatus[SKILL_WHEEL_SECTIONS] = {
	UNIT_STATUS_ATTACK,
	UNIT_STATUS_POISON,
	UNIT_STATUS_DEFENSE,
	UNIT_STATUS_SLEEP,
	UNIT_STATUS_CRIT,
	UNIT_STATUS_SILENCED,
	UNIT_STATUS_AVOID,
	UNIT_STATUS_RECOVER,
};

static const u16 sSpinForLuckPal[16] = {
	0x0000, 0x0865, 0x235F, 0x11F6, 0x6586, 0x4CCF, 0x7FFF, 0x6B17,
	0x41CD, 0x14BB, 0x563F, 0x232B, 0x15C4, 0x7F73, 0x37BF, 0x1572,
};

/* Fills the target list with the allies next to unit; returns how many there are. */
static int SpinForLuck_MakeTargets(struct Unit *unit)
{
	InitTargets(unit->xPos, unit->yPos);
	MakeTargetListForAdjacentSameFaction(unit);
	return GetSelectTargetCount();
}

u8 SpinForLuck_Usability(const struct MenuItemDef *def, int number)
{
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if ((int)GetPartyGoldAmount() < SPIN_FOR_LUCK_COST)
		return MENU_DISABLED;

	if (SpinForLuck_MakeTargets(gActiveUnit) == 0)
		return MENU_DISABLED;

	return MENU_ENABLED;
}

u8 SpinForLuck_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED) {
		if ((int)GetPartyGoldAmount() < SPIN_FOR_LUCK_COST)
			MenuFrozenHelpBox(menu, MSG_SKILL_SpinForLuck_FRtext);
		else
			MenuFrozenHelpBox(menu, MSG_No_Allies);

		return MENU_ACT_SND6B;
	}

	gActionData.unk08 = SID_SpinForLuck;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static const char *SpinForLuck_Describe(int section, char *buf, int *color)
{
	const struct DebuffInfo *info = &gpDebuffInfos[sSpinForLuckStatus[section]];

	if (info->positive_type == STATUS_DEBUFF_NEGATIVE) {
		*color = TEXT_COLOR_SYSTEM_GRAY;
		PlaySoundEffect(0x6C);
	} else {
		*color = TEXT_COLOR_SYSTEM_GOLD;
	}

	return GetStringFromIndex(info->name);
}

static void SpinForLuck_Apply(ProcPtr proc, int section)
{
	int i, count = SpinForLuck_MakeTargets(gActiveUnit);

	for (i = 0; i < count; i++)
		SetUnitStatus(GetUnit(GetTarget(i)->uid), sSpinForLuckStatus[section]);
}

static const struct SkillWheelConf sSpinForLuckWheel = {
	.gfx = GFX_SpinForLuck_Wheel,
	.pal = sSpinForLuckPal,
	.prompt = MSG_SKILL_SpinForLuck_PROMPT,
	.describe = SpinForLuck_Describe,
	.apply = SpinForLuck_Apply,
};

bool Action_SpinForLuck(ProcPtr parent)
{
	SetPartyGoldAmount(GetPartyGoldAmount() - SPIN_FOR_LUCK_COST);
	StartSkillWheel(&sSpinForLuckWheel, parent);
	return false;
}

#endif
