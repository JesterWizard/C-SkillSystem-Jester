# Timer

<p align="center">
  <img src="../Gifs/Infuse_Menu.gif" alt="Timer Demo" width="600"/>
</p>

---

## 📑 Index
- [Introduction](#introduction)
- [How-To-Use](#How-To-Use)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

``gpKernelDesignerConfig->prep_menu_infuse``

This features provides a forging system in the prep menu by using a new resource (that I've called Dragon Glass).

Spending that resource allows the user to upgrade weapons in their supply to the next level in return for a certain amount of Dragon Glass.

---

## 🛠️ How To Use

- Inside [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) set the `.prep_menu_infuse` option to true.
- Set ``gInfuseMenuArray[0]`` to a given value using an ASMC or just gradually increment it through different actions, like combat, victory etc.
- Navigate to the prep menu and select "Infuse"
- Select an item in the supply and pay the required dragon glass cost to turn it into aother item

---

## 🛠️ Plan

- Add a different style of forging menu to the prep screen
- Use a resource distinct from gold to apply it
- Provide an array with combinations of input/output items and dragon glass costs

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| **Infuse table** | `gInfusionLookupTable` in [`Infuse.c`](../../Kernel/Wizardry/Core/SkillSys/PrepSkill/Source/CustomMenuOptions/Infuse.c) | Holds the table for input/output items and costs |
| **Infuse popup** | `InfusedPopup` in [`Infuse.c`](../../Kernel/Wizardry/Core/SkillSys/PrepSkill/Source/CustomMenuOptions/Infuse.c) | The proc for the popup |
| **Infuse sprites** | `drawInfuseSprites` in [`Infuse.c`](../../Kernel/Wizardry/Core/SkillSys/PrepSkill/Source/CustomMenuOptions/Infuse.c) | Handles the continous drawing of sprites every frame |
| **Setup graphics** | `PrepItemList_InitGfx_INFUSE` in [`Infuse.c`](../../Kernel/Wizardry/Core/SkillSys/PrepSkill/Source/CustomMenuOptions/Infuse.c) | Setup graphics at the init stage |
| **Setup sprite text** | `SetupSpriteTextDestination` in [`Infuse.c`](../../Kernel/Wizardry/Core/SkillSys/PrepSkill/Source/CustomMenuOptions/Infuse.c) | Setup the yes/no and popup item sprite texts |
| **Backend infuse logic** | `PerformInfusion` in [`Infuse.c`](../../Kernel/Wizardry/Core/SkillSys/PrepSkill/Source/CustomMenuOptions/Infuse.c) | Handles the backend logic for infusion |
| **Frame loop** | `PrepItemList_Loop_MainKeyHandler_INFUSE` in [`Infuse.c`](../../Kernel/Wizardry/Core/SkillSys/PrepSkill/Source/CustomMenuOptions/Infuse.c) | The loop that runs every frame check for button states etc |

---

## 📝 TODO

- Display the target item text dynamically in OB Tile space as a sprite

---

## 🐛 Limitations & Bugs

Please report issues in the repository’s **Issues** tab.

- Currently target item text does not display as a sprite dynamically. My current solution is just to hardcode "Infused an item"

---
