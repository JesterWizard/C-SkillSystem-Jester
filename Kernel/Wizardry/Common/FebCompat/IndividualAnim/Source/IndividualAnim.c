#include "common-chax.h"
#include "kernel-lib.h"

// Individual_animation_ea_2
extern const u8 pr_IndividualAnimation7743_Hook[];

const struct IndividualAnimConf *GetPrIndividualAnimConf(void)
{
	/**
	 * Auto get pointer to gpCustomAnimeTable1
	 */
	if (gpKernelDesignerConfig->custom_character_animations == true)
	{
		u32 pr_IndividualAnimation7743  = (u32)(pr_IndividualAnimation7743_Hook + 4);
		u32 loc_IndividualAnimation7743 = *((u32 *)pr_IndividualAnimation7743);
		u32 pr_CustomAnimeTable  = loc_IndividualAnimation7743 - 1 + 0x54;
		u32 loc_CustomAnimeTable = *((u32 *)pr_CustomAnimeTable);

		return ((const struct IndividualAnimConf *)loc_CustomAnimeTable);
	}
	else
	{
		return NULL;
	}
}
