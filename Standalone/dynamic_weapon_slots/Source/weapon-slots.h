#pragma once

#include "global.h"
#include "bmunit.h"

#define WEAPON_SLOT_NONE 0xFF
#define UNIT_WEAPON_SLOT_COUNT 8

struct ClassWeaponSlotConf {
	u8 jid;
	u8 wtypes[UNIT_WEAPON_SLOT_COUNT];
	u8 baseRanks[UNIT_WEAPON_SLOT_COUNT];
};

extern struct ClassWeaponSlotConf const gClassWeaponSlotConf[];
extern struct ClassWeaponSlotConf const *const gpClassWeaponSlotConf;

const struct ClassWeaponSlotConf *GetClassWeaponSlotConf(int jid);
int GetClassWeaponSlotType(int jid, int slot);
int GetClassWeaponSlotBaseRank(int jid, int slot);
int GetClassWeaponRankSlot(int jid, int wtype);
int GetClassWeaponRank(int jid, int wtype);
bool ClassHasWeaponType(int jid, int wtype);

int GetUnitWeaponRankSlot(struct Unit *unit, int wtype);
int GetUnitWeaponSlotType(struct Unit *unit, int slot);
int GetUnitWeaponExp(struct Unit *unit, int wtype);
void SetUnitWeaponExp(struct Unit *unit, int wtype, int exp);
int GetUnitWeaponExpBySlot(struct Unit *unit, int slot);
void SetUnitWeaponExpBySlot(struct Unit *unit, int slot, int exp);

void InitUnitWeaponRanks(struct Unit *unit, const struct CharacterData *character);
void RemapUnitWeaponRanksOnClassChange(struct Unit *unit, const struct ClassData *oldClass, const struct ClassData *newClass, bool zeroUnmapped);
int ListUnitMappedWeaponTypes(struct Unit *unit, u8 *out, int max);
