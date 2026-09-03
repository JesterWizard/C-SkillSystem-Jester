#include <string.h>

#include "weapon-slots.h"
#include "bmitem.h"
#include "bmunit.h"
#include "bmbattle.h"
#include "bmsave.h"
#include "statscreen.h"
#include "icon.h"
#include "hardware.h"
#include "variables.h"
#include "constants/classes.h"

extern const struct ClassWeaponSlotConf gClassWeaponSlotConf[];
extern const struct ClassWeaponSlotConf *const gpClassWeaponSlotConf;

static bool IsRankBearingWeaponType(int wtype)
{
	if (wtype < 0)
		return false;

	if (wtype == ITYPE_BLLST || wtype == ITYPE_ITEM || wtype == ITYPE_DRAGN)
		return false;

	return true;
}

const struct ClassWeaponSlotConf *GetClassWeaponSlotConf(int jid)
{
	const struct ClassWeaponSlotConf *it;

	if (!gpClassWeaponSlotConf || jid <= 0)
		return NULL;

	for (it = gpClassWeaponSlotConf; it->jid != 0; it++) {
		if (it->jid == jid)
			return it;
	}

	return NULL;
}

int GetClassWeaponSlotType(int jid, int slot)
{
	const struct ClassWeaponSlotConf *conf;

	if (slot < 0 || slot >= UNIT_WEAPON_SLOT_COUNT)
		return WEAPON_SLOT_NONE;

	conf = GetClassWeaponSlotConf(jid);
	if (conf)
		return conf->wtypes[slot];

	return slot;
}

int GetClassWeaponSlotBaseRank(int jid, int slot)
{
	const struct ClassWeaponSlotConf *conf;
	const struct ClassData *jinfo;

	if (slot < 0 || slot >= UNIT_WEAPON_SLOT_COUNT)
		return 0;

	conf = GetClassWeaponSlotConf(jid);
	if (conf && conf->baseRanks[slot] != 0)
		return conf->baseRanks[slot];

	jinfo = GetClassData(jid);
	if (!jinfo)
		return 0;

	return jinfo->baseRanks[slot];
}

int GetClassWeaponRankSlot(int jid, int wtype)
{
	int slot;

	if (!IsRankBearingWeaponType(wtype))
		return -1;

	for (slot = 0; slot < UNIT_WEAPON_SLOT_COUNT; slot++) {
		if (GetClassWeaponSlotType(jid, slot) == wtype)
			return slot;
	}

	return -1;
}

int GetClassWeaponRank(int jid, int wtype)
{
	int slot = GetClassWeaponRankSlot(jid, wtype);

	if (slot < 0)
		return 0;

	return GetClassWeaponSlotBaseRank(jid, slot);
}

bool ClassHasWeaponType(int jid, int wtype)
{
	return GetClassWeaponRankSlot(jid, wtype) >= 0;
}

int GetUnitWeaponRankSlot(struct Unit *unit, int wtype)
{
	if (!unit || !unit->pClassData)
		return -1;

	return GetClassWeaponRankSlot(UNIT_CLASS_ID(unit), wtype);
}

int GetUnitWeaponSlotType(struct Unit *unit, int slot)
{
	if (!unit || !unit->pClassData)
		return WEAPON_SLOT_NONE;

	return GetClassWeaponSlotType(UNIT_CLASS_ID(unit), slot);
}

int GetUnitWeaponExpBySlot(struct Unit *unit, int slot)
{
	if (!unit || slot < 0 || slot >= UNIT_WEAPON_SLOT_COUNT)
		return 0;

	return unit->ranks[slot];
}

void SetUnitWeaponExpBySlot(struct Unit *unit, int slot, int exp)
{
	if (!unit || slot < 0 || slot >= UNIT_WEAPON_SLOT_COUNT)
		return;

	if (exp < 0)
		exp = 0;

	if (exp > WPN_EXP_S)
		exp = WPN_EXP_S;

	unit->ranks[slot] = exp;
}

int GetUnitWeaponExp(struct Unit *unit, int wtype)
{
	int slot = GetUnitWeaponRankSlot(unit, wtype);

	if (slot < 0)
		return 0;

	return unit->ranks[slot];
}

void SetUnitWeaponExp(struct Unit *unit, int wtype, int exp)
{
	int slot = GetUnitWeaponRankSlot(unit, wtype);

	if (slot < 0)
		return;

	SetUnitWeaponExpBySlot(unit, slot, exp);
}

void InitUnitWeaponRanks(struct Unit *unit, const struct CharacterData *character)
{
	int slot;
	int jid;

	if (!unit || !unit->pClassData)
		return;

	jid = UNIT_CLASS_ID(unit);

	for (slot = 0; slot < UNIT_WEAPON_SLOT_COUNT; slot++) {
		int wtype = GetClassWeaponSlotType(jid, slot);
		int exp;

		if (wtype == WEAPON_SLOT_NONE) {
			unit->ranks[slot] = 0;
			continue;
		}

		exp = GetClassWeaponSlotBaseRank(jid, slot);

		if (character && wtype < UNIT_WEAPON_SLOT_COUNT && character->baseRanks[wtype] != 0)
			exp = character->baseRanks[wtype];

		unit->ranks[slot] = exp;
	}
}

void RemapUnitWeaponRanksOnClassChange(struct Unit *unit, const struct ClassData *oldClass, const struct ClassData *newClass, bool zeroUnmapped)
{
	u8 earned[0x100];
	u8 present[0x100];
	int slot;
	int oldJid;
	int newJid;

	if (!unit || !oldClass || !newClass)
		return;

	oldJid = oldClass->number;
	newJid = newClass->number;

	memset(earned, 0, sizeof(earned));
	memset(present, 0, sizeof(present));

	for (slot = 0; slot < UNIT_WEAPON_SLOT_COUNT; slot++) {
		int wtype = GetClassWeaponSlotType(oldJid, slot);
		int exp;
		int baseRank;

		if (wtype == WEAPON_SLOT_NONE)
			continue;

		baseRank = GetClassWeaponSlotBaseRank(oldJid, slot);
		exp = unit->ranks[slot] - baseRank;
		if (exp < 0)
			exp = 0;

		earned[wtype] = exp;
		present[wtype] = 1;
	}

	unit->pClassData = newClass;

	for (slot = 0; slot < UNIT_WEAPON_SLOT_COUNT; slot++) {
		int wtype = GetClassWeaponSlotType(newJid, slot);
		int exp;
		int baseRank;

		if (wtype == WEAPON_SLOT_NONE) {
			unit->ranks[slot] = 0;
			continue;
		}

		baseRank = GetClassWeaponSlotBaseRank(newJid, slot);

		if (zeroUnmapped && baseRank == 0) {
			unit->ranks[slot] = 0;
			continue;
		}

		exp = baseRank;

		if (present[wtype])
			exp += earned[wtype];

		if (exp > WPN_EXP_S)
			exp = WPN_EXP_S;

		unit->ranks[slot] = exp;
	}
}

int ListUnitMappedWeaponTypes(struct Unit *unit, u8 *out, int max)
{
	int slot;
	int count = 0;

	if (!unit || !out || max <= 0)
		return 0;

	for (slot = 0; slot < UNIT_WEAPON_SLOT_COUNT && count < max; slot++) {
		int wtype = GetUnitWeaponSlotType(unit, slot);

		if (wtype == WEAPON_SLOT_NONE)
			continue;

		out[count++] = wtype;
	}

	return count;
}

bool DWS_CanClassWieldWeaponType(u8 classId, u8 wpnType)
{
	return GetClassWeaponRank(classId, wpnType) != 0 ? TRUE : FALSE;
}

int DWS_GetUnitBestWRankType(struct Unit *unit)
{
	int slot;
	int bestSlot = -1;
	int bestExp = 0;

	if (!unit)
		return -1;

	for (slot = 0; slot < UNIT_WEAPON_SLOT_COUNT; slot++) {
		int wtype = GetUnitWeaponSlotType(unit, slot);
		int exp;

		if (wtype == WEAPON_SLOT_NONE)
			continue;

		if (wtype == ITYPE_STAFF)
			continue;

		exp = unit->ranks[slot];
		if (exp > bestExp) {
			bestExp = exp;
			bestSlot = slot;
		}
	}

	if (bestSlot < 0)
		return -1;

	return GetUnitWeaponSlotType(unit, bestSlot);
}

void DWS_UnitLoadStatsFromChracter(struct Unit *unit, const struct CharacterData *character)
{
	unit->maxHP = character->baseHP + unit->pClassData->baseHP;
	unit->pow = character->basePow + unit->pClassData->basePow;
	unit->skl = character->baseSkl + unit->pClassData->baseSkl;
	unit->spd = character->baseSpd + unit->pClassData->baseSpd;
	unit->def = character->baseDef + unit->pClassData->baseDef;
	unit->res = character->baseRes + unit->pClassData->baseRes;
	unit->lck = character->baseLck;

	unit->conBonus = 0;

	InitUnitWeaponRanks(unit, character);

	if (UNIT_FACTION(unit) == FACTION_BLUE && (unit->level != UNIT_LEVEL_MAX))
		unit->exp = 0;
	else
		unit->exp = UNIT_EXP_DISABLED;
}

void DWS_ApplyUnitPromotion(struct Unit *unit, u8 classId)
{
	const struct ClassData *promotedClass = GetClassData(classId);
	const struct ClassData *oldClass = unit->pClassData;
	int baseClassId = oldClass->number;
	int promClassId = promotedClass->number;

	unit->maxHP += promotedClass->promotionHp;

	if (unit->maxHP > promotedClass->maxHP)
		unit->maxHP = promotedClass->maxHP;

	unit->pow += promotedClass->promotionPow;

	if (unit->pow > promotedClass->maxPow)
		unit->pow = promotedClass->maxPow;

	unit->skl += promotedClass->promotionSkl;

	if (unit->skl > promotedClass->maxSkl)
		unit->skl = promotedClass->maxSkl;

	unit->spd += promotedClass->promotionSpd;

	if (unit->spd > promotedClass->maxSpd)
		unit->spd = promotedClass->maxSpd;

	unit->def += promotedClass->promotionDef;

	if (unit->def > promotedClass->maxDef)
		unit->def = promotedClass->maxDef;

	unit->res += promotedClass->promotionRes;

	if (unit->res > promotedClass->maxRes)
		unit->res = promotedClass->maxRes;

	RemapUnitWeaponRanksOnClassChange(unit, oldClass, promotedClass, false);

	if (baseClassId == CLASS_PUPIL && promClassId == CLASS_SHAMAN)
		SetUnitWeaponExp(unit, ITYPE_ANIMA, 0);

	unit->level = 1;
	unit->exp = 0;

	unit->curHP += promotedClass->promotionHp;

	if (unit->curHP > GetUnitMaxHp(unit))
		unit->curHP = GetUnitMaxHp(unit);
}

s8 DWS_CanUnitUseWeapon(struct Unit *unit, int item)
{
	if (item == 0)
		return FALSE;

	if (!(GetItemAttributes(item) & IA_WEAPON))
		return FALSE;

	if (GetItemAttributes(item) & IA_LOCK_ANY) {
		if ((GetItemAttributes(item) & IA_LOCK_1) && !(UNIT_CATTRIBUTES(unit) & CA_LOCK_1))
			return FALSE;

		if ((GetItemAttributes(item) & IA_LOCK_4) && !(UNIT_CATTRIBUTES(unit) & CA_LOCK_4))
			return FALSE;

		if ((GetItemAttributes(item) & IA_LOCK_5) && !(UNIT_CATTRIBUTES(unit) & CA_LOCK_5))
			return FALSE;

		if ((GetItemAttributes(item) & IA_LOCK_6) && !(UNIT_CATTRIBUTES(unit) & CA_LOCK_6))
			return FALSE;

		if ((GetItemAttributes(item) & IA_LOCK_7) && !(UNIT_CATTRIBUTES(unit) & CA_LOCK_7))
			return FALSE;

		if ((GetItemAttributes(item) & IA_LOCK_2) && !(UNIT_CATTRIBUTES(unit) & CA_LOCK_2))
			return FALSE;

		if (GetItemAttributes(item) & IA_LOCK_3) {
			if (!(UNIT_CATTRIBUTES(unit) & CA_LOCK_3))
				return FALSE;

			return TRUE;
		}

		if (GetItemAttributes(item) & IA_UNUSABLE)
			if (!(IsItemUnsealedForUnit(unit, item)))
				return FALSE;
	}

	if ((unit->statusIndex == UNIT_STATUS_SILENCED) && (GetItemAttributes(item) & IA_MAGIC))
		return FALSE;

	return GetUnitWeaponExp(unit, GetItemType(item)) >= GetItemRequiredExp(item) ? TRUE : FALSE;
}

int DWS_GetBattleUnitUpdatedWeaponExp(struct BattleUnit *bu)
{
	int i, result, weapon_slot;

	if (UNIT_FACTION(&bu->unit) != FACTION_BLUE)
		return -1;

	if (bu->unit.curHP == 0)
		return -1;

	if (gPlaySt.chapterStateBits & PLAY_FLAG_EXTRA_MAP)
		return -1;

	if (gBmSt.gameStateBits & 0x40)
		return -1;

	if (!(gBattleStats.config & BATTLE_CONFIG_ARENA)) {
		if (!bu->canCounter)
			return -1;

		if (!(bu->weaponAttributes & IA_REQUIRES_WEXP))
			return -1;

		if (bu->weaponAttributes & (IA_MAGICDAMAGE | IA_LOCK_3))
			return -1;
	}

	result = GetUnitWeaponExp(&bu->unit, bu->weaponType);
	result += GetItemAwardedExp(bu->weapon) * bu->wexpMultiplier;

	weapon_slot = GetUnitWeaponRankSlot(&bu->unit, bu->weaponType);

	for (i = 0; i < UNIT_WEAPON_SLOT_COUNT; ++i) {
		int slot_wtype;

		if (i == weapon_slot)
			continue;

		slot_wtype = GetUnitWeaponSlotType(&bu->unit, i);
		if (slot_wtype == WEAPON_SLOT_NONE)
			continue;

		if (GetClassWeaponRank(UNIT_CLASS_ID(&bu->unit), slot_wtype) == WPN_EXP_S)
			continue;

		if (bu->unit.ranks[i] < WPN_EXP_S)
			continue;

		if (result >= WPN_EXP_S)
			result = (WPN_EXP_S - 1);

		break;
	}

	if (UNIT_CATTRIBUTES(&bu->unit) & CA_PROMOTED) {
		if (result > WPN_EXP_S)
			result = WPN_EXP_S;
	} else if (UNIT_CATTRIBUTES(&bu->unit) & CA_MAXLEVEL10) {
		if (result > WPN_EXP_C)
			result = WPN_EXP_C;
	} else {
		if (result > WPN_EXP_A)
			result = WPN_EXP_A;
	}

	return result;
}

s8 DWS_HasBattleUnitGainedWeaponLevel(struct BattleUnit *bu)
{
	int oldWexp = GetUnitWeaponExp(&bu->unit, bu->weaponType);
	int newWexp = DWS_GetBattleUnitUpdatedWeaponExp(bu);

	if (newWexp < 0)
		return FALSE;

	return GetWeaponLevelFromExp(oldWexp) != GetWeaponLevelFromExp(newWexp);
}

void DWS_UpdateUnitFromBattle(struct Unit *unit, struct BattleUnit *bu)
{
	int tmp;

	unit->level = bu->unit.level;
	unit->exp = bu->unit.exp;
	unit->curHP = bu->unit.curHP;
	unit->state = bu->unit.state;

	gUnknown_03003060 = UNIT_ARENA_LEVEL(unit);

	if (bu->statusOut >= 0)
		SetUnitStatus(unit, bu->statusOut);

	unit->maxHP += bu->changeHP;
	unit->pow += bu->changePow;
	unit->skl += bu->changeSkl;
	unit->spd += bu->changeSpd;
	unit->def += bu->changeDef;
	unit->res += bu->changeRes;
	unit->lck += bu->changeLck;

	UnitCheckStatCaps(unit);

	tmp = DWS_GetBattleUnitUpdatedWeaponExp(bu);

	if (tmp > 0)
		SetUnitWeaponExp(unit, bu->weaponType, tmp);

	for (tmp = 0; tmp < UNIT_ITEM_COUNT; ++tmp)
		unit->items[tmp] = bu->unit.items[tmp];

	UnitRemoveInvalidItems(unit);

	if (bu->expGain)
		PidStatsAddExpGained(unit->pCharacterData->number, bu->expGain);
}

void DWS_UpdateUnitDuringBattle(struct Unit *unit, struct BattleUnit *bu)
{
	int wexp;

	unit->curHP = bu->unit.curHP;

	wexp = DWS_GetBattleUnitUpdatedWeaponExp(bu);

	if (wexp > 0)
		SetUnitWeaponExp(unit, bu->weaponType, wexp);
}

void DWS_DisplayWeaponExp(int num, int x, int y, int wtype)
{
	int progress, progressMax, color;
	int wexp = GetUnitWeaponExp(gStatScreen.unit, wtype);

	DrawIcon(gUiTmScratchA + TILEMAP_INDEX(x, y),
		0x70 + wtype,
		TILEREF(0, STATSCREEN_BGPAL_EXTICONS));

	color = wexp >= WPN_EXP_S
		? TEXT_COLOR_SYSTEM_GREEN
		: TEXT_COLOR_SYSTEM_BLUE;

	PutSpecialChar(gUiTmScratchA + TILEMAP_INDEX(x + 4, y),
		color,
		GetDisplayRankStringFromExp(wexp));

	GetWeaponExpProgressState(wexp, &progress, &progressMax);

	DrawStatBarGfx(0x401 + num * 6, 5,
		gUiTmScratchC + TILEMAP_INDEX(x + 2, y + 1), TILEREF(0, STATSCREEN_BGPAL_6),
		0x22, (progress * 34) / (progressMax - 1), 0);
}

void DWS_DisplayPage2(void)
{
	int i;
	int wcount;
	u8 wtypes[UNIT_WEAPON_SLOT_COUNT];
	const int wexp_y[UNIT_WEAPON_SLOT_COUNT] = {
		0x1, 0x3, 0x5, 0x7, 0x9, 0xB, 0xD, 0xF,
	};

	wcount = ListUnitMappedWeaponTypes(gStatScreen.unit, wtypes, UNIT_WEAPON_SLOT_COUNT);

	for (i = 0; i < wcount; i++)
		DWS_DisplayWeaponExp(i, 1, wexp_y[i], wtypes[i]);

	DisplaySupportList();
}
