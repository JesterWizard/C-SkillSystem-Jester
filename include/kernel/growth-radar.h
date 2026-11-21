#pragma once

#include "common-chax.h"
#include "kernel-lib.h"

/*
 * Growth Radar: the stat screen's "Potential" page (PAGE_GROWTH_RADAR), a radar chart of the unit's growth rates and
 * of its stats against their caps (Kernel/Wizardry/GrowthRadar, Documentation/Features/GrowthRadar.md).
 */

/* The page shows when the designer config has it, the Options row is ON, and growths are shown at all */
static inline bool GrowthRadarEnabled(void)
{
	if (!gpKernelDesignerConfig->stat_page_growth_radar)
		return false;

	if (gPlaySt.config.growth_radar_off)
		return false;

	return !KernelLvupHidesGrowthDisplay();
}

void DrawPageGrowthRadar(void);
void GrowthRadar_RestoreVram(void);
