# Tonic Items

<p align="center">
  <img src="../Gifs/Item_Tonics.gif" alt="Tonics" width="600"/>
</p>

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

The tonic items are the custom stat-boosting consumables that mark a unit as having used a specific tonic type for the current chapter. From the player’s perspective, a tonic should feel like a simple pre-battle buff item: use it once, see the confirmation popup, and keep the bonus until the chapter changes.

The implementation is more stateful than a normal stat booster because the project wants tonic use to be tracked separately from ordinary permanent bonuses. That means the game has to remember both the tonic type and the chapter it was used in, then expose the result through the unit status getters.

---

## 🛠️ Plan

Tonic handling follows a small use-and-query pipeline:

| Step | Player Result | Implementation Responsibility |
|------|---------------|-------------------------------|
| 1 | The player uses a tonic item like a normal consumable | Route the item through the item revamp stat-boost path |
| 2 | The game stores which tonic was used | Save the tonic index in per-unit RAM and record the active chapter |
| 3 | A confirmation popup appears | Show a tonic-specific popup that uses the unit name and item icon |
| 4 | The stat screen reflects the tonic bonus | Add a tonic bonus through the status getter helpers for the tracked stat |
| 5 | The bonus disappears in the wrong chapter | Reject tonic bonuses when the stored chapter does not match the current chapter |

The design intentionally keeps tonic logic chapter-bound. That makes tonic effects easy to reason about and avoids letting tonic state leak across unrelated maps or long-term saves.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| **Tonic use flow** | `ExecStatBoostItem` and `ApplyStatBoostItem` in [`IER-extra.c`](../../../Kernel/Wizardry/Common/ItemSys/IERevamp/Source/IER-extra.c) | Detects tonic usage, records the selected tonic index, and triggers the tonic popup |
| **Tonic validity check** | `IsTonicCampaignActive` and `IsTonicCampaignActiveIndex` in [`IER-extra.c`](../../../Kernel/Wizardry/Common/ItemSys/IERevamp/Source/IER-extra.c) | Verifies that the stored tonic matches the current unit and chapter |
| **Tonic bonus lookup** | `GetTonicStatBonus` in [`IER-extra.c`](../../../Kernel/Wizardry/Common/ItemSys/IERevamp/Source/IER-extra.c) | Returns the +2 tonic bonus when the stored chapter and tonic type are valid |
| **HP tonic getter** | `HPTonic` in [`TonicGetter.c`](../../../Kernel/Wizardry/Core/UnitStatusGetter/source/TonicGetter.c) | Feeds the tonic bonus into HP status calculation |
| **STR tonic getter** | `PowTonic` in [`TonicGetter.c`](../../../Kernel/Wizardry/Core/UnitStatusGetter/source/TonicGetter.c) | Feeds the tonic bonus into Strength status calculation |
| **MAG tonic getter** | `MagTonic` in [`TonicGetter.c`](../../../Kernel/Wizardry/Core/UnitStatusGetter/source/TonicGetter.c) | Feeds the tonic bonus into Magic status calculation |
| **SKL tonic getter** | `SklTonic` in [`TonicGetter.c`](../../../Kernel/Wizardry/Core/UnitStatusGetter/source/TonicGetter.c) | Feeds the tonic bonus into Skill status calculation |
| **SPD tonic getter** | `SpdTonic` in [`TonicGetter.c`](../../../Kernel/Wizardry/Core/UnitStatusGetter/source/TonicGetter.c) | Feeds the tonic bonus into Speed status calculation |
| **LCK tonic getter** | `LckTonic` in [`TonicGetter.c`](../../../Kernel/Wizardry/Core/UnitStatusGetter/source/TonicGetter.c) | Feeds the tonic bonus into Luck status calculation |
| **DEF tonic getter** | `DefTonic` in [`TonicGetter.c`](../../../Kernel/Wizardry/Core/UnitStatusGetter/source/TonicGetter.c) | Feeds the tonic bonus into Defense status calculation |
| **RES tonic getter** | `ResTonic` in [`TonicGetter.c`](../../../Kernel/Wizardry/Core/UnitStatusGetter/source/TonicGetter.c) | Feeds the tonic bonus into Resistance status calculation |
| **OMNI tonic getter** | `OmniTonic` in [`TonicGetter.c`](../../../Kernel/Wizardry/Core/UnitStatusGetter/source/TonicGetter.c) | Feeds the tonic bonus into every tracked stat getter that reads the omni tonic |
| **Save/load state** | `MSU_SaveTonicState` and `MSU_LoadTonicState` in [`MsuFunc.c`](../../../Kernel/Wizardry/Common/SaveData/Source/MsuFunc.c) | Persists tonic chapter state and the per-unit tonic state array in suspend/save data |
| **RAM storage** | `gUnitTonicState` and `gTonicChapterState` in [`config-memmap.s`](../../../include/link/config-memmap.s) and [`save-data.h`](../../../include/kernel/save-data.h) | Reserves the shared runtime storage used by the tonic system |
| **Item text** | `MSG_ITEM_TONIC_*` entries in [`Items.txt`](../../../Contents/Texts/Source/texts/Items.txt) | Stores the visible tonic names, descriptions, and use text |
| **Item icons** | `GFX_TonicIcon_*` in [`GfxInstaller.event`](../../../Contents/Gfx/GfxInstaller.event) and [`IconTable.c`](../../../Data/Misc/IconTable.c) | Registers the tonic item icons shown in menus and popups |

---

## 📝 TODO

- [ ] Confirm whether the tonic popup wording should be shared with any future tonic-like items.
- [ ] Add screenshots if the tonic UI ever gets a dedicated presentation pass.
- [ ] Expand this doc if more tonic variants are added later.

---

## 🐛 Limitations & Bugs

Tonic bonuses are chapter-scoped. If the stored chapter no longer matches the active chapter, the tonic bonus is ignored until a tonic is used again.

The tonic state is stored per unit index, so any future work that changes unit indexing or long-term unit persistence should review the save and load helpers at the same time.

The current behavior assumes tonic items are the only consumables that should write to the tonic state. If another item type starts reusing the same state path, the popup and getter logic will need to be revisited.