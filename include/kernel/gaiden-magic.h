#pragma once

#include <common-chax.h>
#include <kernel-lib.h>
#include <list-verify.h>
#include <battle-system.h>

struct GaidenPinfoConfigEnt { u8 level, iid; };
struct GaidenPinfoConfig { const struct GaidenPinfoConfigEnt *ent; };

// extern struct GaidenPinfoConfig const gGaidenPinfoConfigList[0x100];
// extern struct GaidenPinfoConfig const *const gpGaidenPinfoConfigList;
extern struct GaidenPinfoConfig const *const *const pr_gpGaidenPinfoConfigList;
static inline const struct GaidenPinfoConfig *GetGaidenPinfoConfigList()
{
	return *pr_gpGaidenPinfoConfigList;
}

// extern const u8 gGaidenWeaponCostList[0x100];
// extern u8 const *const gpGaidenWeaponCostList;
extern u8 const *const *const pr_gpGaidenWeaponCostList;
static inline const u8 *GetGaidenWeaponCostList()
{
	return *pr_gpGaidenWeaponCostList;
}

struct GaidenChaxConfigEnt {
	u8 iid;
	u8 pid, jid, level;
	u8 faction;

	u16 skill, evflag;
};

// extern const struct GaidenChaxConfigEnt gGaidenChaxConfigs[];
extern struct GaidenChaxConfigEnt const *const gpGaidenChaxConfigs;

#define GAIDEN_MAGIC_LIST_LEN 7

struct GaidenMagicList {
	struct UnitListHeader header;

	u8 bmag_cnt;
	u8 bmags[GAIDEN_MAGIC_LIST_LEN];

	u8 wmag_cnt;
	u8 wmags[GAIDEN_MAGIC_LIST_LEN];

	/* Anima/Light/Dark/Staff WEXP the list was built with (Three Houses mode) */
	u8 ranks[4];
};

extern struct GaidenMagicList sGaidenMagicListObj;
extern struct GaidenMagicList *const gGaidenMagicList;

void ResetGaidenMagicList(struct GaidenMagicList *list);
void UpdateGaidenMagicList(struct Unit *unit, struct GaidenMagicList *list);
struct GaidenMagicList *GetGaidenMagicList(struct Unit *unit);

bool CanUnitUseGaidenMagic(struct Unit *unit, int item);
bool CanUnitUseGaidenMagicNow(struct Unit *unit, int item);

void BattleGenerateGaidenBMagSimulation(struct Unit *actor, struct Unit *target, int x, int y);
void BattleGenerateGaidenBMagReal(struct Unit *actor, struct Unit *target);

static inline bool CheckGaidenMagicAttack(struct BattleUnit *bu)
{
	switch (bu->weaponSlotIndex) {
	case CHAX_BUISLOT_GAIDEN_BMAG1:
	case CHAX_BUISLOT_GAIDEN_BMAG2:
	case CHAX_BUISLOT_GAIDEN_BMAG3:
	case CHAX_BUISLOT_GAIDEN_BMAG4:
	case CHAX_BUISLOT_GAIDEN_BMAG5:
	case CHAX_BUISLOT_GAIDEN_BMAG6:
	case CHAX_BUISLOT_GAIDEN_BMAG7:
	case CHAX_BUISLOT_GAIDEN_WMAG1:
	case CHAX_BUISLOT_GAIDEN_WMAG2:
	case CHAX_BUISLOT_GAIDEN_WMAG3:
	case CHAX_BUISLOT_GAIDEN_WMAG4:
	case CHAX_BUISLOT_GAIDEN_WMAG5:
	case CHAX_BUISLOT_GAIDEN_WMAG6:
	case CHAX_BUISLOT_GAIDEN_WMAG7:
		return true;

	default:
		return false;
	}
}

/**
 * Returns true if this is the first time this attacker is attacking in the
 * current battle. Follow-up attacks (speed doubles, brave second hit, etc.)
 * return false. The distinction is made by checking whether any previous
 * battle hit was made by the same side (actor vs. retaliator).
 */
static inline bool IsFirstGaidenMagicAttackInBattle(struct BattleUnit *attacker)
{
	bool isRetaliation = (attacker == &gBattleTarget);
	int currentRound = GetCurrentBattleHitRound();

	for (int i = 0; i < currentRound; i++) {
		bool prevIsRetaliation = (gBattleHitArrayRe[i].info & BATTLE_HIT_INFO_RETALIATION) != 0;
		if (prevIsRetaliation == isRetaliation)
			return false;
	}
	return true;
}

int GetGaidenWeaponHpCost(struct Unit *unit, int item);
void BattleGenerateHitHpCostForGaidenMagic(struct BattleUnit *attacker, struct BattleUnit *defender);
int GetGaidenMagicAutoEquipSlot(struct Unit *unit);
int GetGaidenMagicAutoEquipStaff(struct Unit *unit);
int GetGaidenMagicItem(struct Unit *unit, int slot);
void TryChangeGaidenMagicAction(void);

void PutGaidenMagicCostNumber(u16 *tm, int color, int number);
void PutGaidenMagicMenuNumber(u16 *tm, int color, struct Unit *unit, int item);
void DrawGaidenMagItemMenuLine(struct Text *text, int item, s8 isUsable, u16 *mapOut);

u8 GaidenBMagActionCommandUsability(const struct MenuItemDef *def, int number);
int GaidenBMagActionCommandOnDarw(struct MenuProc *menu, struct MenuItemProc *item);
u8 GaidenBMagActionCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem);
int GaidenBMagActionCommandHover(struct MenuProc *menu, struct MenuItemProc *menuItem);
int GaidenBMagActionCommandUnhover(struct MenuProc *menu, struct MenuItemProc *menuItem);

u8 GaidenWMagActionCommandUsability(const struct MenuItemDef *def, int number);
int GaidenWMagActionCommandOnDarw(struct MenuProc *menu, struct MenuItemProc *item);
u8 GaidenWMagActionCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem);
int GaidenWMagActionCommandHover(struct MenuProc *menu, struct MenuItemProc *menuItem);

extern void TryAddGaidenMagicPConf(struct Unit * unit, int level);

/**
 * Three Houses-style magic (KernelDesigerConfig::gaiden_magic_three_houses)
 *
 * Spells have a limited number of uses per chapter instead of an HP/MP cost.
 * Uses refill at the start of every chapter. Uses spent are tracked per
 * (unit index, item index) in a small pool that is saved in suspend data.
 */
#define GAIDEN_MAGIC_USES_INFINITE 0xFF
#define GAIDEN_MAGIC_USES_POOL_LEN 64

enum gaiden_magic_class_uses_bonus {
	GAIDEN_MAGIC_USES_BMAG_X2 = 1 << 0,
	GAIDEN_MAGIC_USES_WMAG_X2 = 1 << 1,
};

/* Base uses per item, 0 = fall back to the item's max uses */
extern const u8 gGaidenMagicUsesList[0x100];

/* Per-class uses multiplier, see enum gaiden_magic_class_uses_bonus */
extern const u8 gGaidenMagicClassUsesBonus[0x100];

struct GaidenMagicUsesEnt {
	u8 uid, iid, spent;
};

extern struct GaidenMagicUsesEnt sGaidenMagicUsesPool[GAIDEN_MAGIC_USES_POOL_LEN];

static inline bool IsGaidenMagicUsesSystem(void)
{
	return gpKernelDesignerConfig->gaiden_magic && gpKernelDesignerConfig->gaiden_magic_three_houses;
}

int GetGaidenMagicMaxUses(struct Unit *unit, int item);
int GetGaidenMagicUses(struct Unit *unit, int item);
bool HasGaidenMagicUses(struct Unit *unit, int item);
void ConsumeGaidenMagicUse(struct Unit *unit, int item);
void ResetGaidenMagicUses(void);

/**
 * In Three Houses mode, tomes and staves are not items: player units learn
 * them as spells from their weapon ranks, and other units cast the ones in
 * their inventory as spells.
 */
struct GaidenMagicRankEnt {
	u8 wrank; /* WPN_LEVEL_*, checked against the rank of the spell's own type */
	u8 iid;
};

struct GaidenMagicRankConfig {
	const struct GaidenMagicRankEnt *ent;
};

/* Personal spell lists, indexed by character */
extern const struct GaidenMagicRankConfig gGaidenMagicRankPList[0x100];

/* Spell list for player units without a personal list */
extern const struct GaidenMagicRankEnt gGaidenMagicRankDefaultList[];

bool IsThreeHousesMagicItem(int item);
const struct GaidenMagicRankEnt *GetGaidenMagicRankList(struct Unit *unit);
void RemoveThreeHousesMagicItems(struct Unit *unit);
void TryLearnGaidenMagicFromBattleUnit(struct BattleUnit *bu);
void ResetGaidenMagicLearnPopups(void);
bool PopR_SetupLearnGaidenMagic(void);
extern const struct PopupInstruction PopupScr_GaidenLearn[];

static inline bool IsGaidenMagicAiEnabled(void)
{
	return gpKernelDesignerConfig->gaiden_magic && (gpKernelDesignerConfig->gaiden_magic_ai_use || IsGaidenMagicUsesSystem());
}