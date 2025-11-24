#include "common-chax.h"
#include "rn.h"
#include "bwl.h"
#include "bmunit.h"
#include "skill-system.h"
#include "jester_headers/miscellaneous.h"
#include "debuff.h"
#include "jester_headers/Forging.h"
// struct LearnedSkillList {
//     u32 data[8]; /* 8 * 32 = 0x100 */
// };

// extern struct LearnedSkillList sLearnedSkillPLists[NEW_BWL_ARRAY_NUM];

void SetGameOptions_Chapter0() 
{
    SetGameOption(GAME_OPTION_ANIMATION, 2);  // Set battle animations off
    SetGameOption(GAME_OPTION_TEXT_SPEED, 1); // Set game speed to max
    SetGameOption(GAME_OPTION_GAME_SPEED, 3); // Set text speed to max

    /* Fog activates when this is between 1 - 4 */
    // gPlaySt.chapterVisionRange = 1;

    SetPartyGoldAmount(100000);

    // EnableFreeMovementASMC();
    
    // gEventSlots[EVT_SLOT_1] = 2;
    // gEventSlots[EVT_SLOT_2] = 0; 
    // WLC_baseChapterAvailabilityASMC();

    struct Unit * unit_eirika = GetUnitFromCharId(CHARACTER_EIRIKA);
    struct Unit * unit_seth = GetUnitFromCharId(CHARACTER_SETH);
    unit_seth->ranks[ITYPE_STAFF] = WPN_EXP_A;
    unit_seth->items[1] = MakeNewItem(CONFIG_ITEM_INDEX_MINE_STAFF);
    // unit_eirika->pow = 20;
    // unit_eirika->spd = 20;

    unit_eirika->ranks[ITYPE_STAFF] = WPN_EXP_A;
    unit_eirika->items[0] = MakeNewItem(ITEM_SWORD_IRON);
    unit_eirika->items[1] = MakeNewItem(CONFIG_ITEM_INDEX_ENFEEBLE_STAFF);
    // unit_eirika->items[2] = MakeNewItem(CONFIG_ITEM_INDEX_FORGE_STAFF);
    // unit_eirika->items[3] = MakeNewItem(CONFIG_ITEM_INDEX_REWARP_STAFF);

    // struct NewBwl * bwl_eirika = GetNewBwl(CHARACTER_EIRIKA);
    // struct NewBwl * bwl_seth = GetNewBwl(CHARACTER_SETH);
    // bwl_eirika->currentMP += 10;
    // bwl_eirika->maxMP = 30;
    // // bwl_seth->maxMP = 20;
    // unit_eirika->pow = 20;


    // unit_eirika->ranks[ITYPE_STAFF] = WPN_EXP_A;
    // unit_eirika->items[1] = ITEM_KNIGHTCREST;
    // unit_eirika->items[2] = ITEM_MASTERSEAL;

    // unit_seth->pClassData = GetClassData(CLASS_CAVALIER);
    // unit_seth->level = 10;

    // unit_eirika->pClassData = GetClassData(CLASS_ARMOR_KNIGHT);
    // unit_eirika->level = 10;
    
    // // Seed the random number generator
    // InitRandC();

    // // Randomly generate items for the unit
    // for (int i = 0; i < 5; i++)
    //     unit->items[i] = MakeNewItem(RandNextC_N(200));

    // for (int i = 0; i < SKILL_ROM_DATA_AMT; i++)
    //     SET_SKILL(unit, i, RandNextC_N(711));

    // SetFlag(0xAB); // Gamma UI flag

    SetFlag(0xB4); // Guide Flag

    // NoCashGBAPrintf("Chapter 1 - Total BEXP BEFORE is: %d", sBEXP);

    // sBEXP.value += 100;

    // NoCashGBAPrintf("Chapter 1 - Total BEXP AFTER is: %d", sBEXP);

    // gBmMapTerrain[5][3] = TERRAIN_TILE_00;
 

    // SetUnitStatusIndex(unit_seth, NEW_UNIT_STATUS_DOOM);
    // SetUnitStatusDuration(unit_seth, 2);

}