# Quality of Life Fixes

---

## Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## Introduction

This document covers all features gated by gpKernelDesignerConfig->quality_of_life_fixes.

These are small usability and presentation improvements that reduce UI friction, improve readability, and prevent a few edge-case visual issues without changing core system identity.

---

## Plan

When quality_of_life_fixes is enabled, the engine applies a compact bundle of UI and flow refinements.

### How It Works

| Feature | Disabled (`quality_of_life_fixes = false`) | Enabled (`quality_of_life_fixes = true`) | Player-facing impact |
|--------|--------------------------------------------|-------------------------------------------|----------------------|
| Avoid label shorthand | Uses vanilla avoid label text | Uses Avd in item panel combat preview | Cleaner stat panel text in tight UI space |
| Popup unit naming | Legacy popup selection for item/gold events | Popup scripts include unit-name token flows | Less ambiguity when many popups trigger |
| Epilogue transition | Vanilla solo ending display loop | Alternate solo ending proc with blend phase | Smoother end-card presentation |
| Effective crit cap | Effective crit can exceed 100 before later checks | Effective crit is clamped to 100 | Aligns with common player expectation |
| Trade portrait safety | Always attempts to start both faces | Skips StartFace for units with portraitId = 0 | Prevents portrait glitches in trade scenes |
| Weapon-rank helpbox compact mode | Standard 3-line pretext behavior | 2-line pretext plus focused WEXP/Rank Up fields | Faster readability for weapon-rank details |
| Map support event routing | Calls baseline map support event | Calls QoL map support event variant | Cleaner support-conversation event flow |
| World-map Home command | No Home command in world-map general menu | Adds Home command; fades out and returns to title through title-direct flow | Faster session exit and correct title music resume |

---

## Code Locations

All modifications are gated behind gpKernelDesignerConfig->quality_of_life_fixes in designer-config.c.

| Feature | Location | Description |
|--------|----------|-------------|
| Avoid label shorthand | UpdateMenuItemPanel in Kernel/Wizardry/Common/IconDisplay/Source/hooks.c | Replaces the avoid label draw string with Avd in the item panel combat stats UI |
| Popup unit naming | NewPopup_ItemGot_unused, NewPopup_GoldGot, NewPopup_ItemStealing in Kernel/Wizardry/Misc/MiscFunctions/Source/MiscFunctions.c | Selects popup scripts that include unit-name-aware message flows |
| Epilogue transition | StartSoloEndingBattleDisplay and gProcScr_EndingBattleDisplay_Solo_NEW in Kernel/Wizardry/Misc/MiscFunctions/Source/MiscFunctions.c | Routes solo ending display into a version with fade/blend steps |
| Effective crit cap | ComputeBattleUnitEffectiveCritRate in Kernel/Wizardry/Core/BattleSys/Source/BattleCalcReal.c | Clamps battleEffectiveCritRate to 100 in QoL mode |
| Trade portrait safety | TradeMenu_InitItemDisplay in Kernel/Wizardry/Misc/MiscFunctions/Source/MiscFunctions.c | Guards StartFace calls when a participant has portraitId = 0 |
| Weapon-rank helpbox compact mode | DrawHelpBoxLabels_WrankBonus and DrawHelpBoxStats_WrankBonus in Kernel/Wizardry/Core/BattleSys/WrankBonus/Source/WrankBonus.c | Reworks labels and displayed fields for compact helpbox output |
| Weapon-rank helpbox line budget | HelpBoxSetupstringLines in Kernel/Wizardry/Core/CombatArt/HelpBoxFix/Source/HelpBoxHack.c | Reduces pretext line count for NEW_HB_WRANK_STATSCREEN in QoL mode |
| Map support event routing | CallMapSupportEvent in Kernel/Wizardry/Common/BwlRework/source/BwlSupport.c | Selects EventScr_MapSupportConversation_NEW when QoL mode is active |
| World-map Home command | WMMenu_IsHomeAvailable_NEW and WMMenu_OnHomeSelected_NEW in Kernel/Wizardry/Misc/EnterTown/EnterTown.c | Shows Home only in QoL mode and routes to LGAMECTRL_TITLE_DIRECT for title music-aware return |

---

## TODO

- [ ] Add a single gif showing all QoL-visible UI changes in sequence
- [ ] Add before/after screenshots for the avoid label, popup text, and weapon-rank helpbox
- [ ] Document script-level differences between EventScr_MapSupportConversation and EventScr_MapSupportConversation_NEW

---

## Limitations & Bugs

This flag bundles multiple unrelated micro-fixes under one switch. Teams that need per-feature toggles will need to split this into independent config flags.

Please report issues in the repository Issues tab with the affected feature name and whether quality_of_life_fixes was enabled.
