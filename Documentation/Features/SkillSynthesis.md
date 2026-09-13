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
- Add or edit recipes in [`SkillSynthRecipes.event`](../../Kernel/Wizardry/SkillSys/PrepSkill/SkillSynthRecipes.event). Each row is `SkillSynthRecipe(SID_A, SID_B, SID_Result)`.
- Keep the table terminated with `SkillSynthRecipe(0, 0, 0)`.
- Event files use SID names from [`skills.event`](../../include/constants/skills.event), which is generated from [`skills.h`](../../include/constants/skills.h).
- Open **Synthesize** from the prep menu or from a world map node menu.
- Press `A` to pick the first scroll, then `A` on a different scroll for the second ingredient.
- Confirm synthesis when a valid recipe is shown.
- Press `B` to clear the current pick or exit. Press `R` for the scroll help box. With R-text open, up/down moves the list and `A` pages long descriptions.

---

## Plan

| Stage | Player-facing behavior | Implementation notes |
|---|---|---|
| Availability | Synthesize appears when `prep_menu_skill_synth` is enabled. | Gated in `InitPrepScreenMainMenu` and `WMMenu_IsSkillSynthAvailable`. |
| Browsing | One scroll-only list from convoy and unit inventories. | `SkillSynth_BuildScrollList` compacts `gPrepScreenItemList` to skill scroll items. |
| Preview | Three single-row TSA boxes show ingredient 1, ingredient 2, and the result. | `DrawUiFrame2` height-4 frames; icon and name sit on the interior row. |
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

The ROM table is authored in Event Assembler as packed `SHORT` triplets:

```
SkillSynthRecipe(SID_Fury, SID_Fury, SID_FuryPlus)
SkillSynthRecipe(SID_Sol, SID_Luna, SID_Aether)
SkillSynthRecipe(0, 0, 0)
```

Lookup is order-independent: `A + B` and `B + A` match the same row. Duplicate skill pairs only work when the table lists that pair and two different list slots are selected.

Starter recipes include every `SID_*` / `SID_*Plus` self-pair plus mixed combinations (combat procs, movement, blows/stances, rallies, and utility).

---

## Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Recipe table | `gSkillSynthRecipeTable` in [`SkillSynthRecipes.event`](../../Kernel/Wizardry/SkillSys/PrepSkill/SkillSynthRecipes.event) | Designer-controlled `(sid_a, sid_b) -> sid_result` mappings. |
| EA skill IDs | `SID_*` in [`skills.event`](../../include/constants/skills.event) | Event Assembler SID defines generated from `skills.h`. |
| Recipe lookup | `SkillSynth_LookupResult` in [`SkillSynth.c`](../../Kernel/Wizardry/SkillSys/PrepSkill/Source/CustomMenuOptions/SkillSynth.c) | Normalizes unordered pairs and finds a result SID. |
| Preview frames | `SkillSynth_DrawPreviewFrames` in [`SkillSynth.c`](../../Kernel/Wizardry/SkillSys/PrepSkill/Source/CustomMenuOptions/SkillSynth.c) | Draws three vanilla TSA one-row windows on the left. |
| Scroll list builder | `SkillSynth_BuildScrollList` in [`SkillSynth.c`](../../Kernel/Wizardry/SkillSys/PrepSkill/Source/CustomMenuOptions/SkillSynth.c) | Collects skill scrolls from convoy and player inventories. |
| Synthesis apply | `SkillSynth_PerformSynthesis` in [`SkillSynth.c`](../../Kernel/Wizardry/SkillSys/PrepSkill/Source/CustomMenuOptions/SkillSynth.c) | Removes both ingredients and writes the result scroll. |
| Prep menu entry | `PrepScreenMenu_OnSkillSynth` and `gPrepMenuTable` in [`AtMenu.c`](../../Kernel/Wizardry/SkillSys/PrepSkill/Source/AtMenu.c) | Prep menu dispatch and ROM table wiring. |
| World map entry | `WMMenu_OnSkillSynthSelected` in [`EnterTown.c`](../../Kernel/Wizardry/EnterTown/EnterTown.c) | Launches the same screen from the node menu. |
| Header chibi | `FID_SKILL_SYNTH` (`0xAD`) in [`skill_synth.png`](../../Data/CustomPortraits/Portraits/skill_synth.png) | Unique two-gem fusion icon for the top-left mug slot. |
| Scroll item helper | `MakeSkillScrollItem` in [`SkillScroll.c`](../../Kernel/Wizardry/SkillSys/SkillScroll/Source/SkillScroll.c) | Builds the result scroll item ID from a SID. |
| Designer flag | `prep_menu_skill_synth` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) | Enables or disables both prep and world map entry points. |

---

## TODO

- Consider highlighting already-selected list rows while picking the second ingredient.

---

## Limitations & Bugs

Please report issues in the repository's **Issues** tab.

- Only skill scroll items in convoy or unit inventories are eligible; learned-skill bits are not used as ingredients.
- Recipes must be authored explicitly; there is no automatic Plus-upgrade fallback outside the event table.
- The result scroll always replaces the first selected ingredient's inventory slot.
