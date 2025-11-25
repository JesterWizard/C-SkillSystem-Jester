#include "common-chax.h"
#include "strmag.h"
#include "bwl.h"
#include "lvup.h"
#include "debuff.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/custom-arrays.h"

typedef int (*LoadUnitFunc_t)(struct Unit* unit, const struct CharacterData* character);
// extern const LoadUnitFunc_t gLoadUnitHooks[];
extern LoadUnitFunc_t const* const gpLoadUnitHooks;

void UnitLoadStatsFromChracterVanilla(struct Unit* unit, const struct CharacterData* character)
{
    int i;

    unit->maxHP = character->baseHP + unit->pClassData->baseHP;
    unit->pow = character->basePow + unit->pClassData->basePow;
    unit->skl = character->baseSkl + unit->pClassData->baseSkl;
    unit->spd = character->baseSpd + unit->pClassData->baseSpd;
    unit->def = character->baseDef + unit->pClassData->baseDef;
    unit->res = character->baseRes + unit->pClassData->baseRes;
    unit->lck = character->baseLck;

    unit->conBonus = 0;

#ifdef CONFIG_MP_SYSTEM
    struct NewBwl* bwl = GetNewBwl(UNIT_CHAR_ID(unit));

    if (bwl != NULL)
    {
        bwl->currentMP = gMpSystemPInfoConfigList[UNIT_CHAR_ID(unit)].initialMP; 
        bwl->maxMP = gMpSystemPInfoConfigList[UNIT_CHAR_ID(unit)].maxMP;

#if defined(SID_ManaRush) && (COMMON_SKILL_VALID(SID_ManaRush))
    if (SkillTester(unit, SID_ManaRush))
        bwl->maxMP = bwl->maxMP * 2;
#endif

#if defined(SID_Enlightenment) && (COMMON_SKILL_VALID(SID_Enlightenment))
	if (SkillTester(unit, SID_Enlightenment))
		bwl->currentMP = bwl->maxMP;
#endif
    }

#endif

#if defined(SID_Replicate) && (COMMON_SKILL_VALID(SID_Replicate))
    if (gActionData.unk08 == SID_Replicate)
    {
        SetUnitStatusIndex(unit, 30);

        for (i = 0; i < 8; ++i) {
            unit->ranks[i] = gActiveUnit->ranks[i];
        }

        unit->exp = gActiveUnit->exp;
    }
    else
    {
        for (i = 0; i < 8; ++i) {
            unit->ranks[i] = unit->pClassData->baseRanks[i];

            if (unit->pCharacterData->baseRanks[i])
                unit->ranks[i] = unit->pCharacterData->baseRanks[i];
        }

        if (UNIT_FACTION(unit) == FACTION_BLUE && (unit->level != CHAX_MAX_LEVEL))
            unit->exp = 0;
        else
            unit->exp = UNIT_EXP_DISABLED;
    }

#else

    for (i = 0; i < 8; ++i) {
        unit->ranks[i] = unit->pClassData->baseRanks[i];

        if (unit->pCharacterData->baseRanks[i])
            unit->ranks[i] = unit->pCharacterData->baseRanks[i];
    }

    if (UNIT_FACTION(unit) == FACTION_BLUE && (unit->level != CHAX_MAX_LEVEL))
        unit->exp = 0;
    else
        unit->exp = UNIT_EXP_DISABLED;

#endif

}

LYN_REPLACE_CHECK(UnitLoadStatsFromChracter);
void UnitLoadStatsFromChracter(struct Unit* unit, const struct CharacterData* character)
{
    const LoadUnitFunc_t* it;

    for (it = gpLoadUnitHooks; *it; it++)
        (*it)(unit, character);
}

// For now, no need to hack this funcion
// struct Unit *LoadUnit(const struct UnitDefinition *uDef)

LYN_REPLACE_CHECK(LoadUnit);
struct Unit* LoadUnit(const struct UnitDefinition* uDef) {
    struct UnitDefinition buf;

    struct Unit* unit = NULL;

    if (uDef->genMonster) {
        u32 packedItems;
        u16 item1, item2;

        u16 monsterClass = GenerateMonsterClass(uDef->classIndex);

        buf = *uDef;

        buf.autolevel = TRUE;
        buf.classIndex = monsterClass;
        buf.level = GenerateMonsterLevel(uDef->level);

        packedItems = GenerateMonsterItems(monsterClass);

        // ew
        item1 = packedItems >> 16;
        item2 = packedItems & 0xFFFF;

        buf.items[0] = item1;
        buf.items[1] = item2;
        buf.items[2] = 0;
        buf.items[3] = 0;

        if ((GetItemWeaponEffect(item1) == 1) || !item2)
            buf.itemDrop = FALSE;
        else
            buf.itemDrop = TRUE;

        if (item1 == ITEM_MONSTER_SHADOW_SHOT || item1 == ITEM_MONSTER_STONE) {
            // Add another weapon item if weapon is either Shadowshot or Stone

            buf.items[2] = buf.items[1];

            switch (monsterClass) {

            case CLASS_MOGALL:
                buf.items[1] = ITEM_MONSTER_EVIL_EYE;
                break;

            case CLASS_ARCH_MOGALL:
                buf.items[1] = ITEM_MONSTER_CRIMSON_EYE;
                break;

            case CLASS_GORGON:
                buf.items[1] = ITEM_MONSTER_DEMON_SURGE;

            } // switch (monsterClass)
        }

        if (CanClassWieldWeaponType(monsterClass, ITYPE_BOW) == TRUE) {
            // TODO: AI BIT DEFINITIONS
            buf.ai[2] = buf.ai[2] & (1 | 2 | 4);
            buf.ai[2] = buf.ai[2] | (8 | 32);
        }

        uDef = &buf;
    } // (uDef->genMonster)

    switch (uDef->allegiance) {

    // TODO: unit definition faction constants

    case 0:
        unit = GetFreeBlueUnit(uDef);
        break;

    case 1:
        unit = GetFreeUnit(FACTION_GREEN);
        break;

    case 2:
        unit = GetFreeUnit(FACTION_RED);
        break;

#ifdef CONFIG_FOURTH_ALLEGIANCE
    case 3:
        unit = GetFreeUnit(FACTION_PURPLE);
        break;
#endif

    } // switch (uDef->allegiance)

    if (!unit)
        return NULL;

    ClearUnit(unit);

    UnitInitFromDefinition(unit, uDef);
    UnitLoadStatsFromChracter(unit, unit->pCharacterData);
    UnitHideIfUnderRoof(unit);

    if (UNIT_IS_GORGON_EGG(unit))
        SetUnitStatus(unit, UNIT_STATUS_RECOVER);

    if (uDef->autolevel) {
        if (UNIT_FACTION(unit) == FACTION_BLUE) {
            UnitAutolevelRealistic(unit);
            UnitAutolevelWExp(unit, uDef);
        }
        else {
            if ((UNIT_CATTRIBUTES(unit) & CA_BOSS) || (unit->pCharacterData->number < 0x40)) {
                struct Unit* unit2 = GetFreeUnit(0);

                CopyUnit(unit, unit2);

                unit2->exp = 0;
                UnitAutolevelRealistic(unit2);

                ClearUnit(unit);
                CopyUnit(unit2, unit);

                ClearUnit(unit2);

                unit->exp = UNIT_EXP_DISABLED;
                unit->level = uDef->level;
            }
            else
                UnitAutolevel(unit);

            UnitAutolevelWExp(unit, uDef);
            SetUnitLeaderCharId(unit, uDef->leaderCharIndex);
        }

        if (UNIT_IS_GORGON_EGG(unit))
            unit->maxHP = (unit->level + 1) * 5;
    } // if (uDef->autolevel)

    FixROMUnitStructPtr(unit);
    UnitLoadSupports(unit);

    if (uDef->itemDrop)
        unit->state |= US_DROP_ITEM;

    UnitCheckStatCaps(unit);

    if (gActionData.unk08 == SID_Replicate)
    {
        unit->curHP = gActiveUnit->curHP;

        /* Need to reset this to prevent newly loaded units from having the replicate status set */
        gActionData.unk08 = 0;
    }
    else
    {
        unit->curHP = GetUnitMaxHp(unit);
    }

    if (UNIT_IS_GORGON_EGG(unit))
        SetUnitHp(unit, 5);

    return unit;
}
