#include "common-chax.h"
#include "kernel-lib.h"
#include "status-getter.h"
#include "rescue-penalty.h"

/*
 * Rescue penalty relative to Con: the rescuer loses pct% of Skl and Spd, where
 * pct = rescue_penalty_pct * carried Con / rescuer Con, clamped to [rescue_penalty_min_pct, rescue_penalty_max_pct].
 * With the defaults (50, 10, 75): equal Con 50% (vanilla); 15 Con carrying 10 33%; 8 Con carrying 7 43%.
 */
int RescuePenalty_Scale(int status, const struct Unit *rescuer)
{
	const struct KernelDesigerConfig *cfg = gpKernelDesignerConfig;
	struct Unit *carried;
	int con_rescuer, con_carried, pct;

	if (!cfg->rescue_penalty_by_con || !rescuer->rescue)
		return status / 2;

	carried = GetUnit(rescuer->rescue);
	if (!carried || !carried->pCharacterData)
		return status / 2;

	con_rescuer = ConGetter((struct Unit *)rescuer);
	con_carried = ConGetter(carried);

	if (con_rescuer > 0)
		pct = cfg->rescue_penalty_pct * con_carried / con_rescuer;
	else
		pct = cfg->rescue_penalty_max_pct;

	if (pct < cfg->rescue_penalty_min_pct)
		pct = cfg->rescue_penalty_min_pct;
	if (pct > cfg->rescue_penalty_max_pct)
		pct = cfg->rescue_penalty_max_pct;

	/* status * (100 - pct) / 100 rather than status - status * pct / 100: at pct 50 this is status / 2 (vanilla's rounding) */
	return status * (100 - pct) / 100;
}
