#include "common-chax.h"
#include "playst-expa.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "map-anims.h"
#include "constants/skills.h"
#include "constants/texts.h"

#if defined(SID_GraceOfFire) && (COMMON_SKILL_VALID(SID_GraceOfFire))

u8 GraceOfFire_Usability(const struct MenuItemDef *def, int number)
{
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_GraceOfFire_Used))
		return MENU_DISABLED;

	return MENU_ENABLED;
}

u8 GraceOfFire_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, MSG_SKILL_CannotUseAgain_FRtext);
		return MENU_ACT_SND6B;
	}

	gActionData.unk08 = SID_GraceOfFire;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static void callback_anim(ProcPtr proc)
{
	PlaySoundEffect(0x269);
	Proc_StartBlocking(ProcScr_DanceringAnim, proc);

		BG_SetPosition(
		BG_0,
		-SCREEN_TILE_IX(gActiveUnit->xPos - 1),
		-SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

static void callback_exec(ProcPtr proc)
{
	struct Unit * unit;
	int enemyFaction = gPlaySt.faction == FACTION_BLUE ? FACTION_RED : FACTION_RED ? FACTION_BLUE : FACTION_RED;

	PlayStExpa_SetBit(PLAYSTEXPA_BIT_GraceOfFire_Used);

	 for (int uid = enemyFaction + 1; uid <= (enemyFaction + GetFactionUnitAmount(gPlaySt.faction)); uid++)
	 {
		unit = GetUnit(uid);

		if (!UNIT_IS_VALID(unit))
            break;

		if (unit->curHP <= SKILL_EFF0(SID_GraceOfFire))
			unit->curHP = 1;
		else
			unit->curHP -= SKILL_EFF0(SID_GraceOfFire);
	 }
}

bool Action_GraceOfFire(ProcPtr parent)
{
	NewMuSkillAnimOnActiveUnitWithDeamon(parent, gActionData.unk08, callback_anim, callback_exec);
	return true;
}

#endif /* SID_GraceOfFire */