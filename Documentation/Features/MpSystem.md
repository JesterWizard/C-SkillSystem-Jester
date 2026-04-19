# MP System

<p align="center">
  <img src="../Gifs/MP_System.gif" alt="MP System Demo" width="600"/>
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

This system expands upon **Mokha’s FE Gaiden magic implementation**.  
In Gaiden, magic is divided into **Black** and **White** categories and consumes **HP** to cast.  
Given HP’s importance as a survival resource—especially in enemy-heavy gameplay—using HP for magic creates undesirable tradeoffs.

This hack replaces HP-based casting with a **true MP system**, offering a more flexible and sustainable way to manage magic.

---

## 🛠️ Plan

This system implements:

- A full **MP pool** for every playable unit with BWL data  
- Both **current MP** and **maximum MP** stored in the BWL struct  
- Support for up to **51 units** (C Skill System change from FE8's 0x46)

### 🔵 MP rules (FFTA2-style)

This implementation follows **Final Fantasy Tactics A2** MP logic:

- Units begin each chapter with **0 MP**
- Units gain **MP each turn and/or after combat**
- Stronger spells cannot be fired off immediately
- MP is a **self-replenishing resource**, unlike the HP-based Gaiden system

### 📘 Stat Page Enhancements

A dedicated stat page displays all **Black** and **White** magic and their MP costs.  
The default Gaiden system tried to display magic as items, which becomes problematic when inventories fill up.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| **MP costs** | `gGaidenWeaponCostList` in [`GaidenMagicData.c`](../../Data/GaidenMagic/GaidenMagicData.c) | MP cost per Gaiden magic |
| **MP configuration per unit** | `gGaidenPinfoConfigList` in [`GaidenMagicData.c`](../../Data/GaidenMagic/GaidenMagicData.c) | Unit MP stats and growth behavior |
| **Stat screen: MP cost colors** | `PutGaidenCost` in [`DrawPage5.c`](../../Kernel/Wizardry/Core/StatScreen/DrawPages/DrawPage5.c) | Colors MP values based on affordability |
| **Stat screen: White Magic list** | `WhiteMagicList` in [`DrawPage5.c`](../../Kernel/Wizardry/Core/StatScreen/DrawPages/DrawPage5.c) | Displays white magic on page 5 |
| **Stat screen: Black Magic list** | `BlackMagicList` in [`DrawPage5.c`](../../Kernel/Wizardry/Core/StatScreen/DrawPages/DrawPage5.c) | Displays black magic on page 5 |
| **MP generation** | `gMpSystemPInfoConfigList` in [`MpSystem.c`](../../Data/MpSystem/MpSystem.c) | MP gain per turn/battle |

---

## 📝 TODO

- Replace MMB with **GORGON-EGG** for MP minimug display  
- Add option for **Three Houses-style magic**  
- Add option for **FFTA-style MP** (alongside current FFTA2 rules)

---

## 🐛 Limitations & Bugs

Please report issues in the repository’s **Issues** tab.

### Known Issues

- Getter functions `GetUnitCurrentMP` and `GetUnitMaxMP` (from [`MiscFunctions.c`](../../Kernel/Wizardry/Misc/MiscFunctions/Source/MiscFunctions.c)) exist,  
  **but are not fully compatible** with the MP display module:

  - The MP minimug display is handled in  
    [`MMBDrawMP.s`](../../Kernel/Wizardry//Misc/ModularMinimugBox/Modules/MMBDrawMP.s)  
  - Because this is assembly-based, **MP cannot dynamically update** outside of static ROM values

Potential solution: Rewrite the minimug MP renderer using **GORGON-EGG**, which supports C and may allow dynamic MP drawing.

---
