#include "common-chax.h"
#include "bmmenu.h"
#include "bmudisp.h"
#include "kernel-lib.h"
#include "bwl.h"
#include "strmag.h"
#include "debuff.h"
#include "skill-system.h"

u8 PrestigeCommandUsability(const struct MenuItemDef *def, int number)
{
	struct NewBwl *bwl;

	if (!gpKernelDesignerConfig->prestige)
		return MENU_NOTSHOWN;

	if (!UNIT_IS_VALID(gActiveUnit))
		return MENU_NOTSHOWN;

	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if (UNIT_CATTRIBUTES(gActiveUnit) & CA_PROMOTED)
		return MENU_NOTSHOWN;

	if (gActiveUnit->level < 10)
		return MENU_NOTSHOWN;

	bwl = GetNewBwl(UNIT_CHAR_ID(gActiveUnit));
	if (!bwl)
		return MENU_NOTSHOWN;

	if (bwl->prestigeAmt >= 3)
		return MENU_NOTSHOWN;

	return MENU_ENABLED;
}

static void Prestige_Apply(ProcPtr proc)
{
	struct NewBwl *bwl = GetNewBwl(UNIT_CHAR_ID(gActiveUnit));
	const struct CharacterData *character;
	const struct ClassData *baseClass;
	u32 prestigeCount;

	(void)proc;

	if (!gpKernelDesignerConfig->prestige || !UNIT_IS_VALID(gActiveUnit) || !bwl)
		return;

	character = gActiveUnit->pCharacterData;
	if (!character)
		return;

	baseClass = GetClassData(character->defaultClass);
	if (!baseClass)
		return;

	prestigeCount = (bwl->prestigeAmt < 3) ? (u32)bwl->prestigeAmt + 1 : 3;

	gActiveUnit->pClassData = baseClass;
	WriteUnitLevelSafe(gActiveUnit, 1);
	bwl->prestigeAmt = prestigeCount;
	gActiveUnit->exp = 0;
	gActiveUnit->maxHP = character->baseHP + baseClass->baseHP;
	gActiveUnit->pow = character->basePow + baseClass->basePow;
	gActiveUnit->skl = character->baseSkl + baseClass->baseSkl;
	gActiveUnit->spd = character->baseSpd + baseClass->baseSpd;
	gActiveUnit->def = character->baseDef + baseClass->baseDef;
	gActiveUnit->res = character->baseRes + baseClass->baseRes;
	gActiveUnit->lck = character->baseLck;
	UNIT_MAG(gActiveUnit) = GetUnitBaseMagic(gActiveUnit);
	SetUnitStatusIndex(gActiveUnit, UNIT_STATUS_NONE);
	SetUnitStatusDuration(gActiveUnit, 0);
	UnitCheckStatCaps(gActiveUnit);
	SetUnitHp(gActiveUnit, GetUnitMaxHp(gActiveUnit));
	RefreshUnitSprites();
}

u8 PrestigeCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem)
{
	ProcPtr playerPhase;

	(void)menu;
	(void)menuItem;

	if (!gpKernelDesignerConfig->prestige)
		return MENU_ACT_SND6B;

	if (!UNIT_IS_VALID(gActiveUnit) || !GetNewBwl(UNIT_CHAR_ID(gActiveUnit)))
		return MENU_ACT_SND6B;

	if (!gActiveUnit->pCharacterData || !GetClassData(gActiveUnit->pCharacterData->defaultClass))
		return MENU_ACT_SND6B;

	gActionData.unitActionType = UNIT_ACTION_WAIT;
	playerPhase = Proc_Find(gProcScr_PlayerPhase);
	if (playerPhase)
		NewMuSkillAnimOnActiveUnitWithDeamon(playerPhase, 0, NULL, Prestige_Apply);
	else
		NewMuSkillAnimOnActiveUnit(0, NULL, Prestige_Apply);

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}