#include "common-chax.h"
#include "rn.h"
#include "bwl.h"
#include "bmunit.h"
#include "skill-system.h"
#include "jester_headers/miscellaneous.h"
#include "debuff.h"
#include "jester_headers/Forging.h"

void SetGameOptions_BaseChapter5() 
{
#ifdef CONFIG_FREE_MOVEMENT
    EnableFreeMovementASMC();
#endif
}