#include "common-chax.h"
#include "kernel-lib.h"
#include "feb-compat.h"

/* Contents/External/Individual_animation_ea_2/data.event */
extern const struct IndividualAnimConf CustomAnimeTable[];

const struct IndividualAnimConf *GetPrIndividualAnimConf(void)
{
	if (gpKernelDesignerConfig->custom_character_animations != true)
		return NULL;

	return CustomAnimeTable;
}

/* The unit's battle animations: its row in CustomAnimeTable (character and class), else its class's */
const struct BattleAnimDef *GetUnitBattleAnimDef(struct Unit *unit)
{
	const struct IndividualAnimConf *aconf = GetPrIndividualAnimConf();
	int i;

	if (aconf != NULL) {
		for (i = 0; aconf[i].pid != 0 || aconf[i].jid != 0; i++) {
			if (aconf[i].pid == UNIT_CHAR_ID(unit) && aconf[i].jid == UNIT_CLASS_ID(unit))
				return aconf[i].anim_conf;
		}
	}
	return unit->pClassData->pBattleAnimDef;
}
