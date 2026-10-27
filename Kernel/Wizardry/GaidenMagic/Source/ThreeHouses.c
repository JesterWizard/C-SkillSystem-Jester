#include <common-chax.h>
#include <battle-system.h>
#include <skill-system.h>
#include <kernel-lib.h>
#include <constants/skills.h>

#include <gaiden-magic.h>

/**
 * Three Houses-style magic
 *
 * Each spell gets a number of uses per chapter. A combat spends one use
 * no matter how many times the caster hits (follow-ups and counters
 * included), and a W.Mag staff spends one use per cast.
 */

/* unit->index is s8, so uid is taken as u8 to match enemy/NPC indexes (0x80+) */
STATIC_DECLAR struct GaidenMagicUsesEnt *FindGaidenMagicUsesEnt(u8 uid, u8 iid)
{
	int i;

	for (i = 0; i < GAIDEN_MAGIC_USES_POOL_LEN; i++) {
		struct GaidenMagicUsesEnt *ent = &sGaidenMagicUsesPool[i];

		if (ent->iid == iid && ent->uid == uid)
			return ent;
	}
	return NULL;
}

STATIC_DECLAR bool IsGaidenMagicUsesEntStale(struct GaidenMagicUsesEnt *ent)
{
	struct Unit *unit = GetUnit(ent->uid);

	if (!UNIT_IS_VALID(unit))
		return true;

	return (unit->state & US_UNAVAILABLE) != 0;
}

STATIC_DECLAR struct GaidenMagicUsesEnt *AllocGaidenMagicUsesEnt(u8 uid, u8 iid)
{
	int i;
	struct GaidenMagicUsesEnt *ent = NULL;

	for (i = 0; i < GAIDEN_MAGIC_USES_POOL_LEN; i++) {
		if (sGaidenMagicUsesPool[i].iid == ITEM_NONE) {
			ent = &sGaidenMagicUsesPool[i];
			break;
		}
	}

	/* Pool is full, reclaim an entry of a unit that left the map */
	if (!ent) {
		for (i = 0; i < GAIDEN_MAGIC_USES_POOL_LEN; i++) {
			if (IsGaidenMagicUsesEntStale(&sGaidenMagicUsesPool[i])) {
				ent = &sGaidenMagicUsesPool[i];
				break;
			}
		}
	}

	if (!ent) {
		Errorf("Gaiden magic uses pool full: uid=%#x, iid=%#x", uid, iid);
		return NULL;
	}

	ent->uid = uid;
	ent->iid = iid;
	ent->spent = 0;
	return ent;
}

STATIC_DECLAR void RemoveGaidenMagicUsesOfUnit(struct Unit *unit)
{
	int i;

	/* Skip battle-unit copies and temporary units */
	if (!unit || GetUnit(unit->index) != unit)
		return;

	for (i = 0; i < GAIDEN_MAGIC_USES_POOL_LEN; i++) {
		struct GaidenMagicUsesEnt *ent = &sGaidenMagicUsesPool[i];

		if (ent->iid != ITEM_NONE && ent->uid == (u8)unit->index)
			memset(ent, 0, sizeof(*ent));
	}
}

int GetGaidenMagicMaxUses(struct Unit *unit, int item)
{
	int uses = gGaidenMagicUsesList[ITEM_INDEX(item)];
	int bonus;

#if (defined(SID_GrandMagician) && COMMON_SKILL_VALID(SID_GrandMagician))
	if (SkillTester(unit, SID_GrandMagician))
		return GAIDEN_MAGIC_USES_INFINITE;
#endif

	if (uses == 0)
		uses = GetItemMaxUses(item);

	if (uses == 0 || uses >= GAIDEN_MAGIC_USES_INFINITE)
		return GAIDEN_MAGIC_USES_INFINITE;

	bonus = gGaidenMagicClassUsesBonus[UNIT_CLASS_ID(unit)];

	if (GetItemType(item) == ITYPE_STAFF) {
		if (bonus & GAIDEN_MAGIC_USES_WMAG_X2)
			uses *= 2;
	} else {
		if (bonus & GAIDEN_MAGIC_USES_BMAG_X2)
			uses *= 2;
	}

	if (uses >= GAIDEN_MAGIC_USES_INFINITE)
		uses = GAIDEN_MAGIC_USES_INFINITE - 1;

	return uses;
}

int GetGaidenMagicUses(struct Unit *unit, int item)
{
	struct GaidenMagicUsesEnt *ent;
	int max = GetGaidenMagicMaxUses(unit, item);

	if (max == GAIDEN_MAGIC_USES_INFINITE)
		return GAIDEN_MAGIC_USES_INFINITE;

	ent = FindGaidenMagicUsesEnt(unit->index, ITEM_INDEX(item));
	if (!ent)
		return max;

	if (ent->spent >= max)
		return 0;

	return max - ent->spent;
}

bool HasGaidenMagicUses(struct Unit *unit, int item)
{
	if (!IsGaidenMagicUsesSystem())
		return true;

	/* Group Cast spends two uses */
	if (IsGroupCastActive(unit))
		return GetGaidenMagicUses(unit, item) >= 2;

	return GetGaidenMagicUses(unit, item) != 0;
}

void ConsumeGaidenMagicUse(struct Unit *unit, int item)
{
	struct GaidenMagicUsesEnt *ent;
	int iid = ITEM_INDEX(item);

	if (!IsGaidenMagicUsesSystem())
		return;

	if (!unit || unit->index == 0 || iid == ITEM_NONE)
		return;

	if (GetGaidenMagicMaxUses(unit, item) == GAIDEN_MAGIC_USES_INFINITE)
		return;

	ent = FindGaidenMagicUsesEnt(unit->index, iid);
	if (!ent)
		ent = AllocGaidenMagicUsesEnt(unit->index, iid);

	if (ent && ent->spent < 0xFF)
		ent->spent++;
}

void ResetGaidenMagicUses(void)
{
	memset(sGaidenMagicUsesPool, 0, sizeof(sGaidenMagicUsesPool));
}

/**
 * Hooks
 */
void GaidenMagicUses_OnClearUnit(struct Unit *unit)
{
	RemoveGaidenMagicUsesOfUnit(unit);
}

void GaidenMagicUses_OnUnitKill(struct Unit *unit)
{
	RemoveGaidenMagicUsesOfUnit(unit);
}

int GaidenMagicUses_OnBattleToUnit(struct BattleUnit *bu, struct Unit *unit)
{
	int item;

	if (!IsGaidenMagicUsesSystem())
		return 0;

	/* Set in BattleGenerateHit() when this side cast its first gaiden spell */
	if (!GetBattleGlobalFlags(bu)->gaiden_magic_cast)
		return 0;

	if (!CheckGaidenMagicAttack(bu))
		return 0;

	item = bu->weaponBefore ? bu->weaponBefore : bu->weapon;

	/* Staves spend their use in ActionGaidenMagicStaff() */
	if (!(GetItemAttributes(item) & IA_WEAPON))
		return 0;

	ConsumeGaidenMagicUse(unit, item);
	return 0;
}

/**
 * Save data: the pool only lives for one chapter, so a game save
 * keeps nothing and loading one starts from full uses.
 */
void MSU_SaveGaidenMagicUses(u8 *dst, const u32 size)
{
	WriteAndVerifySramFast(sGaidenMagicUsesPool, dst, sizeof(sGaidenMagicUsesPool));
}

void MSU_LoadGaidenMagicUses(u8 *src, const u32 size)
{
	ReadSramFast(src, sGaidenMagicUsesPool, sizeof(sGaidenMagicUsesPool));
}

void MSA_SaveGaidenMagicUses(u8 *dst, const u32 size) {}

void MSA_LoadGaidenMagicUses(u8 *src, const u32 size)
{
	ResetGaidenMagicUses();
}

/**
 * Tomes and staves as spells
 */
bool IsThreeHousesMagicItem(int item)
{
	if (ITEM_INDEX(item) == ITEM_NONE)
		return false;

	switch (GetItemType(item)) {
	case ITYPE_ANIMA:
	case ITYPE_LIGHT:
	case ITYPE_DARK:
		/* Monster weapons (Evil Eye, Stone...) are dark-typed but have no rank: they stay weapons */
		return (GetItemAttributes(item) & IA_WEAPON) && GetItemRequiredExp(item) > 0;

	case ITYPE_STAFF:
		return true;

	default:
		return false;
	}
}

const struct GaidenMagicRankEnt *GetGaidenMagicRankList(struct Unit *unit)
{
	const struct GaidenMagicRankEnt *ent = gGaidenMagicRankPList[UNIT_CHAR_ID(unit)].ent;

	return ent ? ent : gGaidenMagicRankDefaultList;
}

void RemoveThreeHousesMagicItems(struct Unit *unit)
{
	int i;

	if (!IsGaidenMagicUsesSystem() || !UNIT_IS_VALID(unit))
		return;

	if (UNIT_FACTION(unit) != FACTION_BLUE)
		return;

	for (i = UNIT_ITEM_COUNT - 1; i >= 0; i--)
		if (IsThreeHousesMagicItem(unit->items[i]))
			UnitRemoveItem(unit, i);
}

STATIC_DECLAR void RemoveThreeHousesMagicItemsFromConvoy(void)
{
	extern const int sExpaConvoyItemAmount;
	int i;
	bool removed = false;
	u16 *convoy = GetConvoyItemArray();

	for (i = 0; i < sExpaConvoyItemAmount; i++) {
		if (IsThreeHousesMagicItem(convoy[i])) {
			convoy[i] = 0;
			removed = true;
		}
	}

	if (removed)
		ShrinkConvoyItemList();
}

/* Catches tomes from recruits, events and old saves */
void ChapterInit_RemoveThreeHousesMagicItems(void)
{
	int i;

	if (!IsGaidenMagicUsesSystem())
		return;

	for (i = FACTION_BLUE + 1; i < FACTION_GREEN; i++)
		RemoveThreeHousesMagicItems(GetUnit(i));

	RemoveThreeHousesMagicItemsFromConvoy();
}

bool PrePhase_RemoveThreeHousesMagicItems(ProcPtr proc)
{
	ChapterInit_RemoveThreeHousesMagicItems();
	return false;
}

/**
 * Learning spells on rank-up
 */
#define GAIDEN_MAGIC_LEARN_POPUP_MAX 4

extern struct {
	u8 cnt;
	u8 uids[GAIDEN_MAGIC_LEARN_POPUP_MAX];
	u8 iids[GAIDEN_MAGIC_LEARN_POPUP_MAX];
} sGaidenMagicLearnPopups;

void ResetGaidenMagicLearnPopups(void)
{
	memset(&sGaidenMagicLearnPopups, 0, sizeof(sGaidenMagicLearnPopups));
}

void TryLearnGaidenMagicFromBattleUnit(struct BattleUnit *bu)
{
	struct Unit *unit;
	const struct GaidenMagicRankEnt *ent;
	int newExp, oldLv, newLv;

	if (!IsGaidenMagicUsesSystem() || !bu)
		return;

	unit = GetUnit(bu->unit.index);
	if (!UNIT_IS_VALID(unit) || UNIT_FACTION(unit) != FACTION_BLUE)
		return;

	newExp = GetBattleUnitUpdatedWeaponExp(bu);
	if (newExp <= 0)
		return;

	oldLv = GetWeaponLevelFromExp(UNIT_WRANK(&bu->unit, bu->weaponType));
	newLv = GetWeaponLevelFromExp(newExp);
	if (newLv <= oldLv)
		return;

	for (ent = GetGaidenMagicRankList(unit); ent->iid != ITEM_NONE; ent++) {
		if (GetItemType(ent->iid) != bu->weaponType)
			continue;

		if (ent->wrank <= oldLv || ent->wrank > newLv)
			continue;

		if (sGaidenMagicLearnPopups.cnt >= GAIDEN_MAGIC_LEARN_POPUP_MAX)
			break;

		sGaidenMagicLearnPopups.uids[sGaidenMagicLearnPopups.cnt] = bu->unit.index;
		sGaidenMagicLearnPopups.iids[sGaidenMagicLearnPopups.cnt] = ent->iid;
		sGaidenMagicLearnPopups.cnt++;
	}
}

bool PopR_SetupLearnGaidenMagic(void)
{
	int i;

	if (sGaidenMagicLearnPopups.cnt == 0)
		return false;

	i = --sGaidenMagicLearnPopups.cnt;

	SetPopupItem(sGaidenMagicLearnPopups.iids[i]);
	SetPopupUnit(GetUnit(sGaidenMagicLearnPopups.uids[i]));
	return true;
}
