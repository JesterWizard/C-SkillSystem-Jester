#include "common-chax.h"
#include "stat-screen.h"
#include "strmag.h"
#include "bwl.h"
#include "lvup.h"
#include "debuff.h"
#include "status-getter.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "skill-system.h"
#include "constants/skills.h"

LYN_REPLACE_CHECK(GetUnitFogViewRange);
int GetUnitFogViewRange(struct Unit * unit)
{
    int result = gPlaySt.chapterVisionRange;

	if (gpKernelDesignerConfig->custom_fog_sight == true) 
    {
        switch (unit->pClassData->number)
        {        
        case CLASS_EIRIKA_MASTER_LORD:
        case CLASS_EPHRAIM_MASTER_LORD:
        case CLASS_CAVALIER:
        case CLASS_CAVALIER_F:
        case CLASS_PALADIN:
        case CLASS_PALADIN_F:
        case CLASS_GREAT_KNIGHT:
        case CLASS_GREAT_KNIGHT_F:
        case CLASS_TROUBADOUR:
        case CLASS_MAGE_KNIGHT:
        case CLASS_MAGE_KNIGHT_F:
        case CLASS_VALKYRIE:
        case CLASS_RANGER:
        case CLASS_RANGER_F:
        case CLASS_TARVOS:
        case CLASS_MAELDUIN:
            result += 1;
            break;

        case CLASS_ARCHER:
        case CLASS_ARCHER_F:
        case CLASS_SNIPER:
        case CLASS_SNIPER_F:
        case CLASS_PEGASUS_KNIGHT:
        case CLASS_FALCON_KNIGHT:
        case CLASS_WYVERN_RIDER:
        case CLASS_WYVERN_RIDER_F:
        case CLASS_WYVERN_KNIGHT:
        case CLASS_WYVERN_KNIGHT_F:
        case CLASS_WYVERN_LORD:
        case CLASS_WYVERN_LORD_F:
        case CLASS_GARGOYLE:
        case CLASS_DEATHGOYLE:
        case CLASS_DRACO_ZOMBIE:
        case CLASS_MANAKETE_MYRRH:
        case CLASS_MANAKETE:
        case CLASS_MANAKETE_2:
        case CLASS_MOGALL:
        case CLASS_ARCH_MOGALL:
        case CLASS_PHANTOM:
            result += 2;
            break;

        case CLASS_THIEF:
        case CLASS_ROGUE:
        case CLASS_ASSASSIN:
        case CLASS_ASSASSIN_F:
            result += 5;

        default:
            break;
        }
    }
    else
    {
        if (UNIT_CATTRIBUTES(unit) & CA_THIEF)
            result += 5;
    }

/* 
** JESTER - For no discernable reason the skill tester call freezes the game when viewing
** a unit's stat screen, so I will have to move this call after GetUnitFogViewRange is called
*/
// #if defined(SID_HazeHunter) && (COMMON_SKILL_VALID(SID_HazeHunter))
//     if (SkillTester(unit, SID_HazeHunter))
//         result += 5;
// #endif

    return result + unit->torchDuration;
}