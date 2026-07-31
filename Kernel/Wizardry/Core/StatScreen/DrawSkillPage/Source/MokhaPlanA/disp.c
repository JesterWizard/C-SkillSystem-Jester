#include "common-chax.h"
#include "stat-screen.h"
#include "skill-system.h"
#include "icon-rework.h"
#include "weapon-slots.h"
#include "constants/texts.h"
#include "bwl.h"

#define STAT_SKILL_NUM_MAX 8

void DrawSkillPage_MokhaPlanA(void)
{
	int i;
	int wcount;
	u8 wtypes[UNIT_WEAPON_SLOT_COUNT];
	struct Text *text;

	const u8 text_id[8] = {
		STATSCREEN_TEXT_ITEM0,
		STATSCREEN_TEXT_ITEM1,
		STATSCREEN_TEXT_ITEM2,
		STATSCREEN_TEXT_ITEM3,
		STATSCREEN_TEXT_ITEM4,
		STATSCREEN_TEXT_SUPPORT0,
		STATSCREEN_TEXT_SUPPORT1,
		STATSCREEN_TEXT_SUPPORT2,
	};

	const int wexp_y[UNIT_WEAPON_SLOT_COUNT] = {
		0x1, 0x3, 0x5, 0x7, 0x9, 0xB, 0xD, 0xF,
	};

	struct SkillList *list = GetUnitSkillList(&gBattleActor.unit /* gStatScreen.unit */);

	wcount = ListUnitMappedWeaponTypes(gStatScreen.unit, wtypes, UNIT_WEAPON_SLOT_COUNT);
	for (i = 0; i < wcount; i++)
		DisplayWeaponExp(i, 1, wexp_y[i], wtypes[i]);

	for (i = 0; i < STAT_SKILL_NUM_MAX; i++) {
		struct Text *text = &gStatScreen.text[text_id[i]];

		if (i >= list->amt)
			break;

		DrawIcon(gUiTmScratchA + TILEMAP_INDEX(8, 0x1 + 2 * i),
				 SKILL_ICON(list->sid[i]),
				 TILEREF(0, STATSCREEN_BGPAL_ITEMICONS + GetSkillIconPal(list->sid[i])));

		ClearText(text);

		PutDrawText(
			text,
			gUiTmScratchA + TILEMAP_INDEX(10, 0x1 + 2 * i),
			TEXT_COLOR_SYSTEM_WHITE, 0, 0,
			GetSkillNameStr(list->sid[i]));
	}

	// /* Skill Points*/
	// if (gpKernelDesignerConfig->skill_shop)
	// {
	// 	if (UNIT_FACTION(gStatScreen.unit) == FACTION_BLUE) {
	// 		struct NewBwl* bwl = GetNewBwl(UNIT_CHAR_ID(gStatScreen.unit));

	// 		text = &gStatScreen.text[STATSCREEN_TEXT_SUPPORT3];
	// 		ClearText(text);
	// 		PutDrawText(
	// 			text,
	// 			gUiTmScratchA + TILEMAP_INDEX(8, 11),
	// 			TEXT_COLOR_SYSTEM_GOLD, 0, 0,
	// 			GetStringFromIndex(MSG_STAT_SCREEN_SKILL_POINTS));

	// 		PutNumber(gUiTmScratchA + TILEMAP_INDEX(14, 11), TEXT_COLOR_SYSTEM_BLUE, bwl->skillPoints);
	// 	}
	// }

	if (gpKernelDesignerConfig->tellius_skill_capacity_system == true)
	{
		text = &gStatScreen.text[STATSCREEN_TEXT_STATUS];
		ClearText(text);
		PutDrawText(text, gUiTmScratchA + TILEMAP_INDEX(8, 11), TEXT_COLOR_SYSTEM_GOLD, 0, 0, "Skill Capacity");

		int maxCapacity = gpKernelDesignerConfig->tellius_skill_capacity_base;

		if (UNIT_CATTRIBUTES(gStatScreen.unit) & CA_PROMOTED)
			maxCapacity += gpKernelDesignerConfig->tellius_skill_capacity_promoted;

		PutNumber((gUiTmScratchA + TILEMAP_INDEX(0xE, 0xD)), TEXT_COLOR_SYSTEM_BLUE, GetUnitBattleAmt(gStatScreen.unit));
		PutSpecialChar((gUiTmScratchA + TILEMAP_INDEX(0xF, 0xD)), TEXT_COLOR_SYSTEM_WHITE, TEXT_SPECIAL_SLASH);
		PutNumber((gUiTmScratchA + TILEMAP_INDEX(0xE, 0xF)), TEXT_COLOR_SYSTEM_GREEN, maxCapacity);
	}
}
