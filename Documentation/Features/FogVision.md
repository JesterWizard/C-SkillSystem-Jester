# Modular Fog Vision Range

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

In vanilla **FE8**, fog vision is extremely limited and largely static.  
Most units only see **1 tile** ahead, with **thieves** being the notable exception (+5 range).  
Beyond that, the only ways to extend vision are **torch items** or **torch staves**.

This system is restrictive and doesn’t reflect the diversity of classes in the game.

The same per-class range is also consumed by [Enemy Fog Vision](EnemyFogVision.md) when that designer flag is enabled.

---

## 🛠️ Plan

This feature introduces a **modular fog vision system**, allowing vision ranges to be defined **per class**.

This opens the door to more realistic and strategic class design:

- Archers could have **extended vision**, representing their observational skill  
- Flying units might see further due to aerial perspective  
- Infantry can remain limited, maintaining class identity  

The result is more interesting map design and more dynamic fog gameplay.

Enemy AI reuses this calculation through a separate vision map; see [Enemy Fog Vision](EnemyFogVision.md) for the AI-side contract.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| **Vision range logic** | `GetUnitFogViewRange` in [`FogVision.c`](../../Kernel/Wizardry/Common/FogVision/FogVision.c) | Controls how far each unit can see under fog |
| **Enemy AI consumer** | [Enemy Fog Vision](EnemyFogVision.md) | Applies the same range table to non-player faction vision |
| **Vision value text on stat screen** | `DrawPage1TextCommon` in [`Page1WithBwl.c`](../../Kernel/Wizardry/Core/StatScreen/DrawUnitPage/PlanA/Page1WithBwl.c) or `DrawPage1TextCommon` in [`Page1WithLeadership.c`](../../Kernel/Wizardry/Core/StatScreen/DrawUnitPage/PlanB/Page1WithLeadership.c) | Writes the fog vision number |
| **Vision bar on stat screen** | `DisplayPage_WithBWL` in [`Page1WithBwl.c`](../../Kernel/Wizardry/Core/StatScreen/DrawUnitPage/PlanA/Page1WithBwl.c) or `DisplayPage_WithLeadership` in [`Page1WithLeadership.c`](../../Kernel/Wizardry/Core/StatScreen/DrawUnitPage/PlanB/Page1WithLeadership.c) | Draws the graphical vision bar |
| **Help box navigation links** | `RText` arrays in [`Page1WithBwlHelpBox.c`](../../Kernel/Wizardry/Core/StatScreen/DrawUnitPage/PlanA/Page1WithBwlHelpBox.c) or [`Page1WithLeadershipHelpBox.c`](../../Kernel/Wizardry/Core/StatScreen/DrawUnitPage/PlanB/Page1WithLeadershipHelpBox.c) | Controls which help boxes appear when cycling through stats |

---

## 📝 TODO

- Expand designer-facing examples for class vision spreads once more maps lean on fog as a core design tool.

---

## 🐛 Limitations & Bugs

Please report any bugs using the repository’s **Issues** tab.

---
