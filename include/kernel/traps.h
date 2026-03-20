#pragma once

#include "common-chax.h"

void GenerateFireTileTrapTargets(int x, int y, int damage);
void GenerateArrowTrapTargets(int x, int y, int damage);
void GenerateGasTrapTargets(int x, int y, int damage, int facing);
struct Trap* AddTeleportTile(int x, int y, int destX, int destY);
void AddTeleportTilePair(int x1, int y1, int x2, int y2);
bool PostAction_TeleportTile(ProcPtr parent);