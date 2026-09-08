#include "common-chax.h"
#include "stat-screen.h"
#include "skill-system.h"
#include "combat-art.h"
#include "icon-rework.h"
#include "weapon-slots.h"
#include "constants/texts.h"
#include "bwl.h"

void DrawSkillPage_MokhaPlanB(void)
{
	int iy, ix, i, wcount;
	u8 wtypes[UNIT_WEAPON_SLOT_COUNT];
	struct Text *text;

	const int wexp_y[UNIT_WEAPON_SLOT_COUNT] = {
		0x1, 0x3, 0x5, 0x7, 0x9, 0xB, 0xD, 0xF,
	};

	struct SkillList *slist = GetUnitSkillList(&gBattleActor.unit /* gStatScreen.unit */);

	wcount = ListUnitMappedWeaponTypes(gStatScreen.unit, wtypes, UNIT_WEAPON_SLOT_COUNT);
	for (i = 0; i < wcount; i++)
		DisplayWeaponExp(i, 1, wexp_y[i], wtypes[i]);

	/* Skills */
	text = &gStatScreen.text[STATSCREEN_TEXT_ITEM0];
	ClearText(text);
	PutDrawText(
		text,
		gUiTmScratchA + TILEMAP_INDEX(9, 1),
		TEXT_COLOR_SYSTEM_GOLD, 0, 0,
		GetStringFromIndex(MSG_MSS_SKILLS));

	if (slist->amt == 0) {
		text = &gStatScreen.text[STATSCREEN_TEXT_ITEM1];
		ClearText(text);
		PutDrawText(
			text,
			gUiTmScratchA + TILEMAP_INDEX(9, 3),
			TEXT_COLOR_SYSTEM_GRAY, 0, 0,
			GetStringFromIndex(MSG_MSS_NOSKILLS));
	}

	for (iy = 0; iy < 4; iy++) {
		for (ix = 0; ix < 4; ix++) {
			int _index = ix + iy * 4;

			if (_index >= slist->amt)
				break;

			DrawIcon(gUiTmScratchA + TILEMAP_INDEX(9 + 2 * ix, 3 + 2 * iy),
					SKILL_ICON(slist->sid[_index]),
					TILEREF(0, STATSCREEN_BGPAL_ITEMICONS + GetSkillIconPal(slist->sid[_index])));
		}
	}

	/* Skill Points*/
	if (gpKernelDesignerConfig->skill_shop)
	{
		if (UNIT_FACTION(gStatScreen.unit) == FACTION_BLUE) {
			struct NewBwl* bwl = GetNewBwl(UNIT_CHAR_ID(gStatScreen.unit));

			text = &gStatScreen.text[STATSCREEN_TEXT_ITEM2];
			ClearText(text);
			PutDrawText(
				text,
				gUiTmScratchA + TILEMAP_INDEX(9, 9),
				TEXT_COLOR_SYSTEM_GOLD, 0, 0,
				GetStringFromIndex(MSG_STAT_SCREEN_SKILL_POINTS));

			PutNumber(gUiTmScratchA + TILEMAP_INDEX(13, 9), TEXT_COLOR_SYSTEM_BLUE, bwl->skillPoints);
		}
	}

	if (gpKernelDesignerConfig->tellius_skill_capacity_system == true)
	{
		text = &gStatScreen.text[STATSCREEN_TEXT_ITEM3];
		ClearText(text);
		PutDrawText(text, gUiTmScratchA + TILEMAP_INDEX(9, 11), TEXT_COLOR_SYSTEM_GOLD, 0, 0, "Skill Capacity");

		int maxCapacity = gpKernelDesignerConfig->tellius_skill_capacity_base;

		if (UNIT_CATTRIBUTES(gStatScreen.unit) & CA_PROMOTED)
			maxCapacity += gpKernelDesignerConfig->tellius_skill_capacity_promoted;

		PutNumber((gUiTmScratchA + TILEMAP_INDEX(0xE, 0xD)), TEXT_COLOR_SYSTEM_BLUE, GetUnitBattleAmt(gStatScreen.unit));
		PutSpecialChar((gUiTmScratchA + TILEMAP_INDEX(0xF, 0xD)), TEXT_COLOR_SYSTEM_WHITE, TEXT_SPECIAL_SLASH);
		PutNumber((gUiTmScratchA + TILEMAP_INDEX(0xE, 0xF)), TEXT_COLOR_SYSTEM_GREEN, maxCapacity);
	}
	else
	{
		/* Arts */
		if (UNIT_FACTION(gStatScreen.unit) == FACTION_BLUE) {
			struct CombatArtList *clist = AutoGetCombatArtList(gStatScreen.unit);

			if (clist->amt == 0) {
				text = &gStatScreen.text[STATSCREEN_TEXT_ITEM4];
				ClearText(text);
				PutDrawText(
					text,
					gUiTmScratchA + TILEMAP_INDEX(9, 13),
					TEXT_COLOR_SYSTEM_GRAY, 0, 0,
					GetStringFromIndex(MSG_MSS_NOARTS));
			}

			for (iy = 0; iy < 2; iy++) {
				for (ix = 0; ix < 4; ix++) {
					int _index = ix + iy * 4;

					if (_index >= clist->amt)
						break;

					DrawIcon(gUiTmScratchA + TILEMAP_INDEX(9 + 2 * ix, 13 + 2 * iy),
							COMBART_ICON(clist->cid[_index]),
							TILEREF(0, STATSCREEN_BGPAL_ITEMICONS));
				}
			}
		}
	}
}

void HbRedirect_ArtPageOnlyAlly(struct HelpBoxProc *proc)
{
	if (UNIT_FACTION(gStatScreen.unit) == FACTION_BLUE) {
		HbRedirect_ArtPageCommon(proc);
		return;
	}

	switch (proc->moveKey) {
	case DPAD_DOWN:
		TryRelocateHbDown(proc);
		break;

	case DPAD_UP:
		TryRelocateHbUp(proc);
		break;

	case DPAD_LEFT:
		TryRelocateHbLeft(proc);
		break;

	case DPAD_RIGHT:
	default:
		TryRelocateHbRight(proc);
		break;
	}
}
