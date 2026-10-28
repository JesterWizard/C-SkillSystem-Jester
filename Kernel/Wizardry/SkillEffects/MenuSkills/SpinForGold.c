#include "common-chax.h"
#include "skill-system.h"
#include "skill-wheel.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "constants/gfx.h"

#if defined(SID_SpinForGold) && (COMMON_SKILL_VALID(SID_SpinForGold))

enum {
	SPIN_FOR_GOLD_COST = 100,
	SPIN_FOR_GOLD_PAYOUT = 100,
};

/* Must match SECTIONS and PAL in Contents/Gfx/Scripts/SpinForGold/make_wheel.py. */
static const u8 sSpinForGoldValues[SKILL_WHEEL_SECTIONS] = { 5, 0, 1, 2, 0, 1, 3, 0 };

static const u16 sSpinForGoldPal[16] = {
	0x0000, 0x0865, 0x235F, 0x11F6, 0x18F9, 0x6586, 0x37BF, 0x6BFF,
	0x0E3A, 0x19B4, 0x110D, 0x7FFF, 0x4E52, 0x2508, 0x1093, 0x4D04,
};

u8 SpinForGold_Usability(const struct MenuItemDef *def, int number)
{
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if ((int)GetPartyGoldAmount() < SPIN_FOR_GOLD_COST)
		return MENU_DISABLED;

	return MENU_ENABLED;
}

u8 SpinForGold_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, MSG_SKILL_SpinForGold_FRtext);
		return MENU_ACT_SND6B;
	}

	gActionData.unk08 = SID_SpinForGold;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

/* Writes n (>= 0) in decimal into buf and returns the end of the string. */
static char *SpinForGold_NumberToString(int n, char *buf)
{
	char tmp[12];
	int i = 0;

	do {
		tmp[i++] = '0' + DivRem(n, 10);
		n = Div(n, 10);
	} while (n > 0);

	while (i > 0)
		*buf++ = tmp[--i];

	*buf = '\0';
	return buf;
}

static const char *SpinForGold_Describe(int section, char *buf, int *color)
{
	int value = sSpinForGoldValues[section];
	char *end;

	if (value == 0) {
		*color = TEXT_COLOR_SYSTEM_GRAY;
		PlaySoundEffect(0x6C);
		return GetStringFromIndex(MSG_SKILL_SpinForGold_LOSE);
	}

	/* "+300G!" */
	buf[0] = '+';
	end = SpinForGold_NumberToString(value * SPIN_FOR_GOLD_PAYOUT, buf + 1);
	end[0] = 'G';
	end[1] = '!';
	end[2] = '\0';

	*color = TEXT_COLOR_SYSTEM_GOLD;
	return buf;
}

static void SpinForGold_Apply(ProcPtr proc, int section)
{
	int value = sSpinForGoldValues[section];

	if (value > 0)
		NewPopup_GoldGot(proc, gActiveUnit, value * SPIN_FOR_GOLD_PAYOUT);
}

static const struct SkillWheelConf sSpinForGoldWheel = {
	.gfx = GFX_SpinForGold_Wheel,
	.pal = sSpinForGoldPal,
	.prompt = MSG_SKILL_SpinForGold_PROMPT,
	.describe = SpinForGold_Describe,
	.apply = SpinForGold_Apply,
};

bool Action_SpinForGold(ProcPtr parent)
{
	SetPartyGoldAmount(GetPartyGoldAmount() - SPIN_FOR_GOLD_COST);
	StartSkillWheel(&sSpinForGoldWheel, parent);
	return false;
}

#endif
