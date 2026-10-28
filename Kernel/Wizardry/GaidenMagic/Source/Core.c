#include <common-chax.h>
#include <battle-system.h>
#include <skill-system.h>
#include <kernel-lib.h>
#include <bwl.h>
#include <list-verify.h>
#include <gaiden-magic.h>
#include "jester_headers/custom-functions.h"
#include "constants/texts.h"

struct GaidenMagicList *const gGaidenMagicList = &sGaidenMagicListObj;

NOINLINE static bool IsBMag(int iid)
{
	u32 attr;

	if (IsGaidenMapSpell(iid))
		return true;

	attr = GetItemAttributes(iid);

	if (!(attr & IA_WEAPON))
		return false;

	if (gpKernelDesignerConfig->gaiden_magic_must_be_magic)
		if (!(attr & IA_MAGIC))
			return false;

	return true;
}

NOINLINE static bool IsWMag(int iid)
{
	if (GetItemType(iid) != ITYPE_STAFF)
		return false;

	return true;
}

void ResetGaidenMagicList(struct GaidenMagicList *list)
{
	memset(list, 0, sizeof(*list));
}

static const u8 sGaidenMagicRankTypes[4] = { ITYPE_ANIMA, ITYPE_LIGHT, ITYPE_DARK, ITYPE_STAFF };

/* The Three Houses spell list depends on weapon ranks, which the list header does not track */
STATIC_DECLAR void GetGaidenMagicListRanks(struct Unit *unit, u8 *ranks)
{
	int i;

	for (i = 0; i < 4; i++)
		ranks[i] = 0;

	if (!IsGaidenMagicUsesSystem() || UNIT_FACTION(unit) != FACTION_BLUE)
		return;

	for (i = 0; i < 4; i++)
		ranks[i] = UNIT_WRANK(unit, sGaidenMagicRankTypes[i]);
}

STATIC_DECLAR void AddThreeHousesGaidenMagic(struct Unit *unit, u8 *tmpbuf)
{
	int i;
	const struct GaidenMagicRankEnt *ent;

	/* Enemies and NPCs cast the tomes and staves they carry */
	if (UNIT_FACTION(unit) != FACTION_BLUE) {
		for (i = 0; i < UNIT_ITEM_COUNT; i++) {
			int item = unit->items[i];

			if (item == ITEM_NONE)
				break;

			if (IsThreeHousesMagicItem(item))
				tmpbuf[ITEM_INDEX(item)] = 1;
		}
		return;
	}

	/* Player units learn spells from the rank of each spell's weapon type */
	for (ent = GetGaidenMagicRankList(unit); ent && ent->iid != ITEM_NONE; ent++) {
		int wexp = UNIT_WRANK(unit, GetItemType(ent->iid));

		if (GetWeaponLevelFromExp(wexp) >= ent->wrank && wexp > 0)
			tmpbuf[ent->iid] = 1;
	}
}
void UpdateGaidenMagicList(struct Unit *unit, struct GaidenMagicList *list)
{
	int i;
	const struct GaidenPinfoConfigEnt *conf1 = GetGaidenPinfoConfigList()[UNIT_CHAR_ID(unit)].ent;
	const struct GaidenChaxConfigEnt  *conf2 = gpGaidenChaxConfigs;

	u8 tmpbuf[0x100];
	int effectiveLevel = unit->level;

	CpuFastFill16(0, tmpbuf, 0x100);

	/* +20 levels for promoted units */
	if (UNIT_CATTRIBUTES(unit) & CA_PROMOTED)
		effectiveLevel += 20;

	if (conf1) {
		for (; conf1->iid != ITEM_NONE; conf1++) {
			if ((conf1->level & 0x80) && !(UNIT_CATTRIBUTES(unit) & CA_PROMOTED))
				continue;

			if (effectiveLevel < (conf1->level & 0x7F))
				continue;

			tmpbuf[conf1->iid] = 1;
		}
	}

	if (gpKernelDesignerConfig->gaiden_magic_skill_extensions) {
		for (; conf2->iid != ITEM_NONE; conf2++) {
			if (conf2->faction != UNIT_FACTION(unit))
				continue;

			if (conf2->pid != 0 && UNIT_CHAR_ID(unit) != conf2->pid)
				continue;

			if (conf2->jid != 0 && UNIT_CLASS_ID(unit) != conf2->jid)
				continue;

			if (effectiveLevel < conf2->level)
				continue;

			if (COMMON_SKILL_VALID(conf2->skill) && !SkillTester(unit, conf2->skill))
				continue;

			if (conf2->evflag != 0 && !CheckFlag(conf2->evflag))
				continue;

			tmpbuf[conf2->iid] = 1;
		}
	}

	if (IsGaidenMagicUsesSystem())
		AddThreeHousesGaidenMagic(unit, tmpbuf);

	ResetGaidenMagicList(list);
	GetGaidenMagicListRanks(unit, list->ranks);

	for (i = 0xFF; i > 0; i--) {
		if (list->bmag_cnt >= GAIDEN_MAGIC_LIST_LEN)
			break;

		if (tmpbuf[i] && IsBMag(i))
			list->bmags[list->bmag_cnt++] = i;
	}

	for (i = 0xFF; i > 0; i--) {
		if (list->wmag_cnt >= GAIDEN_MAGIC_LIST_LEN)
			break;

		if (tmpbuf[i] && IsWMag(i))
			list->wmags[list->wmag_cnt++] = i;
	}

	WriteUnitList(unit, &list->header);
}

struct GaidenMagicList *GetGaidenMagicList(struct Unit *unit)
{
	struct GaidenMagicList *list = gGaidenMagicList;
	u8 ranks[4];

	GetGaidenMagicListRanks(unit, ranks);

	if (!JudgeUnitList(unit, &list->header) || memcmp(ranks, list->ranks, sizeof(ranks)) != 0) {
		UpdateGaidenMagicList(unit, list);
		WriteUnitList(unit, &list->header);
	}
	return list;
}

bool CanUnitUseGaidenMagic(struct Unit *unit, int item)
{
	/* Three Houses spells are learned from ranks, and tomes can't be wielded as items */
	if (IsGaidenMagicUsesSystem())
		return true;

	if (gpKernelDesignerConfig->gaiden_magic_requires_wrank) {
		if (GetItemAttributes(item) & IA_WEAPON)
			return CanUnitUseWeapon(unit, item);
		else if (GetItemType(item) == ITYPE_STAFF)
			return CanUnitUseStaff(unit, item);

		return false;
	}

	/* We are free! */
	return true;
}

bool CanUnitUseGaidenMagicNow(struct Unit *unit, int item)
{
	if ((GetItemAttributes(item) & (IA_MAGIC | IA_STAFF)) && IsUnitMagicSealed(unit))
		return false;

#if (defined(SID_PhysicalSeal) && (COMMON_SKILL_VALID(SID_PhysicalSeal)))
	if ((GetItemAttributes(item) & IA_WEAPON) && !(GetItemAttributes(item) & IA_MAGIC) && IsUnitPhysicalSealed(unit))
		return false;
#endif

	if (gpKernelDesignerConfig->mp_system == true)
	{
		struct NewBwl * bwl = GetNewBwl(UNIT_CHAR_ID(unit));
		
		if (bwl->currentMP < GetGaidenWeaponHpCost(unit, item))
			return false;
	}
	else
	{
		if (unit->curHP <= GetGaidenWeaponHpCost(unit, item))
			return false;
	}

	/* Spells with an MP cost need it whatever the uses system */
	if (GetGaidenMagicMpCost(unit, item) > GetUnitCurrentMP(unit))
		return false;

	if (!HasGaidenMagicUses(unit, item))
		return false;

	return CanUnitUseGaidenMagic(unit, item);
}

int GetGaidenMagicAutoEquipSlot(struct Unit *unit)
{
	int i;
	struct GaidenMagicList *list = GetGaidenMagicList(unit);

	for (i = 0; i < list->bmag_cnt; i++) {
		int item = list->bmags[i];

		if (!(GetItemAttributes(item) & IA_WEAPON))
			continue;

		if (CanUnitUseGaidenMagicNow(unit, item))
			return CHAX_BUISLOT_GAIDEN_BMAG1 + i;
	}

	for (i = 0; i < list->wmag_cnt; i++) {
		int item = list->wmags[i];

		if (!(GetItemAttributes(item) & IA_WEAPON))
			continue;

		if (CanUnitUseGaidenMagicNow(unit, item))
			return CHAX_BUISLOT_GAIDEN_WMAG1 + i;
	}

	return -1;
}

int GetGaidenMagicAutoEquipStaff(struct Unit *unit)
{
	int i;
	struct GaidenMagicList *list = GetGaidenMagicList(unit);

	for (i = 0; i < list->wmag_cnt; i++) {
		int item = list->wmags[i];

		if (!(GetItemAttributes(item) & IA_STAFF))
			continue;

		if (CanUnitUseGaidenMagicNow(unit, item))
			return item;
	}

	for (i = 0; i < list->bmag_cnt; i++) {
		int item = list->bmags[i];

		if (!(GetItemAttributes(item) & IA_STAFF))
			continue;

		if (CanUnitUseGaidenMagicNow(unit, item))
			return item;
	}

	return ITEM_NONE;
}

int GetGaidenMagicItem(struct Unit *unit, int slot)
{
	int list_index;
	struct GaidenMagicList *list = GetGaidenMagicList(unit);

	switch (slot) {
	case CHAX_BUISLOT_GAIDEN_BMAG1 ... CHAX_BUISLOT_GAIDEN_BMAG7:
		list_index = slot - CHAX_BUISLOT_GAIDEN_BMAG1;

		if (list->bmag_cnt > list_index)
			return list->bmags[list_index];

		break;

	case CHAX_BUISLOT_GAIDEN_WMAG1 ... CHAX_BUISLOT_GAIDEN_WMAG7:
		list_index = slot - CHAX_BUISLOT_GAIDEN_WMAG1;

		if (list->wmag_cnt > list_index)
			return list->wmags[list_index];

		break;

	default:
		break;
	}

	return ITEM_NONE;
}

void TryChangeGaidenMagicAction(void)
{
	int item;
	int slot = gActionData.itemSlotIndex;

	switch (slot) {
	case CHAX_BUISLOT_GAIDEN_BMAG1 ... CHAX_BUISLOT_GAIDEN_BMAG7:
	case CHAX_BUISLOT_GAIDEN_WMAG1 ... CHAX_BUISLOT_GAIDEN_WMAG7:
		break;

	default:
		return;
	}

	item = GetItemFromSlot(gActiveUnit, slot);

	if (GetItemAttributes(item) & IA_WEAPON)
		gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_GaidenMagicCombat;
	else if (GetItemType(item) == ITYPE_STAFF)
		gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_GaidenMagicStaff;
}

void PutGaidenMagicCostNumber(u16 *tm, int color, int number)
{
	if (number <= 0 || number == 0xFF)
		return;

	while (number != 0) {
		PutSpecialChar(tm, color, k_umod(number, 10) + TEXT_SPECIAL_BIGNUM_0);

		number = k_udiv(number, 10);
		tm--;
	}

	PutSpecialChar(tm, color, TEXT_SPECIAL_DASH);
}

/* Remaining uses for Three Houses-style magic, otherwise the HP/MP cost */
void PutGaidenMagicMenuNumber(u16 *tm, int color, struct Unit *unit, int item)
{
	/* Spells with an MP cost show it instead of their uses */
	if (GetGaidenMagicMpCost(unit, item) > 0) {
		PutGaidenMagicCostNumber(tm, color, GetGaidenMagicMpCost(unit, item));
		return;
	}

	if (IsGaidenMagicUsesSystem()) {
		PutNumberOrBlank(tm, color, GetGaidenMagicUses(unit, item));
		return;
	}

	PutGaidenMagicCostNumber(tm, color, GetGaidenWeaponHpCost(unit, item));
}

void DrawGaidenMagItemMenuLine(struct Text *text, int item, s8 isUsable, u16 *mapOut)
{
	Text_SetParams(text, 0, (isUsable ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY));
	Text_DrawString(text, GetItemName(item));
	PutText(text, mapOut + 2);
	DrawIcon(mapOut, GetItemIconId(item), 0x4000);
}

/* Popup script: "Learned" + item name/icon */
const struct PopupInstruction PopupScr_GaidenLearn[] = {
	POPUP_SOUND(SONG_SE_UPDATE),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
	POPUP_UNIT_NAME,
	POPUP_SPACE(2),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
	POPUP_MSG(MSG_Learned),
	POPUP_SPACE(2),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_GOLD),
	POPUP_ITEM_ICON,
	POPUP_SPACE(2),
	POPUP_ITEM_STR,
	POPUP_END
};

void TryAddGaidenMagicPConf(struct Unit * unit, int level)
{
	const struct GaidenPinfoConfigEnt *conf = GetGaidenPinfoConfigList()[UNIT_CHAR_ID(unit)].ent;
	ProcPtr * phaseToBlock = Proc_Find(gProcScr_PlayerPhase);

	for (; conf && conf->iid != 0; conf++)
	{
		u8 entry_level = conf->level & 0x7F;

		if ((conf->level & 0x80) && !(UNIT_CATTRIBUTES(unit) & CA_PROMOTED))
			continue;

		if (entry_level != level)
			continue;

		/* Show popup for this gaiden magic item id */
		int item = conf->iid;

		switch (gPlaySt.faction) {
			case FACTION_BLUE:
				phaseToBlock = Proc_Find(gProcScr_PlayerPhase);
				break;
			case FACTION_RED:
			case FACTION_GREEN:
			case FACTION_PURPLE:
				phaseToBlock = Proc_Find(gProcScr_CpDecide);
				unit = GetUnit(gActionData.targetIndex);
				break;
			default:
				break;
		}

		SetPopupItem(item);
		SetPopupUnit(unit);

		NewPopup_Simple(PopupScr_GaidenLearn, SONG_SE_UPDATE, 0x00, phaseToBlock);
	}
}