#include "common-chax.h"
#include "kernel-lib.h"

bool LightsOutGameEnabled(void)
{
	return gpKernelDesignerConfig->lights_out_game != false;
}
