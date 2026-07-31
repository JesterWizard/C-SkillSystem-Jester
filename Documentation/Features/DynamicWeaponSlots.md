# Dynamic Weapon Slots

---

## Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## Introduction

`gpKernelDesignerConfig->dynamic_weapon_slots`

Vanilla FE8 stores weapon EXP in a fixed eight-byte array on every unit, indexed by the global weapon type (`sword=0` … `dark=7`). Adding a new rank-bearing type such as knives or guns normally means permanently replacing one of those eight slots for every class in the game.

This feature keeps the eight physical `Unit::ranks` bytes (and the existing save formats) unchanged, but lets each class assign which weapon types use those slots. A class that never uses swords can put knives in that slot instead, without affecting other classes.

When the config flag is **off**, slot `N` always stores weapon type `N` (vanilla behaviour).

---

## Plan

### Storage model

| Layer | Meaning |
|-------|---------|
| `Unit::ranks[8]` | Physical WEXP bytes (saved as-is) |
| `ClassData::baseRanks[8]` | Starting EXP for each **physical slot** |
| `gClassWeaponSlotConf` | Sparse per-class map: slot → weapon type |
| Identity default | Classes absent from the table use slot `N` ↔ type `N` |

### Behaviour

- Usability, WEXP gain, rank bonuses, and the stat screen go through `GetUnitWeaponExp` / `SetUnitWeaponExp` so they never index `ranks[]` with a raw global type ID.
- On promotion/reclass, earned EXP is snapshotted **by weapon type**, then remapped into the new class’s slot layout (type-attached EXP).
- Auxiliary types (`ballista`, `item`, `dragonstone`) never claim a rank slot.
- New rank-bearing IDs already reserved: `ITYPE_KNIFE` (11), `ITYPE_GUN` (12).

### Adding a custom type to a class

1. Set the item’s `weaponType` to `ITYPE_KNIFE` / `ITYPE_GUN` (or another rank-bearing ID).
2. Add a `gClassWeaponSlotConf` entry that places that type in an unused slot for the class.
3. Put the starting EXP in `ClassData.baseRanks[slot]` for that physical slot.
4. Ensure `gpKernelDesignerConfig->dynamic_weapon_slots` is `true`.

Example (knives in the sword slot for Thief):

```c
{
    .jid = CLASS_THIEF,
    .wtypes = {
        [0] = ITYPE_KNIFE,
        [1] = WEAPON_SLOT_NONE,
        /* … */
    },
},
```

Pair with `ClassData.baseRanks[0] = WPN_EXP_E`.

---

## Code Locations

All dynamic remapping is gated behind `gpKernelDesignerConfig->dynamic_weapon_slots` (set in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c)).

| Feature | Location | Description |
|--------|----------|-------------|
| **Config flag** | `dynamic_weapon_slots` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) / [`kernel-lib.h`](../../include/kernel/kernel-lib.h) | Enables class slot overrides |
| **API** | helpers in [`weapon-slots.h`](../../include/kernel/weapon-slots.h) / [`weapon-slots.c`](../../Kernel/Wizardry/Common/WeaponSlots/Source/weapon-slots.c) | Slot lookup, get/set WEXP, init, remap |
| **Class overrides** | `gClassWeaponSlotConf` in [`WeaponSlots.c`](../../Data/WeaponSlots/WeaponSlots.c) | Sparse slot→type tables per class |
| **Unit load** | `InitUnitWeaponRanks` via [`LoadUnit.c`](../../Kernel/Wizardry/Common/UnitHooks/Source/LoadUnit.c) | Seeds ranks from class/character using the slot map |
| **Promotion** | `RemapUnitWeaponRanksOnClassChange` in [`Promotion.c`](../../Kernel/Wizardry/Core/Lvup/Source/Promotion.c) | Type-preserving WEXP across promotion |
| **Reclass** | same remap in [`VeslyReclass/C_Code.c`](../../Kernel/Wizardry/Misc/VeslyReclass/C_Code.c) | Type-preserving WEXP across reclass |
| **Usability / WEXP** | `CanUnitUseWeapon` / `GetBattleUnitUpdatedWeaponExp` and related battle hooks | Rank checks and writes through the API |
| **Stat screen** | `DrawSkillPage_MokhaPlanA` / `PlanB` | Enumerates mapped weapon types for display |
| **Init validation** | `GameInit_ValidateWeaponSlots` | Duplicate/invalid type checks when enabled |

---

## TODO

- Icon/name strings for `ITYPE_KNIFE` / `ITYPE_GUN` if those types are used in a romhack.
- Character-level base-rank overrides for types above `7` (currently only class slot bases apply).

---

## Limitations & Bugs

- Still limited to **eight simultaneous** rank-bearing types per class (one per physical slot).
- Disabling the config mid-campaign after using custom mappings can misread saved EXP bytes (slots still hold the old type’s EXP). Prefer keeping the flag stable for a given project.
- Summoner-index storage still abuses physical `ranks[ITYPE_STAFF]` for phantoms; keep phantoms on the identity map.

Please report issues in the repository’s **Issues** tab.

---
