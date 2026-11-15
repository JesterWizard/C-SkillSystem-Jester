#pragma once

#include "common-chax.h"

/*
 * UI styles and their EXP bars (Kernel/Wizardry/ExpBarStyles), picked on the Options screen's "UI Style" row:
 * gPlaySt.config.exp_bar_style holds the row's value.
 */
int UiStyle_Count(void);
const char *UiStyle_Name(int id);
const char *UiStyle_Desc(int id);
int UiStyle_GetUiId(void);	/* the VeslyUI style ID (GetUI_id) */

/* VeslyUI: the map battle box (animations off) in the UI style's colours, over palette palId (Kernel/Wizardry/VeslyUI) */
void ApplyMapBattleInfoStylePalette(struct Unit *unit, int palId);
