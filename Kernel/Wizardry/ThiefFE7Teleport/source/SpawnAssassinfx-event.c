#include "gbafe.h"

void StartSpawnClassFx(ProcPtr proc, int x, int y, int classId);

void CallEvent_SpawnAssassinfx(struct EventEngineProc * proc)
{
    int x, y, z;
    x = gEventSlots[3] * 16 - gBmSt.camera.x;
    y = gEventSlots[4] * 16 - gBmSt.camera.y;
    z = gEventSlots[5];

    StartSpawnClassFx(proc, x, y, z);
}
