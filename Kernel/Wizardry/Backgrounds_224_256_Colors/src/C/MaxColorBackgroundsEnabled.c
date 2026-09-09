#include "common-chax.h"
#include "kernel-lib.h"
#include "main.h"

bool MaxColorBackgroundsEnabled(void)
{
	return gpKernelDesignerConfig->max_color_backgrounds != false;
}
