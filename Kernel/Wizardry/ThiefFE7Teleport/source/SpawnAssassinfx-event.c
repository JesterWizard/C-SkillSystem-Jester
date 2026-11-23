#include "gbafe.h"

void StartSpawnClassFxFaction(ProcPtr proc, int x, int y, int classId, int faction);

/* Slots 3 and 4: the tile, 5: the class. In the colours of the unit on that tile (blue if none) */
void CallEvent_SpawnAssassinfx(struct EventEngineProc * proc)
{
    int x, y, z, uid;
    x = gEventSlots[3] * 16 - gBmSt.camera.x;
    y = gEventSlots[4] * 16 - gBmSt.camera.y;
    z = gEventSlots[5];
    uid = gBmMapUnit[gEventSlots[4]][gEventSlots[3]];

    StartSpawnClassFxFaction(proc, x, y, z, uid ? UNIT_FACTION(GetUnit(uid)) : FACTION_BLUE);
}
