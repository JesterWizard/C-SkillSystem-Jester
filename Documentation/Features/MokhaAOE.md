# Data-Driven Mokha Gambits

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Current Unit and Gambit Access](#current-unit-and-gambit-access)
- [Adding a Unit](#adding-a-unit)
- [Adding or Changing a Gambit](#adding-or-changing-a-gambit)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

Mokha Gambits add a Three Houses-style area attack to the unit menu. An eligible unit chooses **Gambit**, selects one of the configured attacks, chooses a target tile, and damages every non-allied unit inside that attack's area.

The system is data-driven for its core attack properties. Attack names, descriptions, range, damage, and map-routine indices are stored in `gMokhaAoeAttackTable`. Unit access is controlled separately by a character-PID allowlist.

---

## 🛠️ Plan

The Gambit flow is:

```text
Unit menu
   │
   ├─ Runtime flag enabled?
   ├─ Active unit is allowlisted?
   ├─ Unit has not moved, cantoed, or entered a ballista?
   └─ At least one gambit has a valid target?
          │
          ▼
      Gambit menu
          │
          ▼
   Attack range preview
          │
          ▼
   Target tile selection
          │
          ▼
   Save affected target UIDs
          │
          ▼
   Move-unit attack animation → damage → kill cleanup → accumulated EXP
```

During target selection, the selected gambit's map routine fills the area map. The target list contains non-allied units found in that area. The action then processes the saved targets one at a time, applying the configured damage up to the target's current HP.

---

## 🎯 Current Unit and Gambit Access

The current allowlist is defined in `gMokhaAoeEligibleByPid`:

| Unit | Character constant | Available gambits |
|------|--------------------|-------------------|
| Eirika | `CHARACTER_EIRIKA` | Strike, Blaze, Absorption, Fusillade, Fire Arrows, Group Lance |
| Seth | `CHARACTER_SETH` | Strike, Blaze, Absorption, Fusillade, Fire Arrows, Group Lance |
| Every other character PID | — | None |

There is currently **no per-unit/per-gambit matrix**. Once a unit is allowlisted, it receives every attack in the six-entry attack table. The menu can still disable an individual gambit when that attack has no valid target.

The feature is disabled for everyone when `mokha_aoe_enabled` is false. The default designer configuration enables it.

| Index | Gambit | Shape | Range | Damage |
|-------|--------|-------|-------|--------|
| `0` | Strike | Single tile | `2` | `10` |
| `1` | Blaze | Forward cone | `2` | `10` |
| `2` | Absorption | Wide diamond | `2` | `10` |
| `3` | Fusillade | Cross | `2` | `10` |
| `4` | Fire Arrows | Straight line | `2` | `10` |
| `5` | Group Lance | Horizontal row | `2` | `10` |

---

## ➕ Adding a Unit

1. Open [`GambitData.c`](../../Kernel/Wizardry/Misc/MokhaAOE/EngineHacks/Gambit/src/GambitData.c).
2. Confirm the character has a generated constant in `constants/characters.h`.
3. Add the character PID to `gMokhaAoeEligibleByPid`:

   ```c
   const u8 gMokhaAoeEligibleByPid[0x100] = {
       [CHARACTER_EIRIKA] = true,
       [CHARACTER_SETH] = true,
       [CHARACTER_NEW_UNIT] = true,
   };
   ```

4. Rebuild with `make chax`.

The allowlist is based on character PID, not class, weapon rank, inventory, or promotion state. A promoted unit keeps access if it retains the same character PID. The new unit will receive all six current gambits.

To give a unit only some gambits, the data model must first be extended; see [TODO](#todo).

---

## 🧱 Adding or Changing a Gambit

Adding a gambit requires keeping four indices synchronized:

1. Add a new enum value before `MOKHA_AOE_ATK_COUNT` in [`mokha-aoe.h`](../../include/kernel/mokha-aoe.h).
2. Add its name, description, range, damage, and map-routine index to `gMokhaAoeAttackTable` in [`GambitData.c`](../../Kernel/Wizardry/Misc/MokhaAOE/EngineHacks/Gambit/src/GambitData.c).
3. Add a matching `MenuItemDef` entry to `sGambitSelectMenuItems` in [`GambitMenuCore.c`](../../Kernel/Wizardry/Misc/MokhaAOE/EngineHacks/Gambit/GambitMenu/src/GambitMenuCore.c).
4. Add the matching map-routine pointer and included `.lyn.event` output to [`GambitEffectMap.event`](../../Kernel/Wizardry/Misc/MokhaAOE/EngineHacks/Gambit/GambitEffectMap/GambitEffectMap.event).
5. Add the attack's text entries to [`misc.txt`](../../Contents/Texts/Source/texts/misc.txt).
6. Rebuild with `make chax`.

The attack table and menu item number must agree. For example, an attack with index `6` must use item number `6`, table entry `6`, and map-routine entry `6`.

The checked-in `.lyn.event` files under `GambitEffectMap/FillAOEMapFucs` are active build inputs. Keep the corresponding generated output in the repository when adding or replacing an area routine.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Runtime enable flag | `mokha_aoe_enabled` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) | Global designer-config switch; defaults to enabled |
| Configuration field | `KernelDesigerConfig` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) | Declares the runtime flag |
| Unit allowlist | `gMokhaAoeEligibleByPid` in [`GambitData.c`](../../Kernel/Wizardry/Misc/MokhaAOE/EngineHacks/Gambit/src/GambitData.c) | Maps character PIDs to Gambit eligibility |
| Attack data | `gMokhaAoeAttackTable` in [`GambitData.c`](../../Kernel/Wizardry/Misc/MokhaAOE/EngineHacks/Gambit/src/GambitData.c) | Stores text IDs, range, damage, and map-routine index |
| Shared declarations | [`mokha-aoe.h`](../../include/kernel/mokha-aoe.h) | Attack enum, attack data structure, globals, and public functions |
| Unit-menu command | Gambit row in [`UnitMenu.c`](../../Data/UnitMenu/Source/UnitMenu.c) | Places Gambit in the unit menu |
| Gambit menu usability | `Gambit_UpperMenu_Usability` in [`GambitMenuCore.c`](../../Kernel/Wizardry/Misc/MokhaAOE/EngineHacks/Gambit/GambitMenu/src/GambitMenuCore.c) | Checks unit state, eligibility, and available targets |
| Attack selection | `sGambitSelectMenuItems` in [`GambitMenuCore.c`](../../Kernel/Wizardry/Misc/MokhaAOE/EngineHacks/Gambit/GambitMenu/src/GambitMenuCore.c) | Defines the six selectable attacks |
| Area-map routines | `GambitEffectMap_DrawMapRoutineTable` in [`GambitEffectMap.event`](../../Kernel/Wizardry/Misc/MokhaAOE/EngineHacks/Gambit/GambitEffectMap/GambitEffectMap.event) | Connects attack indices to shape routines |
| Target selection | `gSelectInfo_Gambit` in [`TargetSelectCore.c`](../../Kernel/Wizardry/Misc/MokhaAOE/EngineHacks/Gambit/GambitTargetSelect/src/TargetSelectCore.c) | Displays the area, selects a tile, and cleans up map graphics |
| Target persistence | `SaveTarget_PostGambitTargetSelection` in [`GambitSaveCore.c`](../../Kernel/Wizardry/Misc/MokhaAOE/EngineHacks/Gambit/SaveAOETarget/src/GambitSaveCore.c) | Saves up to `0x40` affected target UIDs |
| Damage, animation, kills, and EXP | `GambitAction` and related procs in [`GambitActionCore.c`](../../Kernel/Wizardry/Misc/MokhaAOE/EngineHacks/Gambit/GambitAction/src/GambitActionCore.c) | Runs the attack sequence, applies damage, kills defeated units, and grants accumulated EXP |
| Unit-action injection | [`GambitAction.event`](../../Kernel/Wizardry/Misc/MokhaAOE/EngineHacks/Gambit/GambitAction/GambitAction.event) | Installs the Gambit unit action at `CONFIG_UNIT_ACTION_EXPA_Gambit` |
| Feature installer | [`MokhaAOE_Installer.event`](../../Kernel/Wizardry/Misc/MokhaAOE/MokhaAOE_Installer.event) | Includes the active Gambit installer |
| Player-facing text | Gambit entries in [`misc.txt`](../../Contents/Texts/Source/texts/misc.txt) | Defines menu labels, descriptions, and attack names |

---

## 📝 TODO

- Add an optional per-PID gambit bitmask so units can receive different attack subsets.
- Move menu entries into the attack data table so adding a gambit does not require a separate hardcoded menu entry.
- Add a contributor-facing map-routine generation workflow for new area shapes.
- Add automated checks that verify attack-table, menu, text, and map-routine indices remain synchronized.

---

## 🐛 Limitations & Bugs

- All six current gambits use range `2` and damage `10`; their area shapes are the main difference.
- Eligibility is character-PID based. Class-based, weapon-based, faction-based, and skill-based restrictions are not implemented.
- The target buffer stores at most `0x40` targets.
- EXP is only accumulated for eligible blue units that can gain levels and are not on extra maps. The attack itself still resolves when EXP is unavailable.
- Only non-allied units in the selected area are targeted. There is no ally healing or support effect in the current implementation.
- Area routines are included as generated `.lyn.event` files, so deleting one of those outputs breaks the corresponding attack shape.

Please report gameplay, animation, or map-cleanup regressions in the repository's **Issues** tab.

---
