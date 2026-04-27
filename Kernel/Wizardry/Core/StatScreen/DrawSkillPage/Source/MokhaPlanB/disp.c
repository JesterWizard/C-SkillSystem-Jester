#include "common-chax.h"
#include "stat-screen.h"
#include "skill-system.h"
#include "combat-art.h"
#include "icon-rework.h"
#include "constants/texts.h"
#include "bwl.h"

void DrawSkillPage_MokhaPlanB(void)
{
	int iy, ix;
	struct Text *text;
	bool hasSkill = false;

	DisplayWeaponExp(0, 1, 0x1, ITYPE_SWORD);
	DisplayWeaponExp(1, 1, 0x3, ITYPE_LANCE);
	DisplayWeaponExp(2, 1, 0x5, ITYPE_AXE);
	DisplayWeaponExp(3, 1, 0x7, ITYPE_BOW);

	DisplayWeaponExp(4, 1, 0x9, ITYPE_ANIMA);
	DisplayWeaponExp(5, 1, 0xB, ITYPE_LIGHT);
	DisplayWeaponExp(6, 1, 0xD, ITYPE_DARK);
	DisplayWeaponExp(7, 1, 0xF, ITYPE_STAFF);

	/* Skills */
	text = &gStatScreen.text[STATSCREEN_TEXT_ITEM0];
	ClearText(text);
	PutDrawText(
		text,
		gUiTmScratchA + TILEMAP_INDEX(9, 1),
		TEXT_COLOR_SYSTEM_GOLD, 0, 0,
		GetStringFromIndex(MSG_MSS_SKILLS));

	for (iy = 0; iy < 4; iy++) {
		for (ix = 0; ix < 4; ix++) {
			int _index = ix + iy * 4;
			int sid = GET_SKILL(gStatScreen.unit, _index);

			if (_index >= UNIT_RAM_SKILLS_LEN)
				break;

			if (!sid)
				continue;

			hasSkill = true;

			DrawIcon(gUiTmScratchA + TILEMAP_INDEX(9 + 2 * ix, 3 + 2 * iy),
					SKILL_ICON(sid),
					TILEREF(0, STATSCREEN_BGPAL_ITEMICONS + GetSkillIconPal(sid)));
		}
	}

	if (!hasSkill) {
		text = &gStatScreen.text[STATSCREEN_TEXT_ITEM1];
		ClearText(text);
		PutDrawText(
			text,
			gUiTmScratchA + TILEMAP_INDEX(9, 3),
			TEXT_COLOR_SYSTEM_GRAY, 0, 0,
			GetStringFromIndex(MSG_MSS_NOSKILLS));
	}

	/* Skill Points*/
	if (gpKernelDesignerConfig->skill_points_engage)
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

			PutNumberOrBlank(gUiTmScratchA + TILEMAP_INDEX(13, 9), TEXT_COLOR_SYSTEM_BLUE, bwl->skillPoints);
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
