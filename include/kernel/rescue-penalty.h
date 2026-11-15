#pragma once

#include "common-chax.h"

/*
 * The rescuer's Skl/Spd while carrying a unit (Kernel/Wizardry/RescuePenalty): halved as in vanilla, or with
 * gpKernelDesignerConfig->rescue_penalty_by_con, reduced by a percentage relative to the two units' Con.
 */
int RescuePenalty_Scale(int status, const struct Unit *rescuer);
