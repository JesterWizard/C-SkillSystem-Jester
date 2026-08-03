#pragma once

#include "common-chax.h"

void BuildEnemyFogVision(void);
bool EnemyFogVisionCanSeeUnit(struct Unit *unit);
bool EnemyFogVisionCanTargetUnit(struct Unit *unit);
