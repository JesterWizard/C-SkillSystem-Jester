#pragma once

#include "common-chax.h"

enum ai_conf_expa {
	AI_UNIT_CONFIG_EXTFLAG_TELEPORTATION = 1 << 8,
};

/* Per-phase record of units that actually performed an AI action */
extern u32 sAiPhasePerformedBits[7];

void AiPhaseClearPerformedFlags(void);
void AiPhaseMarkUnitPerformed(u8 uid);
bool AiPhaseDidUnitPerform(u8 uid);
