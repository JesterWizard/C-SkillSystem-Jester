#include "common-chax.h"
#include "kernel-lib.h"
#include "mokha-aoe.h"
#include "constants/texts.h"
#include "constants/characters.h"

const struct MokhaAoeAttackInfo gMokhaAoeAttackTable[MOKHA_AOE_ATK_COUNT] = {
	[MOKHA_AOE_ATK_DEFAULT] = {
		.nameMsg = MSG_MokhaAoe_Default_NAME,
		.descMsg = MSG_MokhaAoe_Default_DESC,
		.range = 2,
		.damage = 10,
		.mapRoutine = MOKHA_AOE_ATK_DEFAULT,
	},
	[MOKHA_AOE_ATK_BLAZE] = {
		.nameMsg = MSG_MokhaAoe_Blaze_NAME,
		.descMsg = MSG_MokhaAoe_Blaze_DESC,
		.range = 2,
		.damage = 10,
		.mapRoutine = MOKHA_AOE_ATK_BLAZE,
	},
	[MOKHA_AOE_ATK_ABSORPTION] = {
		.nameMsg = MSG_MokhaAoe_Absorption_NAME,
		.descMsg = MSG_MokhaAoe_Absorption_DESC,
		.range = 2,
		.damage = 10,
		.mapRoutine = MOKHA_AOE_ATK_ABSORPTION,
	},
	[MOKHA_AOE_ATK_FUSILLADE] = {
		.nameMsg = MSG_MokhaAoe_Fusillade_NAME,
		.descMsg = MSG_MokhaAoe_Fusillade_DESC,
		.range = 2,
		.damage = 10,
		.mapRoutine = MOKHA_AOE_ATK_FUSILLADE,
	},
	[MOKHA_AOE_ATK_FIRE_ARROWS] = {
		.nameMsg = MSG_MokhaAoe_FireArrows_NAME,
		.descMsg = MSG_MokhaAoe_FireArrows_DESC,
		.range = 2,
		.damage = 10,
		.mapRoutine = MOKHA_AOE_ATK_FIRE_ARROWS,
	},
	[MOKHA_AOE_ATK_GROUP_LANCE] = {
		.nameMsg = MSG_MokhaAoe_GroupLance_NAME,
		.descMsg = MSG_MokhaAoe_GroupLance_DESC,
		.range = 2,
		.damage = 10,
		.mapRoutine = MOKHA_AOE_ATK_GROUP_LANCE,
	},
};

const u8 gMokhaAoeEligibleByPid[0x100] = {
	[CHARACTER_EIRIKA] = true,
	[CHARACTER_SETH] = true,
};

bool IsMokhaAoeEnabled(void)
{
	return gpKernelDesignerConfig->mokha_aoe_enabled != false;
}

bool IsUnitMokhaAoeEligible(struct Unit *unit)
{
	if (!UNIT_IS_VALID(unit))
		return false;

	if (!IsMokhaAoeEnabled())
		return false;

	return gMokhaAoeEligibleByPid[UNIT_CHAR_ID(unit)] != 0;
}

const struct MokhaAoeAttackInfo *GetMokhaAoeAttackInfo(int index)
{
	if (index < 0 || index >= MOKHA_AOE_ATK_COUNT)
		return NULL;

	return &gMokhaAoeAttackTable[index];
}

int GetMokhaAoeDamage(struct Unit *target, int attackIndex)
{
	const struct MokhaAoeAttackInfo *info = GetMokhaAoeAttackInfo(attackIndex);
	int maxDmg;
	int curHp;

	if (!info || !UNIT_IS_VALID(target))
		return 0;

	maxDmg = info->damage;
	curHp = GetUnitCurrentHp(target);

	if (curHp <= 0)
		return 0;

	if (curHp > maxDmg)
		return maxDmg;

	/* Allow lethal damage. */
	return curHp;
}
