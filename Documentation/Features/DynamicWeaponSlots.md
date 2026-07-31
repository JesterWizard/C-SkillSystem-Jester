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
| `ClassData::baseRanks[8]` | Vanilla starting EXP fallback for each physical slot |
| `gClassWeaponSlotConf.wtypes[8]` | Sparse per-class map: physical slot → weapon type |
| `gClassWeaponSlotConf.baseRanks[8]` | Optional starting WEXP override for each mapped slot |
| Identity default | Classes absent from the table use slot `N` ↔ type `N` |

### Behaviour

- Usability, WEXP gain, rank bonuses, and the stat screen go through `GetUnitWeaponExp` / `SetUnitWeaponExp` so they never index `ranks[]` with a raw global type ID.
- On promotion/reclass, earned EXP is snapshotted **by weapon type**, then remapped into the new class’s slot layout (type-attached EXP).
- Auxiliary types (`ballista`, `item`, `dragonstone`) never claim a rank slot.
- New rank-bearing IDs already reserved: `ITYPE_KNIFE` (11), `ITYPE_GUN` (12).
- A nonzero `gClassWeaponSlotConf.baseRanks[slot]` supplies that slot’s default WEXP. Zero falls back to `ClassData.baseRanks[slot]`.
- Custom weapon-type display names use a bounded `GetWeaponTypeDisplayString` lookup, preventing types above vanilla index `10` from corrupting item help boxes.

### Adding a custom type to a class

1. Set the item’s `weaponType` to `ITYPE_KNIFE` / `ITYPE_GUN` (or another rank-bearing ID) and include `IA_WEAPON` in its attributes.
2. Add a `gClassWeaponSlotConf` entry that places that type in an unused slot for the class.
3. Set `baseRanks[slot]` in that same entry if the mapped type needs default WEXP.
4. Ensure `gpKernelDesignerConfig->dynamic_weapon_slots` is `true`.

Example (Thief keeps swords in slot `0` and gains E-rank knives in slot `1`):

```c
{
    .jid = CLASS_THIEF,
    .wtypes = {
        [0] = ITYPE_SWORD,
        [1] = ITYPE_KNIFE,
        /* … */
    },
    .baseRanks = {
        [1] = WPN_EXP_E,
    },
},
```

The `baseRanks` array is indexed by the **physical slot**, not the global weapon-type ID. Existing units do not receive newly configured ranks retroactively; reload the chapter or recreate the unit so `InitUnitWeaponRanks` runs.

### Adding weapon-type graphics and text

Item icons and weapon-type icons are separate assets:

| Asset | Purpose | Setup |
|-------|---------|-------|
| Item icon | Inventory, item menu, and item help icon | Add a 16×16 indexed PNG and install its generated `.4bpp` into an unused `CONFIG_PR_ITEM_ICON` slot |
| Weapon-type icon | Weapon-rank display and weapon panel type icon | Add a 16×16 indexed PNG under [`Contents/Gfx/Sources/WtypeIcon`](../../Contents/Gfx/Sources/WtypeIcon) and register it in `gWTypeIcons` |

Use an existing file such as [`Sword.png`](../../Contents/Gfx/Sources/WtypeIcon/Sword.png) as the palette/dimension template. Save a copy under the new type’s name and preserve the 16×16 indexed palette. `make` regenerates the `.4bpp`, `GfxDefs.h`, and `GfxInstaller.event` files.

For a new type, also:

1. Add its `GFX_WtypeIcon_*` entry to `gWTypeIcons` in [`IconTable.c`](../../Data/Misc/IconTable.c).
2. Add a text entry such as `MSG_WTYPE_KNIFE` in [`Items.txt`](../../Contents/Texts/Source/texts/Items.txt).
3. Add the type to `GetWeaponTypeDisplayString` in [`ItemInfos.c`](../../Kernel/Wizardry/Common/ItemSys/ItemInfoRemap/Source/ItemInfos.c).
4. Add its weapon-rank help text/configuration if it appears on the stat screen.

`ITYPE_KNIFE` and `ITYPE_GUN` reuse vanilla monster-weapon IDs `11` and `12`. UI code must distinguish a custom weapon with `IA_WEAPON` from an actual monster/non-weapon entry. The menu panel and item help paths already make this distinction.

---

## Code Locations

All dynamic remapping is gated behind `gpKernelDesignerConfig->dynamic_weapon_slots` (set in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c)).

| Feature | Location | Description |
|--------|----------|-------------|
| **Config flag** | `dynamic_weapon_slots` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) / [`kernel-lib.h`](../../include/kernel/kernel-lib.h) | Enables class slot overrides |
| **API** | helpers in [`weapon-slots.h`](../../include/kernel/weapon-slots.h) / [`weapon-slots.c`](../../Kernel/Wizardry/Common/WeaponSlots/Source/weapon-slots.c) | Slot lookup, get/set WEXP, init, remap |
| **Class overrides** | `gClassWeaponSlotConf` in [`WeaponSlots.c`](../../Data/WeaponSlots/WeaponSlots.c) | Sparse slot→type mappings and optional per-slot default WEXP |
| **Unit load** | `InitUnitWeaponRanks` via [`LoadUnit.c`](../../Kernel/Wizardry/Common/UnitHooks/Source/LoadUnit.c) | Seeds ranks from class/character using the slot map |
| **Promotion** | `RemapUnitWeaponRanksOnClassChange` in [`Promotion.c`](../../Kernel/Wizardry/Core/Lvup/Source/Promotion.c) | Type-preserving WEXP across promotion |
| **Reclass** | same remap in [`VeslyReclass/C_Code.c`](../../Kernel/Wizardry/Misc/VeslyReclass/C_Code.c) | Type-preserving WEXP across reclass |
| **Usability / WEXP** | `CanUnitUseWeapon` / `GetBattleUnitUpdatedWeaponExp` and related battle hooks | Rank checks and writes through the API |
| **Stat screen** | `DrawSkillPage_MokhaPlanA` / `PlanB` | Enumerates mapped weapon types for display |
| **Weapon-type icons** | `gWTypeIcons` in [`IconTable.c`](../../Data/Misc/IconTable.c) and PNGs in [`WtypeIcon`](../../Contents/Gfx/Sources/WtypeIcon) | Maps custom type IDs to generated rank/panel graphics |
| **Weapon-type names** | `GetWeaponTypeDisplayString` in [`ItemInfos.c`](../../Kernel/Wizardry/Common/ItemSys/ItemInfoRemap/Source/ItemInfos.c) and messages in [`Items.txt`](../../Contents/Texts/Source/texts/Items.txt) | Provides bounded custom type labels for weapon help |
| **Item-help classification** | `GetHelpBoxItemInfoKind` in [`HelpBoxHack.c`](../../Kernel/Wizardry/Core/CombatArt/HelpBoxFix/Source/HelpBoxHack.c) | Treats `IA_WEAPON` custom types as normal weapon help entries |
| **Equipment panel** | `UpdateMenuItemPanel` in [`hooks.c`](../../Kernel/Wizardry/Common/IconDisplay/Source/hooks.c) | Prevents types `11`/`12` with `IA_WEAPON` from taking the vanilla item/monster path |
| **Init validation** | `GameInit_ValidateWeaponSlots` | Duplicate/invalid type checks when enabled |

---

## TODO

- Character-level base-rank overrides for types above `7` (currently only class slot bases apply).
- Add finished item/WTYPE art and rank-help configuration before enabling `ITYPE_GUN` in gameplay.
- Add a save migration if custom slot mappings must be introduced during an existing campaign.

---

## Limitations & Bugs

- Still limited to **eight simultaneous** rank-bearing types per class (one per physical slot).
- Disabling the config mid-campaign after using custom mappings can misread saved EXP bytes (slots still hold the old type’s EXP). Prefer keeping the flag stable for a given project.
- Changing a class’s slot map or default WEXP does not rewrite units already loaded into a save. Reload/recreate those units or provide an explicit migration.
- `baseRanks[slot] = 0` means “fall back to `ClassData`,” so it cannot explicitly override a nonzero class rank to zero. Mark the slot unused with `WEAPON_SLOT_NONE` when the class should not have that type.
- Types above `12` need their own enum, icon-table entry, display string, and rank-help configuration.
- Summoner-index storage still abuses physical `ranks[ITYPE_STAFF]` for phantoms; keep phantoms on the identity map.

Please report issues in the repository’s **Issues** tab.

---
