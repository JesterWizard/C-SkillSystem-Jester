# Expanded HP

<p align="center">
  <img src="../Gifs/Expanded_HP.gif" alt="Timer Demo" width="600"/>
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

``gpKernelDesignerConfig->expanded_hp``

This features expands the HP of all units in the game. By default the game holds unit HP in a signed byte, causing the range of values to be between -127/127.
This can be easily expanded with a few key adjustments to some calculations to ensure unit HP never goes below 0 and hey presto! You have access to all that extra HP.

```OLD:``` Maximum of 127 (limited to 99 in the Minimug Box and 80 in battle animations)
```NEW:``` Maximum of 254, in the minimug box and in battle animations

---

## 🛠️ How To Use

Inside [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) set the `.expanded_hp` option to true.

---

## 🛠️ Plan

- Unsign HP in all appropriate structs
- Extend the length of the minimug box to compensate for the extra two digits
- Shift the locations of the digits in both the minimug box and the stat screen

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| **HP in stat screen** | `DisplayHpBmValue` in [`DrawPageLeft.c`](../../Kernel/Wizardry/Core/StatScreen/DrawPages/DrawPageLeft.c) | Displays unit HP |
| **Clamp HP values** | `StatusGetterCheckCpas` in [`MiscGetter.c`](../../Kernel/Wizardry/Core/UnitStatusGetter//source/MiscGetter.c) | Prevent HP from ging over the specified limits |
| **HP bar palettes in battle screen** | `EfxHPBarColorChangeMain` in [`HpBarPalettes.c`](../../Kernel/Wizardry/Misc/HPBarPalettes/HPBarPalettes.c) | Handles display of HP in battle screen as well as bar palettes |
| **Display dashes** | `StoreNumberStringOrDashesToSmallBuffer` in [`MiscFunctions.c`](../../Kernel/Wizardry/Misc/MiscFunctions//Source/MiscFunctions.c) | Handles display of dashes in Minimug box and chapter/status screen |
| **HP in minimug box** | `UnitMapUiUpdate` in [`ModularMinimugBox.c`](../../Kernel/Wizardry/Misc/ModularMinimugBox/ModularMinimugBox.c) | Handles display of HP in minimug box |

---

## 📝 TODO


---

## 🐛 Limitations & Bugs

Please report issues in the repository’s **Issues** tab.

- The forecast window and the chapter screen still both display "--" when dealing with HP 

---
