#pragma once

#include "common-chax.h"

/**
 * Dynamic class weapon slots
 *
 * Unit::ranks[8] remains the physical WEXP storage.
 * Each class may assign any rank-bearing weapon type to any of those
 * eight slots when gpKernelDesignerConfig->dynamic_weapon_slots is set.
 * Classes without an override (or when the config is off) keep the vanilla
 * identity mapping (slot N stores weapon type N).
 */

#define WEAPON_SLOT_NONE 0xFF
#define UNIT_WEAPON_SLOT_COUNT 8

struct ClassWeaponSlotConf {
	/* 00 */ u8 jid;
	/* 01 */ u8 wtypes[UNIT_WEAPON_SLOT_COUNT]; /* slot -> weapon type; WEAPON_SLOT_NONE = unused */
	/* 09 */ u8 baseRanks[UNIT_WEAPON_SLOT_COUNT]; /* optional slot starting WEXP; 0 = ClassData fallback */
};

extern struct ClassWeaponSlotConf const gClassWeaponSlotConf[];
extern struct ClassWeaponSlotConf const *const gpClassWeaponSlotConf;

/* Class / slot queries */
const struct ClassWeaponSlotConf *GetClassWeaponSlotConf(int jid);
int GetClassWeaponSlotType(int jid, int slot);
int GetClassWeaponSlotBaseRank(int jid, int slot);
int GetClassWeaponRankSlot(int jid, int wtype);
int GetClassWeaponRank(int jid, int wtype);
bool ClassHasWeaponType(int jid, int wtype);

/* Unit queries / mutation */
int GetUnitWeaponRankSlot(struct Unit *unit, int wtype);
int GetUnitWeaponSlotType(struct Unit *unit, int slot);
int GetUnitWeaponExp(struct Unit *unit, int wtype);
void SetUnitWeaponExp(struct Unit *unit, int wtype, int exp);
int GetUnitWeaponExpBySlot(struct Unit *unit, int slot);
void SetUnitWeaponExpBySlot(struct Unit *unit, int slot, int exp);

/* Lifecycle */
void InitUnitWeaponRanks(struct Unit *unit, const struct CharacterData *character);
void RemapUnitWeaponRanksOnClassChange(struct Unit *unit, const struct ClassData *oldClass, const struct ClassData *newClass, bool zeroUnmapped);

/* Enumeration helper: fills out[] with mapped weapon types, returns count */
int ListUnitMappedWeaponTypes(struct Unit *unit, u8 *out, int max);

void GameInit_ValidateWeaponSlots(void);
