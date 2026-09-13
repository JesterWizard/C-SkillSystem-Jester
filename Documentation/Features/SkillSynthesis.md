# Skill Synthesis Menu

---

## Index
- [Introduction](#introduction)
- [How To Use](#how-to-use)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## Introduction

``gpKernelDesignerConfig->prep_menu_skill_synth``

Skill Synthesis adds a prep and world map menu that combines two skill scrolls from the supply or any player unit inventory into a new skill scroll. Only recipe-table pairs succeed; invalid combinations are rejected without consuming scrolls.

---

## How To Use

- Inside [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) set `.prep_menu_skill_synth` to true.
- Add or edit entries in `gSkillSynthRecipeTable` inside [`SkillSynth.c`](../../Kernel/Wizardry/SkillSys/PrepSkill/Source/CustomMenuOptions/SkillSynth.c).
- Open **Synthesize** from the prep menu or from a world map node menu.
- Press `A` to pick the first scroll, then `A` on a different scroll for the second ingredient.
- Confirm synthesis when a valid recipe is shown.
- Press `B` to clear the current pick or exit. Press `R` for the scroll help box.

---

## Plan

| Stage | Player-facing behavior | Implementation notes |
|---|---|---|
| Availability | Synthesize appears when `prep_menu_skill_synth` is enabled. | Gated in `InitPrepScreenMainMenu` and `WMMenu_IsSkillSynthAvailable`. |
| Browsing | One scroll-only list from convoy and unit inventories. | `SkillSynth_BuildScrollList` compacts `gPrepScreenItemList` to skill scroll items. |
| Selection | `A` picks ingredient 1, then ingredient 2 from a different list slot. | `firstIdx` / `secondIdx` on `SkillSynthListProc`. |
| Validation | Invalid pairs show an error line and play the deny sound. | `SkillSynth_LookupResult` checks `gSkillSynthRecipeTable`. |
| Confirmation | Valid pairs open a Yes/No prompt with preview icons. | `SKILL_SYNTH_STATE_CONFIRM` in the main loop. |
| Apply | Both scrolls are consumed; the result scroll replaces the first slot. | `SkillSynth_PerformSynthesis` removes higher inventory slot first when both scrolls share an owner. |
| Exit | `B` from the list returns to prep or the world map. | World map entry restores camera and unit state like BEXP. |

### Recipe table format

```c
struct SkillSynthRecipe {
    u16 sid_a;
    u16 sid_b;
    u16 sid_result;
};
```

Lookup is order-independent: `A + B` and `B + A` match the same row. Duplicate skill pairs only work when the table lists that pair and two different list slots are selected.

---

## Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Recipe table | `gSkillSynthRecipeTable` in [`SkillSynth.c`](../../Kernel/Wizardry/SkillSys/PrepSkill/Source/CustomMenuOptions/SkillSynth.c) | Designer-controlled `(sid_a, sid_b) -> sid_result` mappings. |
| Scroll list builder | `SkillSynth_BuildScrollList` in [`SkillSynth.c`](../../Kernel/Wizardry/SkillSys/PrepSkill/Source/CustomMenuOptions/SkillSynth.c) | Collects skill scrolls from convoy and player inventories. |
| Recipe lookup | `SkillSynth_LookupResult` in [`SkillSynth.c`](../../Kernel/Wizardry/SkillSys/PrepSkill/Source/CustomMenuOptions/SkillSynth.c) | Normalizes unordered pairs and finds a result SID. |
| Synthesis apply | `SkillSynth_PerformSynthesis` in [`SkillSynth.c`](../../Kernel/Wizardry/SkillSys/PrepSkill/Source/CustomMenuOptions/SkillSynth.c) | Removes both ingredients and writes the result scroll. |
| Prep menu entry | `PrepScreenMenu_OnSkillSynth` and `gPrepMenuTable` in [`AtMenu.c`](../../Kernel/Wizardry/SkillSys/PrepSkill/Source/AtMenu.c) | Prep menu dispatch and ROM table wiring. |
| World map entry | `WMMenu_OnSkillSynthSelected` in [`EnterTown.c`](../../Kernel/Wizardry/EnterTown/EnterTown.c) | Launches the same screen from the node menu. |
| Scroll item helper | `MakeSkillScrollItem` in [`SkillScroll.c`](../../Kernel/Wizardry/SkillSys/SkillScroll/Source/SkillScroll.c) | Builds the result scroll item ID from a SID. |
| Designer flag | `prep_menu_skill_synth` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) | Enables or disables both prep and world map entry points. |

---

## TODO

- Add more starter recipes or move the table to a dedicated data file if the list grows large.
- Consider highlighting already-selected list rows while picking the second ingredient.

---

## Limitations & Bugs

Please report issues in the repository's **Issues** tab.

- Only skill scroll items in convoy or unit inventories are eligible; learned-skill bits are not used as ingredients.
- Recipes must be authored explicitly; there is no automatic Plus-upgrade fallback.
- The result scroll always replaces the first selected ingredient's inventory slot.
