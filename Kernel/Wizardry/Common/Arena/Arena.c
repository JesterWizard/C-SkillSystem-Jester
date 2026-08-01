#include "common-chax.h"
#include "kernel-lib.h"
#include "save-data.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "event-rework.h"
#include "bmarena.h"
#include "weapon-slots.h"
#include "kernel/realtime-battle.h"

static const struct ProcCmd gProcScr_ArenaUiMain_NEW[];
static const struct ProcCmd gProcScr_ArenaUiResults_NEW[];

extern struct ProcCmd CONST_DATA ProcScr_Popup2[];
extern const u8 gClassList_MeleeArena[];
extern const u8 gClassList_BowArena[];
extern const u8 gClassList_MagicArena[];

// LYN_REPLACE_CHECK(ArenaBeginInternal);
// void ArenaBeginInternal(struct Unit* unit) {
//     int i;

//     gArenaState.playerUnit = unit;
//     gArenaState.opponentUnit = &gArenaOpponent;

//     gUnknown_03003060 = UNIT_ARENA_LEVEL(unit);

//     gArenaState.playerClassId = unit->pClassData->number;
//     gArenaState.playerWpnType = GetUnitBestWRankType(unit);

//     gArenaState.opponentClassId = ArenaGenerateOpposingClassId(gArenaState.playerWpnType);
//     gArenaState.opponentWpnType = GetClassBestWRankType(GetClassData(gArenaState.opponentClassId));

//     gArenaState.playerIsMagic = IsWeaponMagic(gArenaState.playerWpnType);
//     gArenaState.opponentIsMagic = IsWeaponMagic(gArenaState.opponentWpnType);

//     gArenaState.playerLevel = unit->level;

//     if (UNIT_ARENA_LEVEL(unit) < 5) {
//         gArenaState.opponentLevel = ArenaGetOpposingLevel(gArenaState.playerLevel);
//     } else {
//         gArenaState.opponentLevel = ArenaGetOpposingLevel(gArenaState.playerLevel) + 7;
//     }

//     ArenaGenerateOpponentUnit();
//     ArenaGenerateBaseWeapons();

//     for (i = 0; i < 10; i++) {
//         if (!ArenaAdjustOpponentPowerRanking()) {
//             break;
//         }
//     }

//     for (i = 0; i < 5; i++) {
//         if (!ArenaAdjustOpponentDamage()) {
//             break;
//         }
//     }

//     gArenaState.playerPowerWeight = ArenaGetPowerRanking(gArenaState.playerUnit, gArenaState.opponentIsMagic);
//     gArenaState.opponentPowerWeight = ArenaGetPowerRanking(gArenaState.opponentUnit, gArenaState.playerIsMagic);

//     ArenaGenerateMatchupGoldValue();

//     gArenaState.unk0B = 1;

//     ArenaSetResult(0);

//     ArenaSetFallbackWeaponsMaybe();

//     return;
// };

LYN_REPLACE_CHECK(GetClassBestWRankType);
int GetClassBestWRankType(const struct ClassData *classData)
{
	int slot;
	int bestExp = 0;
	int bestType = -1;

	if (!classData)
		return -1;

	for (slot = 0; slot < UNIT_WEAPON_SLOT_COUNT; ++slot) {
		int wtype = GetClassWeaponSlotType(classData->number, slot);
		int exp;

		if (wtype == WEAPON_SLOT_NONE || wtype == ITYPE_STAFF)
			continue;

		exp = GetClassWeaponSlotBaseRank(classData->number, slot);
		if (exp > bestExp) {
			bestExp = exp;
			bestType = wtype;
		}
	}

	return bestType;
}

LYN_REPLACE_CHECK(ArenaGenerateOpposingClassId);
int ArenaGenerateOpposingClassId(int weaponType)
{
	const u8 *classList;
	int i;
	int classCount = 0;
	int classNum;
	u32 promotedFlag;

	switch (weaponType) {
	case ITYPE_SWORD:
	case ITYPE_LANCE:
	case ITYPE_AXE:
	case ITYPE_KNIFE:
		classList = gClassList_MeleeArena;
		break;

	case ITYPE_BOW:
	case ITYPE_GUN:
		classList = gClassList_BowArena;
		break;

	case ITYPE_ANIMA:
	case ITYPE_LIGHT:
	case ITYPE_DARK:
		classList = gClassList_MagicArena;
		break;

	default:
		/* Keep custom physical types from reaching a null class list. */
		classList = gClassList_MeleeArena;
		break;
	}

	promotedFlag = UNIT_CATTRIBUTES(gArenaState.playerUnit) & CA_PROMOTED;

	for (i = 0; classList[i] != 0; ++i) {
		if ((GetClassData(classList[i])->attributes & CA_PROMOTED) != promotedFlag)
			continue;

		++classCount;
	}

	if (classCount == 0)
		return classList[0];

	classNum = NextRN_N(classCount);

	for (i = 0, classCount = 0; classList[i] != 0; ++i) {
		if ((GetClassData(classList[i])->attributes & CA_PROMOTED) != promotedFlag)
			continue;

		if (classCount == classNum)
			return classList[i];

		++classCount;
	}

	return classList[0];
}

LYN_REPLACE_CHECK(IsWeaponMagic);
s8 IsWeaponMagic(int weaponType)
{
	switch (weaponType) {
	case ITYPE_ANIMA:
	case ITYPE_LIGHT:
	case ITYPE_DARK:
		return true;

	default:
		return false;
	}
}

static u16 ArenaGetBaseWeaponForType(int weaponType)
{
	switch (weaponType) {
	case ITYPE_SWORD:
		return ITEM_SWORD_IRON;
	case ITYPE_LANCE:
		return ITEM_LANCE_IRON;
	case ITYPE_AXE:
		return ITEM_AXE_IRON;
	case ITYPE_BOW:
		return ITEM_BOW_IRON;
	case ITYPE_ANIMA:
		return ITEM_ANIMA_FIRE;
	case ITYPE_LIGHT:
		return ITEM_LIGHT_LIGHTNING;
	case ITYPE_DARK:
		return ITEM_DARK_FLUX;
	case ITYPE_KNIFE:
		return ITEM_KNIFE_IRON;
	default:
		return ITEM_NONE;
	}
}

static u16 ArenaGetUpgradedWeapon_NEW(struct Unit * unit, u16 item) {

    u8 *iter;

    u8 arenaWeaponUpgrades[] = {
        ITEM_SWORD_IRON, ITEM_SWORD_STEEL, ITEM_SWORD_SILVER, 0,
        ITEM_LANCE_IRON, ITEM_LANCE_STEEL, ITEM_LANCE_SILVER, 0,
        ITEM_AXE_IRON, ITEM_AXE_STEEL, ITEM_AXE_SILVER, 0,
        ITEM_BOW_IRON, ITEM_BOW_STEEL, ITEM_BOW_SILVER, 0,
        ITEM_ANIMA_FIRE, ITEM_ANIMA_ELFIRE, ITEM_ANIMA_FIMBULVETR, 0,
        ITEM_LIGHT_LIGHTNING, ITEM_LIGHT_DIVINE, 0,
        ITEM_DARK_FLUX, 0,

        -1
    };

    if (gpKernelDesignerConfig->arena_calculate_weapon_based_on_level == true)
    {
        u8 effectiveLevel = unit->level;

        if (UNIT_CATTRIBUTES(unit) & CA_PROMOTED)
            effectiveLevel += 20;

        u8 desiredStage =
            (effectiveLevel <= 10) ? 0 :
            (effectiveLevel <= 20) ? 1 :
                                    2;
                
        for (int i = 0; i < (int)ARRAY_COUNT(arenaWeaponUpgrades); i += 4)
        {
            if (GetItemIndex(item) == arenaWeaponUpgrades[i * 4])
                return MakeNewItem(arenaWeaponUpgrades[(i * 4) + desiredStage]);
        }
    }

    for (iter = arenaWeaponUpgrades; *iter != (u8) -1; iter++)
    {
        if (GetItemIndex(item) != *iter) {
            continue;
        }

        if (*++iter != 0) {
            return MakeNewItem(*iter);
        }

        return item;
    }

    /* The vanilla function lacks a return type outside of the loop */
    /* This will never be reached */
    return item;
};

LYN_REPLACE_CHECK(ArenaGenerateBaseWeapons);
void ArenaGenerateBaseWeapons(void)
{
    u16 playerWeapon = ArenaGetBaseWeaponForType(gArenaState.playerWpnType);
    u16 opponentWeapon = ArenaGetBaseWeaponForType(gArenaState.opponentWpnType);

    gArenaState.playerWeapon = playerWeapon ? MakeNewItem(playerWeapon) : ITEM_NONE;

    if (gpKernelDesignerConfig->arena_let_player_use_upgraded_weapons == true)
        gArenaState.playerWeapon = ArenaGetUpgradedWeapon_NEW(gArenaState.playerUnit, gArenaState.playerWeapon);

    if (ArenaRosterHasConfiguredOpponent())
        gArenaState.opponentWeapon = MakeNewItem(ArenaRosterGetSelectedWeapon());
    else
        gArenaState.opponentWeapon = opponentWeapon ? MakeNewItem(opponentWeapon) : ITEM_NONE;

    gArenaState.range = 1;

    if (gArenaState.playerWpnType == ITYPE_BOW)
    {
        gArenaState.range = 2;
    }

    if (gArenaState.opponentWpnType == ITYPE_BOW)
    {
        gArenaState.range = 2;
    }

    /*
    ** If the opponent is using a bow, the skill holder's equipped weapon must
    ** allow the opponent to counter in order for the skill to activate
    */
#if (defined(SID_ConcealedWeapon) && COMMON_SKILL_VALID(SID_ConcealedWeapon))
    if (SkillTester(gActiveUnit, SID_ConcealedWeapon))
    {
        if (GetUnitEquippedWeapon(gActiveUnit) != 0)
        {
            if ((GetItemMinRange(gArenaState.opponentWeapon) == GetItemMinRange(GetUnitEquippedWeapon(gActiveUnit))) ||
                (GetItemMaxRange(gArenaState.opponentWeapon) == GetItemMaxRange(GetUnitEquippedWeapon(gActiveUnit))))
            {
                gArenaState.playerWeapon = GetUnitEquippedWeapon(gActiveUnit);

                if (gArenaState.opponentWpnType != ITYPE_BOW && GetItemType(GetUnitEquippedWeapon(gActiveUnit)) != ITYPE_BOW)
                    gArenaState.range = 1;
            }
        }
    }
#endif

    return;
}

/* This seems to cause crashes now on the arena screen */
// LYN_REPLACE_CHECK(ArenaAdjustOpponentDamage);
// s8 ArenaAdjustOpponentDamage(void) {
//     s8 result = 0;

//     gBattleActor.battleAttack = GetUnitPower(gArenaState.playerUnit) + 5;

//     if (gArenaState.opponentIsMagic) {
//         gBattleActor.battleDefense = GetUnitResistance(gArenaState.playerUnit);
//     } else {
//         gBattleActor.battleDefense = GetUnitDefense(gArenaState.playerUnit);
//     }

//     gBattleTarget.battleAttack = GetUnitPower(gArenaState.opponentUnit) + 5;

//     if (gArenaState.playerIsMagic) {
//         gBattleTarget.battleDefense = GetUnitResistance(gArenaState.opponentUnit);
//     } else {
//         gBattleTarget.battleDefense = GetUnitDefense(gArenaState.opponentUnit);
//     }

//     if ((gBattleActor.battleAttack - gBattleTarget.battleDefense) < (GetUnitMaxHp(gArenaState.opponentUnit) / 6)) {
//         result = 1;

//         if (gArenaState.playerIsMagic) {
//             gArenaState.opponentUnit->res -= 4;

//             if (gArenaState.opponentUnit->res < 0) {
//                 gArenaState.opponentUnit->res = 0;
//             }
//         } else {
//             gArenaState.opponentUnit->def -= 4;

//             if (gArenaState.opponentUnit->def < 0) {
//                 gArenaState.opponentUnit->def = 0;
//             }
//         }

//         gArenaState.opponentUnit->spd += 1;
//         gArenaState.opponentUnit->skl += 1;
//     }

//     if (gBattleTarget.battleAttack - gBattleActor.battleDefense < (GetUnitMaxHp(gArenaState.playerUnit) / 6)) {
//         result = 1;

//         gArenaState.opponentUnit->pow += 3;
//         gArenaState.opponentUnit->spd += 2;
//         gArenaState.opponentUnit->skl += 2;

//         gArenaState.opponentWeapon = ArenaGetUpgradedWeapon(gArenaState.opponentWeapon);
//     }

//     return result;
// }

LYN_REPLACE_CHECK(ArenaAdjustOpponentPowerRanking);
s8 ArenaAdjustOpponentPowerRanking(void) {
    if (ArenaRosterHasConfiguredOpponent()) {
        return 0;
    }

    int max;
    int diff;

    gArenaState.playerPowerWeight = ArenaGetPowerRanking(gArenaState.playerUnit, gArenaState.opponentIsMagic);
    gArenaState.opponentPowerWeight = ArenaGetPowerRanking(gArenaState.opponentUnit, gArenaState.playerIsMagic);

    max = gArenaState.playerPowerWeight > gArenaState.opponentPowerWeight
        ? gArenaState.playerPowerWeight
        : gArenaState.opponentPowerWeight;

    diff = ABS(gArenaState.playerPowerWeight - gArenaState.opponentPowerWeight);

    if (((diff * 100) / max) <= 20) {
        return 0;
    }

    if (gArenaState.playerPowerWeight < gArenaState.opponentPowerWeight) {
        if (gArenaState.opponentUnit->maxHP != 0) {
            gArenaState.opponentUnit->maxHP -= 1;
            gArenaState.opponentUnit->curHP -= 1;
        }

        if (gArenaState.opponentUnit->pow != 0) {
            gArenaState.opponentUnit->pow -= 1;
        }

        if (gArenaState.opponentUnit->skl != 0) {
            gArenaState.opponentUnit->skl -= 1;
        }

        if (gArenaState.opponentUnit->spd != 0) {
            gArenaState.opponentUnit->spd -= 1;
        }

        if (gArenaState.opponentUnit->def != 0) {
            gArenaState.opponentUnit->def -= 1;
        }

        if (gArenaState.opponentUnit->res != 0) {
            gArenaState.opponentUnit->res -= 1;
        }

        if (gArenaState.opponentUnit->lck != 0) {
            gArenaState.opponentUnit->lck -= 1;
        }

    } else {
        if (gArenaState.opponentUnit->maxHP < 80) {
            gArenaState.opponentUnit->maxHP += 2;
            gArenaState.opponentUnit->curHP += 2;
        }

        if (gArenaState.opponentUnit->pow < 30) {
            gArenaState.opponentUnit->pow += 1;
        }

        if (gArenaState.opponentUnit->skl < 30) {
            gArenaState.opponentUnit->skl += 1;
        }

        if (gArenaState.opponentUnit->spd < 30) {
            gArenaState.opponentUnit->spd += 1;
        }

        if (gArenaState.opponentUnit->def < 30) {
            gArenaState.opponentUnit->def += 1;
        }

        if (gArenaState.opponentUnit->res < 30) {
            gArenaState.opponentUnit->res += 1;
        }

        if (gArenaState.opponentUnit->lck < 30) {
            gArenaState.opponentUnit->lck += 1;
        }
    }

    return 1;
}

static void ArenaUi_RedrawRosterOpponentDetails(ProcPtr proc)
{
    if (!ArenaRosterHasConfiguredOpponent())
        return;

    if (gpKernelDesignerConfig->arena_show_opponent_in_advance != true)
        return;

    DrawUiFrame2(7, 9, 0x10, 8, 0);
    DrawArenaOpponentDetailsText(proc);
    RefreshUnitSprites();
    SyncUnitSpriteSheet();
    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void ArenaUi_ClearRosterUi(void);

static bool ArenaRosterPopup2Exists(void)
{
    if (Proc_Find(ProcScr_Popup2))
        return true;

    if (Proc_Find(ProcScr_Popup))
        return true;
    else
        return false;
}

//! FE8U = 0x080B59CC
LYN_REPLACE_CHECK(ArenaUi_WagerGoldDialogue);
void ArenaUi_WagerGoldDialogue(ProcPtr proc)
{
    int multiplier = 1;

    if (ArenaRosterStartSelection(proc))
        return;
    

#if defined(SID_Ludopathy) && (COMMON_SKILL_VALID(SID_Ludopathy))
    if (SkillTester(gActiveUnit, SID_Ludopathy))
        multiplier = 2;
#endif

    if (gpKernelDesignerConfig->arena_show_opponent_in_advance == true)
    {
        ArenaUi_RedrawRosterOpponentDetails(proc);

        if (!ArenaRosterHasConfiguredOpponent()) {
            DrawUiFrame2(7, 9, 0x10, 8, 0);
        }

        SetTextFont(0);
        InitSystemTextFont();

        PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 8, 10), 0, GetStringFromIndex(gMid_Lv));
        PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 12, 10), 2, gArenaState.opponentUnit->level);
        PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 8, 12), 0, GetStringFromIndex(gArenaState.opponentUnit->pCharacterData->nameTextId));
        PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 15, 10), 0, GetStringFromIndex(gArenaState.opponentUnit->pClassData->nameTextId));
        PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 15, 12), 0, GetItemName(gArenaState.opponentWeapon));

        if (gArenaState.playerPowerWeight - gArenaState.opponentPowerWeight >= 20)
            PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 12, 14), TEXT_COLOR_SYSTEM_GREEN, "Good match");
        else if (gArenaState.opponentPowerWeight - gArenaState.playerPowerWeight <= 20)
            PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 12, 14), TEXT_COLOR_SYSTEM_BLUE, "Okay match");
        else
            PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 12, 14), TEXT_COLOR_SYSTEM_GRAY, "Bad match");
    }

    SetTalkNumber(ArenaGetMatchupGoldValue() * multiplier);
    StartArenaDialogue(0x8D2, proc);

    // TODO: msgid "Would you like to wager[.][NL][G] gold?[Yes]"

    return;
}

//! FE8U = 0x080B5A7C
LYN_REPLACE_CHECK(ArenaUi_InstructionsDialogue);
void ArenaUi_InstructionsDialogue(ProcPtr proc) {
    // ArenaUi_RedrawRosterOpponentDetails(proc);
    StartArenaDialogue(0x8D5, proc);
    // TODO: msgid "Fight 'til you drop, or press[.][NL]the B Button to yield.[A]"
    return;
}

//! FE8U = 0x080B5A90
LYN_REPLACE_CHECK(ArenaUi_GoodLuckDialogue);
void ArenaUi_GoodLuckDialogue(ProcPtr proc) {
    ArenaUi_RedrawRosterOpponentDetails(proc);
    StartArenaDialogue(0x8D3, proc);
    // TODO: msgid "Good luck. Don't get[NL]yourself killed.[A]"
    return;
}

//! FE8U = 0x080B59EC
LYN_REPLACE_CHECK(ArenaUi_CheckConfirmation);
void ArenaUi_CheckConfirmation(ProcPtr proc)
{
    if (ArenaRosterHandleConfirmation(proc))
        return;

    ArenaUi_RedrawRosterOpponentDetails(proc);

    int multiplier = 1;

#if defined(SID_Ludopathy) && (COMMON_SKILL_VALID(SID_Ludopathy))
    if (SkillTester(gActiveUnit, SID_Ludopathy))
        multiplier = 2;
#endif

    if (GetTalkChoiceResult() != 1)
    {
        ArenaRosterClearSelection();
        StartArenaDialogue(0x8D4, proc);
        // TODO: msgid "What's that? Bah![.][NL]Get outta here![.][A]"
        Proc_Goto(proc, 2);
    }
    else
    {
        if (ArenaGetMatchupGoldValue() > (int)GetPartyGoldAmount() * multiplier)
        {
            ArenaRosterClearSelection();
            StartArenaDialogue(0x8DA, proc);
            // TODO: msgid "You don't have the money![.][NL]Try again later.[A]"
            Proc_Goto(proc, 2);
        }
    }

    return;
}

//! FE8U = 0x080B5A38
LYN_REPLACE_CHECK(ArenaUi_ConfirmWager);
void ArenaUi_ConfirmWager(ProcPtr proc)
{
    int multiplier = 1;

#if defined(SID_Ludopathy) && (COMMON_SKILL_VALID(SID_Ludopathy))
    if (SkillTester(gActiveUnit, SID_Ludopathy))
        multiplier = 2;
#endif

    SetPartyGoldAmount(GetPartyGoldAmount() - ArenaGetMatchupGoldValue() * multiplier);
    PlaySoundEffect(0xb9);
    DisplayGoldBoxText(TILEMAP_LOCATED(gBG0TilemapBuffer, 0x1B, 0x6));
    DrawArenaOpponentDetailsText(proc);

    return;
};

LYN_REPLACE_CHECK(ArenaGetPowerRanking);
int ArenaGetPowerRanking(struct Unit* unit, s8 opponentIsMagic) {
    int result = unit->maxHP;

    result += unit->maxHP;
    result += unit->pow * 2;
    result += unit->skl * 2;
    result += unit->spd * 2;
    result += unit->lck;
    result += UNIT_CON_BASE(unit);

    if (opponentIsMagic) {
        result += GetUnitResistance(unit) * 2;
    } else {
        result += GetUnitDefense(unit) * 2;
    }

    if (UNIT_CATTRIBUTES(unit) & CA_CRITBONUS) {
        result += GetUnitPower(unit);
    }

    return result;
}

LYN_REPLACE_CHECK(ArenaGenerateOpponentUnit);
void ArenaGenerateOpponentUnit(void) {
    if (ArenaRosterGenerateOpponentUnit(&gArenaOpponent)) {
        return;
    }

    int level;
    int i;

    struct UnitDefinition udef;

    struct Unit* unit = &gArenaOpponent;

    udef.charIndex = CHARACTER_ARENA_OPPONENT;
    udef.classIndex = gArenaState.opponentClassId;
    udef.allegiance = 0;
    udef.level = gArenaState.opponentLevel;
    udef.autolevel = 1;
    udef.items[0] = 0;
    udef.items[1] = 0;
    udef.items[2] = 0;
    udef.items[3] = 0;
    udef.ai[0] = 0;
    udef.ai[0] = 0;
    udef.ai[1] = 0;
    udef.ai[2] = 0;
    udef.ai[3] = 0;

    //ClearUnit(&gArenaOpponent);
    unit->index = 0x80;

    UnitInitFromDefinition(unit, &udef);
    UnitLoadStatsFromChracter(unit, unit->pCharacterData);

    level = unit->level;

    unit->level = ((gPlaySt.chapterStateBits & PLAY_FLAG_HARD) ? level * 24 : level * 12) / 10;

    UnitAutolevel(unit);

    unit->level = level;

    for (i = 0; i < 8; i++) {
        if (unit->ranks[i] != 0) {
            unit->ranks[i] = -75;
        }
    }

    if (unit->level < 1) {
        unit->level = 1;
    }

    if (unit->level > 20) {
        unit->level = 20;
    }

    UnitCheckStatCaps(unit);
    SetUnitHp(unit, GetUnitMaxHp(unit));

    return;
}

/* Why does this exist? It's a secondary weapon setting function for the arena */
LYN_REPLACE_CHECK(ArenaSetFallbackWeaponsMaybe);
void ArenaSetFallbackWeaponsMaybe(void) {
    // ArenaSetFallbackWeaponForUnit(gArenaState.playerUnit, &gArenaState.playerWeapon);
    // ArenaSetFallbackWeaponForUnit(gArenaState.opponentUnit, &gArenaState.opponentWeapon);

    return;
}

//! FE8U = 0x080B5B18
LYN_REPLACE_CHECK(ArenaUi_ResultsDialogue);
void ArenaUi_ResultsDialogue(ProcPtr proc) {
    if (ArenaRosterHandleResultsDialogue(proc))
        return;

    u32 partyGold = GetPartyGoldAmount();

    switch (ArenaGetResult()) {
        case 1:
            SetTalkNumber(ArenaGetMatchupGoldValue() * 2);
            StartArenaDialogue(0x8D6, proc);
            // TODO: msgid "So you won, eh? Here's[NL]your prize. [G] gold.[A]"

            SetPartyGoldAmount(partyGold = partyGold + (ArenaGetMatchupGoldValue() * 2));

            break;

        case 2:
            StartArenaDialogue(0x8D7, proc);
            // TODO: msgid "Ahh, you lost? I'd hoped[NL]for better from you.[A]"

            break;

        case 3:
            StartArenaDialogue(0x8D9, proc);
            // TODO: msgid "Looks like no one wins.[.][NL]Here's your money back.[.][A]"
            SetPartyGoldAmount(partyGold = partyGold + ArenaGetMatchupGoldValue());

            break;

        case 4:
            // _080B5B88
            StartArenaDialogue(0x8D8, proc);
            // TODO: msgid "What? You yield? Well,[NL]your gold is mine, then![A]"
            break;
    }

    return;
}


//! FE8U = 0x080B5BE4
LYN_REPLACE_CHECK(ArenaUi_OnEnd);
void ArenaUi_OnEnd(void) {
    Proc_EndEach(gProcScr_GoldBox);
    Proc_ForEach(ProcScr_Mu, (ProcFunc) ShowMu);
    RealtimeBattle_Resume(RT_PAUSE_ARENA);
    return;
}

#define ARENA_WIN 0x1
#define ARENA_LOSS 0x2
#define ARENA_OK 0x1
#define CANT_ARENA 0x3
#define CENTERTUTORIALTEXTBOX SVAL(0xB, 0xFFFFFFFF)

#define ARENA_ROSTER_VISIBLE_ROWS 5
#define ARENA_ROSTER_MAX_TRACKED_ENTRIES 32
#define ARENA_ROSTER_CURSOR_X 12
#define ARENA_ROSTER_CURSOR_Y 72
#define ARENA_ROSTER_ROW_HEIGHT 16
#define ARENA_ROSTER_LIST_X 1
#define ARENA_ROSTER_LIST_Y 8
#define ARENA_ROSTER_LIST_W 28
#define ARENA_ROSTER_LIST_H 12
#define ARENA_ROSTER_SCROLLBAR_X 228
#define ARENA_ROSTER_SCROLLBAR_Y 80
#define ARENA_ROSTER_TEXT_NAME_WIDTH 10
#define ARENA_ROSTER_TEXT_BUFFER_WIDTH 13
#define ARENA_ROSTER_TEXT_REWARD_WIDTH 8
#define ARENA_ROSTER_REWARD_ICON_X 19
#define ARENA_ROSTER_REWARD_TEXT_X 21
#define ARENA_ROSTER_STATE_RESERVED_BYTES 25

enum ArenaRosterChoiceState {
    ARENA_ROSTER_CHOICE_NONE,
    ARENA_ROSTER_CHOICE_CANCELLED,
    ARENA_ROSTER_CHOICE_SELECTED,
};

#define ARENA_ROSTER_RUNTIME_FLAG_LOADED_FROM_SUSPEND (1 << 7)

enum ArenaRosterRewardType {
    ARENA_ROSTER_REWARD_DOUBLE_PAYOUT,
    ARENA_ROSTER_REWARD_TRIPLE_PAYOUT,
    ARENA_ROSTER_REWARD_ITEM,
};

struct _ArenaLimitTable {
    u8 ChapterID;
    u8 LevelLimit;
    u8 TurnLimit;
};

struct ArenaRosterEntry {
    u8 pid;
    u8 jid;
    u8 level;
    u8 rewardType;
    u16 item;
    u16 price;
    u16 reward;
};

struct ArenaRosterChapter {
    u8 chapterId;
    u8 maxWins;
    u8 entryCount;
    u8 _pad;
    const struct ArenaRosterEntry *entries;
};

struct ArenaRosterSuspendState {
    u8 chapterId;
    u8 wins;
    u8 _pad[2];
    u8 clearedFlags[4];
    u8 reserved[ARENA_ROSTER_STATE_RESERVED_BYTES];
};

struct ProcArenaRosterSelect {
    PROC_HEADER;
    int cursor;
    int scrollTop;
    u8 promptShown;
    u8 _pad[3];
};

struct ArenaRosterRuntimeState {
    u8 choiceState;
    s8 selectedIndex;
    u16 pendingRewardItem;
};

extern struct Text gBanimText[20];
extern struct ArenaRosterSuspendState sArenaRosterSuspendState;
extern struct ArenaRosterRuntimeState sArenaRosterRuntimeState;

STATIC_DECLAR void ArenaUi_ClearRosterUi(void)
{
    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, ARENA_ROSTER_LIST_X, ARENA_ROSTER_LIST_Y), ARENA_ROSTER_LIST_W, ARENA_ROSTER_LIST_H, 0);
    TileMap_FillRect(TILEMAP_LOCATED(gBG1TilemapBuffer, ARENA_ROSTER_LIST_X, ARENA_ROSTER_LIST_Y), ARENA_ROSTER_LIST_W, ARENA_ROSTER_LIST_H, 0);
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

STATIC_DECLAR const EventScr EventScr_ArenaClosed[] = {
    EVBIT_MODIFY(0x4)
    TUTORIALTEXTBOXSTART
    CENTERTUTORIALTEXTBOX
    TEXTSHOW(MSG_ArenaClosedString)
    TEXTEND
    NOFADE
    ENDA
};

STATIC_DECLAR const EventScr EventScr_ArenaLevelLimit[] = {
    EVBIT_MODIFY(0x4)
    TUTORIALTEXTBOXSTART
    CENTERTUTORIALTEXTBOX
    TEXTSHOW(MSG_ArenaLevelLimitString)
    TEXTEND
    NOFADE
    ENDA
};

STATIC_DECLAR const EventScr EventScr_UnpromotedOnly[] = {
    EVBIT_MODIFY(0x4)
    TUTORIALTEXTBOXSTART
    CENTERTUTORIALTEXTBOX
    TEXTSHOW(MSG_UnpromotedOnlyString)
    TEXTEND
    NOFADE
    ENDA
};

STATIC_DECLAR const struct ArenaRosterEntry sArenaRosterChapter6Entries[] = {
    {
        .pid = CHARACTER_JOSHUA,
        .jid = CLASS_MYRMIDON,
        .level = 5,
        .rewardType = ARENA_ROSTER_REWARD_ITEM,
        .item = ITEM_SWORD_KILLER,
        .price = 900,
        .reward = ITEM_SWORD_AUDHULMA,
    },
    {
        .pid = 0x00,
        .jid = CLASS_MERCENARY,
        .level = 6,
        .rewardType = ARENA_ROSTER_REWARD_TRIPLE_PAYOUT,
        .item = 0x01,
        .price = 500,
        .reward = 0,
    },
    {
        .pid = 0x00,
        .jid = CLASS_FIGHTER,
        .level = 5,
        .rewardType = ARENA_ROSTER_REWARD_DOUBLE_PAYOUT,
        .item = 0x1F,
        .price = 480,
        .reward = 0,
    },
    {
        .pid = 0x00,
        .jid = CLASS_ARCHER,
        .level = 5,
        .rewardType = ARENA_ROSTER_REWARD_ITEM,
        .item = ITEM_BOW_IRON,
        .price = 460,
        .reward = ITEM_BOW_IRON,
    },
    {
        .pid = 0x00,
        .jid = CLASS_SOLDIER,
        .level = 5,
        .rewardType = ARENA_ROSTER_REWARD_DOUBLE_PAYOUT,
        .item = 0x14,
        .price = 440,
        .reward = 0,
    },
    {
        .pid = 0x00,
        .jid = CLASS_PALADIN,
        .level = 5,
        .rewardType = ARENA_ROSTER_REWARD_DOUBLE_PAYOUT,
        .item = 0x14,
        .price = 440,
        .reward = 0,
    },
};

const struct ArenaRosterChapter gArenaRosterTable[] = {
    {
        .chapterId = 6,
        .maxWins = sizeof(sArenaRosterChapter6Entries) / sizeof(sArenaRosterChapter6Entries[0]),
        .entryCount = sizeof(sArenaRosterChapter6Entries) / sizeof(sArenaRosterChapter6Entries[0]),
        .entries = sArenaRosterChapter6Entries,
    },
    {
        .chapterId = 0xFF,
    },
};

const struct _ArenaLimitTable ArenaLimitTable[] = {
    { 6, 5, 3 },
    { 0xFF, 0xFF, 0xFF },
};

const u8 NonLethalArena = 0x1;
const u8 MaxLevel = 10;

STATIC_DECLAR void ArenaRosterSelector_Init(struct ProcArenaRosterSelect *proc);
STATIC_DECLAR void ArenaRosterSelector_End(struct ProcArenaRosterSelect *proc);
STATIC_DECLAR void ArenaRosterSelector_Loop(struct ProcArenaRosterSelect *proc);
STATIC_DECLAR void ArenaRosterSelector_WaitForWagerChoice(struct ProcArenaRosterSelect *proc);
STATIC_DECLAR int ArenaRosterGetEntryCount(const struct ArenaRosterChapter *chapter);
STATIC_DECLAR int ArenaRosterGetWagerMultiplier(void);
STATIC_DECLAR unsigned ArenaRosterGetRewardGold(const struct ArenaRosterEntry *entry);
STATIC_DECLAR bool ArenaRosterHasReachedWinLimit(const struct ArenaRosterChapter *chapter);
STATIC_DECLAR void ArenaRosterGrantWinReward(const struct ArenaRosterEntry *entry);
STATIC_DECLAR void ArenaRosterApplyEnemySpritePalettes(void);
STATIC_DECLAR void ArenaRosterUpdateScrollBar(struct ProcArenaRosterSelect *proc, int entryCount);
STATIC_DECLAR void ArenaRosterDrawSpritesOnly(struct ProcArenaRosterSelect *proc, const struct ArenaRosterChapter *chapter);
STATIC_DECLAR void ArenaRosterCancelSelectionAndExit(struct ProcArenaRosterSelect *proc);

STATIC_DECLAR const struct ProcCmd ProcScr_ArenaRosterSelect[] = {
    PROC_CALL(ArenaRosterSelector_Init),
PROC_LABEL(0),
    PROC_REPEAT(ArenaRosterSelector_Loop),
PROC_LABEL(1),
    PROC_REPEAT(ArenaRosterSelector_WaitForWagerChoice),
    PROC_CALL(ArenaRosterSelector_End),
    PROC_END,
};

STATIC_DECLAR const struct ArenaRosterChapter *ArenaRosterGetChapter(int chapter_id)
{
    int index;

    for (index = 0; gArenaRosterTable[index].chapterId != 0xFF; ++index)
        if (gArenaRosterTable[index].chapterId == chapter_id)
            return &gArenaRosterTable[index];

    return NULL;
}

STATIC_DECLAR void ArenaRosterClearProgress(void)
{
    sArenaRosterSuspendState.clearedFlags[0] = 0;
    sArenaRosterSuspendState.clearedFlags[1] = 0;
    sArenaRosterSuspendState.clearedFlags[2] = 0;
    sArenaRosterSuspendState.clearedFlags[3] = 0;
    sArenaRosterSuspendState.wins = 0;
    sArenaRosterSuspendState._pad[0] = 0;
    sArenaRosterSuspendState._pad[1] = 0;
    sArenaRosterRuntimeState.pendingRewardItem = ITEM_NONE;
}

STATIC_DECLAR void ArenaRosterEnsureSuspendStateCurrent(void)
{
    if (sArenaRosterSuspendState.chapterId == gPlaySt.chapterIndex)
        return;

    sArenaRosterSuspendState.chapterId = gPlaySt.chapterIndex;
    ArenaRosterClearProgress();
}

STATIC_DECLAR int ArenaRosterGetWinLimit(const struct ArenaRosterChapter *chapter)
{
    if (!chapter)
        return 0;

    return chapter->maxWins;
}

STATIC_DECLAR int ArenaRosterGetEntryCount(const struct ArenaRosterChapter *chapter)
{
    if (!chapter || !chapter->entries)
        return 0;

    if (chapter->entryCount > ARENA_ROSTER_MAX_TRACKED_ENTRIES)
        return ARENA_ROSTER_MAX_TRACKED_ENTRIES;

    return chapter->entryCount;
}

STATIC_DECLAR bool ArenaRosterHasReachedWinLimit(const struct ArenaRosterChapter *chapter)
{
    int limit = ArenaRosterGetWinLimit(chapter);

    if (limit == 0)
        return false;

    ArenaRosterEnsureSuspendStateCurrent();
    return sArenaRosterSuspendState.wins >= limit;
}

STATIC_DECLAR bool ArenaRosterEntryValid(const struct ArenaRosterEntry *entry)
{
    return entry && entry->jid && entry->item;
}

STATIC_DECLAR bool ArenaRosterEntryCleared(int entry_idx)
{
    int byteIndex;
    int bitIndex;

    ArenaRosterEnsureSuspendStateCurrent();

    if ((entry_idx < 0) || (entry_idx >= ARENA_ROSTER_MAX_TRACKED_ENTRIES))
        return false;

    byteIndex = entry_idx >> 3;
    bitIndex = entry_idx & 7;

    return (sArenaRosterSuspendState.clearedFlags[byteIndex] & (1u << bitIndex)) != 0;
}

STATIC_DECLAR void ArenaRosterSetEntryCleared(int entry_idx)
{
    int byteIndex;
    int bitIndex;

    ArenaRosterEnsureSuspendStateCurrent();

    if ((entry_idx < 0) || (entry_idx >= ARENA_ROSTER_MAX_TRACKED_ENTRIES))
        return;

    if (ArenaRosterEntryCleared(entry_idx))
        return;

    byteIndex = entry_idx >> 3;
    bitIndex = entry_idx & 7;

    sArenaRosterSuspendState.clearedFlags[byteIndex] |= (1u << bitIndex);
    sArenaRosterSuspendState.wins++;
}

STATIC_DECLAR const struct ArenaRosterEntry *ArenaRosterGetSelectedEntry(void)
{
    const struct ArenaRosterChapter *chapter;

    if ((sArenaRosterRuntimeState.choiceState & 0x7F) != ARENA_ROSTER_CHOICE_SELECTED)
        return NULL;

    chapter = ArenaRosterGetChapter(gPlaySt.chapterIndex);
    if (!chapter)
        return NULL;

    if ((sArenaRosterRuntimeState.selectedIndex < 0) ||
        (sArenaRosterRuntimeState.selectedIndex >= ArenaRosterGetEntryCount(chapter)))
        return NULL;

    return &chapter->entries[sArenaRosterRuntimeState.selectedIndex];
}

STATIC_DECLAR char const *ArenaRosterGetDisplayName(const struct ArenaRosterEntry *entry)
{
    if (entry->pid && GetCharacterData(entry->pid)->nameTextId)
        return GetStringFromIndex(GetCharacterData(entry->pid)->nameTextId);

    return GetStringFromIndex(GetClassData(entry->jid)->nameTextId);
}

STATIC_DECLAR int ArenaRosterGetWagerMultiplier(void)
{
    int multiplier = 1;

#if defined(SID_Ludopathy) && (COMMON_SKILL_VALID(SID_Ludopathy))
    if (SkillTester(gActiveUnit, SID_Ludopathy))
        multiplier = 2;
#endif

    return multiplier;
}

STATIC_DECLAR unsigned ArenaRosterGetRewardGold(const struct ArenaRosterEntry *entry)
{
    if (!entry)
        return 0;

    if (entry->rewardType == ARENA_ROSTER_REWARD_TRIPLE_PAYOUT)
        return entry->price * 3;

    if (entry->rewardType == ARENA_ROSTER_REWARD_ITEM)
        return 0;

    return entry->price * 2;
}

STATIC_DECLAR bool ArenaRosterIsDialogueOpen(void)
{
    return Proc_Find(gProcScr_TalkOpen) != NULL;
}

STATIC_DECLAR void ArenaRosterGrantWinReward(const struct ArenaRosterEntry *entry)
{
    int item;

    if (!entry)
        return;

    if (entry->rewardType == ARENA_ROSTER_REWARD_ITEM) {
        item = MakeNewItem(entry->reward);

        ProcPtr proc;

        proc = Proc_Find(gProcScr_ArenaUiMain_NEW);

        if (!proc)
            proc = Proc_Find(gProcScr_ArenaUiResults_NEW);

        if (!proc)
            proc = Proc_Find(gProcScr_PlayerPhase);

        if (!proc)
            proc = Proc_Find(gProc_BMapMain);

        NewPopup2_SendItem(proc, item);

        return;
    }

    SetPartyGoldAmount(GetPartyGoldAmount() + ArenaRosterGetRewardGold(entry));
}

STATIC_DECLAR void ArenaRosterApplyEnemySpritePalettes(void)
{
    int faction = gPlaySt.faction;

    gPlaySt.faction = FACTION_RED;
    ApplyUnitSpritePalettes();
    gPlaySt.faction = faction;
}

STATIC_DECLAR void ArenaRosterUpdateScrollBar(struct ProcArenaRosterSelect *proc, int entryCount)
{
    if (entryCount < 1)
        entryCount = 1;

    UpdateMenuScrollBarConfig(8, proc->scrollTop * ARENA_ROSTER_ROW_HEIGHT, entryCount, ARENA_ROSTER_VISIBLE_ROWS);
}

STATIC_DECLAR void ArenaRosterDrawSpritesOnly(struct ProcArenaRosterSelect *proc, const struct ArenaRosterChapter *chapter)
{
    int row;
    int entryCount;

    if (!chapter)
        return;

    entryCount = ArenaRosterGetEntryCount(chapter);

    ClearSprites();

    for (row = 0; row < ARENA_ROSTER_VISIBLE_ROWS; ++row) {
        int index = proc->scrollTop + row;
        const struct ArenaRosterEntry *entry;

        if (index >= entryCount)
            continue;

        entry = &chapter->entries[index];

        if (!ArenaRosterEntryValid(entry))
            continue;

        PutUnitSpriteForClassId(0, 36, ARENA_ROSTER_CURSOR_Y + row * ARENA_ROSTER_ROW_HEIGHT, 0xD800, entry->jid);
    }

    RefreshUnitSprites();
    SyncUnitSpriteSheet();

    if ((proc->cursor >= proc->scrollTop) && (proc->cursor < proc->scrollTop + ARENA_ROSTER_VISIBLE_ROWS))
        ShowSysHandCursor(ARENA_ROSTER_CURSOR_X, ARENA_ROSTER_CURSOR_Y + (proc->cursor - proc->scrollTop) * ARENA_ROSTER_ROW_HEIGHT, 0x8, 0x800);
}

STATIC_DECLAR void ArenaRosterDrawList(struct ProcArenaRosterSelect *proc)
{
    const struct ArenaRosterChapter *chapter = ArenaRosterGetChapter(gPlaySt.chapterIndex);
    int entryCount;
    int row;

    if (!chapter)
        return;

    entryCount = ArenaRosterGetEntryCount(chapter);

    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, ARENA_ROSTER_LIST_X, ARENA_ROSTER_LIST_Y), ARENA_ROSTER_LIST_W, ARENA_ROSTER_LIST_H, 0);
    DrawUiFrame2(ARENA_ROSTER_LIST_X, ARENA_ROSTER_LIST_Y, ARENA_ROSTER_LIST_W, ARENA_ROSTER_LIST_H, 0);

    for (row = 0; row < ARENA_ROSTER_VISIBLE_ROWS; ++row) {
        int index = proc->scrollTop + row;
        char payoutLabel[14];
        struct Text *nameText = &gBanimText[row];
        struct Text *rewardText = &gBanimText[row + ARENA_ROSTER_VISIBLE_ROWS];

        ClearText(nameText);
        ClearText(rewardText);

        if (index >= entryCount)
            continue;

        {
            const struct ArenaRosterEntry *entry = &chapter->entries[index];
            int color = ArenaRosterEntryCleared(index) ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE;

            if (!ArenaRosterEntryValid(entry))
                continue;

            PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 3, 9 + row * 2), color, index + 1);
            PutDrawText(nameText, TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 9 + row * 2), color, 0, ARENA_ROSTER_TEXT_NAME_WIDTH, ArenaRosterGetDisplayName(entry));
            PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 17, 9 + row * 2), color, entry->price);

            if ((entry->rewardType == ARENA_ROSTER_REWARD_ITEM) && (entry->reward != ITEM_NONE)) {
                int icon = GetItemIconId(entry->reward);

                if (icon >= 0)
                    DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, ARENA_ROSTER_REWARD_ICON_X, 9 + row * 2), icon, 0x4000);

                PutDrawText(rewardText, TILEMAP_LOCATED(gBG0TilemapBuffer, ARENA_ROSTER_REWARD_TEXT_X, 9 + row * 2), color, 0, ARENA_ROSTER_TEXT_REWARD_WIDTH, GetItemName(entry->reward));
            } else {
                if (entry->rewardType == ARENA_ROSTER_REWARD_TRIPLE_PAYOUT) {
                    payoutLabel[0] = 'T';
                    payoutLabel[1] = 'r';
                    payoutLabel[2] = 'i';
                    payoutLabel[3] = 'p';
                    payoutLabel[4] = 'l';
                    payoutLabel[5] = 'e';
                } else {
                    payoutLabel[0] = 'D';
                    payoutLabel[1] = 'o';
                    payoutLabel[2] = 'u';
                    payoutLabel[3] = 'b';
                    payoutLabel[4] = 'l';
                    payoutLabel[5] = 'e';
                }

                payoutLabel[6] = ' ';
                payoutLabel[7] = 'P';
                payoutLabel[8] = 'a';
                payoutLabel[9] = 'y';
                payoutLabel[10] = 'o';
                payoutLabel[11] = 'u';
                payoutLabel[12] = 't';
                payoutLabel[13] = 0;

                PutDrawText(rewardText, TILEMAP_LOCATED(gBG0TilemapBuffer, ARENA_ROSTER_REWARD_TEXT_X - 2, 9 + row * 2), color, 0, ARENA_ROSTER_TEXT_BUFFER_WIDTH, payoutLabel);
            }
        }
    }

    ArenaRosterUpdateScrollBar(proc, entryCount);
    ArenaRosterDrawSpritesOnly(proc, chapter);
    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

STATIC_DECLAR void ArenaRosterCancelSelectionAndExit(struct ProcArenaRosterSelect *proc)
{
    sArenaRosterRuntimeState.choiceState = ARENA_ROSTER_CHOICE_CANCELLED;
    sArenaRosterRuntimeState.selectedIndex = -1;

    if (proc->proc_parent) {
        ArenaRosterClearSelection();
        Proc_Goto(proc->proc_parent, 2);
    }

    Proc_Break(proc);
}

STATIC_DECLAR void ArenaRosterApplySelection(const struct ArenaRosterEntry *entry)
{
    gArenaState.opponentUnit = &gArenaOpponent;
    gArenaState.opponentClassId = entry->jid;
    gArenaState.opponentWpnType = GetItemType(entry->item);
    gArenaState.opponentIsMagic = IsWeaponMagic(gArenaState.opponentWpnType);
    gArenaState.opponentLevel = entry->level;
    gArenaState.matchupGoldValue = entry->price;

    ArenaGenerateOpponentUnit();
    ArenaGenerateBaseWeapons();
    gArenaState.playerPowerWeight = ArenaGetPowerRanking(gArenaState.playerUnit, gArenaState.opponentIsMagic);
    gArenaState.opponentPowerWeight = ArenaGetPowerRanking(gArenaState.opponentUnit, gArenaState.playerIsMagic);
    ArenaSetResult(0);
    gArenaState.unk0B = 1;
    ArenaSetFallbackWeaponsMaybe();
}

STATIC_DECLAR void ArenaRosterSelector_Init(struct ProcArenaRosterSelect *proc)
{
    unsigned index;

    proc->cursor = 0;
    proc->scrollTop = 0;
    proc->promptShown = 0;
    SetTextFont(0);
    InitSystemTextFont();
    StartUiCursorHand(proc);
    ResetSysHandCursor(proc);
    DisplaySysHandCursorTextShadow(0x600, 1);
    ArenaRosterApplyEnemySpritePalettes();
    LoadIconPalettes(4);
    StartMenuScrollBar(proc);
    PutMenuScrollBarAt(ARENA_ROSTER_SCROLLBAR_X, ARENA_ROSTER_SCROLLBAR_Y);
    InitMenuScrollBarImg(0x7A60, 2);

    for (index = 0; index < (ARENA_ROSTER_VISIBLE_ROWS * 2) + 2; ++index)
        InitText(&gBanimText[index], ARENA_ROSTER_TEXT_BUFFER_WIDTH);

    ArenaRosterDrawList(proc);
}

STATIC_DECLAR void ArenaRosterSelector_End(struct ProcArenaRosterSelect *proc)
{
    (void) proc;

    EndMenuScrollBar();
    HideSysHandCursor();
}

STATIC_DECLAR void ArenaRosterSelector_Loop(struct ProcArenaRosterSelect *proc)
{
    const struct ArenaRosterChapter *chapter = ArenaRosterGetChapter(gPlaySt.chapterIndex);
    int entryCount;
    ProcPtr parent = proc->proc_parent ? proc->proc_parent : proc;
    bool redraw = false;

    if (!chapter) {
        ArenaRosterCancelSelectionAndExit(proc);
        return;
    }

    entryCount = ArenaRosterGetEntryCount(chapter);

    if (!proc->promptShown) {
        proc->promptShown = 1;
        return;
    }

    if (ArenaRosterIsDialogueOpen())
        return;

    if (gKeyStatusPtr->newKeys & B_BUTTON) {
        PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
        ArenaRosterCancelSelectionAndExit(proc);
        return;
    }

    if (gKeyStatusPtr->newKeys & DPAD_UP) {
        if (proc->cursor > 0) {
            proc->cursor--;

            if (proc->cursor < proc->scrollTop)
                proc->scrollTop = proc->cursor;

            redraw = true;
        }

        PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
    }

    if (gKeyStatusPtr->newKeys & DPAD_DOWN) {
        if (proc->cursor < entryCount - 1) {
            proc->cursor++;

            if (proc->cursor >= proc->scrollTop + ARENA_ROSTER_VISIBLE_ROWS)
                proc->scrollTop = proc->cursor - ARENA_ROSTER_VISIBLE_ROWS + 1;

            redraw = true;
        }

        PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
    }

    if (gKeyStatusPtr->newKeys & A_BUTTON) {
        const struct ArenaRosterEntry *entry = &chapter->entries[proc->cursor];

        if (!ArenaRosterEntryValid(entry) || ArenaRosterEntryCleared(proc->cursor)) {
            ArenaRosterClearSelection();
            PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
            ArenaRosterDrawList(proc);
            return;
        }

        sArenaRosterRuntimeState.choiceState = ARENA_ROSTER_CHOICE_SELECTED;
        sArenaRosterRuntimeState.selectedIndex = proc->cursor;
        ArenaRosterApplySelection(entry);

        if (gpKernelDesignerConfig->arena_show_opponent_in_advance == true) {
            DrawUiFrame2(7, 9, 0x10, 8, 0);
            DrawArenaOpponentDetailsText(parent);
        }

        SetTalkNumber(ArenaGetMatchupGoldValue() * ArenaRosterGetWagerMultiplier());
        StartArenaDialogue(0x8D2, parent);
        PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
        Proc_Goto(proc, 1);
        return;
    }

    if (redraw)
        ArenaRosterDrawList(proc);
    else
        ArenaRosterDrawSpritesOnly(proc, chapter);
}

STATIC_DECLAR void ArenaRosterSelector_WaitForWagerChoice(struct ProcArenaRosterSelect *proc)
{
    const struct ArenaRosterChapter *chapter = ArenaRosterGetChapter(gPlaySt.chapterIndex);

    if (!chapter) {
        ArenaRosterCancelSelectionAndExit(proc);
        return;
    }

    ArenaRosterDrawSpritesOnly(proc, chapter);

    if (ArenaRosterIsDialogueOpen())
        return;

    Proc_Break(proc);
}

bool ArenaRosterMenuEnabled(void)
{
    if (gpKernelDesignerConfig->arena_roster_menu != true)
        return false;

    if (UNIT_FACTION(gActiveUnit) != FACTION_BLUE)
        return false;

    return ArenaRosterGetChapter(gPlaySt.chapterIndex) != NULL;
}

bool ArenaRosterStartSelection(ProcPtr proc)
{
    const struct ArenaRosterChapter *chapter;

    if (!ArenaRosterMenuEnabled())
        return false;

    ArenaRosterEnsureSuspendStateCurrent();
    chapter = ArenaRosterGetChapter(gPlaySt.chapterIndex);

    if (ArenaRosterHasReachedWinLimit(chapter)) {
        StartArenaDialogue(MSG_ArenaCleanedOutString, proc);
        Proc_Goto(proc, 2);
        return true;
    }

    sArenaRosterRuntimeState.choiceState = ARENA_ROSTER_CHOICE_NONE;
    sArenaRosterRuntimeState.selectedIndex = -1;
    Proc_StartBlocking(ProcScr_ArenaRosterSelect, proc);
    return true;
}

bool ArenaRosterHandleConfirmation(ProcPtr proc)
{
    if (!ArenaRosterMenuEnabled())
        return false;

    if ((sArenaRosterRuntimeState.choiceState & 0x7F) != ARENA_ROSTER_CHOICE_CANCELLED)
        return false;

    ArenaRosterClearSelection();
    Proc_Goto(proc, 2);
    return true;
}

bool ArenaRosterHasConfiguredOpponent(void)
{
    return ArenaRosterGetSelectedEntry() != NULL;
}

void ArenaRosterFlushDeferredPopup(ProcPtr proc)
{
    int item;
    struct Unit *unit;

    item = sArenaRosterRuntimeState.pendingRewardItem;

    if (item == ITEM_NONE)
        return;

    sArenaRosterRuntimeState.pendingRewardItem = ITEM_NONE;

    unit = gArenaState.playerUnit;

    if (!UNIT_IS_VALID(unit))
        unit = gActiveUnit;

    const struct ArenaRosterEntry *entry = ArenaRosterGetSelectedEntry();

    ClearBg0Bg1();
    Proc_EndEach(gProcScr_GoldBox);
    Proc_EndEach(gProcScr_E_FACE);

    NewPopup_ItemGot(proc, unit, MakeNewItem(entry->reward));

    ArenaRosterClearSelection();
}

bool ArenaRosterHandleResultsDialogue(ProcPtr proc)
{
    const struct ArenaRosterEntry *entry = ArenaRosterGetSelectedEntry();
    int selectedIndex = sArenaRosterRuntimeState.selectedIndex;

    if (!entry)
        return false;

    switch (ArenaGetResult()) {
    case ARENA_WIN:
        if (entry->rewardType == ARENA_ROSTER_REWARD_ITEM) {
            ArenaRosterSetEntryCleared(selectedIndex);
            sArenaRosterRuntimeState.pendingRewardItem = MakeNewItem(entry->reward);
            StartArenaDialogue(MSG_ArenaItemPrizeString, proc);
            return true;
        }

        ArenaRosterSetEntryCleared(selectedIndex);

        SetTalkNumber(ArenaRosterGetRewardGold(entry));
        StartArenaDialogue(0x8D6, proc);
        ArenaRosterGrantWinReward(entry);
        break;

    case ARENA_LOSS:
        StartArenaDialogue(0x8D7, proc);
        break;

    case 3:
        StartArenaDialogue(0x8D9, proc);
        SetPartyGoldAmount(GetPartyGoldAmount() + ArenaGetMatchupGoldValue());
        break;

    case 4:
        StartArenaDialogue(0x8D8, proc);
        break;
    }

    ArenaRosterClearSelection();
    return true;
}

bool ArenaRosterGenerateOpponentUnit(struct Unit *unit)
{
    const struct ArenaRosterEntry *entry = ArenaRosterGetSelectedEntry();
    struct UnitDefinition udef;

    if (!entry)
        return false;

    CpuFill16(0, &udef, sizeof(udef));
    udef.charIndex = entry->pid ? entry->pid : CHARACTER_ARENA_OPPONENT;
    udef.classIndex = entry->jid;
    udef.level = entry->level;
    udef.autolevel = 1;
    udef.items[0] = entry->item;

    unit->index = 0x80;
    UnitInitFromDefinition(unit, &udef);
    UnitLoadStatsFromChracter(unit, unit->pCharacterData);
    unit->level = entry->level;
    UnitAutolevel(unit);

    SetUnitWeaponExp(unit, GetItemType(entry->item), 0xFB);

    UnitCheckStatCaps(unit);
    SetUnitHp(unit, GetUnitMaxHp(unit));
    return true;
}

u16 ArenaRosterGetSelectedWeapon(void)
{
    const struct ArenaRosterEntry *entry = ArenaRosterGetSelectedEntry();

    if (!entry)
        return ITEM_NONE;

    return entry->item;
}

void ArenaRosterClearSelection(void)
{
    sArenaRosterRuntimeState.choiceState = ARENA_ROSTER_CHOICE_NONE;
    sArenaRosterRuntimeState.selectedIndex = -1;
}

void ChapterInit_ResetArenaRosterState(ProcPtr proc)
{
    (void) proc;

    if (sArenaRosterRuntimeState.choiceState & ARENA_ROSTER_RUNTIME_FLAG_LOADED_FROM_SUSPEND) {
        sArenaRosterRuntimeState.choiceState = ARENA_ROSTER_CHOICE_NONE;
        ArenaRosterClearSelection();
        return;
    }

    sArenaRosterSuspendState.chapterId = gPlaySt.chapterIndex;
    ArenaRosterClearProgress();
    ArenaRosterClearSelection();
}

void SaveArenaRosterProgress(u8 *dst, const u32 size)
{
    if (size < sizeof(sArenaRosterSuspendState))
        return;

    ArenaRosterEnsureSuspendStateCurrent();
    WriteAndVerifySramFast(&sArenaRosterSuspendState, dst, sizeof(sArenaRosterSuspendState));
}

void LoadArenaRosterProgress(u8 *src, const u32 size)
{
    if (size < sizeof(sArenaRosterSuspendState))
        return;

    ReadSramFast(src, &sArenaRosterSuspendState, sizeof(sArenaRosterSuspendState));
    sArenaRosterRuntimeState.choiceState = ARENA_ROSTER_RUNTIME_FLAG_LOADED_FROM_SUSPEND;
    sArenaRosterRuntimeState.selectedIndex = -1;
    sArenaRosterRuntimeState.pendingRewardItem = ITEM_NONE;
    ArenaRosterEnsureSuspendStateCurrent();
}

u8 CheckArenaLimits(void)
{
    ProcPtr playerPhaseProc;
    int x;

    if (gpKernelDesignerConfig->arena_limits != true)
        return ARENA_OK;

    playerPhaseProc = Proc_Find(gProcScr_PlayerPhase);

    for (x = 0; ArenaLimitTable[x].ChapterID != 0xFF; x++) {
        if (ArenaLimitTable[x].ChapterID != gPlaySt.chapterIndex)
            continue;

        if (ArenaLimitTable[x].TurnLimit) {
            if (gPlaySt.chapterTurnNumber > ArenaLimitTable[x].TurnLimit) {
                gEventSlots[EVT_SLOT_7] = 1;
                KernelCallEvent(EventScr_ArenaClosed, EV_EXEC_CUTSCENE, playerPhaseProc);
                return CANT_ARENA;
            }
        }

        if (ArenaLimitTable[x].LevelLimit) {
            u8 temp_level = gActiveUnit->level;

            if (gActiveUnit->pClassData->attributes & CA_PROMOTED)
                temp_level += MaxLevel;

            if (temp_level > ArenaLimitTable[x].LevelLimit) {
                if (temp_level > MaxLevel && MaxLevel > 20)
                    gEventSlots[EVT_SLOT_7] = 3;
                else if (temp_level > MaxLevel)
                    gEventSlots[EVT_SLOT_7] = 2;
                else
                    return ARENA_OK;

                if (gEventSlots[EVT_SLOT_7] == 2)
                    KernelCallEvent(EventScr_ArenaLevelLimit, EV_EXEC_CUTSCENE, playerPhaseProc);
                else if (gEventSlots[EVT_SLOT_7] == 4)
                    KernelCallEvent(EventScr_UnpromotedOnly, EV_EXEC_CUTSCENE, playerPhaseProc);

                return CANT_ARENA;
            }
        }

        return ARENA_OK;
    }

    return ARENA_OK;
}

void KillUnitIfNoHealth(struct Unit *unit)
{
    if (GetUnitCurrentHp(unit) == 0)
        if (gArenaState.result == ARENA_LOSS && NonLethalArena)
            unit->curHP = 1;
}

// LYN_REPLACE_CHECK(DidUnitDie);
// bool DidUnitDie(struct Unit *unit)
// {
//     if (GetUnitCurrentHp(unit) == 0) {
//         if (gArenaState.result == ARENA_LOSS && NonLethalArena) {
//             unit->curHP = 1;
//             return TRUE;
//         }

//         return FALSE;
//     }

//     return TRUE;
// }


//! FE8U = 0x080327B4
LYN_REPLACE_CHECK(DidUnitDie);
bool DidUnitDie(struct Unit* unit) {
    if (GetUnitCurrentHp(unit) != 0) {
        return false;
    }

    return true;
}

//! FE8U = 0x080B5998
LYN_REPLACE_CHECK(ArenaUi_WelcomeDialogue);
void ArenaUi_WelcomeDialogue(ProcPtr proc) 
{
    if (gpKernelDesignerConfig->arena_roster_menu == true)
    {
        StartArenaDialogue(MSG_ArenaPickOpponentString, proc);
        return;
    }

    if (UNIT_ARENA_LEVEL(gArenaState.playerUnit) < 5) {
        StartArenaDialogue(0x8d0, proc);
        // TODO: msgid "Welcome to the arena![.][A]"
    } else {
        StartArenaDialogue(0x8D1, proc);
        // TODO: msgid "Welcome to the arena.[.][A][NL]Oh! It's you again.[.][A][NL2][NL]I've lost a lot of gold[.][NL]thanks to you...[A][NL2][NL]If you want to continue,[A][NL]we're going to have to[NL]do things differently.[A][NL2][NL]I'm going to prepare some[.][NL]more challenging foes.[A]"
    }

    return;
}

//! FE8U = 0x080B576C
LYN_REPLACE_CHECK(StartArenaScreen);
void StartArenaScreen(void) {
    RealtimeBattle_QuiesceInFlight();
    RealtimeBattle_Pause(RT_PAUSE_ARENA);
    ArenaBegin(gActiveUnit);
    if (gpKernelDesignerConfig->arena_roster_menu == true)
        Proc_Start(gProcScr_ArenaUiMain_NEW, PROC_TREE_3);
    else
        Proc_Start(gProcScr_ArenaUiMain, PROC_TREE_3);
    return;
}

//! FE8U = 0x080B5C48
LYN_REPLACE_CHECK(DrawArenaOpponentDetailsText);
void DrawArenaOpponentDetailsText(ProcPtr proc) {

    if (gpKernelDesignerConfig->arena_roster_menu == true)
    {
        ArenaUi_ClearRosterUi();
        return;
    }

    DrawUiFrame2(7, 9, 0x10, 6, 0);
    SetTextFont(0);
    InitSystemTextFont();

    PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 8, 10), 0, GetStringFromIndex(gMid_Lv));
    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 12, 10), 2, gArenaState.opponentUnit->level);
    PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 8, 12), 0, GetStringFromIndex(gArenaState.opponentUnit->pCharacterData->nameTextId));
    PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 15, 10), 0, GetStringFromIndex(gArenaState.opponentUnit->pClassData->nameTextId));
    PutString(TILEMAP_LOCATED(gBG0TilemapBuffer, 15, 12), 0, GetItemName(gArenaState.opponentWeapon));

    return;
}

FORCE_DECLARE static const struct ProcCmd gProcScr_ArenaUiMain_NEW[] = {
    PROC_CALL(LockGame),

    PROC_SLEEP(1),
    PROC_CALL_ARG(_FadeBgmOut, 65535),
    PROC_CALL(StartMidFadeToBlack),
    PROC_REPEAT(WaitForFade),

    PROC_CALL(BMapDispSuspend),

    PROC_CALL_ARG(_StartBgm, 56),

    PROC_CALL(ArenaUi_Init),
    PROC_CALL(FadeInBlackSpeed20),
    PROC_SLEEP(1),

    PROC_CALL(ArenaUi_WelcomeDialogue),
    PROC_SLEEP(1),

    PROC_CALL(ArenaUi_WagerGoldDialogue),
    PROC_SLEEP(1),

    PROC_CALL(ArenaUi_CheckConfirmation),
    PROC_SLEEP(1),

    PROC_CALL(ArenaUi_ConfirmWager),
    PROC_SLEEP(1),

    PROC_CALL(ArenaUi_InstructionsDialogue),
    PROC_SLEEP(1),

    PROC_CALL(ArenaUi_GoodLuckDialogue),
    PROC_SLEEP(1),

PROC_LABEL(0),
    PROC_CALL_ARG(_FadeBgmOut, 2),
    PROC_CALL(sub_8013F40),
    PROC_SLEEP(1),

    PROC_CALL(ArenaUi_StartArenaBattle),
    PROC_SLEEP(1),

    PROC_CALL(UnlockGame),
    PROC_CALL(BMapDispResume),

    PROC_JUMP(gProcScr_ArenaUiResults_NEW),

PROC_LABEL(2),
    PROC_SLEEP(1),

    PROC_CALL(sub_8013F40),
    PROC_SLEEP(1),

    PROC_CALL(ArenaUi_OnEnd),

    PROC_CALL(ResetDialogueScreen),

    PROC_CALL(BMapDispResume),
    PROC_CALL(RefreshBMapGraphics),
    PROC_CALL(StartMapSongBgm),

    PROC_CALL(StartMidFadeFromBlack),
    PROC_REPEAT(WaitForFade),

    PROC_CALL(UnlockGame),

    PROC_END,
};

FORCE_DECLARE static const struct ProcCmd gProcScr_ArenaUiResults_NEW[] = {
PROC_LABEL(1),
    PROC_CALL(sub_80B5B00),

    PROC_CALL(LockGame),
    PROC_CALL(BMapDispSuspend),
    PROC_SLEEP(0),

    PROC_START_CHILD(gProcScr_ArenaUiResultBgm),

    PROC_CALL(ArenaUi_Init),

    PROC_CALL(FadeInBlackSpeed20),
    PROC_SLEEP(0),

    PROC_CALL(ArenaUi_ResultsDialogue),
    PROC_WHILE(ArenaRosterIsDialogueOpen),
    PROC_SLEEP(0),

    PROC_CALL(ArenaUi_ShowGoldBoxOnVictoryOrDraw),
    PROC_CALL(ArenaRosterFlushDeferredPopup),
    PROC_WHILE(ArenaRosterPopup2Exists),
    PROC_SLEEP(0),

PROC_LABEL(2),
    PROC_SLEEP(1),

    PROC_END_EACH(gProcScr_ArenaUiResultBgm),
    PROC_SLEEP(0),

    PROC_CALL_ARG(_FadeBgmOut, 2),
    PROC_CALL(sub_8013F40),
    PROC_SLEEP(0),

    PROC_CALL(sub_80B5970),

    PROC_CALL(ArenaUi_OnEnd),

    PROC_CALL(ResetDialogueScreen),

    PROC_CALL(BMapDispResume),
    PROC_CALL(RefreshBMapGraphics),
    PROC_CALL(StartMapSongBgm),

    PROC_CALL(StartMidFadeFromBlack),
    PROC_REPEAT(WaitForFade),

    PROC_CALL(UnlockGame),

    PROC_END,
};

struct ProcCmd gProcScr_ArenaUiResultBgm[] = {
    PROC_CALL(Arena_PlayResultSong),
    PROC_SLEEP(210),

    PROC_CALL(Arena_PlayArenaSong),

    PROC_END,
};