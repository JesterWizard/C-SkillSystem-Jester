#include "common-chax.h"
#include "skill-system.h"
#include "playst-expa.h"
#include "constants/skills.h"

/* Close the Guarantee window that CheckBattleGuarantee opened in this action's battle. */
bool PostAction_Guarantee(ProcPtr parent)
{
#if (defined(SID_Guarantee) && (COMMON_SKILL_VALID(SID_Guarantee)))
	PlayStExpa_ClearBit(PLAYSTEXPA_BIT_Guarantee_InForce);
#endif
	return false;
}
