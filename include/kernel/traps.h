#pragma once

#include "common-chax.h"

void GenerateFireTileTrapTargets(int x, int y, int damage);
void GenerateArrowTrapTargets(int x, int y, int damage);
void GenerateGasTrapTargets(int x, int y, int damage, int facing);
int GetTrapMapSpritePalette(const struct Trap *trap);
void SetTrapMapSpritePalette(struct Trap *trap, int palette);
struct Trap* AddTeleportTile(int x, int y, int destX, int destY, int palette);
void AddTeleportTilePair(int x1, int y1, int x2, int y2);
struct Trap* AddGrassTile(int x, int y, int turnsLeft);
struct Trap* AddHealTile(int x, int y, int healAmount, int turnsLeft, int palette);
struct Trap* AddToggleTorch(int x, int y, int duration, int startsLit, int palette);
struct Trap* AddLightRune(int x, int y, int palette);
int GetEffectiveTerrainAt(int x, int y);
bool PostAction_TeleportTile(ProcPtr parent);