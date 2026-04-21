#include "common-chax.h"
#include "bmmenu.h"
#include "kernel-lib.h"
#include "bwl.h"
#include "strmag.h"
#include "debuff.h"
#include "jester_headers/custom-structs.h"

static int Prestige_GetMaxPromotionCount(struct Unit *unit)
{
	int maxPromotions = 0;
	int charId;

	if (!UNIT_IS_VALID(unit))
		return 0;

	charId = UNIT_CHAR_ID(unit);

	for (int i = 0; unit_promotions[i].key != 0; i++) {
		if (unit_promotions[i].key != charId)
			continue;

		for (unsigned promotionIndex = 0; promotionIndex < ARRAY_COUNT(unit_promotions[i].promotions); promotionIndex++) {
			if (unit_promotions[i].promotions[promotionIndex].classId != 0)
				maxPromotions++;
		}

		break;
	}

	return maxPromotions;
}

u8 PrestigeCommandUsability(const struct MenuItemDef *def, int number)
{
	struct NewBwl *bwl;
	int maxPromotions;

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

	maxPromotions = Prestige_GetMaxPromotionCount(gActiveUnit);
	if (maxPromotions <= 0)
		return MENU_NOTSHOWN;

	if (bwl->prestigeAmt >= maxPromotions)
		return MENU_NOTSHOWN;

	if (bwl->prestigeAmt >= 3)
		return MENU_NOTSHOWN;

	return MENU_ENABLED;
}

u8 PrestigeCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem)
{
	struct NewBwl *bwl = GetNewBwl(UNIT_CHAR_ID(gActiveUnit));
	const struct CharacterData *character = gActiveUnit->pCharacterData;
	const struct ClassData *baseClass = GetClassData(character->defaultClass);
	u32 prestigeCount;

	if (!gpKernelDesignerConfig->prestige)
		return MENU_ACT_SND6B;

	if (!bwl || !character || !baseClass)
		return MENU_ACT_SND6B;

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

	return EffectWait(menu, menuItem);
}