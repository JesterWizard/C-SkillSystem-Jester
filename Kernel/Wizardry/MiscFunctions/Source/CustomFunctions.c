#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "battle-system.h"
#include "unit-expa.h"
#include "eventinfo.h"
#include "bmmenu.h"
#include "eventscript.h"
#include "EAstdlib.h"
#include "worldmap.h"
#include "constants/texts.h"
#include "action-expa.h"

void IsTraineeLevelCappedOrPromoted(void)
{
    u16 charId = gEventSlots[EVT_SLOT_3];
    u16 level = gEventSlots[EVT_SLOT_4];
    u16 class = gEventSlots[EVT_SLOT_5];

    struct Unit * unit;
    unit = GetUnitFromCharId(charId);

    if (unit->level >= level || unit->pClassData->number != class)
        gEventSlots[EVT_SLOT_C] = true;
    else
        gEventSlots[EVT_SLOT_C] = false;
}