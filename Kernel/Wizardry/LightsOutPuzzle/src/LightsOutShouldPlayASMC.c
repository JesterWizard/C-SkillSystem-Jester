#include "common-chax.h"
#include "kernel-lib.h"

extern bool LightsOutGameEnabled(void);

void LightsOutShouldPlayASMC(void)
{
	gEventSlots[0xC] = LightsOutGameEnabled();
}
